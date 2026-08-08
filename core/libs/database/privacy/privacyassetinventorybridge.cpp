/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyassetinventorybridge.h"

// C++ includes

#include <algorithm>
#include <utility>

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QUuid>
#include <QVariant>

// Local includes

#ifndef PRIVACY_INVENTORY_BRIDGE_NO_PRODUCTION_ADAPTERS

#include <QCryptographicHash>
#include <QFile>

#include "collectionlocation.h"
#include "collectionmanager.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "privacyrepository.h"
#include "privacyruntime.h"
#include "privacytypes.h"

#ifdef Q_OS_UNIX

// C includes

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

#endif // PRIVACY_INVENTORY_BRIDGE_NO_PRODUCTION_ADAPTERS

namespace Digikam
{

namespace
{

bool canonicalUuid(const QString& value)
{
    const QUuid uuid(value);

    return !uuid.isNull() && (value == uuid.toString(QUuid::WithoutBraces));
}

bool safeRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) || path.contains(QChar::Null) ||
        path.contains(QLatin1Char('\\')) || (QDir::cleanPath(path) != path))
    {
        return false;
    }

    const QStringList parts = path.split(QLatin1Char('/'));

    for (const QString& part : parts)
    {
        if (part.isEmpty() || (part == QLatin1String(".")) ||
            (part == QLatin1String("..")))
        {
            return false;
        }
    }

    return true;
}

QString locationKey(const QString& rootUuid, const QString& relativePath)
{
    return rootUuid + QLatin1Char('\n') + relativePath;
}

QString selectionCollisionKey(const PrivacyInventoryCatalogueItem& item)
{
    return item.publicRootUuid + QLatin1Char('\n') +
           item.publicRelativePath.normalized(QString::NormalizationForm_C).toCaseFolded();
}

#ifndef PRIVACY_INVENTORY_BRIDGE_NO_PRODUCTION_ADAPTERS

void addDigestField(QCryptographicHash* const digest, const QByteArray& value)
{
    digest->addData(QByteArray::number(value.size()));
    digest->addData(":", 1);
    digest->addData(value);
    digest->addData("\n", 1);
}

void addDigestField(QCryptographicHash* const digest, const QString& value)
{
    addDigestField(digest, value.toUtf8());
}

void addDigestField(QCryptographicHash* const digest, qlonglong value)
{
    addDigestField(digest, QByteArray::number(value));
}

QString cataloguePath(const QString& albumPath, const QString& name)
{
    QString path = albumPath;

    while (path.startsWith(QLatin1Char('/')))
    {
        path.remove(0, 1);
    }

    while (path.endsWith(QLatin1Char('/')))
    {
        path.chop(1);
    }

    return path.isEmpty() ? name : (path + QLatin1Char('/') + name);
}

void sortCatalogueSnapshot(PrivacyInventoryCatalogueSnapshot* const snapshot)
{
    std::sort(snapshot->items.begin(), snapshot->items.end(),
              [](const PrivacyInventoryCatalogueItem& left,
                 const PrivacyInventoryCatalogueItem& right)
              {
                  if (left.imageId != right.imageId)
                  {
                      return (left.imageId < right.imageId);
                  }

                  const int root = QString::compare(left.publicRootUuid,
                                                    right.publicRootUuid,
                                                    Qt::CaseSensitive);

                  return (root != 0)
                       ? (root < 0)
                       : (QString::compare(left.publicRelativePath,
                                           right.publicRelativePath,
                                           Qt::CaseSensitive) < 0);
              });
    std::sort(snapshot->groups.begin(), snapshot->groups.end(),
              [](const PrivacyInventoryGroupRelation& left,
                 const PrivacyInventoryGroupRelation& right)
              {
                  return (left.memberImageId == right.memberImageId)
                       ? (left.leaderImageId < right.leaderImageId)
                       : (left.memberImageId < right.memberImageId);
              });
}

QByteArray catalogueGeneration(const PrivacyInventoryCatalogueSnapshot& snapshot)
{
    QCryptographicHash digest(QCryptographicHash::Sha256);

    addDigestField(&digest, snapshot.complete ? 1 : 0);

    for (const PrivacyInventoryCatalogueItem& item : snapshot.items)
    {
        addDigestField(&digest, item.imageId);
        addDigestField(&digest, item.publicRootUuid);
        addDigestField(&digest, item.publicRelativePath);
        addDigestField(&digest, item.fileSize);
        addDigestField(&digest, item.databaseIdentity);
        addDigestField(&digest, item.storedContentIdentity);
        addDigestField(&digest, item.contentIdentityAuthoritative ? 1 : 0);
    }

    for (const PrivacyInventoryGroupRelation& group : snapshot.groups)
    {
        addDigestField(&digest, group.memberImageId);
        addDigestField(&digest, group.leaderImageId);
    }

    QList<qlonglong> protectedIds = snapshot.protectedImageIds.values();
    std::sort(protectedIds.begin(), protectedIds.end());

    for (qlonglong imageId : std::as_const(protectedIds))
    {
        addDigestField(&digest, imageId);
    }

    return digest.result();
}

QByteArray rootGeneration(const PrivacyInventoryRootSnapshot& snapshot)
{
    QCryptographicHash digest(QCryptographicHash::Sha256);

    addDigestField(&digest, snapshot.complete ? 1 : 0);

    for (const PrivacyInventoryRootRecord& root : snapshot.roots)
    {
        addDigestField(&digest, root.uuid);
        addDigestField(&digest, static_cast<qlonglong>(root.state));
        addDigestField(&digest, static_cast<qlonglong>(root.epoch));
        addDigestField(&digest, root.scope.root.absolutePath);
        addDigestField(&digest, static_cast<qlonglong>(root.scope.expectedDeviceId));
        addDigestField(&digest, static_cast<qlonglong>(root.scope.expectedInode));
    }

    return digest.result();
}

PrivacyInventoryCatalogueSnapshot readCoreDbSnapshot(
    const QHash<int, QString>& rootUuidByAlbumRootId,
    qsizetype maximumItems,
    qsizetype maximumGroups)
{
    PrivacyInventoryCatalogueSnapshot result;

    if ((maximumItems <= 0) || (maximumGroups <= 0))
    {
        return result;
    }

    CoreDbAccess access;
    CoreDbBackend* const backend = access.backend();

    if (!backend || backend->isInTransaction() || !backend->beginTransaction())
    {
        return result;
    }

    const auto fail = [&result, backend]()
    {
        backend->rollbackTransaction();
        result.complete = false;
        result.generation.clear();

        return result;
    };

    QVariantList settings;

    if (!backend->execSql(QLatin1String(
            "SELECT value FROM Settings WHERE keyword='uniqueHashVersion';"), &settings))
    {
        return fail();
    }

    const int hashVersion = settings.isEmpty() ? 1 : settings.constFirst().toInt();
    QVariantList values;
    const qlonglong itemLimit = static_cast<qlonglong>(maximumItems) + 1;

    if (!backend->execSql(QString::fromUtf8(
            "SELECT Images.id, Albums.albumRoot, Albums.relativePath, Images.name, "
            "Images.fileSize, Images.uniqueHash, ImageHistory.uuid "
            "FROM Images INNER JOIN Albums ON Albums.id=Images.album "
            "LEFT JOIN ImageHistory ON ImageHistory.imageid=Images.id "
            "WHERE Images.album IS NOT NULL AND Images.status<3 "
            "ORDER BY Images.id LIMIT ?;"), itemLimit, &values))
    {
        return fail();
    }

    if ((values.size() % 7) != 0)
    {
        return fail();
    }

    if ((values.size() / 7) > maximumItems)
    {
        return fail();
    }

    for (auto it = values.cbegin() ; it != values.cend() ; )
    {
        PrivacyInventoryCatalogueItem item;
        item.imageId = (*it++).toLongLong();
        const int albumRootId = (*it++).toInt();
        const QString albumPath = (*it++).toString();
        const QString name = (*it++).toString();
        item.fileSize = (*it++).toLongLong();
        const QString uniqueHash = (*it++).toString();
        item.databaseIdentity = (*it++).toString();
        item.publicRootUuid = rootUuidByAlbumRootId.value(albumRootId);
        item.publicRelativePath = cataloguePath(albumPath, name);

        if (!uniqueHash.isEmpty() && (item.fileSize >= 0))
        {
            item.storedContentIdentity = QString::fromLatin1("digikam-unique-hash-v%1:%2:%3")
                .arg(hashVersion).arg(uniqueHash).arg(item.fileSize);
        }

        // Every shipped digiKam unique-hash version samples file ranges. It is
        // useful candidate evidence, but cannot prove exact byte identity.
        item.contentIdentityAuthoritative = false;

        if (!item.isValid())
        {
            return fail();
        }

        result.items << item;
    }

    values.clear();
    const qlonglong groupLimit = static_cast<qlonglong>(maximumGroups) + 1;

    if (!backend->execSql(QString::fromUtf8(
            "SELECT ImageRelations.subject, ImageRelations.object "
            "FROM ImageRelations "
            "INNER JOIN Images AS Members ON Members.id=ImageRelations.subject "
            "INNER JOIN Images AS Leaders ON Leaders.id=ImageRelations.object "
            "WHERE ImageRelations.type=2 AND Members.status<3 AND Leaders.status<3 "
            "ORDER BY ImageRelations.subject, ImageRelations.object LIMIT ?;"),
            groupLimit, &values))
    {
        return fail();
    }

    if (((values.size() % 2) != 0) || ((values.size() / 2) > maximumGroups))
    {
        return fail();
    }

    for (auto it = values.cbegin() ; it != values.cend() ; )
    {
        PrivacyInventoryGroupRelation group;
        group.memberImageId = (*it++).toLongLong();
        group.leaderImageId = (*it++).toLongLong();

        if (!group.isValid())
        {
            return fail();
        }

        result.groups << group;
    }

    values.clear();

    if (!backend->execSql(QLatin1String(
            "SELECT imageId FROM PrivacyItems ORDER BY imageId;"), &values))
    {
        return fail();
    }

    if (values.size() > maximumItems)
    {
        return fail();
    }

    for (const QVariant& value : std::as_const(values))
    {
        const qlonglong imageId = value.toLongLong();

        if (imageId <= 0)
        {
            return fail();
        }

        result.protectedImageIds.insert(imageId);
    }

    if (!backend->commitTransaction())
    {
        backend->rollbackTransaction();
        return result;
    }

    result.complete = true;
    sortCatalogueSnapshot(&result);
    result.generation = catalogueGeneration(result);

    return result;
}

#endif // PRIVACY_INVENTORY_BRIDGE_NO_PRODUCTION_ADAPTERS

void addBridgeIssue(QList<PrivacyAssetInventoryBridgeIssue>* const issues,
                    PrivacyAssetInventoryBridgeIssueCode code,
                    qlonglong imageId = -1,
                    const QString& detail = QString())
{
    PrivacyAssetInventoryBridgeIssue issue;
    issue.code    = code;
    issue.imageId = imageId;
    issue.detail  = detail;
    issues->append(issue);
}

bool itemHasFatalIssue(const PrivacyAssetInventoryBridgeItemResult& item)
{
    for (const PrivacyAssetInventoryBridgeIssue& issue : item.issues)
    {
        switch (issue.code)
        {
            case PrivacyAssetInventoryBridgeIssueCode::RootOffline:
            case PrivacyAssetInventoryBridgeIssueCode::Canceled:
            case PrivacyAssetInventoryBridgeIssueCode::ExactContentIdentityIncomplete:
            case PrivacyAssetInventoryBridgeIssueCode::CatalogueEvidenceIncomplete:
            case PrivacyAssetInventoryBridgeIssueCode::RootEvidenceIncomplete:
                break;

            default:
                return true;
        }
    }

    return false;
}

} // namespace

bool PrivacyInventoryCatalogueItem::isValid() const
{
    return (imageId > 0) && canonicalUuid(publicRootUuid) &&
           safeRelativePath(publicRelativePath) && (fileSize >= -1) &&
           (!contentIdentityAuthoritative || !storedContentIdentity.isEmpty());
}

bool PrivacyInventoryGroupRelation::isValid() const
{
    return (memberImageId > 0) && (leaderImageId > 0) &&
           (memberImageId != leaderImageId);
}

bool PrivacyAssetInventoryBridgeLimits::isValid() const
{
    return (maximumSelectionItems > 0) && (maximumCatalogueItems > 0) &&
           (maximumGroupRelations > 0) && (maximumResultEntries > 0) &&
           filesystemLimits.isValid();
}

class PrivacyCatalogueAssetIdentityProvider::Private
{
public:

    PrivacyInventoryCatalogueSnapshot catalogue;
    QHash<QString, PrivacyInventoryRoot> availableRoots;
    QHash<qlonglong, PrivacyInventoryCatalogueItem> itemsById;
    QMultiHash<QString, qlonglong> idsByPath;
    QMultiHash<QString, qlonglong> idsByDatabaseIdentity;
    QMultiHash<QString, qlonglong> idsByContentIdentity;
    QMultiHash<qlonglong, qlonglong> groupPartners;
};

PrivacyCatalogueAssetIdentityProvider::PrivacyCatalogueAssetIdentityProvider(
    const PrivacyInventoryCatalogueSnapshot& catalogue,
    const QList<PrivacyInventoryRootRecord>& roots)
    : d(new Private)
{
    d->catalogue = catalogue;

    for (const PrivacyInventoryRootRecord& root : roots)
    {
        if ((root.state == PrivacyRootRuntimeState::VerifiedAvailable) &&
            root.scope.isValid())
        {
            d->availableRoots.insert(root.uuid, root.scope.root);
        }
    }

    for (const PrivacyInventoryCatalogueItem& item : catalogue.items)
    {
        d->itemsById.insert(item.imageId, item);
        d->idsByPath.insert(locationKey(item.publicRootUuid,
                                        item.publicRelativePath), item.imageId);

        if (!item.databaseIdentity.isEmpty())
        {
            d->idsByDatabaseIdentity.insert(item.databaseIdentity, item.imageId);
        }

        if (!item.storedContentIdentity.isEmpty())
        {
            d->idsByContentIdentity.insert(item.storedContentIdentity, item.imageId);
        }
    }

    for (const PrivacyInventoryGroupRelation& group : catalogue.groups)
    {
        d->groupPartners.insert(group.memberImageId, group.leaderImageId);
        d->groupPartners.insert(group.leaderImageId, group.memberImageId);
    }
}

PrivacyCatalogueAssetIdentityProvider::~PrivacyCatalogueAssetIdentityProvider() = default;

PrivacyInventoryAliasEvidence PrivacyCatalogueAssetIdentityProvider::aliasesFor(
    const PrivacyInventoryAsset& asset) const
{
    PrivacyInventoryAliasEvidence result;
    result.complete = d->catalogue.complete;
    const QList<qlonglong> sourceIds = d->idsByPath.values(
        locationKey(asset.location.root.uuid, asset.location.relativePath));
    QSet<QString> candidates;

    const auto appendCandidate = [this, &result, &candidates]
                                 (PrivacyInventoryAliasKind kind,
                                  qlonglong imageId,
                                  const QString& contentIdentity = QString())
    {
        const auto item = d->itemsById.constFind(imageId);

        if (item == d->itemsById.constEnd())
        {
            result.complete = false;
            return;
        }

        const auto root = d->availableRoots.constFind(item->publicRootUuid);

        if (root == d->availableRoots.constEnd())
        {
            result.complete = false;
            return;
        }

        const QString key = QString::number(static_cast<int>(kind)) + QLatin1Char('\n') +
                            QString::number(imageId) + QLatin1Char('\n') + contentIdentity;

        if (candidates.contains(key))
        {
            return;
        }

        PrivacyInventoryAliasCandidate candidate;
        candidate.kind                  = kind;
        candidate.imageId               = imageId;
        candidate.contentIdentity       = contentIdentity;
        candidate.location.root         = root.value();
        candidate.location.relativePath = item->publicRelativePath;
        result.candidates << candidate;
        candidates.insert(key);
    };

    for (qlonglong sourceId : sourceIds)
    {
        const PrivacyInventoryCatalogueItem source = d->itemsById.value(sourceId);

        for (qlonglong aliasId : d->idsByPath.values(
                 locationKey(source.publicRootUuid, source.publicRelativePath)))
        {
            if (aliasId != sourceId)
            {
                appendCandidate(PrivacyInventoryAliasKind::DatabaseItemAlias, aliasId);
            }
        }

        if (!source.databaseIdentity.isEmpty())
        {
            for (qlonglong aliasId : d->idsByDatabaseIdentity.values(source.databaseIdentity))
            {
                if (aliasId != sourceId)
                {
                    appendCandidate(PrivacyInventoryAliasKind::DatabaseItemAlias, aliasId,
                                    source.databaseIdentity);
                }
            }
        }

        if (source.contentIdentityAuthoritative && !source.storedContentIdentity.isEmpty())
        {
            for (qlonglong aliasId : d->idsByContentIdentity.values(source.storedContentIdentity))
            {
                if (aliasId != sourceId)
                {
                    appendCandidate(PrivacyInventoryAliasKind::ContentIdentityAlias,
                                    aliasId, source.storedContentIdentity);
                }
            }
        }
        else
        {
            // digiKam's sampled unique hash is only a complete candidate index.
            // Preview those rows as database warnings; protect preflight can
            // full-hash this bounded set without hashing the whole library.
            if (!source.storedContentIdentity.isEmpty())
            {
                for (qlonglong aliasId : d->idsByContentIdentity.values(
                         source.storedContentIdentity))
                {
                    if (aliasId != sourceId)
                    {
                        appendCandidate(PrivacyInventoryAliasKind::DatabaseItemAlias,
                                        aliasId, source.storedContentIdentity);
                    }
                }
            }
        }

        for (qlonglong groupId : d->groupPartners.values(sourceId))
        {
            appendCandidate(PrivacyInventoryAliasKind::DigikamGroupMember, groupId);
        }
    }

    std::sort(result.candidates.begin(), result.candidates.end(),
              [](const PrivacyInventoryAliasCandidate& left,
                 const PrivacyInventoryAliasCandidate& right)
              {
                  if (left.kind != right.kind)
                  {
                      return (static_cast<int>(left.kind) < static_cast<int>(right.kind));
                  }

                  if (left.imageId != right.imageId)
                  {
                      return (left.imageId < right.imageId);
                  }

                  return (QString::compare(left.location.relativePath,
                                           right.location.relativePath,
                                           Qt::CaseSensitive) < 0);
              });

    return result;
}

#ifndef PRIVACY_INVENTORY_BRIDGE_NO_PRODUCTION_ADAPTERS

PrivacyCoreDbAssetInventoryProvider::PrivacyCoreDbAssetInventoryProvider(
    const QHash<int, QString>& rootUuidByAlbumRootId)
    : m_rootUuidByAlbumRootId(rootUuidByAlbumRootId)
{
}

PrivacyCoreDbAssetInventoryProvider::~PrivacyCoreDbAssetInventoryProvider() = default;

PrivacyInventoryCatalogueSnapshot PrivacyCoreDbAssetInventoryProvider::snapshot(
    qsizetype maximumItems, qsizetype maximumGroups) const
{
    return readCoreDbSnapshot(m_rootUuidByAlbumRootId, maximumItems, maximumGroups);
}

bool PrivacyCoreDbAssetInventoryProvider::generationMatches(
    const QByteArray& generation, qsizetype maximumItems, qsizetype maximumGroups) const
{
    if (generation.isEmpty())
    {
        return false;
    }

    const PrivacyInventoryCatalogueSnapshot current = snapshot(maximumItems, maximumGroups);

    return current.complete && (current.generation == generation);
}

PrivacyRuntimeAssetRootProvider::PrivacyRuntimeAssetRootProvider(
    const QSharedPointer<const PrivacyRuntimeCoordinator>& runtime)
    : m_runtime(runtime)
{
}

PrivacyRuntimeAssetRootProvider::~PrivacyRuntimeAssetRootProvider() = default;

PrivacyInventoryRootSnapshot PrivacyRuntimeAssetRootProvider::snapshot() const
{
    PrivacyInventoryRootSnapshot result;
    PrivacyRepositorySnapshot repository;

    if (!m_runtime || !PrivacyRepository().loadSnapshot(&repository))
    {
        return result;
    }

    result.complete = true;
    QSet<QString> seenUuids;
    QSet<int> seenAlbumRoots;

    for (const PrivacyStorageRoot& storedRoot : repository.storageRoots)
    {
        if (storedRoot.kind != PrivacyStorageRootKind::AlbumRoot)
        {
            continue;
        }

        PrivacyInventoryRootRecord record;
        record.uuid  = storedRoot.uuid;
        record.state = m_runtime->rootState(storedRoot.uuid);
        record.epoch = m_runtime->rootEpoch(storedRoot.uuid);

        if (!storedRoot.isValid() || seenUuids.contains(storedRoot.uuid) ||
            seenAlbumRoots.contains(storedRoot.albumRootId))
        {
            result.complete = false;
            record.state    = PrivacyRootRuntimeState::IdentityMismatch;
            result.roots << record;
            continue;
        }

        seenUuids.insert(storedRoot.uuid);
        seenAlbumRoots.insert(storedRoot.albumRootId);

        if (record.state == PrivacyRootRuntimeState::VerifiedAvailable)
        {
            const CollectionLocation location = CollectionManager::instance()
                ->locationForAlbumRootId(storedRoot.albumRootId);
            const QString path = QDir::cleanPath(location.albumRootPath());

            if (location.isNull() || !location.isAvailable() ||
                !PrivacyRootIdentityCodec::matchesAlbumRootV1(
                    storedRoot.identityData, storedRoot.albumRootId, location.identifier) ||
                !QDir::isAbsolutePath(path) || (path == QLatin1String("/")) ||
                (QFileInfo(path).canonicalFilePath() != path))
            {
                record.state    = PrivacyRootRuntimeState::IdentityMismatch;
                result.complete = false;
            }
            else
            {
                record.scope.root.uuid         = storedRoot.uuid;
                record.scope.root.absolutePath = path;

#ifdef Q_OS_UNIX

                const QByteArray encodedPath = QFile::encodeName(path);
                struct stat pathFacts = {};
                const int descriptor = ::open(encodedPath.constData(),
                                              O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
                struct stat descriptorFacts = {};

                if ((::lstat(encodedPath.constData(), &pathFacts) != 0) ||
                    !S_ISDIR(pathFacts.st_mode) || S_ISLNK(pathFacts.st_mode) ||
                    (descriptor < 0) || (::fstat(descriptor, &descriptorFacts) != 0) ||
                    !S_ISDIR(descriptorFacts.st_mode) ||
                    (pathFacts.st_dev != descriptorFacts.st_dev) ||
                    (pathFacts.st_ino != descriptorFacts.st_ino))
                {
                    record.state    = PrivacyRootRuntimeState::IdentityMismatch;
                    result.complete = false;
                }
                else
                {
                    record.scope.expectedDeviceId = static_cast<quint64>(descriptorFacts.st_dev);
                    record.scope.expectedInode    = static_cast<quint64>(descriptorFacts.st_ino);
                }

                if (descriptor >= 0)
                {
                    ::close(descriptor);
                }

#else

                record.state    = PrivacyRootRuntimeState::IdentityMismatch;
                result.complete = false;

#endif
            }
        }

        result.roots << record;
    }

    std::sort(result.roots.begin(), result.roots.end(),
              [](const PrivacyInventoryRootRecord& left,
                 const PrivacyInventoryRootRecord& right)
              {
                  return (QString::compare(left.uuid, right.uuid,
                                           Qt::CaseSensitive) < 0);
              });
    result.generation = rootGeneration(result);

    return result;
}

bool PrivacyRuntimeAssetRootProvider::generationMatches(const QByteArray& generation) const
{
    if (generation.isEmpty())
    {
        return false;
    }

    const PrivacyInventoryRootSnapshot current = snapshot();

    return current.complete && (current.generation == generation);
}

#endif // PRIVACY_INVENTORY_BRIDGE_NO_PRODUCTION_ADAPTERS

PrivacyAssetInventoryBridgeResult PrivacyAssetInventoryBridge::build(
    const PrivacyAssetInventoryBridgeRequest& request,
    const PrivacyInventoryCatalogueProvider& catalogueProvider,
    const PrivacyInventoryRootProvider& rootProvider,
    const PrivacyPosixInventoryControl* control)
{
    PrivacyAssetInventoryBridgeResult result;

    if (!request.limits.isValid() || request.imageIds.isEmpty())
    {
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::InvalidRequest);
        return result;
    }

    if (request.imageIds.size() > request.limits.maximumSelectionItems)
    {
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::SelectionLimitExceeded);
        return result;
    }

    QList<qlonglong> imageIds = request.imageIds;
    std::sort(imageIds.begin(), imageIds.end());

    for (qsizetype i = 0 ; i < imageIds.size() ; ++i)
    {
        if ((imageIds.at(i) <= 0) || ((i > 0) && (imageIds.at(i) == imageIds.at(i - 1))))
        {
            addBridgeIssue(&result.issues,
                           (imageIds.at(i) <= 0)
                               ? PrivacyAssetInventoryBridgeIssueCode::InvalidRequest
                               : PrivacyAssetInventoryBridgeIssueCode::DuplicateSelectedImageId,
                           imageIds.at(i));
        }
    }

    if (!result.issues.isEmpty())
    {
        return result;
    }

    if (control && control->isCanceled())
    {
        result.status = PrivacyInventoryStatus::Incomplete;
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::Canceled);
        return result;
    }

    const PrivacyInventoryCatalogueSnapshot catalogue = catalogueProvider.snapshot(
        request.limits.maximumCatalogueItems, request.limits.maximumGroupRelations);
    const PrivacyInventoryRootSnapshot roots = rootProvider.snapshot();
    result.catalogueGeneration = catalogue.generation;
    result.rootGeneration      = roots.generation;

    if (!catalogue.complete)
    {
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::CatalogueEvidenceIncomplete);
    }

    if (!roots.complete)
    {
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::RootEvidenceIncomplete);
    }

    QHash<qlonglong, PrivacyInventoryCatalogueItem> catalogueItems;

    for (const PrivacyInventoryCatalogueItem& item : catalogue.items)
    {
        if (!item.isValid() || catalogueItems.contains(item.imageId))
        {
            addBridgeIssue(&result.issues,
                           PrivacyAssetInventoryBridgeIssueCode::CatalogueEvidenceIncomplete,
                           item.imageId);
            continue;
        }

        catalogueItems.insert(item.imageId, item);
    }

    QHash<QString, PrivacyInventoryRootRecord> rootsByUuid;
    QList<PrivacyPosixRootScope> scopes;

    for (const PrivacyInventoryRootRecord& root : roots.roots)
    {
        if (!canonicalUuid(root.uuid) || rootsByUuid.contains(root.uuid))
        {
            addBridgeIssue(&result.issues,
                           PrivacyAssetInventoryBridgeIssueCode::RootEvidenceIncomplete,
                           -1, root.uuid);
            continue;
        }

        rootsByUuid.insert(root.uuid, root);

        if ((root.state == PrivacyRootRuntimeState::VerifiedAvailable) &&
            root.scope.isValid())
        {
            scopes << root.scope;
        }
    }

    PrivacyPosixFilesystemAdapter filesystem(scopes, request.limits.filesystemLimits,
                                             control);
    PrivacyCatalogueAssetIdentityProvider identities(catalogue, roots.roots);
    QHash<QString, QList<qlonglong> > selectedPaths;

    for (qlonglong imageId : std::as_const(imageIds))
    {
        const auto item = catalogueItems.constFind(imageId);

        if (item != catalogueItems.constEnd())
        {
            selectedPaths[selectionCollisionKey(item.value())] << imageId;
        }
    }

    for (qlonglong imageId : std::as_const(imageIds))
    {
        PrivacyAssetInventoryBridgeItemResult itemResult;
        itemResult.imageId = imageId;
        const auto item = catalogueItems.constFind(imageId);

        if (item == catalogueItems.constEnd())
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::SelectedImageMissing,
                           imageId);
            result.items << itemResult;
            continue;
        }

        const auto root = rootsByUuid.constFind(item->publicRootUuid);

        if (root == rootsByUuid.constEnd())
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::RootUnavailable,
                           imageId, item->publicRootUuid);
        }
        else if (root->state == PrivacyRootRuntimeState::Offline)
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::RootOffline,
                           imageId, item->publicRootUuid);
        }
        else if (root->state == PrivacyRootRuntimeState::IdentityMismatch)
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::RootIdentityMismatch,
                           imageId, item->publicRootUuid);
        }
        else if ((root->state != PrivacyRootRuntimeState::VerifiedAvailable) ||
                 !root->scope.isValid())
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::RootUnavailable,
                           imageId, item->publicRootUuid);
        }
        else
        {
            itemResult.request.primary.root         = root->scope.root;
            itemResult.request.primary.relativePath = item->publicRelativePath;
            itemResult.request.configuredSidecarExtensions =
                request.configuredSidecarExtensions;
        }

        if (catalogue.protectedImageIds.contains(imageId))
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::AlreadyProtected,
                           imageId);
        }

        if (QFileInfo(item->publicRelativePath).fileName().endsWith(
                QLatin1String(".digikam-private.zip"), Qt::CaseInsensitive))
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::ReservedPrivateArchivePath,
                           imageId);
        }

        if (selectedPaths.value(selectionCollisionKey(item.value())).size() > 1)
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::DuplicateSelectedPath,
                           imageId, item->publicRelativePath);
        }

        if (itemResult.request.isValid() && !itemHasFatalIssue(itemResult) &&
            (!control || !control->isCanceled()))
        {
            itemResult.inventory = PrivacyAssetInventory::build(
                itemResult.request, filesystem, identities);
        }
        else if (control && control->isCanceled())
        {
            addBridgeIssue(&itemResult.issues,
                           PrivacyAssetInventoryBridgeIssueCode::Canceled,
                           imageId);
        }

        result.items << itemResult;
    }

    qsizetype resultEntries = result.issues.size();

    for (const PrivacyAssetInventoryBridgeItemResult& item : std::as_const(result.items))
    {
        resultEntries += item.issues.size() + item.inventory.requiredAssets.size() +
                         item.inventory.exposureWarnings.size() + item.inventory.issues.size();
    }

    if (resultEntries > request.limits.maximumResultEntries)
    {
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::ResultLimitExceeded);
    }

    const bool canceled = control && control->isCanceled();
    const bool catalogueCurrent = !catalogue.complete ||
        catalogueProvider.generationMatches(catalogue.generation,
                                             request.limits.maximumCatalogueItems,
                                             request.limits.maximumGroupRelations);
    const bool rootsCurrent = !roots.complete ||
                              rootProvider.generationMatches(roots.generation);

    if (canceled)
    {
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::Canceled);
    }
    else if (!catalogueCurrent || !rootsCurrent)
    {
        addBridgeIssue(&result.issues,
                       PrivacyAssetInventoryBridgeIssueCode::GenerationChanged);
    }

    bool rejected = false;
    bool incomplete = !result.issues.isEmpty();

    for (const PrivacyAssetInventoryBridgeIssue& issue : std::as_const(result.issues))
    {
        if ((issue.code != PrivacyAssetInventoryBridgeIssueCode::CatalogueEvidenceIncomplete) &&
            (issue.code != PrivacyAssetInventoryBridgeIssueCode::RootEvidenceIncomplete) &&
            (issue.code != PrivacyAssetInventoryBridgeIssueCode::Canceled))
        {
            rejected = true;
        }
    }

    for (const PrivacyAssetInventoryBridgeItemResult& item : std::as_const(result.items))
    {
        rejected   = rejected || itemHasFatalIssue(item) ||
                     (item.inventory.status == PrivacyInventoryStatus::Rejected);
        incomplete = incomplete || !item.issues.isEmpty() ||
                     (item.inventory.status == PrivacyInventoryStatus::Incomplete);
    }

    result.status = rejected
                  ? PrivacyInventoryStatus::Rejected
                  : (incomplete ? PrivacyInventoryStatus::Incomplete
                                : PrivacyInventoryStatus::Ready);

    return result;
}

} // namespace Digikam
