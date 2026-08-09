/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description : Core database interface.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2026      by Srirupa Datta <srirupa dot sps at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredb.h"

// C++ includes

#include <algorithm>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Qt includes

#include <QSqlQuery>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "coredbbackend.h"
#include "collectionmanager.h"
#include "dbengineactiontype.h"
#include "tagscache.h"
#include "privacytransactionjournal.h"
#include "privacyruntime.h"

namespace Digikam
{

namespace
{

QString manualTagVisibilitySql(const QString& imageIdExpression,
                               QList<QVariant>* boundValues)
{
    QSet<QString> visibleCategorySet;

    if (!PrivacyManualTagVisibilityGate::queryState(&visibleCategorySet))
    {
        return QString();
    }

    QStringList visibleCategories = visibleCategorySet.values();
    std::sort(visibleCategories.begin(), visibleCategories.end());
    QString sql = QString::fromUtf8(
        " AND NOT EXISTS (SELECT 1 FROM PrivacyItems AS PrivacyTagItems "
        "WHERE PrivacyTagItems.imageId=%1").arg(imageIdExpression);

    if (!visibleCategories.isEmpty())
    {
        sql += QLatin1String(" AND PrivacyTagItems.categoryUuid NOT IN (");
        CoreDB::addBoundValuePlaceholders(sql, visibleCategories.size());
        sql += QLatin1Char(')');

        for (const QString& categoryUuid : std::as_const(visibleCategories))
        {
            *boundValues << categoryUuid;
        }
    }

    return sql + QLatin1Char(')');
}

QVariantList activePrivacyTransactionStates()
{
    return {
        static_cast<int>(PrivacyTransactionState::Created),
        static_cast<int>(PrivacyTransactionState::Prepared),
        static_cast<int>(PrivacyTransactionState::Applying),
        static_cast<int>(PrivacyTransactionState::Exposed),
        static_cast<int>(PrivacyTransactionState::Relocking),
        static_cast<int>(PrivacyTransactionState::NeedsReconciliation),
        static_cast<int>(PrivacyTransactionState::Error)
    };
}

} // namespace

class Q_DECL_HIDDEN CoreDB::Private
{

public:

    Private() = default;

    const QString configGroupName           = QLatin1String("CoreDB Settings");
    const QString configRecentlyUsedTags    = QLatin1String("Recently Used Tags");

    CoreDbBackend*       db                 = nullptr;
    QList<int>           recentlyAssignedTags;

    int                  uniqueHashVersion  = -1;

public:

    QString constructRelatedImagesSQL(bool fromOrTo, DatabaseRelation::Type type, bool boolean);
    QList<qlonglong> execRelatedImagesQuery(QSqlQuery& query, qlonglong id, DatabaseRelation::Type type);
};

QString CoreDB::Private::constructRelatedImagesSQL(bool fromOrTo, DatabaseRelation::Type type, bool boolean)
{
    QString sql;

    if (fromOrTo)
    {
        sql = QString::fromUtf8("SELECT object FROM ImageRelations "
                                "INNER JOIN Images ON ImageRelations.object=Images.id "
                                " WHERE subject=? %1 AND status<3 %2;");
    }
    else
    {
        sql = QString::fromUtf8("SELECT subject FROM ImageRelations "
                                "INNER JOIN Images ON ImageRelations.subject=Images.id "
                                " WHERE object=? %1 AND status<3 %2;");
    }

    if (type != DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString::fromUtf8("AND type=?"));
    }
    else
    {
        sql = sql.arg(QString());
    }

    if (boolean)
    {
        sql = sql.arg(QString::fromUtf8("LIMIT 1"));
    }
    else
    {
        sql = sql.arg(QString());
    }

    return sql;
}

QList<qlonglong> CoreDB::Private::execRelatedImagesQuery(QSqlQuery& query, qlonglong id, DatabaseRelation::Type type)
{
    QVariantList values;

    if (type == DatabaseRelation::UndefinedType)
    {
        db->execSql(query, id, &values);
    }
    else
    {
        db->execSql(query, id, type, &values);
    }

    QList<qlonglong> imageIds;

    if (values.isEmpty())
    {
        return imageIds;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        imageIds << (*it).toInt();
    }

    return imageIds;
}

// --------------------------------------------------------

CoreDB::CoreDB(CoreDbBackend* const backend)
    : d(new Private)
{
    d->db = backend;
    readSettings();
}

CoreDB::~CoreDB()
{
    writeSettings();
    delete d;
}

bool CoreDB::insertPrivacyCategory(const PrivacyCategory& category) const
{
    if (!category.isValid())
    {
        return false;
    }

    QVariantList values;
    values << category.uuid
           << category.name
           << static_cast<int>(category.backend)
           << static_cast<int>(category.presentationMode)
           << static_cast<int>(category.unlockedThumbnailMode)
           << static_cast<int>(category.tagVisibilityMode)
           << static_cast<int>(category.lifecycleState)
           << category.currentCredentialGeneration
           << category.schemaVersion
           << d->db->asDBDateTime(category.createdAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyCategories "
                                             "(uuid, name, backend, presentationMode, "
                                             "unlockedThumbnailMode, tagVisibilityMode, lifecycleState, "
                                             "currentCredentialGeneration, schemaVersion, createdAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyCategories(QList<PrivacyCategory>* categories) const
{
    if (!categories)
    {
        return false;
    }

    categories->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT uuid, name, backend, presentationMode, "
                                         "unlockedThumbnailMode, tagVisibilityMode, lifecycleState, "
                                         "currentCredentialGeneration, schemaVersion, createdAt "
                                         "FROM PrivacyCategories ORDER BY name, uuid;"), &values))
    {
        return false;
    }

    if ((values.size() % 10) != 0)
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyCategory category;

        category.uuid             = (*it++).toString();
        category.name             = (*it++).toString();
        category.backend          = static_cast<PrivacyBackend>((*it++).toInt());
        category.presentationMode = static_cast<PrivacyPresentationMode>((*it++).toInt());
        category.unlockedThumbnailMode = static_cast<PrivacyUnlockedThumbnailMode>((*it++).toInt());
        category.tagVisibilityMode = static_cast<PrivacyTagVisibilityMode>((*it++).toInt());
        category.lifecycleState   = static_cast<PrivacyCategoryLifecycleState>((*it++).toInt());
        category.currentCredentialGeneration = (*it++).toLongLong();
        category.schemaVersion    = (*it++).toInt();
        category.createdAt        = (*it++).toDateTime();

        categories->append(category);
    }

    return true;
}

PrivacyCategory CoreDB::getPrivacyCategory(const QString& uuid) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT uuid, name, backend, presentationMode, "
                                     "unlockedThumbnailMode, tagVisibilityMode, lifecycleState, "
                                     "currentCredentialGeneration, schemaVersion, createdAt "
                                     "FROM PrivacyCategories WHERE uuid=?;"), uuid, &values);

    if (values.size() != 10)
    {
        return PrivacyCategory();
    }

    PrivacyCategory category;
    auto it = values.constBegin();

    category.uuid             = (*it++).toString();
    category.name             = (*it++).toString();
    category.backend          = static_cast<PrivacyBackend>((*it++).toInt());
    category.presentationMode = static_cast<PrivacyPresentationMode>((*it++).toInt());
    category.unlockedThumbnailMode = static_cast<PrivacyUnlockedThumbnailMode>((*it++).toInt());
    category.tagVisibilityMode = static_cast<PrivacyTagVisibilityMode>((*it++).toInt());
    category.lifecycleState   = static_cast<PrivacyCategoryLifecycleState>((*it++).toInt());
    category.currentCredentialGeneration = (*it++).toLongLong();
    category.schemaVersion    = (*it++).toInt();
    category.createdAt        = (*it++).toDateTime();

    return category;
}

bool CoreDB::updatePrivacyCategoryTagVisibilityMode(
    const QString& uuid,
    PrivacyTagVisibilityMode mode) const
{
    if ((mode != PrivacyTagVisibilityMode::UnlockedOnly) &&
        (mode != PrivacyTagVisibilityMode::AlwaysVisible))
    {
        return false;
    }

    QVariantList values;
    values << static_cast<int>(mode) << uuid;

    const QSqlQuery query = d->db->execQuery(
        QString::fromUtf8("UPDATE PrivacyCategories SET tagVisibilityMode=? WHERE uuid=?;"),
        values);

    return (query.isActive() && (query.numRowsAffected() == 1));
}

bool CoreDB::insertPrivacyItem(const PrivacyItem& item) const
{
    if (!item.isValid())
    {
        return false;
    }

    QVariantList parentValues;
    d->db->execSql(QString::fromUtf8("SELECT "
                                     "EXISTS(SELECT 1 FROM Images WHERE id=?), "
                                     "EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=?);"),
                   item.imageId, item.categoryUuid, &parentValues);

    if ((parentValues.size() != 2) ||
        !parentValues.at(0).toBool() ||
        !parentValues.at(1).toBool())
    {
        return false;
    }

    QVariantList values;
    values << item.imageId
           << item.uuid
           << item.categoryUuid
           << item.originalHash
           << item.originalSize
           << item.originalWidth
           << item.originalHeight
           << d->db->asDBDateTime(item.originalCreationDate)
           << item.expectedProxyHash
           << item.expectedProxySize
           << item.presentationVersion
           << item.generation
           << item.transactionState;

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyItems "
                                             "(imageId, uuid, categoryUuid, originalHash, originalSize, "
                                             "originalWidth, originalHeight, originalCreationDate, "
                                             "expectedProxyHash, expectedProxySize, presentationVersion, "
                                             "generation, transactionState) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyItems(QList<PrivacyItem>* items) const
{
    if (!items)
    {
        return false;
    }

    items->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT imageId, uuid, categoryUuid, originalHash, originalSize, "
                                         "originalWidth, originalHeight, originalCreationDate, expectedProxyHash, "
                                         "expectedProxySize, presentationVersion, generation, transactionState "
                                         "FROM PrivacyItems ORDER BY imageId;"), &values))
    {
        return false;
    }

    if ((values.size() % 13) != 0)
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyItem item;

        item.imageId              = (*it++).toLongLong();
        item.uuid                 = (*it++).toString();
        item.categoryUuid         = (*it++).toString();
        item.originalHash         = (*it++).toString();
        item.originalSize         = (*it++).toLongLong();
        item.originalWidth        = (*it++).toInt();
        item.originalHeight       = (*it++).toInt();
        item.originalCreationDate = (*it++).toDateTime();
        item.expectedProxyHash    = (*it++).toString();
        item.expectedProxySize    = (*it++).toLongLong();
        item.presentationVersion  = (*it++).toInt();
        item.generation           = (*it++).toLongLong();
        item.transactionState     = (*it++).toInt();

        items->append(item);
    }

    return true;
}

PrivacyItem CoreDB::getPrivacyItem(qlonglong imageId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT imageId, uuid, categoryUuid, originalHash, originalSize, "
                                     "originalWidth, originalHeight, originalCreationDate, expectedProxyHash, "
                                     "expectedProxySize, presentationVersion, generation, transactionState "
                                     "FROM PrivacyItems WHERE imageId=?;"), imageId, &values);

    if (values.size() != 13)
    {
        return PrivacyItem();
    }

    PrivacyItem item;
    auto it = values.constBegin();

    item.imageId              = (*it++).toLongLong();
    item.uuid                 = (*it++).toString();
    item.categoryUuid         = (*it++).toString();
    item.originalHash         = (*it++).toString();
    item.originalSize         = (*it++).toLongLong();
    item.originalWidth        = (*it++).toInt();
    item.originalHeight       = (*it++).toInt();
    item.originalCreationDate = (*it++).toDateTime();
    item.expectedProxyHash    = (*it++).toString();
    item.expectedProxySize    = (*it++).toLongLong();
    item.presentationVersion  = (*it++).toInt();
    item.generation           = (*it++).toLongLong();
    item.transactionState     = (*it++).toInt();

    return item;
}

bool CoreDB::insertPrivacyCredential(const PrivacyCredential& credential) const
{
    if (!credential.isValid())
    {
        return false;
    }

    QVariantList parentValues;
    d->db->execSql(QString::fromUtf8("SELECT EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=?);"),
                   credential.categoryUuid, &parentValues);

    if ((parentValues.size() != 1) || !parentValues.constFirst().toBool())
    {
        return false;
    }

    QVariantList values;
    values << credential.categoryUuid
           << credential.generation
           << credential.encodingVersion
           << credential.envelopeFormat
           << credential.envelopeBlob
           << credential.envelopeHashAlgorithm
           << credential.envelopeHash
           << credential.recoveryMode
           << credential.recoveryState
           << credential.recoveryRecordVersion
           << (credential.recoveryDocumentUuid.isEmpty() ? QVariant() : credential.recoveryDocumentUuid)
           << (credential.recoveryAcknowledgedAt.isValid()
               ? d->db->asDBDateTime(credential.recoveryAcknowledgedAt) : QVariant())
           << (credential.recoveryVerifiedAt.isValid()
               ? d->db->asDBDateTime(credential.recoveryVerifiedAt) : QVariant())
           << d->db->asDBDateTime(credential.createdAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyCredentials "
                                             "(categoryUuid, generation, encodingVersion, envelopeFormat, "
                                             "envelopeBlob, envelopeHashAlgorithm, envelopeHash, recoveryMode, "
                                             "recoveryState, recoveryRecordVersion, recoveryDocumentUuid, "
                                             "recoveryAcknowledgedAt, recoveryVerifiedAt, createdAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyCredentials(QList<PrivacyCredential>* credentials) const
{
    if (!credentials)
    {
        return false;
    }

    credentials->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT categoryUuid, generation, encodingVersion, envelopeFormat, "
                                         "envelopeBlob, envelopeHashAlgorithm, envelopeHash, recoveryMode, "
                                         "recoveryState, recoveryRecordVersion, recoveryDocumentUuid, "
                                         "recoveryAcknowledgedAt, recoveryVerifiedAt, createdAt "
                                         "FROM PrivacyCredentials ORDER BY categoryUuid, generation;"), &values) ||
        ((values.size() % 14) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyCredential credential;
        credential.categoryUuid           = (*it++).toString();
        credential.generation             = (*it++).toLongLong();
        credential.encodingVersion        = (*it++).toString();
        credential.envelopeFormat         = (*it++).toString();
        credential.envelopeBlob           = (*it++).toByteArray();
        credential.envelopeHashAlgorithm  = (*it++).toString();
        credential.envelopeHash           = (*it++).toString();
        credential.recoveryMode           = (*it++).toInt();
        credential.recoveryState          = (*it++).toInt();
        credential.recoveryRecordVersion  = (*it++).toInt();
        credential.recoveryDocumentUuid   = (*it++).toString();
        credential.recoveryAcknowledgedAt = (*it++).toDateTime();
        credential.recoveryVerifiedAt     = (*it++).toDateTime();
        credential.createdAt              = (*it++).toDateTime();
        credentials->append(credential);
    }

    return true;
}

bool CoreDB::insertPrivacyStorageRoot(const PrivacyStorageRoot& root) const
{
    if (!root.isValid())
    {
        return false;
    }

    if (root.kind == PrivacyStorageRootKind::AlbumRoot)
    {
        QVariantList parentValues;
        d->db->execSql(QString::fromUtf8("SELECT EXISTS(SELECT 1 FROM AlbumRoots WHERE id=?);"),
                       root.albumRootId, &parentValues);

        if ((parentValues.size() != 1) || !parentValues.constFirst().toBool())
        {
            return false;
        }
    }

    QVariantList values;
    values << root.uuid
           << static_cast<int>(root.kind)
           << ((root.albumRootId > 0) ? QVariant(root.albumRootId) : QVariant())
           << root.configuredPath
           << root.identityVersion
           << root.identityData
           << (root.markerUuid.isEmpty() ? QVariant() : root.markerUuid)
           << root.schemaVersion
           << d->db->asDBDateTime(root.createdAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyStorageRoots "
                                             "(uuid, kind, albumRootId, configuredPath, identityVersion, "
                                             "identityData, markerUuid, schemaVersion, createdAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::ensurePrivacyAlbumRoot(const PrivacyStorageRoot& candidate,
                                    PrivacyStorageRoot* const persisted,
                                    bool* const created) const
{
    if (!persisted || !created || !candidate.isValid() ||
        (candidate.kind != PrivacyStorageRootKind::AlbumRoot) ||
        d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    QList<PrivacyStorageRoot> roots;

    if (!getPrivacyStorageRoots(&roots))
    {
        return abort();
    }

    PrivacyStorageRoot existing;
    int albumRootMatches = 0;

    for (const PrivacyStorageRoot& root : std::as_const(roots))
    {
        if ((root.kind == PrivacyStorageRootKind::AlbumRoot) &&
            (root.albumRootId == candidate.albumRootId))
        {
            existing = root;
            ++albumRootMatches;
        }

        if ((root.uuid == candidate.uuid) &&
            ((root.kind != PrivacyStorageRootKind::AlbumRoot) ||
             (root.albumRootId != candidate.albumRootId)))
        {
            return abort();
        }
    }

    if (albumRootMatches > 1)
    {
        return abort();
    }

    if (albumRootMatches == 1)
    {
        if (d->db->commitTransaction() != BdEngineBackend::NoErrors)
        {
            return false;
        }

        *persisted = existing;
        *created   = false;

        return true;
    }

    if (!insertPrivacyStorageRoot(candidate))
    {
        return abort();
    }

    if (d->db->commitTransaction() != BdEngineBackend::NoErrors)
    {
        return false;
    }

    *persisted = candidate;
    *created   = true;

    return true;
}

bool CoreDB::removeUnreferencedPrivacyAlbumRoot(const QString& uuid,
                                                bool* const absent) const
{
    if (!absent || uuid.isEmpty() || d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    const QVariantList evidenceBindings = {
        uuid,
        static_cast<int>(PrivacyStorageRootKind::AlbumRoot),
        uuid,
        uuid,
        uuid,
        uuid
    };
    QVariantList evidence;
    d->db->execSql(QString::fromUtf8(
        "SELECT "
        "EXISTS(SELECT 1 FROM PrivacyStorageRoots WHERE uuid=? AND kind=?), "
        "EXISTS(SELECT 1 FROM PrivacyStores WHERE rootUuid=?), "
        "EXISTS(SELECT 1 FROM PrivacyContainers WHERE rootUuid=?), "
        "EXISTS(SELECT 1 FROM PrivacyAssets WHERE publicRootUuid=?), "
        "EXISTS(SELECT 1 FROM PrivacyTransactionJournals WHERE rootUuid=?);"),
        evidenceBindings, &evidence);

    if (evidence.size() != 5)
    {
        return abort();
    }

    const bool rootExists = evidence.at(0).toBool();
    const bool rootReferenced = evidence.at(1).toBool() ||
                                evidence.at(2).toBool() ||
                                evidence.at(3).toBool() ||
                                evidence.at(4).toBool();

    if (!rootExists || rootReferenced)
    {
        if (d->db->commitTransaction() != BdEngineBackend::NoErrors)
        {
            return false;
        }

        *absent = !rootExists;
        return true;
    }

    QSqlQuery query = d->db->execQuery(
        QString::fromUtf8("DELETE FROM PrivacyStorageRoots WHERE uuid=? AND kind=?;"),
        QVariantList { uuid, static_cast<int>(PrivacyStorageRootKind::AlbumRoot) });

    if (!query.isActive() || (query.numRowsAffected() != 1))
    {
        return abort();
    }

    if (d->db->commitTransaction() != BdEngineBackend::NoErrors)
    {
        return false;
    }

    *absent = true;

    return true;
}

bool CoreDB::getPrivacyStorageRoots(QList<PrivacyStorageRoot>* roots) const
{
    if (!roots)
    {
        return false;
    }

    roots->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT uuid, kind, albumRootId, configuredPath, identityVersion, "
                                         "identityData, markerUuid, schemaVersion, createdAt "
                                         "FROM PrivacyStorageRoots ORDER BY uuid;"), &values) ||
        ((values.size() % 9) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyStorageRoot root;
        root.uuid           = (*it++).toString();
        root.kind           = static_cast<PrivacyStorageRootKind>((*it++).toInt());
        const QVariant albumRootId = *it++;
        root.albumRootId    = albumRootId.isNull() ? -1 : albumRootId.toInt();
        root.configuredPath = (*it++).toString();
        root.identityVersion = (*it++).toInt();
        root.identityData   = (*it++).toByteArray();
        root.markerUuid     = (*it++).toString();
        root.schemaVersion  = (*it++).toInt();
        root.createdAt      = (*it++).toDateTime();
        roots->append(root);
    }

    return true;
}

bool CoreDB::insertPrivacyStore(const PrivacyStore& store) const
{
    if (!store.isValid())
    {
        return false;
    }

    QVariantList parentValues;
    d->db->execSql(QString::fromUtf8("SELECT "
                                     "EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=?), "
                                     "EXISTS(SELECT 1 FROM PrivacyStorageRoots WHERE uuid=?), "
                                     "EXISTS(SELECT 1 FROM PrivacyCredentials "
                                     "WHERE categoryUuid=? AND generation=?);"),
                   store.categoryUuid, store.rootUuid,
                   store.categoryUuid, store.configGeneration, &parentValues);

    const bool credentialValid = ((store.configGeneration < 0) ||
                                  ((parentValues.size() == 3) && parentValues.at(2).toBool()));

    if ((parentValues.size() != 3) || !parentValues.at(0).toBool() ||
        !parentValues.at(1).toBool() || !credentialValid)
    {
        return false;
    }

    QVariantList values;
    values << store.uuid
           << store.categoryUuid
           << store.rootUuid
           << store.format
           << store.formatVersion
           << store.cipherRelativePath
           << store.configRelativePath
           << ((store.configGeneration >= 0) ? QVariant(store.configGeneration) : QVariant())
           << static_cast<int>(store.lifecycleState)
           << store.schemaVersion
           << d->db->asDBDateTime(store.createdAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyStores "
                                             "(uuid, categoryUuid, rootUuid, format, formatVersion, "
                                             "cipherRelativePath, configRelativePath, configGeneration, "
                                             "lifecycleState, schemaVersion, createdAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyStores(QList<PrivacyStore>* stores) const
{
    if (!stores)
    {
        return false;
    }

    stores->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT uuid, categoryUuid, rootUuid, format, formatVersion, "
                                         "cipherRelativePath, configRelativePath, configGeneration, "
                                         "lifecycleState, schemaVersion, createdAt "
                                         "FROM PrivacyStores ORDER BY uuid;"), &values) ||
        ((values.size() % 11) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyStore store;
        store.uuid               = (*it++).toString();
        store.categoryUuid       = (*it++).toString();
        store.rootUuid           = (*it++).toString();
        store.format             = (*it++).toString();
        store.formatVersion      = (*it++).toInt();
        store.cipherRelativePath = (*it++).toString();
        store.configRelativePath = (*it++).toString();
        const QVariant configGeneration = *it++;
        store.configGeneration   = configGeneration.isNull() ? -1 : configGeneration.toLongLong();
        store.lifecycleState     = static_cast<PrivacyStoreLifecycleState>((*it++).toInt());
        store.schemaVersion      = (*it++).toInt();
        store.createdAt          = (*it++).toDateTime();
        stores->append(store);
    }

    return true;
}

bool CoreDB::insertPrivacyStoreBinding(const PrivacyStoreBinding& binding) const
{
    if (!binding.isValid())
    {
        return false;
    }

    QVariantList parentValues;
    QVariantList parentBindings;
    parentBindings << binding.categoryUuid
                   << binding.storeUuid
                   << binding.categoryUuid
                   << binding.categoryUuid
                   << binding.storeUuid;
    d->db->execSql(QString::fromUtf8("SELECT "
                                     "EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=?), "
                                     "EXISTS(SELECT 1 FROM PrivacyStores WHERE uuid=? AND categoryUuid=?), "
                                     "NOT EXISTS(SELECT 1 FROM PrivacyStoreBindings "
                                     "WHERE categoryUuid=? AND storeUuid<>?);"),
                   parentBindings, &parentValues);

    if ((parentValues.size() != 3) || !parentValues.at(0).toBool() ||
        !parentValues.at(1).toBool() || !parentValues.at(2).toBool())
    {
        return false;
    }

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyStoreBindings "
                                             "(categoryUuid, role, storeUuid, schemaVersion) "
                                             "VALUES (?, ?, ?, ?);"),
                           binding.categoryUuid, static_cast<int>(binding.role),
                           binding.storeUuid, binding.schemaVersion));
}

bool CoreDB::getPrivacyStoreBindings(QList<PrivacyStoreBinding>* bindings) const
{
    if (!bindings)
    {
        return false;
    }

    bindings->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT categoryUuid, role, storeUuid, schemaVersion "
                                         "FROM PrivacyStoreBindings ORDER BY categoryUuid, role;"), &values) ||
        ((values.size() % 4) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyStoreBinding binding;
        binding.categoryUuid = (*it++).toString();
        binding.role         = static_cast<PrivacyStoreRole>((*it++).toInt());
        binding.storeUuid    = (*it++).toString();
        binding.schemaVersion = (*it++).toInt();
        bindings->append(binding);
    }

    return true;
}

bool CoreDB::insertPrivacyContainer(const PrivacyContainer& container) const
{
    if (!container.isValid())
    {
        return false;
    }

    QVariantList parentValues;

    if (container.kind == PrivacyContainerKind::CasualArchive)
    {
        QVariantList parentBindings;
        parentBindings << container.rootUuid
                       << static_cast<int>(PrivacyStorageRootKind::AlbumRoot)
                       << container.credentialGeneration
                       << container.itemUuid
                       << static_cast<int>(PrivacyBackend::Casual);
        d->db->execSql(QString::fromUtf8("SELECT EXISTS("
                                         "SELECT 1 FROM PrivacyItems "
                                         "INNER JOIN PrivacyCategories "
                                         "ON PrivacyCategories.uuid=PrivacyItems.categoryUuid "
                                         "INNER JOIN PrivacyStorageRoots "
                                         "ON PrivacyStorageRoots.uuid=? AND PrivacyStorageRoots.kind=? "
                                         "INNER JOIN PrivacyCredentials "
                                         "ON PrivacyCredentials.categoryUuid=PrivacyItems.categoryUuid "
                                         "AND PrivacyCredentials.generation=? "
                                         "WHERE PrivacyItems.uuid=? AND PrivacyCategories.backend=?);"),
                       parentBindings, &parentValues);
    }
    else
    {
        QVariantList parentBindings;
        parentBindings << static_cast<int>(PrivacyStoreRole::Originals)
                       << container.credentialGeneration
                       << container.itemUuid
                       << static_cast<int>(PrivacyBackend::Strong)
                       << container.storeUuid;
        d->db->execSql(QString::fromUtf8("SELECT EXISTS("
                                         "SELECT 1 FROM PrivacyItems "
                                         "INNER JOIN PrivacyCategories "
                                         "ON PrivacyCategories.uuid=PrivacyItems.categoryUuid "
                                         "INNER JOIN PrivacyStoreBindings "
                                         "ON PrivacyStoreBindings.categoryUuid=PrivacyItems.categoryUuid "
                                         "AND PrivacyStoreBindings.role=? "
                                         "INNER JOIN PrivacyStores "
                                         "ON PrivacyStores.uuid=PrivacyStoreBindings.storeUuid "
                                         "INNER JOIN PrivacyCredentials "
                                         "ON PrivacyCredentials.categoryUuid=PrivacyItems.categoryUuid "
                                         "AND PrivacyCredentials.generation=? "
                                         "WHERE PrivacyItems.uuid=? AND PrivacyCategories.backend=? "
                                         "AND PrivacyStores.uuid=?);"),
                       parentBindings, &parentValues);
    }

    if ((parentValues.size() != 1) || !parentValues.constFirst().toBool())
    {
        return false;
    }

    QVariantList values;
    values << container.uuid
           << container.itemUuid
           << static_cast<int>(container.kind)
           << (container.rootUuid.isEmpty() ? QVariant() : container.rootUuid)
           << (container.storeUuid.isEmpty() ? QVariant() : container.storeUuid)
           << container.objectRelativePath
           << container.protectedSize
           << container.protectedHashAlgorithm
           << container.protectedHash
           << container.formatVersion
           << container.credentialGeneration
           << static_cast<int>(container.state)
           << d->db->asDBDateTime(container.createdAt)
           << d->db->asDBDateTime(container.updatedAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyContainers "
                                             "(uuid, itemUuid, kind, rootUuid, storeUuid, objectRelativePath, "
                                             "protectedSize, protectedHashAlgorithm, protectedHash, formatVersion, "
                                             "credentialGeneration, state, createdAt, updatedAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyContainers(QList<PrivacyContainer>* containers) const
{
    if (!containers)
    {
        return false;
    }

    containers->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT uuid, itemUuid, kind, rootUuid, storeUuid, objectRelativePath, "
                                         "protectedSize, protectedHashAlgorithm, protectedHash, formatVersion, "
                                         "credentialGeneration, state, createdAt, updatedAt "
                                         "FROM PrivacyContainers ORDER BY itemUuid, uuid;"), &values) ||
        ((values.size() % 14) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyContainer container;
        container.uuid                    = (*it++).toString();
        container.itemUuid                = (*it++).toString();
        container.kind                    = static_cast<PrivacyContainerKind>((*it++).toInt());
        container.rootUuid                = (*it++).toString();
        container.storeUuid               = (*it++).toString();
        container.objectRelativePath      = (*it++).toString();
        container.protectedSize           = (*it++).toLongLong();
        container.protectedHashAlgorithm  = (*it++).toString();
        container.protectedHash           = (*it++).toString();
        container.formatVersion           = (*it++).toInt();
        container.credentialGeneration    = (*it++).toLongLong();
        container.state                   = static_cast<PrivacyContainerState>((*it++).toInt());
        container.createdAt               = (*it++).toDateTime();
        container.updatedAt               = (*it++).toDateTime();
        containers->append(container);
    }

    return true;
}

bool CoreDB::insertPrivacyAsset(const PrivacyAsset& asset) const
{
    if (!asset.isValid())
    {
        return false;
    }

    QVariantList parentValues;
    QVariantList parentBindings;
    parentBindings << asset.itemUuid
                   << asset.publicRootUuid
                   << static_cast<int>(PrivacyStorageRootKind::AlbumRoot)
                   << asset.containerUuid
                   << asset.itemUuid;
    d->db->execSql(QString::fromUtf8("SELECT "
                                     "EXISTS(SELECT 1 FROM PrivacyItems WHERE uuid=?), "
                                     "EXISTS(SELECT 1 FROM PrivacyStorageRoots WHERE uuid=? AND kind=?), "
                                     "EXISTS(SELECT 1 FROM PrivacyContainers WHERE uuid=? AND itemUuid=?);"),
                   parentBindings, &parentValues);

    if ((parentValues.size() != 3) || !parentValues.at(0).toBool() ||
        !parentValues.at(1).toBool() || !parentValues.at(2).toBool())
    {
        return false;
    }

    QVariantList values;
    values << asset.itemUuid
           << asset.role
           << asset.ordinal
           << asset.originalName
           << asset.publicRootUuid
           << asset.publicRelativePath
           << asset.containerUuid
           << asset.protectedRelativePath
           << asset.hashAlgorithm
           << asset.originalHash
           << asset.originalSize
           << (asset.originalCreationDate.isValid()
               ? d->db->asDBDateTime(asset.originalCreationDate) : QVariant())
           << (asset.originalModificationDate.isValid()
               ? d->db->asDBDateTime(asset.originalModificationDate) : QVariant())
           << (asset.portableAttributes.isEmpty() ? QVariant() : asset.portableAttributes)
           << (asset.proxyHashAlgorithm.isEmpty() ? QVariant() : asset.proxyHashAlgorithm)
           << (asset.proxyHash.isEmpty() ? QVariant() : asset.proxyHash)
           << ((asset.proxySize >= 0) ? QVariant(asset.proxySize) : QVariant())
           << ((asset.proxyPresentationVersion > 0) ? QVariant(asset.proxyPresentationVersion) : QVariant())
           << ((asset.proxyGeneration >= 0) ? QVariant(asset.proxyGeneration) : QVariant());

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyAssets "
                                             "(itemUuid, role, ordinal, originalName, publicRootUuid, "
                                             "publicRelativePath, containerUuid, protectedRelativePath, "
                                             "hashAlgorithm, originalHash, originalSize, originalCreationDate, "
                                             "originalModificationDate, portableAttributes, proxyHashAlgorithm, "
                                             "proxyHash, proxySize, proxyPresentationVersion, proxyGeneration) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"),
                           values));
}

bool CoreDB::getPrivacyAssets(QList<PrivacyAsset>* assets) const
{
    if (!assets)
    {
        return false;
    }

    assets->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT itemUuid, role, ordinal, originalName, publicRootUuid, "
                                         "publicRelativePath, containerUuid, protectedRelativePath, hashAlgorithm, "
                                         "originalHash, originalSize, originalCreationDate, originalModificationDate, "
                                         "portableAttributes, proxyHashAlgorithm, proxyHash, proxySize, "
                                         "proxyPresentationVersion, proxyGeneration "
                                         "FROM PrivacyAssets ORDER BY itemUuid, role, ordinal;"), &values) ||
        ((values.size() % 19) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyAsset asset;
        asset.itemUuid                  = (*it++).toString();
        asset.role                      = (*it++).toInt();
        asset.ordinal                   = (*it++).toInt();
        asset.originalName              = (*it++).toString();
        asset.publicRootUuid            = (*it++).toString();
        asset.publicRelativePath        = (*it++).toString();
        asset.containerUuid             = (*it++).toString();
        asset.protectedRelativePath     = (*it++).toString();
        asset.hashAlgorithm             = (*it++).toString();
        asset.originalHash              = (*it++).toString();
        asset.originalSize              = (*it++).toLongLong();
        asset.originalCreationDate      = (*it++).toDateTime();
        asset.originalModificationDate  = (*it++).toDateTime();
        asset.portableAttributes        = (*it++).toByteArray();
        asset.proxyHashAlgorithm        = (*it++).toString();
        asset.proxyHash                 = (*it++).toString();
        const QVariant proxySize = *it++;
        asset.proxySize                 = proxySize.isNull() ? -1 : proxySize.toLongLong();
        const QVariant presentationVersion = *it++;
        asset.proxyPresentationVersion  = presentationVersion.isNull() ? 0 : presentationVersion.toInt();
        const QVariant proxyGeneration = *it++;
        asset.proxyGeneration           = proxyGeneration.isNull() ? -1 : proxyGeneration.toLongLong();
        assets->append(asset);
    }

    return true;
}

bool CoreDB::insertPrivacyDerivative(const PrivacyDerivative& derivative) const
{
    if (!derivative.isValid())
    {
        return false;
    }

    QVariantList parentValues;
    d->db->execSql(QString::fromUtf8("SELECT EXISTS(SELECT 1 FROM PrivacyStores "
                                     "INNER JOIN PrivacyItems "
                                     "ON PrivacyItems.categoryUuid=PrivacyStores.categoryUuid "
                                     "INNER JOIN PrivacyStoreBindings "
                                     "ON PrivacyStoreBindings.categoryUuid=PrivacyItems.categoryUuid "
                                     "AND PrivacyStoreBindings.storeUuid=PrivacyStores.uuid "
                                     "AND PrivacyStoreBindings.role=? "
                                     "WHERE PrivacyStores.uuid=? AND PrivacyItems.uuid=?);"),
                   static_cast<int>(PrivacyStoreRole::Derivatives),
                   derivative.storeUuid, derivative.itemUuid, &parentValues);

    if ((parentValues.size() != 1) || !parentValues.constFirst().toBool())
    {
        return false;
    }

    QVariantList values;
    values << derivative.itemUuid
           << static_cast<int>(derivative.kind)
           << derivative.ordinal
           << derivative.storeUuid
           << derivative.protectedRelativePath
           << derivative.sourceHashAlgorithm
           << derivative.sourceOriginalHash
           << derivative.derivativeFormat
           << derivative.derivativeHashAlgorithm
           << derivative.derivativeHash
           << derivative.derivativeSize
           << derivative.presentationVersion
           << derivative.generation
           << d->db->asDBDateTime(derivative.createdAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyDerivatives "
                                             "(itemUuid, kind, ordinal, storeUuid, protectedRelativePath, "
                                             "sourceHashAlgorithm, sourceOriginalHash, derivativeFormat, "
                                             "derivativeHashAlgorithm, derivativeHash, derivativeSize, "
                                             "presentationVersion, generation, createdAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyDerivatives(QList<PrivacyDerivative>* derivatives) const
{
    if (!derivatives)
    {
        return false;
    }

    derivatives->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT itemUuid, kind, ordinal, storeUuid, protectedRelativePath, "
                                         "sourceHashAlgorithm, sourceOriginalHash, derivativeFormat, "
                                         "derivativeHashAlgorithm, derivativeHash, derivativeSize, "
                                         "presentationVersion, generation, createdAt "
                                         "FROM PrivacyDerivatives ORDER BY itemUuid, kind, ordinal;"), &values) ||
        ((values.size() % 14) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyDerivative derivative;
        derivative.itemUuid                = (*it++).toString();
        derivative.kind                    = static_cast<PrivacyDerivativeKind>((*it++).toInt());
        derivative.ordinal                 = (*it++).toInt();
        derivative.storeUuid               = (*it++).toString();
        derivative.protectedRelativePath   = (*it++).toString();
        derivative.sourceHashAlgorithm     = (*it++).toString();
        derivative.sourceOriginalHash      = (*it++).toString();
        derivative.derivativeFormat        = (*it++).toString();
        derivative.derivativeHashAlgorithm = (*it++).toString();
        derivative.derivativeHash          = (*it++).toString();
        derivative.derivativeSize          = (*it++).toLongLong();
        derivative.presentationVersion     = (*it++).toInt();
        derivative.generation              = (*it++).toLongLong();
        derivative.createdAt               = (*it++).toDateTime();
        derivatives->append(derivative);
    }

    return true;
}

bool CoreDB::insertPrivacyTransaction(const PrivacyTransaction& transaction) const
{
    if (!transaction.isValid())
    {
        return false;
    }

    QVariantList parentValues;

    if (transaction.itemUuid.isEmpty())
    {
        d->db->execSql(QString::fromUtf8("SELECT EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=?);"),
                       transaction.categoryUuid, &parentValues);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT "
                                         "EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=?), "
                                         "EXISTS(SELECT 1 FROM PrivacyItems WHERE uuid=? AND categoryUuid=?);"),
                       transaction.categoryUuid, transaction.itemUuid,
                       transaction.categoryUuid, &parentValues);
    }

    if (parentValues.isEmpty())
    {
        return false;
    }

    for (const QVariant& value : std::as_const(parentValues))
    {
        if (!value.toBool())
        {
            return false;
        }
    }

    QVariantList credentialValues;
    QVariantList credentialBindings;
    credentialBindings << transaction.fromCredentialGeneration
                       << transaction.categoryUuid
                       << transaction.fromCredentialGeneration
                       << transaction.toCredentialGeneration
                       << transaction.categoryUuid
                       << transaction.toCredentialGeneration;
    d->db->execSql(QString::fromUtf8("SELECT "
                                     "(? < 0 OR EXISTS(SELECT 1 FROM PrivacyCredentials "
                                     "WHERE categoryUuid=? AND generation=?)), "
                                     "(? < 0 OR EXISTS(SELECT 1 FROM PrivacyCredentials "
                                     "WHERE categoryUuid=? AND generation=?));"),
                   credentialBindings, &credentialValues);

    if ((credentialValues.size() != 2) || !credentialValues.at(0).toBool() ||
        !credentialValues.at(1).toBool())
    {
        return false;
    }

    QVariantList values;
    values << transaction.uuid
           << transaction.categoryUuid
           << (transaction.itemUuid.isEmpty() ? QVariant() : transaction.itemUuid)
           << static_cast<int>(transaction.type)
           << static_cast<int>(transaction.state)
           << transaction.generation
           << ((transaction.fromCredentialGeneration >= 0)
               ? QVariant(transaction.fromCredentialGeneration) : QVariant())
           << ((transaction.toCredentialGeneration >= 0)
               ? QVariant(transaction.toCredentialGeneration) : QVariant())
           << transaction.payloadFormatVersion
           << (transaction.payloadData.isEmpty() ? QVariant() : transaction.payloadData)
           << d->db->asDBDateTime(transaction.createdAt)
           << d->db->asDBDateTime(transaction.updatedAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyTransactions "
                                             "(uuid, categoryUuid, itemUuid, type, state, generation, "
                                             "fromCredentialGeneration, toCredentialGeneration, "
                                             "payloadFormatVersion, payloadData, createdAt, updatedAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyTransactions(QList<PrivacyTransaction>* transactions) const
{
    if (!transactions)
    {
        return false;
    }

    transactions->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT uuid, categoryUuid, itemUuid, type, state, generation, "
                                         "fromCredentialGeneration, toCredentialGeneration, "
                                         "payloadFormatVersion, payloadData, createdAt, updatedAt "
                                         "FROM PrivacyTransactions ORDER BY createdAt, uuid;"), &values) ||
        ((values.size() % 12) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyTransaction transaction;
        transaction.uuid                     = (*it++).toString();
        transaction.categoryUuid             = (*it++).toString();
        transaction.itemUuid                 = (*it++).toString();
        transaction.type                     = static_cast<PrivacyTransactionType>((*it++).toInt());
        transaction.state                    = static_cast<PrivacyTransactionState>((*it++).toInt());
        transaction.generation               = (*it++).toLongLong();
        const QVariant fromGeneration = *it++;
        transaction.fromCredentialGeneration = fromGeneration.isNull() ? -1 : fromGeneration.toLongLong();
        const QVariant toGeneration = *it++;
        transaction.toCredentialGeneration   = toGeneration.isNull() ? -1 : toGeneration.toLongLong();
        transaction.payloadFormatVersion     = (*it++).toInt();
        transaction.payloadData              = (*it++).toByteArray();
        transaction.createdAt                = (*it++).toDateTime();
        transaction.updatedAt                = (*it++).toDateTime();
        transactions->append(transaction);
    }

    return true;
}

bool CoreDB::getActivePrivacyTransactions(QList<PrivacyTransaction>* transactions) const
{
    if (!transactions)
    {
        return false;
    }

    transactions->clear();
    QVariantList values;
    const QVariantList activeStates = activePrivacyTransactionStates();
    QString sql = QString::fromUtf8("SELECT uuid, categoryUuid, itemUuid, type, state, generation, "
                                    "fromCredentialGeneration, toCredentialGeneration, "
                                    "payloadFormatVersion, payloadData, createdAt, updatedAt "
                                    "FROM PrivacyTransactions WHERE state IN (");
    addBoundValuePlaceholders(sql, activeStates.size());
    sql += QLatin1String(") ORDER BY createdAt, uuid;");

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(sql, activeStates, &values) ||
        ((values.size() % 12) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyTransaction transaction;
        transaction.uuid                     = (*it++).toString();
        transaction.categoryUuid             = (*it++).toString();
        transaction.itemUuid                 = (*it++).toString();
        transaction.type                     = static_cast<PrivacyTransactionType>((*it++).toInt());
        transaction.state                    = static_cast<PrivacyTransactionState>((*it++).toInt());
        transaction.generation               = (*it++).toLongLong();
        const QVariant fromGeneration = *it++;
        transaction.fromCredentialGeneration = fromGeneration.isNull() ? -1 : fromGeneration.toLongLong();
        const QVariant toGeneration = *it++;
        transaction.toCredentialGeneration   = toGeneration.isNull() ? -1 : toGeneration.toLongLong();
        transaction.payloadFormatVersion     = (*it++).toInt();
        transaction.payloadData              = (*it++).toByteArray();
        transaction.createdAt                = (*it++).toDateTime();
        transaction.updatedAt                = (*it++).toDateTime();
        transactions->append(transaction);
    }

    return true;
}

bool CoreDB::compareAndUpdatePrivacyTransaction(const PrivacyTransaction& transaction,
                                                PrivacyTransactionState expectedState,
                                                qlonglong expectedGeneration) const
{
    if (!transaction.isValid() || (expectedGeneration < 0) ||
        (transaction.generation <= expectedGeneration))
    {
        return false;
    }

    QVariantList values;
    values << static_cast<int>(transaction.state)
           << transaction.generation
           << ((transaction.fromCredentialGeneration >= 0)
               ? QVariant(transaction.fromCredentialGeneration) : QVariant())
           << ((transaction.toCredentialGeneration >= 0)
               ? QVariant(transaction.toCredentialGeneration) : QVariant())
           << transaction.payloadFormatVersion
           << (transaction.payloadData.isEmpty() ? QVariant() : transaction.payloadData)
           << d->db->asDBDateTime(transaction.updatedAt)
           << transaction.uuid
           << static_cast<int>(expectedState)
           << expectedGeneration
           << transaction.categoryUuid
           << static_cast<int>(transaction.type)
           << (transaction.itemUuid.isEmpty() ? QVariant() : transaction.itemUuid)
           << (transaction.itemUuid.isEmpty() ? QVariant() : transaction.itemUuid)
           << transaction.fromCredentialGeneration
           << transaction.categoryUuid
           << transaction.fromCredentialGeneration
           << transaction.toCredentialGeneration
           << transaction.categoryUuid
           << transaction.toCredentialGeneration;

    QSqlQuery query = d->db->execQuery(
        QString::fromUtf8("UPDATE PrivacyTransactions SET state=?, generation=?, "
                          "fromCredentialGeneration=?, toCredentialGeneration=?, "
                          "payloadFormatVersion=?, payloadData=?, updatedAt=? "
                          "WHERE uuid=? AND state=? AND generation=? AND categoryUuid=? AND type=? "
                          "AND ((itemUuid IS NULL AND ? IS NULL) OR itemUuid=?) "
                          "AND (? < 0 OR EXISTS(SELECT 1 FROM PrivacyCredentials "
                          "WHERE categoryUuid=? AND generation=?)) "
                          "AND (? < 0 OR EXISTS(SELECT 1 FROM PrivacyCredentials "
                          "WHERE categoryUuid=? AND generation=?));"), values);

    return (query.isActive() && (query.numRowsAffected() == 1));
}

bool CoreDB::insertPrivacyTransactionJournal(const PrivacyTransactionJournal& journal) const
{
    if (!journal.isValid())
    {
        return false;
    }

    QVariantList parentValues;
    d->db->execSql(QString::fromUtf8("SELECT "
                                     "EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=?), "
                                     "EXISTS(SELECT 1 FROM PrivacyStorageRoots WHERE uuid=?);"),
                   journal.transactionUuid, journal.rootUuid, &parentValues);

    if ((parentValues.size() != 2) || !parentValues.at(0).toBool() || !parentValues.at(1).toBool())
    {
        return false;
    }

    QVariantList values;
    values << journal.transactionUuid
           << journal.rootUuid
           << journal.journalRelativePath
           << journal.journalFormatVersion
           << journal.stage
           << (journal.expectedHashAlgorithm.isEmpty() ? QVariant() : journal.expectedHashAlgorithm)
           << (journal.expectedJournalHash.isEmpty() ? QVariant() : journal.expectedJournalHash)
           << d->db->asDBDateTime(journal.updatedAt);

    return (BdEngineBackend::NoErrors ==
            d->db->execSql(QString::fromUtf8("INSERT INTO PrivacyTransactionJournals "
                                             "(transactionUuid, rootUuid, journalRelativePath, "
                                             "journalFormatVersion, stage, expectedHashAlgorithm, "
                                             "expectedJournalHash, updatedAt) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?);"), values));
}

bool CoreDB::getPrivacyTransactionJournals(QList<PrivacyTransactionJournal>* journals) const
{
    if (!journals)
    {
        return false;
    }

    journals->clear();
    QVariantList values;

    if (BdEngineBackend::NoErrors !=
        d->db->execSql(QString::fromUtf8("SELECT transactionUuid, rootUuid, journalRelativePath, "
                                         "journalFormatVersion, stage, expectedHashAlgorithm, "
                                         "expectedJournalHash, updatedAt "
                                         "FROM PrivacyTransactionJournals ORDER BY transactionUuid, rootUuid;"),
                       &values) || ((values.size() % 8) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyTransactionJournal journal;
        journal.transactionUuid       = (*it++).toString();
        journal.rootUuid              = (*it++).toString();
        journal.journalRelativePath   = (*it++).toString();
        journal.journalFormatVersion  = (*it++).toInt();
        journal.stage                 = (*it++).toInt();
        journal.expectedHashAlgorithm = (*it++).toString();
        journal.expectedJournalHash   = (*it++).toString();
        journal.updatedAt             = (*it++).toDateTime();
        journals->append(journal);
    }

    return true;
}

bool CoreDB::getActivePrivacyTransactionJournals(
    QList<PrivacyTransactionJournal>* journals) const
{
    if (!journals)
    {
        return false;
    }

    journals->clear();
    QVariantList values;
    const QVariantList activeStates = activePrivacyTransactionStates();
    QString sql = QString::fromUtf8(
        "SELECT PrivacyTransactionJournals.transactionUuid, "
        "PrivacyTransactionJournals.rootUuid, "
        "PrivacyTransactionJournals.journalRelativePath, "
        "PrivacyTransactionJournals.journalFormatVersion, "
        "PrivacyTransactionJournals.stage, "
        "PrivacyTransactionJournals.expectedHashAlgorithm, "
        "PrivacyTransactionJournals.expectedJournalHash, "
        "PrivacyTransactionJournals.updatedAt "
        "FROM PrivacyTransactionJournals "
        "INNER JOIN PrivacyTransactions ON "
        "PrivacyTransactions.uuid=PrivacyTransactionJournals.transactionUuid "
        "WHERE PrivacyTransactions.state IN (");
    addBoundValuePlaceholders(sql, activeStates.size());
    sql += QLatin1String(
        ") ORDER BY PrivacyTransactionJournals.transactionUuid, "
        "PrivacyTransactionJournals.rootUuid;");

    if ((BdEngineBackend::NoErrors !=
         d->db->execSql(sql, activeStates, &values)) ||
        ((values.size() % 8) != 0))
    {
        return false;
    }

    for (auto it = values.constBegin() ; it != values.constEnd() ; )
    {
        PrivacyTransactionJournal journal;
        journal.transactionUuid       = (*it++).toString();
        journal.rootUuid              = (*it++).toString();
        journal.journalRelativePath   = (*it++).toString();
        journal.journalFormatVersion  = (*it++).toInt();
        journal.stage                 = (*it++).toInt();
        journal.expectedHashAlgorithm = (*it++).toString();
        journal.expectedJournalHash   = (*it++).toString();
        journal.updatedAt             = (*it++).toDateTime();
        journals->append(journal);
    }

    return true;
}

bool CoreDB::compareAndUpdatePrivacyTransactionJournal(const PrivacyTransactionJournal& journal,
                                                       int expectedStage) const
{
    if (!journal.isValid() || (expectedStage < 0) || (journal.stage < expectedStage))
    {
        return false;
    }

    QVariantList values;
    values << journal.stage
           << (journal.expectedHashAlgorithm.isEmpty() ? QVariant() : journal.expectedHashAlgorithm)
           << (journal.expectedJournalHash.isEmpty() ? QVariant() : journal.expectedJournalHash)
           << d->db->asDBDateTime(journal.updatedAt)
           << journal.transactionUuid
           << journal.rootUuid
           << journal.journalRelativePath
           << journal.journalFormatVersion
           << expectedStage;

    QSqlQuery query = d->db->execQuery(
        QString::fromUtf8("UPDATE PrivacyTransactionJournals SET stage=?, expectedHashAlgorithm=?, "
                          "expectedJournalHash=?, updatedAt=? WHERE transactionUuid=? AND rootUuid=? "
                          "AND journalRelativePath=? AND journalFormatVersion=? AND stage=?;"), values);

    return (query.isActive() && (query.numRowsAffected() == 1));
}

bool CoreDB::beginPrivacyCategoryCreation(
    const PrivacyCategory& category,
    const PrivacyStorageRoot& root,
    const PrivacyStore& store,
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    if (!category.isValid() || !root.isValid() || !store.isValid() ||
        !transaction.isValid() || !journal.isValid() ||
        (category.lifecycleState != PrivacyCategoryLifecycleState::Creating) ||
        (category.currentCredentialGeneration != 0) ||
        (root.kind != PrivacyStorageRootKind::ManagedStoreRoot) ||
        (store.categoryUuid != category.uuid) || (store.rootUuid != root.uuid) ||
        (store.lifecycleState != PrivacyStoreLifecycleState::Creating) ||
        (store.configGeneration != -1) ||
        (transaction.categoryUuid != category.uuid) ||
        (transaction.type != PrivacyTransactionType::CreateCategory) ||
        (transaction.state != PrivacyTransactionState::Created) ||
        (transaction.generation != 0) ||
        (journal.transactionUuid != transaction.uuid) ||
        (journal.rootUuid != root.uuid) || d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    QVariantList conflicts;
    d->db->execSql(QString::fromUtf8(
        "SELECT EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=? OR LOWER(name)=LOWER(?)), "
        "EXISTS(SELECT 1 FROM PrivacyStores WHERE uuid=?), "
        "EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=?);"),
        category.uuid, category.name, store.uuid, transaction.uuid, &conflicts);

    if ((conflicts.size() != 3) || conflicts.at(0).toBool() ||
        conflicts.at(1).toBool() || conflicts.at(2).toBool())
    {
        return abort();
    }

    QList<PrivacyStorageRoot> roots;

    if (!getPrivacyStorageRoots(&roots))
    {
        return abort();
    }

    bool rootAlreadyPresent = false;

    for (const PrivacyStorageRoot& existing : std::as_const(roots))
    {
        if (existing.uuid != root.uuid)
        {
            continue;
        }

        rootAlreadyPresent = true;

        if ((existing.kind != root.kind) ||
            (existing.configuredPath != root.configuredPath) ||
            (existing.identityVersion != root.identityVersion) ||
            (existing.identityData != root.identityData) ||
            (existing.markerUuid != root.markerUuid))
        {
            return abort();
        }
    }

    if (!insertPrivacyCategory(category) ||
        (!rootAlreadyPresent && !insertPrivacyStorageRoot(root)) ||
        !insertPrivacyStore(store) || !insertPrivacyTransaction(transaction) ||
        !insertPrivacyTransactionJournal(journal))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

bool CoreDB::publishPrivacyCategoryCreation(
    const PrivacyCategory& category,
    const PrivacyCredential& credential,
    const PrivacyStore& store,
    const QList<PrivacyStoreBinding>& bindings,
    const PrivacyTransaction& transaction) const
{
    if (!category.isValid() || !credential.isValid() || !store.isValid() ||
        !transaction.isValid() || bindings.isEmpty() ||
        (category.lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (category.currentCredentialGeneration != credential.generation) ||
        (credential.categoryUuid != category.uuid) ||
        (store.categoryUuid != category.uuid) ||
        (store.lifecycleState != PrivacyStoreLifecycleState::Active) ||
        (store.configGeneration != credential.generation) ||
        (transaction.categoryUuid != category.uuid) ||
        (transaction.type != PrivacyTransactionType::CreateCategory) ||
        (transaction.state != PrivacyTransactionState::Complete) ||
        (transaction.generation != 1) || d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    const QVariantList expectedBindings = {
        category.uuid,
        static_cast<int>(PrivacyCategoryLifecycleState::Creating),
        store.uuid,
        category.uuid,
        static_cast<int>(PrivacyStoreLifecycleState::Creating),
        transaction.uuid,
        category.uuid,
        static_cast<int>(PrivacyTransactionType::CreateCategory),
        static_cast<int>(PrivacyTransactionState::Created)
    };
    QVariantList expected;
    d->db->execSql(QString::fromUtf8(
        "SELECT "
        "EXISTS(SELECT 1 FROM PrivacyCategories WHERE uuid=? AND lifecycleState=? "
        "AND currentCredentialGeneration=0), "
        "EXISTS(SELECT 1 FROM PrivacyStores WHERE uuid=? AND categoryUuid=? "
        "AND lifecycleState=? AND configGeneration IS NULL), "
        "EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=? AND categoryUuid=? "
        "AND type=? AND state=? AND generation=0);"),
        expectedBindings, &expected);

    if ((expected.size() != 3) || !expected.at(0).toBool() ||
        !expected.at(1).toBool() || !expected.at(2).toBool() ||
        !insertPrivacyCredential(credential))
    {
        return abort();
    }

    for (const PrivacyStoreBinding& binding : bindings)
    {
        if ((binding.categoryUuid != category.uuid) ||
            (binding.storeUuid != store.uuid) || !insertPrivacyStoreBinding(binding))
        {
            return abort();
        }
    }

    QVariantList storeValues;
    storeValues << store.configGeneration
                << static_cast<int>(store.lifecycleState)
                << store.uuid << category.uuid
                << static_cast<int>(PrivacyStoreLifecycleState::Creating);
    const QSqlQuery storeQuery = d->db->execQuery(QString::fromUtf8(
        "UPDATE PrivacyStores SET configGeneration=?, lifecycleState=? "
        "WHERE uuid=? AND categoryUuid=? AND lifecycleState=? AND configGeneration IS NULL;"),
        storeValues);

    QVariantList categoryValues;
    categoryValues << static_cast<int>(category.lifecycleState)
                   << category.currentCredentialGeneration << category.uuid
                   << static_cast<int>(PrivacyCategoryLifecycleState::Creating);
    const QSqlQuery categoryQuery = d->db->execQuery(QString::fromUtf8(
        "UPDATE PrivacyCategories SET lifecycleState=?, currentCredentialGeneration=? "
        "WHERE uuid=? AND lifecycleState=? AND currentCredentialGeneration=0;"),
        categoryValues);

    if (!storeQuery.isActive() || (storeQuery.numRowsAffected() != 1) ||
        !categoryQuery.isActive() || (categoryQuery.numRowsAffected() != 1) ||
        !compareAndUpdatePrivacyTransaction(transaction,
                                            PrivacyTransactionState::Created, 0))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

bool CoreDB::beginPrivacyItemProtection(
    const PrivacyItem& item,
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    if (!item.isValid() || !transaction.isValid() || !journal.isValid() ||
        (item.transactionState != static_cast<int>(PrivacyTransactionState::Created)) ||
        (transaction.itemUuid != item.uuid) ||
        (transaction.categoryUuid != item.categoryUuid) ||
        (transaction.type != PrivacyTransactionType::ProtectItem) ||
        (transaction.state != PrivacyTransactionState::Created) ||
        (transaction.generation != 0) ||
        (journal.transactionUuid != transaction.uuid) ||
        (journal.stage != static_cast<int>(PrivacyJournalStage::Created)) ||
        d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    QVariantList conflicts;
    d->db->execSql(QString::fromUtf8(
        "SELECT EXISTS(SELECT 1 FROM PrivacyItems WHERE imageId=? OR uuid=?), "
        "EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=?);"),
        item.imageId, item.uuid, transaction.uuid, &conflicts);

    if ((conflicts.size() != 2) || conflicts.at(0).toBool() ||
        conflicts.at(1).toBool() || !insertPrivacyItem(item) ||
        !insertPrivacyTransaction(transaction) ||
        !insertPrivacyTransactionJournal(journal))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

bool CoreDB::publishPrivacyItemProtection(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets,
    const PrivacyTransaction& transaction) const
{
    if (!item.isValid() || !container.isValid() || assets.isEmpty() ||
        !transaction.isValid() ||
        (item.transactionState != static_cast<int>(PrivacyTransactionState::Complete)) ||
        (container.itemUuid != item.uuid) ||
        (container.state != PrivacyContainerState::Verified) ||
        (transaction.itemUuid != item.uuid) ||
        (transaction.categoryUuid != item.categoryUuid) ||
        (transaction.type != PrivacyTransactionType::ProtectItem) ||
        (transaction.state != PrivacyTransactionState::Complete) ||
        (transaction.generation != 2) || d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    const PrivacyItem existing = getPrivacyItem(item.imageId);

    if (!existing.isValid() || (existing.uuid != item.uuid) ||
        (existing.categoryUuid != item.categoryUuid) ||
        (existing.originalHash != item.originalHash) ||
        (existing.originalSize != item.originalSize) ||
        (existing.originalWidth != item.originalWidth) ||
        (existing.originalHeight != item.originalHeight) ||
        (existing.originalCreationDate != item.originalCreationDate) ||
        (existing.expectedProxyHash != item.expectedProxyHash) ||
        (existing.expectedProxySize != item.expectedProxySize) ||
        (existing.presentationVersion != item.presentationVersion) ||
        (existing.generation != item.generation) ||
        (existing.transactionState !=
         static_cast<int>(PrivacyTransactionState::Created)))
    {
        return abort();
    }

    QVariantList expected;
    QVariantList expectedBindings;
    expectedBindings << transaction.uuid << item.categoryUuid << item.uuid
                     << static_cast<int>(PrivacyTransactionType::ProtectItem)
                     << static_cast<int>(PrivacyTransactionState::Prepared)
                     << transaction.uuid
                     << static_cast<int>(PrivacyJournalStage::Complete)
                     << item.uuid << item.uuid;
    d->db->execSql(QString::fromUtf8(
        "SELECT EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=? "
        "AND categoryUuid=? AND itemUuid=? AND type=? AND state=? AND generation=1), "
        "EXISTS(SELECT 1 FROM PrivacyTransactionJournals WHERE transactionUuid=? "
        "AND stage=?), "
        "NOT EXISTS(SELECT 1 FROM PrivacyContainers WHERE itemUuid=?), "
        "NOT EXISTS(SELECT 1 FROM PrivacyAssets WHERE itemUuid=?);"),
        expectedBindings, &expected);

    if ((expected.size() != 4) ||
        std::any_of(expected.cbegin(), expected.cend(),
                    [](const QVariant& value) { return !value.toBool(); }) ||
        !insertPrivacyContainer(container))
    {
        return abort();
    }

    for (const PrivacyAsset& asset : assets)
    {
        if ((asset.itemUuid != item.uuid) ||
            (asset.containerUuid != container.uuid) || !insertPrivacyAsset(asset))
        {
            return abort();
        }
    }

    QVariantList itemBindings;
    itemBindings << static_cast<int>(PrivacyTransactionState::Complete)
                 << item.imageId << item.uuid << item.categoryUuid
                 << item.generation
                 << static_cast<int>(PrivacyTransactionState::Created);
    const QSqlQuery itemQuery = d->db->execQuery(
        QString::fromUtf8("UPDATE PrivacyItems SET transactionState=? "
                          "WHERE imageId=? AND uuid=? AND categoryUuid=? "
                          "AND generation=? AND transactionState=?;"),
        itemBindings);

    if (!itemQuery.isActive() || (itemQuery.numRowsAffected() != 1) ||
        !compareAndUpdatePrivacyTransaction(transaction,
                                            PrivacyTransactionState::Prepared, 1))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

bool CoreDB::beginPrivacyItemUnprotection(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    if (!transaction.isValid() || !journal.isValid() ||
        transaction.itemUuid.isEmpty() ||
        (transaction.type != PrivacyTransactionType::UnprotectItem) ||
        (transaction.state != PrivacyTransactionState::Created) ||
        (transaction.generation != 0) ||
        (journal.transactionUuid != transaction.uuid) ||
        (journal.stage != static_cast<int>(PrivacyJournalStage::Created)) ||
        d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    QVariantList expected;
    QVariantList expectedBindings;
    expectedBindings << transaction.itemUuid << transaction.categoryUuid
                     << transaction.uuid << transaction.itemUuid
                     << static_cast<int>(PrivacyTransactionState::Complete);
    d->db->execSql(QString::fromUtf8(
        "SELECT EXISTS(SELECT 1 FROM PrivacyItems WHERE uuid=? AND categoryUuid=?), "
        "NOT EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=?), "
        "NOT EXISTS(SELECT 1 FROM PrivacyTransactions WHERE itemUuid=? AND state<>?);"),
        expectedBindings, &expected);

    if ((expected.size() != 3) ||
        std::any_of(expected.cbegin(), expected.cend(),
                    [](const QVariant& value) { return !value.toBool(); }) ||
        !insertPrivacyTransaction(transaction) ||
        !insertPrivacyTransactionJournal(journal))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

bool CoreDB::beginPrivacyCompatibilityUnlock(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    if (!transaction.isValid() || !journal.isValid() ||
        transaction.itemUuid.isEmpty() ||
        (transaction.type != PrivacyTransactionType::CompatibilityUnlock) ||
        (transaction.state != PrivacyTransactionState::Created) ||
        (transaction.generation != 0) ||
        (journal.transactionUuid != transaction.uuid) ||
        (journal.stage != static_cast<int>(PrivacyJournalStage::Created)) ||
        d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    QVariantList expected;
    QVariantList bindings;
    bindings << transaction.itemUuid << transaction.categoryUuid
             << transaction.uuid << transaction.itemUuid
             << static_cast<int>(PrivacyTransactionState::Complete);
    d->db->execSql(QString::fromUtf8(
        "SELECT EXISTS(SELECT 1 FROM PrivacyItems WHERE uuid=? AND categoryUuid=?), "
        "NOT EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=?), "
        "NOT EXISTS(SELECT 1 FROM PrivacyTransactions WHERE itemUuid=? AND state<>?);"),
        bindings, &expected);

    if ((expected.size() != 3) ||
        std::any_of(expected.cbegin(), expected.cend(),
                    [](const QVariant& value) { return !value.toBool(); }) ||
        !insertPrivacyTransaction(transaction) ||
        !insertPrivacyTransactionJournal(journal))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

bool CoreDB::publishPrivacyItemUnprotection(
    qlonglong imageId, const QString& itemUuid, const QString& categoryUuid,
    qlonglong expectedItemGeneration,
    const QString& priorProtectTransactionUuid,
    const PrivacyTransaction& transaction) const
{
    if ((imageId <= 0) || itemUuid.isEmpty() || categoryUuid.isEmpty() ||
        (expectedItemGeneration < 0) || priorProtectTransactionUuid.isEmpty() ||
        (priorProtectTransactionUuid == transaction.uuid) || !transaction.isValid() ||
        (transaction.itemUuid != itemUuid) ||
        (transaction.categoryUuid != categoryUuid) ||
        (transaction.type != PrivacyTransactionType::UnprotectItem) ||
        (transaction.state != PrivacyTransactionState::Applying) ||
        (transaction.generation != 2) || d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    QVariantList expected;
    QVariantList expectedBindings;
    expectedBindings << imageId << itemUuid << categoryUuid
                     << expectedItemGeneration << transaction.uuid << itemUuid
                     << categoryUuid
                     << static_cast<int>(PrivacyTransactionType::UnprotectItem)
                     << static_cast<int>(PrivacyTransactionState::Prepared)
                     << transaction.uuid
                     << static_cast<int>(PrivacyJournalStage::Complete)
                     << priorProtectTransactionUuid << itemUuid << categoryUuid
                     << static_cast<int>(PrivacyTransactionType::ProtectItem)
                     << static_cast<int>(PrivacyTransactionState::Complete)
                     << priorProtectTransactionUuid
                     << static_cast<int>(PrivacyJournalStage::Complete);
    d->db->execSql(QString::fromUtf8(
        "SELECT EXISTS(SELECT 1 FROM PrivacyItems WHERE imageId=? AND uuid=? "
        "AND categoryUuid=? AND generation=?), "
        "EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=? AND itemUuid=? "
        "AND categoryUuid=? AND type=? AND state=? AND generation=1), "
        "EXISTS(SELECT 1 FROM PrivacyTransactionJournals WHERE transactionUuid=? "
        "AND stage=?), "
        "EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=? AND itemUuid=? "
        "AND categoryUuid=? AND type=? AND state=? AND generation=2), "
        "EXISTS(SELECT 1 FROM PrivacyTransactionJournals WHERE transactionUuid=? "
        "AND stage=?);"),
        expectedBindings, &expected);

    if ((expected.size() != 5) ||
        std::any_of(expected.cbegin(), expected.cend(),
                    [](const QVariant& value) { return !value.toBool(); }))
    {
        return abort();
    }

    QVariantList detachBindings;
    detachBindings << static_cast<int>(transaction.state)
                   << transaction.generation
                   << transaction.payloadFormatVersion
                   << (transaction.payloadData.isEmpty()
                       ? QVariant() : QVariant(transaction.payloadData))
                   << d->db->asDBDateTime(transaction.updatedAt)
                   << transaction.uuid << itemUuid << categoryUuid
                   << static_cast<int>(PrivacyTransactionType::UnprotectItem)
                   << static_cast<int>(PrivacyTransactionState::Prepared);
    const QSqlQuery detach = d->db->execQuery(
        QString::fromUtf8("UPDATE PrivacyTransactions SET itemUuid=NULL, state=?, "
                          "generation=?, payloadFormatVersion=?, payloadData=?, updatedAt=? "
                          "WHERE uuid=? AND itemUuid=? AND categoryUuid=? AND type=? "
                          "AND state=? AND generation=1;"),
        detachBindings);

    QVariantList priorDeleteBindings;
    priorDeleteBindings << priorProtectTransactionUuid << itemUuid << categoryUuid
                        << static_cast<int>(PrivacyTransactionType::ProtectItem)
                        << static_cast<int>(PrivacyTransactionState::Complete);

    if (!detach.isActive() || (detach.numRowsAffected() != 1) ||
        (BdEngineBackend::NoErrors != d->db->execSql(QString::fromUtf8(
            "DELETE FROM PrivacyTransactionJournals WHERE transactionUuid=?;"),
            priorProtectTransactionUuid)) ||
        (BdEngineBackend::NoErrors != d->db->execSql(QString::fromUtf8(
            "DELETE FROM PrivacyTransactions WHERE uuid=? AND itemUuid=? "
            "AND categoryUuid=? AND type=? AND state=? AND generation=2;"),
            priorDeleteBindings)) ||
        (BdEngineBackend::NoErrors != d->db->execSql(
            QString::fromUtf8("DELETE FROM PrivacyDerivatives WHERE itemUuid=?;"),
            itemUuid)) ||
        (BdEngineBackend::NoErrors != d->db->execSql(
            QString::fromUtf8("DELETE FROM PrivacyAssets WHERE itemUuid=?;"),
            itemUuid)) ||
        (BdEngineBackend::NoErrors != d->db->execSql(
            QString::fromUtf8("DELETE FROM PrivacyContainers WHERE itemUuid=?;"),
            itemUuid)))
    {
        return abort();
    }

    QVariantList itemDeleteBindings;
    itemDeleteBindings << imageId << itemUuid << categoryUuid
                       << expectedItemGeneration;
    const QSqlQuery itemDelete = d->db->execQuery(
        QString::fromUtf8("DELETE FROM PrivacyItems WHERE imageId=? AND uuid=? "
                          "AND categoryUuid=? AND generation=?;"),
        itemDeleteBindings);

    if (!itemDelete.isActive() || (itemDelete.numRowsAffected() != 1))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

bool CoreDB::finalizePrivacyItemUnprotection(
    const QString& transactionUuid, const QString& categoryUuid) const
{
    if (transactionUuid.isEmpty() || categoryUuid.isEmpty() ||
        d->db->isInTransaction() ||
        (d->db->beginTransaction() != BdEngineBackend::NoErrors))
    {
        return false;
    }

    const auto abort = [this]()
    {
        d->db->rollbackTransactionAndFinish();
        return false;
    };

    QVariantList expected;
    QVariantList expectedBindings;
    expectedBindings << transactionUuid << categoryUuid
                     << static_cast<int>(PrivacyTransactionType::UnprotectItem)
                     << static_cast<int>(PrivacyTransactionState::Applying)
                     << transactionUuid
                     << static_cast<int>(PrivacyJournalStage::Complete);
    d->db->execSql(QString::fromUtf8(
        "SELECT EXISTS(SELECT 1 FROM PrivacyTransactions WHERE uuid=? "
        "AND categoryUuid=? AND itemUuid IS NULL AND type=? AND state=? "
        "AND generation=2), "
        "EXISTS(SELECT 1 FROM PrivacyTransactionJournals WHERE transactionUuid=? "
        "AND stage=?);"),
        expectedBindings, &expected);

    if ((expected.size() != 2) || !expected.at(0).toBool() ||
        !expected.at(1).toBool() ||
        (BdEngineBackend::NoErrors != d->db->execSql(
            QString::fromUtf8("DELETE FROM PrivacyTransactionJournals "
                              "WHERE transactionUuid=?;"), transactionUuid)) ||
        (BdEngineBackend::NoErrors != d->db->execSql(
            QString::fromUtf8("DELETE FROM PrivacyTransactions WHERE uuid=? "
                              "AND categoryUuid=? AND itemUuid IS NULL;"),
            transactionUuid, categoryUuid)))
    {
        return abort();
    }

    return (d->db->commitTransaction() == BdEngineBackend::NoErrors);
}

QList<AlbumRootInfo> CoreDB::getAlbumRoots() const
{
    QList<AlbumRootInfo> list;
    QVariantList         values;

    d->db->execSql(QString::fromUtf8("SELECT id, label, status, type, identifier, specificPath, caseSensitivity "
                                     " FROM AlbumRoots;"), &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        AlbumRootInfo info;

        info.id              = (*it).toInt();
        ++it;
        info.label           = (*it).toString();
        ++it;
        info.status          = (*it).toInt();
        ++it;
        info.type            = (*it).toInt();
        ++it;
        info.identifier      = (*it).toString();
        ++it;
        info.specificPath    = (*it).toString();
        ++it;
        info.caseSensitivity = (*it).toInt();
        ++it;

        list << info;
    }

    return list;
}

int CoreDB::addAlbumRoot(CollectionLocation::Type type, const QString& identifier, const QString& specificPath, const QString& label) const
{
    QVariant id;
    d->db->execSql(QString::fromUtf8("REPLACE INTO AlbumRoots (type, label, status, identifier, specificPath, caseSensitivity) "
                                     "VALUES(?, ?, 0, ?, ?, 0);"),
                   (int)type, label, identifier, specificPath, nullptr, &id);

    d->db->recordChangeset(AlbumRootChangeset(id.toInt(), AlbumRootChangeset::Added));

    return id.toInt();
}

void CoreDB::deleteAlbumRoot(int rootId)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM AlbumRoots WHERE id=?;"),
                   rootId);
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumRoot"), rootId);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumRoot")), parameters))
    {
        return;
    }

    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::Deleted));
}

void CoreDB::migrateAlbumRoot(int rootId, const QString& identifier)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET identifier=? WHERE id=?;"),
                   identifier, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void CoreDB::setAlbumRootLabel(int rootId, const QString& newLabel)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET label=? WHERE id=?;"),
                   newLabel, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void CoreDB::setAlbumRootType(int rootId, CollectionLocation::Type newType)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET type=? WHERE id=?;"),
                   (int)newType, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void CoreDB::setAlbumRootCaseSensitivity(int rootId, CollectionLocation::CaseSensitivity caseSensitivity)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET caseSensitivity=? WHERE id=?;"),
                   (int)caseSensitivity, rootId);

    // record that the album root was changed is not necessary here
}

void CoreDB::setAlbumRootPath(int rootId, const QString& newPath)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET specificPath=? WHERE id=?;"),
                   newPath, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

AlbumInfo::List CoreDB::scanAlbums() const
{
    AlbumInfo::List aList;
    QVariantList    values;

    d->db->execSql(QString::fromUtf8("SELECT albumRoot, id, relativePath, date, caption, collection, icon "
                                     "FROM Albums WHERE albumRoot != 0;"), // exclude stale albums
                   &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        AlbumInfo info;

        info.albumRootId    = (*it).toInt();
        ++it;
        info.id             = (*it).toInt();
        ++it;
        info.relativePath   = (*it).toString();
        ++it;
        info.date           = (*it).toDate();
        ++it;
        info.caption        = (*it).toString();
        ++it;
        info.category       = (*it).toString();
        ++it;
        info.iconId         = (*it).toLongLong();
        ++it;

        aList.append(info);
    }

    return aList;
}

TagInfo::List CoreDB::scanTags() const
{
    TagInfo::List tList;
    QVariantList  values;

    d->db->execSql(QString::fromUtf8("SELECT id, pid, name, icon, iconkde FROM Tags;"),
                   &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        TagInfo info;

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        info.iconId = (*it).toLongLong();
        ++it;
        info.icon   = (*it).toString();
        ++it;

        tList.append(info);
    }

    return tList;
}

TagInfo CoreDB::getTagInfo(int tagId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id, pid, name, icon, iconkde WHERE id=? FROM Tags;"),
                   tagId, &values);

    TagInfo info;

    if (!values.isEmpty() && values.size() == 5)
    {
        QList<QVariant>::const_iterator it = values.constBegin();

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        info.iconId = (*it).toLongLong();
        ++it;
        info.icon   = (*it).toString();
        ++it;
    }

    return info;
}

SearchInfo::List CoreDB::scanSearches() const
{
    SearchInfo::List searchList;
    QVariantList     values;

    d->db->execSql(QString::fromUtf8("SELECT id, type, name, query FROM Searches;"),
                   &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        SearchInfo info;

        info.id    = (*it).toInt();
        ++it;
        info.type  = (DatabaseSearch::Type)(*it).toInt();
        ++it;
        info.name  = (*it).toString();
        ++it;
        info.query = (*it).toString();
        ++it;

        searchList.append(info);
    }

    return searchList;
}

QList<AlbumShortInfo> CoreDB::getAlbumShortInfos() const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id, relativePath, albumRoot FROM Albums ORDER BY id;"),
                   &values);

    QList<AlbumShortInfo> albumList;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        AlbumShortInfo info;

        info.id           = (*it).toInt();
        ++it;
        info.relativePath = (*it).toString();
        ++it;
        info.albumRootId  = (*it).toInt();
        ++it;

        albumList << info;
    }

    return albumList;
}

QList<TagShortInfo> CoreDB::getTagShortInfos() const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id, pid, name FROM Tags ORDER BY id;"),
                   &values);

    QList<TagShortInfo> tagList;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        TagShortInfo info;

        info.id           = (*it).toInt();
        ++it;
        info.pid          = (*it).toInt();
        ++it;
        info.name         = (*it).toString();
        ++it;

        tagList << info;
    }

    return tagList;
}

int CoreDB::addAlbum(int albumRootId, const QString& relativePath,
                     const QString& caption,
                     const QDate& date, const QString& collection) const
{
    QVariant     id;
    QVariantList boundValues;

    boundValues << albumRootId << relativePath << date << caption << collection;

    d->db->execSql(QString::fromUtf8("REPLACE INTO Albums (albumRoot, relativePath, date, caption, collection) "
                                     "VALUES(?, ?, ?, ?, ?);"),
                   boundValues, nullptr, &id);

    d->db->recordChangeset(AlbumChangeset(id.toInt(), AlbumChangeset::Added));

    return id.toInt();
}

void CoreDB::setAlbumCaption(int albumID, const QString& caption)
{
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET caption=? WHERE id=?;"),
                   caption, albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::setAlbumCategory(int albumID, const QString& category)
{
    // TODO : change "collection" property in DB ALbum table to "category"

    d->db->execSql(QString::fromUtf8("UPDATE Albums SET collection=? WHERE id=?;"),
                   category, albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::setAlbumDate(int albumID, const QDate& date)
{
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET date=? WHERE id=?;"),
                   date, albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::setAlbumModificationDate(int albumID, const QDateTime& modificationDate)
{
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET modificationDate=? WHERE id=?;"),
                   d->db->asDBDateTime(modificationDate), albumID);
}

void CoreDB::setAlbumIcon(int albumID, qlonglong iconID)
{
    if (iconID == 0)
    {
        d->db->execSql(QString::fromUtf8("UPDATE Albums SET icon=NULL WHERE id=?;"),
                       albumID);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("UPDATE Albums SET icon=? WHERE id=?;"),
                       iconID, albumID);
    }

    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::deleteAlbum(int albumID)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumId"), albumID);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumID")),
                                                                            parameters))
    {
        return;
    }

    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
}

void CoreDB::makeStaleAlbum(int albumID)
{
    // We need to work around the table constraint, no we want to delete older stale albums with
    // the same relativePath, and adjust relativePaths depending on albumRoot.

    QVariantList values;

    // retrieve information

    d->db->execSql(QString::fromUtf8("SELECT albumRoot, relativePath FROM Albums WHERE id=?;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return;
    }

    // prepend albumRootId to relativePath. relativePath is unused and officially undefined after this call.

    QString newRelativePath = values.at(0).toString() + QLatin1Char('-') + values.at(1).toString();

    // delete older stale albums

    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumRoot"), 0);
    parameters.insert(QLatin1String(":relativePath"), newRelativePath);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumRootPath")),
                                                                            parameters))
    {
        return;
    }

    // now do our update

    d->db->setForeignKeyChecks(false);
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET albumRoot=0, relativePath=? WHERE id=?;"),
                   newRelativePath, albumID);

    // for now, we make no distinction to deleteAlbums wrt to changeset

    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
    d->db->setForeignKeyChecks(true);
}

void CoreDB::deleteStaleAlbums()
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumRoot"), 0);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumRoot")),
                                                                            parameters))
    {
        return;
    }

    // deliberately no changeset here, is done above
}

int CoreDB::addTag(int parentTagID, const QString& name, const QString& iconKDE, qlonglong iconID) const
{
    QVariant                id;
    QMap<QString, QVariant> parameters;

    parameters.insert(QLatin1String(":tagPID"), parentTagID);
    parameters.insert(QLatin1String(":tagname"), name);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("InsertTag")),
                                                                            parameters, nullptr , &id))
    {
        return -1;
    }

    // The creation of a tag can be ignored under MySQL if one with a different
    // case sensitivity already exists. In this case we use the already existing tag.

    if (!id.isValid())
    {
        QVariantList values;

        d->db->execSql(QString::fromUtf8("SELECT id FROM Tags WHERE name=?;"),
                       name, &values);

        if (values.isEmpty())
        {
            return -1;
        }

        id = values.first();
    }

    if      (!iconKDE.isEmpty())
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET iconkde=? WHERE id=?;"),
                       iconKDE, id.toInt());
    }
    else if (iconID == 0)
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET icon=NULL WHERE id=?;"),
                       id.toInt());
    }
    else
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET icon=? WHERE id=?;"),
                       iconID, id.toInt());
    }

    d->db->recordChangeset(TagChangeset(id.toInt(), TagChangeset::Added));

    return id.toInt();
}

void CoreDB::deleteTag(int tagID)
{
/*
    QString("DELETE FROM Tags WHERE id=?;"), tagID
*/

    QMap<QString, QVariant> bindingMap;
    bindingMap.insert(QLatin1String(":tagID"), tagID);

    d->db->execDBAction(d->db->getDBAction(QLatin1String("DeleteTag")), bindingMap);
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Deleted));
}

void CoreDB::setTagIcon(int tagID, const QString& iconKDE, qlonglong iconID)
{
    qlonglong dbIconID = iconKDE.isEmpty() ? iconID : 0;
    QString dbIconKDE  = iconKDE;

    if (
        iconKDE.isEmpty()                            ||
        (iconKDE.toLower() == QLatin1String("tag"))  ||
        (iconKDE.toLower() == QLatin1String("smiley"))
       )
    {
        dbIconKDE.clear();
    }

    if (dbIconID == 0)
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET iconkde=?, icon=NULL WHERE id=?;"),
                       dbIconKDE, tagID);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET iconkde=?, icon=? WHERE id=?;"),
                       dbIconKDE, dbIconID, tagID);
    }

    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::IconChanged));
}

void CoreDB::setTagParentID(int tagID, int newParentTagID)
{
    d->db->execSql(QString::fromUtf8("UPDATE Tags SET pid=? WHERE id=?;"),
                   newParentTagID, tagID);

    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Reparented));
}

QList<TagProperty> CoreDB::getTagProperties(int tagId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT property, value FROM TagProperties WHERE tagid=?;"),
                   tagId, &values);

    QList<TagProperty> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        TagProperty property;

        property.tagId    = tagId;

        property.property = (*it).toString();
        ++it;
        property.value    = (*it).toString();
        ++it;

        properties << property;
    }

    return properties;
}

QList<TagProperty> CoreDB::getTagProperties(const QString& property) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM TagProperties WHERE property=?;"),
                   property, &values);

    QList<TagProperty> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        TagProperty prop;

        prop.tagId    = (*it).toInt();
        ++it;
        prop.property = (*it).toString();
        ++it;
        prop.value    = (*it).toString();
        ++it;

        properties << prop;
    }

    return properties;
}

QList<TagProperty> CoreDB::getTagProperties() const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM TagProperties ORDER BY tagid, property;"),
                   &values);

    QList<TagProperty> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        TagProperty property;

        property.tagId    = (*it).toInt();
        ++it;
        property.property = (*it).toString();
        ++it;
        property.value    = (*it).toString();
        ++it;

        properties << property;
    }

    return properties;
}

QList<int> CoreDB::getTagsWithProperty(const QString& property) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT DISTINCT tagid FROM TagProperties WHERE property=?;"),
                   property, &values);

    QList<int> tagIds;

    for (const QVariant& var : std::as_const(values))
    {
        tagIds << var.toInt();
    }

    return tagIds;
}

void CoreDB::addTagProperty(int tagId, const QString& property, const QString& value)
{
    d->db->execSql(QString::fromUtf8("INSERT INTO TagProperties (tagid, property, value) VALUES(?, ?, ?);"),
                   tagId, property, value);

    d->db->recordChangeset(TagChangeset(tagId, TagChangeset::PropertiesChanged));
}

void CoreDB::addTagProperty(const TagProperty& property)
{
    addTagProperty(property.tagId, property.property, property.value);
}

void CoreDB::removeTagProperties(int tagId, const QString& property, const QString& value)
{
    if      (property.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM TagProperties WHERE tagid=?;"),
                       tagId);
    }
    else if (value.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM TagProperties WHERE tagid=? AND property=?;"),
                       tagId, property);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM TagProperties WHERE tagid=? AND property=? AND value=?;"),
                       tagId, property, value);
    }

    d->db->recordChangeset(TagChangeset(tagId, TagChangeset::PropertiesChanged));
}

int CoreDB::addSearch(DatabaseSearch::Type type, const QString& name, const QString& query) const
{
    QVariant id;

    if (!d->db->execSql(QString::fromUtf8("INSERT INTO Searches (type, name, query) VALUES(?, ?, ?);"),
                        type, name, query, nullptr, &id))
    {
        return -1;
    }

    d->db->recordChangeset(SearchChangeset(id.toInt(), SearchChangeset::Added));

    return id.toInt();
}

void CoreDB::updateSearch(int searchID, DatabaseSearch::Type type,
                          const QString& name, const QString& query)
{
    d->db->execSql(QString::fromUtf8("UPDATE Searches SET type=?, name=?, query=? WHERE id=?;"),
                   type, name, query, searchID);
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Changed));
}

void CoreDB::deleteSearch(int searchID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Searches WHERE id=?;"),
                   searchID);
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Deleted));
}

void CoreDB::deleteSearches(DatabaseSearch::Type type)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Searches WHERE type=?;"),
                   type);
    d->db->recordChangeset(SearchChangeset(0, SearchChangeset::Deleted));
}

QString CoreDB::getSearchQuery(int searchId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT query FROM Searches WHERE id=?;"),
                   searchId, &values);

    if (values.isEmpty())
    {
        return QString();
    }

    return values.first().toString();
}

SearchInfo CoreDB::getSearchInfo(int searchId) const
{
    SearchInfo   info;
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id, type, name, query FROM Searches WHERE id=?;"),
                   searchId, &values);

    if (values.size() == 4)
    {
        QList<QVariant>::const_iterator it = values.constBegin();
        info.id    = (*it).toInt();
        ++it;
        info.type  = (DatabaseSearch::Type)(*it).toInt();
        ++it;
        info.name  = (*it).toString();
        ++it;
        info.query = (*it).toString();
        ++it;
    }

    return info;
}

void CoreDB::setSetting(const QString& keyword, const QString& value)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO Settings VALUES (?,?);"),
                   keyword, value);
}

QString CoreDB::getSetting(const QString& keyword) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT value FROM Settings "
                                     "WHERE keyword=?;"),
                   keyword, &values);

    if (values.isEmpty())
    {
        return QString();
    }

    return values.first().toString();
}

/// helper method
static QStringList joinMainAndUserFilterString(const QChar& sep, const QString& filter,
                                               const QString& userFilter)
{
    QStringList filterList;
    QStringList userFilterList;

    filterList     = filter.split(sep, Qt::SkipEmptyParts);
    userFilterList = userFilter.split(sep, Qt::SkipEmptyParts);

    for (const QString& userFormat : std::as_const(userFilterList))
    {
        if (userFormat.startsWith(QLatin1Char('-')))
        {
            filterList.removeAll(userFormat.mid(1));
        }
        else
        {
            filterList << userFormat;
        }
    }

    filterList.removeDuplicates();
    filterList.sort();

    return filterList;
}

void CoreDB::getFilterSettings(QStringList* imageFilter, QStringList* videoFilter, QStringList* audioFilter)
{
    QString imageFormats, videoFormats, audioFormats, userImageFormats, userVideoFormats, userAudioFormats;

    if (imageFilter)
    {
        imageFormats     = getSetting(QLatin1String("databaseImageFormats"));
        userImageFormats = getSetting(QLatin1String("databaseUserImageFormats"));
        *imageFilter     = joinMainAndUserFilterString(QLatin1Char(';'), imageFormats, userImageFormats);
    }

    if (videoFilter)
    {
        videoFormats     = getSetting(QLatin1String("databaseVideoFormats"));
        userVideoFormats = getSetting(QLatin1String("databaseUserVideoFormats"));
        *videoFilter     = joinMainAndUserFilterString(QLatin1Char(';'), videoFormats, userVideoFormats);
    }

    if (audioFilter)
    {
        audioFormats     = getSetting(QLatin1String("databaseAudioFormats"));
        userAudioFormats = getSetting(QLatin1String("databaseUserAudioFormats"));
        *audioFilter     = joinMainAndUserFilterString(QLatin1Char(';'), audioFormats, userAudioFormats);
    }
}

void CoreDB::getUserFilterSettings(QString* imageFilterString, QString* videoFilterString, QString* audioFilterString)
{
    if (imageFilterString)
    {
        *imageFilterString = getSetting(QLatin1String("databaseUserImageFormats"));
    }

    if (videoFilterString)
    {
        *videoFilterString = getSetting(QLatin1String("databaseUserVideoFormats"));
    }

    if (audioFilterString)
    {
        *audioFilterString = getSetting(QLatin1String("databaseUserAudioFormats"));
    }
}

void CoreDB::getUserIgnoreDirectoryFilterSettings(QString* ignoreDirectoryFilterString)
{
    *ignoreDirectoryFilterString = getSetting(QLatin1String("databaseUserIgnoreDirectoryFormats"));
}

void CoreDB::getIgnoreDirectoryFilterSettings(QStringList* ignoreDirectoryFilter)
{
    QString ignoreDirectoryFormats, userIgnoreDirectoryFormats;

    ignoreDirectoryFormats     = getSetting(QLatin1String("databaseIgnoreDirectoryFormats"));
    userIgnoreDirectoryFormats = getSetting(QLatin1String("databaseUserIgnoreDirectoryFormats"));
    *ignoreDirectoryFilter     = joinMainAndUserFilterString(QLatin1Char(';'),
                                                             ignoreDirectoryFormats, userIgnoreDirectoryFormats);
}

void CoreDB::setFilterSettings(const QStringList& imageFilter, const QStringList& videoFilter, const QStringList& audioFilter)
{
    setSetting(QLatin1String("databaseImageFormats"), imageFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseVideoFormats"), videoFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseAudioFormats"), audioFilter.join(QLatin1Char(';')));
}

void CoreDB::setIgnoreDirectoryFilterSettings(const QStringList& ignoreDirectoryFilter)
{
    setSetting(QLatin1String("databaseIgnoreDirectoryFormats"), ignoreDirectoryFilter.join(QLatin1Char(';')));
}

void CoreDB::setUserFilterSettings(const QStringList& imageFilter,
                                   const QStringList& videoFilter,
                                   const QStringList& audioFilter)
{
    setSetting(QLatin1String("databaseUserImageFormats"), imageFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseUserVideoFormats"), videoFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseUserAudioFormats"), audioFilter.join(QLatin1Char(';')));
}

void CoreDB::setUserIgnoreDirectoryFilterSettings(const QStringList& ignoreDirectoryFilters)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "CoreDB::setUserIgnoreDirectoryFilterSettings. "
                                     "ignoreDirectoryFilterString: "
                                  << ignoreDirectoryFilters.join(QLatin1Char(';'));

    setSetting(QLatin1String("databaseUserIgnoreDirectoryFormats"), ignoreDirectoryFilters.join(QLatin1Char(';')));
}

QUuid CoreDB::databaseUuid()
{
    QString uuidString = getSetting(QLatin1String("databaseUUID"));
    QUuid uuid         = QUuid(uuidString);

    if (uuidString.isNull() || uuid.isNull())
    {
        uuid = QUuid::createUuid();
        setSetting(QLatin1String("databaseUUID"), uuid.toString());
    }

    return uuid;
}

QString CoreDB::getDatabaseEncoding() const
{
    QVariantList values;

    d->db->execDBAction(d->db->getDBAction(QLatin1String("getDatabaseEncoding")), &values);

    if (values.isEmpty())
    {
        return QString();
    }

    return (values.first().toString().toUpper());
}

int CoreDB::getUniqueHashVersion() const
{
    if (d->uniqueHashVersion == -1)
    {
        QString v = getSetting(QLatin1String("uniqueHashVersion"));

        if (v.isEmpty())
        {
            d->uniqueHashVersion = 1;
        }
        else
        {
            d->uniqueHashVersion = v.toInt();
        }
    }

    return d->uniqueHashVersion;
}

void CoreDB::setUniqueHashVersion(int version)
{
    d->uniqueHashVersion = version;
    setSetting(QLatin1String("uniqueHashVersion"),
               QString::number(d->uniqueHashVersion));
}

qlonglong CoreDB::getImageId(int albumID, const QString& name) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                     "WHERE album=? AND name=?;"),
                   albumID, name, &values);

    if (values.isEmpty())
    {
        return -1;
    }

    return values.first().toLongLong();
}

QList<qlonglong> CoreDB::getImageIds(int albumID, const QString& name, DatabaseItem::Status status) const
{
    QVariantList values;

    if (albumID == -1)
    {
        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE album IS NULL AND name=? AND status=?;"),
                       name, status, &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE album=? AND name=? AND status=?;"),
                       albumID, name, status, &values);
    }

    QList<qlonglong> items;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        items << it->toLongLong();
    }

    return items;
}

QList<qlonglong> CoreDB::getImageIds(int albumID, DatabaseItem::Status status, bool scanned) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Tags "
                                        "WHERE name=?"),
                    InternalTagName::scannedForFaces(),
                    &values);

    if (!scanned && (0 < values.count()))
    {

        int scannedTagId = values[0].toInt();

        d->db->execSql(QString::fromUtf8("SELECT DISTINCT id FROM Images "
                                         "LEFT OUTER JOIN ImageTags on ImageTags.imageid = Images.id AND ImageTags.tagid = ? "
                                         "WHERE Images.album=? "
                                         "AND Images.status=? "
                                         "AND ImageTags.imageid IS NULL;"),
                       scannedTagId, albumID, status, &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT DISTINCT id "
                                         "FROM Images "
                                         "WHERE Images.album=? "
                                         "AND Images.status=?;"),
                       albumID, status, &values);
    }

    QList<qlonglong> items;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        items << it->toLongLong();
    }

    return items;
}

QList<qlonglong> CoreDB::getImageIds(DatabaseItem::Status status) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                     "WHERE status=?;"),
                   status, &values);

    QList<qlonglong> imageIds;

    for (const QVariant& object : std::as_const(values))
    {
        imageIds << object.toLongLong();
    }

    return imageIds;
}

QList<qlonglong> CoreDB::getImageIds(DatabaseItem::Status status, DatabaseItem::Category category) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                     "WHERE status=? AND category=?;"),
                   status, category, &values);

    QList<qlonglong> imageIds;

    for (const QVariant& object : std::as_const(values))
    {
        imageIds << object.toLongLong();
    }

    return imageIds;
}

qlonglong CoreDB::findImageId(int albumID, const QString& name,
                              DatabaseItem::Status status,
                              DatabaseItem::Category category,
                              qlonglong fileSize,
                              const QString& uniqueHash) const
{
    QVariantList values;
    QVariantList boundValues;

    // Add the standard bindings

    boundValues << name << (int)status << (int)category
                << fileSize << uniqueHash;

    // If the album id is -1, no album is assigned. Get all images with NULL album

    if (albumID == -1)
    {
        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE name=? AND status=? "
                                         "AND category=? AND fileSize=? "
                                         "AND uniqueHash=? AND album IS NULL;"),
                       boundValues, &values);
    }
    else
    {
        boundValues << albumID;

        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE name=? AND status=? "
                                         "AND category=? AND fileSize=? "
                                         "AND uniqueHash=? AND album=?;"),
                       boundValues, &values);
    }

    // If there are several identical image ids, we do not use
    // any of them, as correct assignment is not possible.

    if (values.size() != 1)
    {
        return -1;
    }

    return values.first().toLongLong();
}

QStringList CoreDB::getItemTagNames(qlonglong imageID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT name FROM Tags "
                                     "WHERE id IN (SELECT tagid FROM ImageTags "
                                     " WHERE imageid=?) "
                                     "  ORDER BY name;"),
                   imageID, &values);

    QStringList names;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        names << it->toString();
    }

    return names;
}

QList<int> CoreDB::getItemTagIDs(qlonglong imageID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT tagid FROM ImageTags WHERE imageID=?;"),
                   imageID, &values);

    QList<int> ids;

    if (values.isEmpty())
    {
        return ids;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        ids << it->toInt();
    }

    return ids;
}

QVector<QList<int> > CoreDB::getItemsTagIDs(const QList<qlonglong>& imageIds) const
{
    if (imageIds.isEmpty())
    {
        return QVector<QList<int> >();
    }

    QSqlQuery query = d->db->prepareQuery(QString::fromUtf8("SELECT tagid FROM ImageTags WHERE imageID=?;"));
    QVector<QList<int> > results(imageIds.size());
    QVariantList values;

    for (int i = 0 ; i < imageIds.size() ; ++i)
    {
        d->db->execSql(query, imageIds[i], &values);
        QList<int>& tagIds = results[i];

        for (const QVariant& v : std::as_const(values))
        {
            tagIds << v.toInt();
        }
    }

    return results;
}

QList<ImageTagProperty> CoreDB::getImageTagProperties(qlonglong imageId, int tagId) const
{
    QVariantList values;

    if (tagId == -1)
    {
        d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM ImageTagProperties "
                                         "WHERE imageid=?;"),
                       imageId, &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM ImageTagProperties "
                                         "WHERE imageid=? AND tagid=?;"),
                       imageId, tagId, &values);
    }

    QList<ImageTagProperty> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        ImageTagProperty property;

        property.imageId  = imageId;

        property.tagId    = (*it).toInt();
        ++it;
        property.property = (*it).toString();
        ++it;
        property.value    = (*it).toString();
        ++it;

        properties << property;
    }

    return properties;
}

QList<int> CoreDB::getTagIdsWithProperties(qlonglong imageId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT DISTINCT tagid FROM ImageTagProperties WHERE imageid=?;"),
                   imageId, &values);

    QList<int> tagIds;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        tagIds << (*it).toInt();
    }

    return tagIds;
}

void CoreDB::addImageTagProperty(qlonglong imageId, int tagId, const QString& property, const QString& value)
{
    d->db->execSql(QString::fromUtf8("INSERT INTO ImageTagProperties (imageid, tagid, property, value) "
                                     "VALUES(?, ?, ?, ?);"),
                   imageId, tagId, property, value);

    d->db->recordChangeset(ImageTagChangeset(imageId, tagId, ImageTagChangeset::PropertiesChanged));
}

void CoreDB::addImageTagProperty(const ImageTagProperty& property)
{
    addImageTagProperty(property.imageId, property.tagId, property.property, property.value);
}

void CoreDB::removeImageTagProperties(qlonglong imageId, int tagId, const QString& property, const QString& value)
{
    if      (tagId == -1)
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties "
                                         "WHERE imageid=?;"),
                       imageId);
    }
    else if (property.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties "
                                         "WHERE imageid=? AND tagid=?;"),
                       imageId, tagId);
    }
    else if (value.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties "
                                         "WHERE imageid=? AND tagid=? AND property=?;"),
                       imageId, tagId, property);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties "
                                         "WHERE imageid=? AND tagid=? AND property=? AND value=?;"),
                       imageId, tagId, property, value);
    }

    d->db->recordChangeset(ImageTagChangeset(imageId, tagId, ImageTagChangeset::PropertiesChanged));
}

ItemShortInfo CoreDB::getItemShortInfo(qlonglong imageID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT Images.name, Albums.albumRoot, Albums.relativePath, Albums.id "
                                     "FROM Images "
                                     " INNER JOIN Albums ON Albums.id=Images.album "
                                     "  WHERE Images.id=?;"),
                   imageID, &values);

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id          = imageID;
        info.itemName    = values.at(0).toString();
        info.albumRootID = values.at(1).toInt();
        info.album       = values.at(2).toString();
        info.albumID     = values.at(3).toInt();
    }

    return info;
}

ItemShortInfo CoreDB::getItemShortInfo(int albumRootId, const QString& relativePath, const QString& name) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT Images.id, Albums.id FROM Images "
                                     "INNER JOIN Albums ON Albums.id=Images.album "
                                     " WHERE name=? AND albumRoot=? AND relativePath=?;"),
                   name, albumRootId, relativePath, &values);

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id          = values.at(0).toLongLong();
        info.itemName    = name;
        info.albumRootID = albumRootId;
        info.album       = relativePath;
        info.albumID     = values.at(1).toInt();
    }

    return info;
}

bool CoreDB::hasTags(const QList<qlonglong>& imageIDList) const
{
    if (imageIDList.isEmpty())
    {
        return false;
    }

    QVariantList values;
    QVariantList boundValues;

    QString sql = QString::fromUtf8("SELECT COUNT(tagid) FROM ImageTags "
                                    "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.constBegin();
    ++it;

    for ( ; it != imageIDList.constEnd() ; ++it)
    {
        sql += QString::fromUtf8(" OR imageid=? ");
        boundValues << (*it);
    }

    sql += QString::fromUtf8(";");
    d->db->execSql(sql, boundValues, &values);

    if (values.isEmpty() || (values.first().toInt() == 0))
    {
        return false;
    }

    return true;
}

QList<int> CoreDB::getItemCommonTagIDs(const QList<qlonglong>& imageIDList) const
{
    QList<int> ids;

    if (imageIDList.isEmpty())
    {
        return ids;
    }

    QVariantList values;
    QVariantList boundValues;

    QString sql = QString::fromUtf8("SELECT DISTINCT tagid FROM ImageTags "
                                    "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.constBegin();
    ++it;

    for ( ; it != imageIDList.constEnd() ; ++it)
    {
        sql += QString::fromUtf8(" OR imageid=? ");
        boundValues << (*it);
    }

    sql += QString::fromUtf8(";");
    d->db->execSql(sql, boundValues, &values);

    if (values.isEmpty())
    {
        return ids;
    }

    for (QList<QVariant>::const_iterator it2 = values.constBegin() ; it2 != values.constEnd() ; ++it2)
    {
        ids << it2->toInt();
    }

    return ids;
}

QVariantList CoreDB::getImagesFields(qlonglong imageID, DatabaseFields::Images fields) const
{
    QVariantList values;

    if (fields != DatabaseFields::ImagesNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = imagesFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM Images WHERE id=?;");

        d->db->execSql(query, imageID, &values);

        if (fieldNames.size() != values.size())
        {
            return QVariantList();
        }

        // Convert date times to QDateTime, they come as QString

        if ((fields & DatabaseFields::ModificationDate))
        {
            int index          = fieldNames.indexOf(QLatin1String("modificationDate"));
            QDateTime dateTime = asDateTimeUTC(values.at(index).toDateTime());
            values[index]      = QVariant(dateTime);
        }
    }

    return values;
}

QVariantList CoreDB::getItemInformation(qlonglong imageID, DatabaseFields::ItemInformation fields) const
{
    QVariantList values;

    if (fields != DatabaseFields::ItemInformationNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = imageInformationFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM ImageInformation WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        if (fieldNames.size() != values.size())
        {
            return QVariantList();
        }

        // Convert date times to QDateTime, they come as QString

        if ((fields & DatabaseFields::CreationDate))
        {
            int index          = fieldNames.indexOf(QLatin1String("creationDate"));
            QDateTime dateTime = asDateTimeUTC(values.at(index).toDateTime());
            values[index]      = QVariant(dateTime);
        }

        if ((fields & DatabaseFields::DigitizationDate))
        {
            int index          = fieldNames.indexOf(QLatin1String("digitizationDate"));
            QDateTime dateTime = asDateTimeUTC(values.at(index).toDateTime());
            values[index]      = QVariant(dateTime);
        }
    }

    return values;
}

QVariantList CoreDB::getImageMetadata(qlonglong imageID, DatabaseFields::ImageMetadata fields) const
{
    QVariantList values;

    if (fields != DatabaseFields::ImageMetadataNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = imageMetadataFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM ImageMetadata WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);
    }

    return values;
}

QVariantList CoreDB::getVideoMetadata(qlonglong imageID, DatabaseFields::VideoMetadata fields) const
{
    QVariantList values;

    if (fields != DatabaseFields::VideoMetadataNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = videoMetadataFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM VideoMetadata WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);
    }

    return values;
}

QVariantList CoreDB::getItemPosition(qlonglong imageID, DatabaseFields::ItemPositions fields) const
{
    QVariantList values;

    if (fields != DatabaseFields::ItemPositionsNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames =  imagePositionsFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM ImagePositions WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // For some reason REAL values may come as QString QVariants. Convert here.

        if (values.size() == fieldNames.size() &&
            (
             (fields & DatabaseFields::LatitudeNumber)      ||
             (fields & DatabaseFields::LongitudeNumber)     ||
             (fields & DatabaseFields::Altitude)            ||
             (fields & DatabaseFields::PositionOrientation) ||
             (fields & DatabaseFields::PositionTilt)        ||
             (fields & DatabaseFields::PositionRoll)        ||
             (fields & DatabaseFields::PositionAccuracy)
            )
           )
        {
            for (int i = 0 ; i < values.size() ; ++i)
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                if ((values.at(i).typeId() == QMetaType::QString) &&

#else

                if ((values.at(i).type() == QVariant::String) &&

#endif

                    (
                     (fieldNames.at(i) == QLatin1String("latitudeNumber"))  ||
                     (fieldNames.at(i) == QLatin1String("longitudeNumber")) ||
                     (fieldNames.at(i) == QLatin1String("altitude"))        ||
                     (fieldNames.at(i) == QLatin1String("orientation"))     ||
                     (fieldNames.at(i) == QLatin1String("tilt"))            ||
                     (fieldNames.at(i) == QLatin1String("roll"))            ||
                     (fieldNames.at(i) == QLatin1String("accuracy"))
                    )
                   )
                {
                    if (!values.at(i).isNull())
                    {
                        values[i] = values.at(i).toDouble();
                    }
                }
            }
        }
    }

    return values;
}

QVariantList CoreDB::getItemPositions(const QList<qlonglong>& imageIDs, DatabaseFields::ItemPositions fields) const
{
    QVariantList values;

    if (fields != DatabaseFields::ItemPositionsNone)
    {
        QString sql(QString::fromUtf8("SELECT "));
        QStringList fieldNames =  imagePositionsFieldList(fields);
        sql                   += fieldNames.join(QString::fromUtf8(", "));
        sql                   += QString::fromUtf8(" FROM ImagePositions WHERE imageid=?;");

        QSqlQuery query        = d->db->prepareQuery(sql);

        for (const qlonglong& imageid : std::as_const(imageIDs))
        {
            QVariantList singleValueList;
            d->db->execSql(query, imageid, &singleValueList);
            values << singleValueList;
        }

        // For some reason REAL values may come as QString QVariants. Convert here.

        if (
            (values.size() == fieldNames.size()) &&
            (
             (fields & DatabaseFields::LatitudeNumber)      ||
             (fields & DatabaseFields::LongitudeNumber)     ||
             (fields & DatabaseFields::Altitude)            ||
             (fields & DatabaseFields::PositionOrientation) ||
             (fields & DatabaseFields::PositionTilt)        ||
             (fields & DatabaseFields::PositionRoll)        ||
             (fields & DatabaseFields::PositionAccuracy)
            )
           )
        {
            for (int i = 0 ; i < values.size() ; ++i)
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                if ((values.at(i).typeId() == QMetaType::QString) &&

#else

                if ((values.at(i).type() == QVariant::String) &&

#endif

                    (
                     (fieldNames.at(i) == QLatin1String("latitudeNumber"))  ||
                     (fieldNames.at(i) == QLatin1String("longitudeNumber")) ||
                     (fieldNames.at(i) == QLatin1String("altitude"))        ||
                     (fieldNames.at(i) == QLatin1String("orientation"))     ||
                     (fieldNames.at(i) == QLatin1String("tilt"))            ||
                     (fieldNames.at(i) == QLatin1String("roll"))            ||
                     (fieldNames.at(i) == QLatin1String("accuracy"))
                    )
                   )
                {
                    if (!values.at(i).isNull())
                    {
                        values[i] = values.at(i).toDouble();
                    }
                }
            }
        }
    }

    return values;
}

void CoreDB::addItemInformation(qlonglong imageID, const QVariantList& infos,
                                DatabaseFields::ItemInformation fields)
{
    if (fields == DatabaseFields::ItemInformationNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO ImageInformation ( imageid, "));

    QStringList fieldNames = imageInformationFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

    QVariantList boundValues;
    boundValues << imageID;
    boundValues << infos;

    if ((fields & DatabaseFields::CreationDate))
    {
        // We have the imageID added to the list, therefore index + 1

        int index          = fieldNames.indexOf(QLatin1String("creationDate")) + 1;
        QDateTime dateTime = d->db->asDBDateTime(boundValues.at(index).toDateTime());
        boundValues[index] = QVariant(dateTime);
    }

    if ((fields & DatabaseFields::DigitizationDate))
    {
        // We have the imageID added to the list, therefore index + 1

        int index          = fieldNames.indexOf(QLatin1String("digitizationDate")) + 1;
        QDateTime dateTime = d->db->asDBDateTime(boundValues.at(index).toDateTime());
        boundValues[index] = QVariant(dateTime);
    }

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(fields)));
}

void CoreDB::changeItemInformation(qlonglong imageId, const QVariantList& infos,
                                   DatabaseFields::ItemInformation fields)
{
    if (fields == DatabaseFields::ItemInformationNone)
    {
        return;
    }

    QStringList  fieldNames = imageInformationFieldList(fields);
    QVariantList boundValues;
    boundValues << infos;

    if ((fields & DatabaseFields::CreationDate))
    {
        int index          = fieldNames.indexOf(QLatin1String("creationDate"));
        QDateTime dateTime = d->db->asDBDateTime(boundValues.at(index).toDateTime());
        boundValues[index] = QVariant(dateTime);
    }

    if ((fields & DatabaseFields::DigitizationDate))
    {
        int index          = fieldNames.indexOf(QLatin1String("digitizationDate"));
        QDateTime dateTime = d->db->asDBDateTime(boundValues.at(index).toDateTime());
        boundValues[index] = QVariant(dateTime);
    }

    d->db->execUpsertDBAction(QLatin1String("changeItemInformation"),
                              imageId, fieldNames, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::Set(fields)));
}

void CoreDB::addImageMetadata(qlonglong imageID, const QVariantList& infos,
                              DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO ImageMetadata ( imageid, "));
    QStringList fieldNames = imageMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(fields)));
}

void CoreDB::changeImageMetadata(qlonglong imageId, const QVariantList& infos,
                                 DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE ImageMetadata SET "));

    QStringList fieldNames = imageMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE imageid=?;");

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::Set(fields)));
}

void CoreDB::addVideoMetadata(qlonglong imageID, const QVariantList& infos, DatabaseFields::VideoMetadata fields)
{
    if (fields == DatabaseFields::VideoMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO VideoMetadata ( imageid, ")); // need to create this database
    QStringList fieldNames = videoMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(fields)));
}

void CoreDB::changeVideoMetadata(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::VideoMetadata fields)
{
    if (fields == DatabaseFields::VideoMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE VideoMetadata SET "));
    QStringList fieldNames = videoMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE imageid=?;");

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::Set(fields)));
}

void CoreDB::addItemPosition(qlonglong imageID, const QVariantList& infos, DatabaseFields::ItemPositions fields)
{
    if (fields == DatabaseFields::ItemPositionsNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO ImagePositions ( imageid, "));
    QStringList fieldNames = imagePositionsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(fields)));
}

void CoreDB::changeItemPosition(qlonglong imageId, const QVariantList& infos,
                                DatabaseFields::ItemPositions fields)
{
    if (fields == DatabaseFields::ItemPositionsNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE ImagePositions SET "));
    QStringList fieldNames = imagePositionsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE imageid=?;");

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::Set(fields)));
}

void CoreDB::removeItemPosition(qlonglong imageid)
{
    d->db->execSql(QString(QString::fromUtf8("DELETE FROM ImagePositions WHERE imageid=?;")),
                   imageid);

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::Set(DatabaseFields::ItemPositionsAll)));
}

void CoreDB::removeItemPositionAltitude(qlonglong imageid)
{
    d->db->execSql(QString(QString::fromUtf8("UPDATE ImagePositions SET altitude=NULL WHERE imageid=?;")),
                   imageid);

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::Set(DatabaseFields::Altitude)));
}

QList<CommentInfo> CoreDB::getItemComments(qlonglong imageID) const
{
    QList<CommentInfo> list;
    QVariantList       values;

    d->db->execSql(QString::fromUtf8("SELECT id, type, language, author, date, comment "
                                     "FROM ImageComments WHERE imageid=?;"),
                   imageID, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        CommentInfo info;

        info.imageId  = imageID;

        info.id       = (*it).toInt();
        ++it;
        info.type     = (DatabaseComment::Type)(*it).toInt();
        ++it;
        info.language = (*it).toString();
        ++it;
        info.author   = (*it).toString();
        ++it;
        info.date     = asDateTimeUTC((*it).toDateTime());
        ++it;
        info.comment  = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

int CoreDB::setImageComment(qlonglong imageID, const QString& comment, DatabaseComment::Type type,
                            const QString& language, const QString& author, const QDateTime& date) const
{
    QVariantList boundValues;
    boundValues << imageID << (int)type << language << author << d->db->asDBDateTime(date) << comment;

    QVariant id;
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageComments "
                           "( imageid, type, language, author, date, comment ) "
                           " VALUES (?,?,?,?,?,?);"),
                   boundValues, nullptr, &id);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::ItemCommentsAll)));

    return id.toInt();
}

void CoreDB::changeImageComment(int commentId, qlonglong imageID, const QVariantList& infos, DatabaseFields::ItemComments fields)
{
    if (fields == DatabaseFields::ItemCommentsNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE ImageComments SET "));
    QStringList fieldNames = imageCommentsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE id=?;");

    QVariantList boundValues;
    boundValues << infos << commentId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(fields)));
}

void CoreDB::removeImageComment(int commentid, qlonglong imageID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageComments WHERE id=?;"),
                   commentid);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::ItemCommentsAll)));
}

void CoreDB::removeAllImageComments(qlonglong imageID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageComments WHERE imageid=?;"),
                   imageID);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::ItemCommentsAll)));
}

QString CoreDB::getImageProperty(qlonglong imageID, const QString& property) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT value FROM ImageProperties "
                                     "WHERE imageid=? AND property=?;"),
                   imageID, property, &values);

    if (values.isEmpty())
    {
        return QString();
    }

    return values.first().toString();
}

void CoreDB::setImageProperty(qlonglong imageID, const QString& property, const QString& value)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageProperties "
                                     "(imageid, property, value) "
                                     " VALUES(?, ?, ?);"),
                   imageID, property, value);
}

void CoreDB::removeImageProperty(qlonglong imageID, const QString& property)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageProperties WHERE imageid=? AND property=?;"),
                   imageID, property);
}

void CoreDB::removeImagePropertyByName(const QString& property)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageProperties WHERE property=?;"),
                   property);
}

void CoreDB::removeAllImageProperties(qlonglong imageID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageProperties WHERE imageid=?;"),
                   imageID);
}

QStringList CoreDB::getAllImagePropertiesByName(const QString& property) const
{
    QVariantList values;
    QStringList  imageProperties;

    d->db->execSql(QString::fromUtf8("SELECT DISTINCT value FROM ImageProperties "
                                     "LEFT JOIN Images ON Images.id=ImageProperties.imageid "
                                     " WHERE Images.status=1 AND property=?;"),
                   property, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        QString str((*it).toString());

        if (!str.isEmpty())
        {
            imageProperties << str;
        }
    }

    return imageProperties;
}

QStringList CoreDB::getAllImageCopyrightValues(const QString& property) const
{
    QVariantList values;
    QStringList  out;

    d->db->execSql(QString::fromUtf8(
        "SELECT DISTINCT value FROM ImageCopyright "
        "LEFT JOIN Images ON Images.id=ImageCopyright.imageid "
        "WHERE Images.status=1 AND ImageCopyright.property=?;"
    ),
    property, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        const QString str = (*it).toString();

        if (!str.isEmpty())
        {
            out << str;
        }
    }

    out.sort();
    return out;
}

QStringList CoreDB::getAllCommentAuthors(int type) const
{
    QVariantList values;
    QStringList  out;

    d->db->execSql(QString::fromUtf8(
        "SELECT DISTINCT author FROM ImageComments "
        "LEFT JOIN Images ON Images.id=ImageComments.imageid "
        "WHERE Images.status=1 AND ImageComments.type=?;"
    ),
    type, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        const QString str = (*it).toString();

        if (!str.isEmpty())
        {
            out << str;
        }
    }

    out.sort();
    return out;
}



QList<CopyrightInfo> CoreDB::getItemCopyright(qlonglong imageID, const QString& property) const
{
    QList<CopyrightInfo> list;
    QVariantList         values;

    if (property.isNull())
    {
        d->db->execSql(QString::fromUtf8("SELECT property, value, extraValue FROM ImageCopyright "
                                         "WHERE imageid=?;"),
                       imageID, &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT property, value, extraValue FROM ImageCopyright "
                                         "WHERE imageid=? AND property=?;"),
                       imageID, property, &values);
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        CopyrightInfo info;

        info.id         = imageID;

        info.property   = (*it).toString();
        ++it;
        info.value      = (*it).toString();
        ++it;
        info.extraValue = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

void CoreDB::setItemCopyrightProperty(qlonglong imageID, const QString& property,
                                      const QString& value, const QString& extraValue,
                                      CopyrightPropertyUnique uniqueness)
{
    if      (uniqueness == PropertyUnique)
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                         "WHERE imageid=? AND property=?;"),
                       imageID, property);
    }
    else if (uniqueness == PropertyExtraValueUnique)
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                         "WHERE imageid=? AND property=? AND extraValue=?;"),
                       imageID, property, extraValue);
    }

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageCopyright "
                                     "(imageid, property, value, extraValue) "
                                     " VALUES(?, ?, ?, ?);"),
                   imageID, property, value, extraValue);
}

void CoreDB::removeItemCopyrightProperties(qlonglong imageID, const QString& property,
                                           const QString& extraValue, const QString& value)
{
    int removeBy = 0;

    if (!property.isNull())
    {
        ++removeBy;
    }

    if (!extraValue.isNull())
    {
        ++removeBy;
    }

    if (!value.isNull())
    {
        ++removeBy;
    }

    switch (removeBy)
    {
        case 0:
        {
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                             "WHERE imageid=?;"),
                           imageID);
            break;
        }

        case 1:
        {
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                             "WHERE imageid=? AND property=?;"),
                           imageID, property);
            break;
        }

        case 2:
        {
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                             "WHERE imageid=? AND property=? AND extraValue=?;"),
                           imageID, property, extraValue);
            break;
        }

        case 3:
        {
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                             "WHERE imageid=? AND property=? AND extraValue=? AND value=?;"),
                           imageID, property, extraValue, value);
            break;
        }
    }
}

void CoreDB::removeAllItemCopyrightProperties(qlonglong imageID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright WHERE imageid=?;"),
                   imageID);
}

QList<qlonglong> CoreDB::findByNameAndCreationDate(const QString& fileName, const QDateTime& creationDate) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                     "LEFT JOIN ImageInformation ON id=imageid "
                                     " WHERE name=? AND creationDate=? AND status<3;"),
                   fileName, d->db->asDBDateTime(creationDate), &values);

    QList<qlonglong> ids;

    for (const QVariant& var : std::as_const(values))
    {
        ids << var.toLongLong();
    }

    return ids;
}

bool CoreDB::hasImageHistory(qlonglong imageId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT history FROM ImageHistory WHERE imageid=?;"),
                   imageId, &values);

    return !values.isEmpty();
}

ImageHistoryEntry CoreDB::getItemHistory(qlonglong imageId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT uuid, history FROM ImageHistory WHERE imageid=?;"),
                   imageId, &values);

    ImageHistoryEntry entry;
    entry.imageId = imageId;

    if (values.count() != 2)
    {
        return entry;
    }

    QList<QVariant>::const_iterator it = values.constBegin();

    entry.uuid    = (*it).toString();
    ++it;
    entry.history = (*it).toString();
    ++it;

    return entry;
}

QList<qlonglong> CoreDB::getItemsForUuid(const QString& uuid) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT imageid FROM ImageHistory "
                                     "INNER JOIN Images ON imageid=id "
                                     " WHERE uuid=? AND status<3;"),
                   uuid, &values);

    QList<qlonglong> imageIds;

    if (values.isEmpty())
    {
        return imageIds;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        imageIds << (*it).toInt();
    }

    return imageIds;
}

QString CoreDB::getImageUuid(qlonglong imageId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT uuid FROM ImageHistory WHERE imageid=?;"),
                   imageId, &values);

    if (values.isEmpty())
    {
        return QString();
    }

    QString uuid = values.first().toString();

    if (uuid.isEmpty())
    {
        return QString();
    }

    return uuid;
}

void CoreDB::setItemHistory(qlonglong imageId, const QString& history)
{
    d->db->execUpsertDBAction(QLatin1String("changeImageHistory"),
                              imageId, QStringList() << QLatin1String("history"), QVariantList() << history);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::Set(DatabaseFields::ImageHistory)));
}

void CoreDB::setImageUuid(qlonglong imageId, const QString& uuid)
{
    d->db->execUpsertDBAction(QLatin1String("changeImageHistory"),
                              imageId, QStringList() << QLatin1String("uuid"), QVariantList() << uuid);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::Set(DatabaseFields::ImageUUID)));
}

void CoreDB::addImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageRelations (subject, object, type) "
                                     "VALUES (?, ?, ?);"),
                   subjectId, objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << subjectId << objectId,
                                          DatabaseFields::Set(DatabaseFields::ImageRelations)));
}

void CoreDB::addImageRelations(const QList<qlonglong>& subjectIds,
                               const QList<qlonglong>& objectIds, DatabaseRelation::Type type)
{
    QSqlQuery query = d->db->prepareQuery(QString::fromUtf8("REPLACE INTO ImageRelations (subject, object, type) "
                                                            "VALUES (?, ?, ?);"));

    QVariantList subjects, objects, types;

    for (int i = 0 ; i < subjectIds.size() ; ++i)
    {
        subjects << subjectIds.at(i);
        objects  << objectIds.at(i);
        types    << type;
    }

    query.addBindValue(subjects);
    query.addBindValue(objects);
    query.addBindValue(types);
    d->db->execBatch(query);
    d->db->recordChangeset(ImageChangeset(subjectIds + objectIds,
                                          DatabaseFields::Set(DatabaseFields::ImageRelations)));
}


void CoreDB::addImageRelation(const ImageRelation& relation)
{
    addImageRelation(relation.subjectId, relation.objectId, relation.type);
}

void CoreDB::removeImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations WHERE subject=? AND object=? AND type=?;"),
                   subjectId, objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << subjectId << objectId,
                                          DatabaseFields::Set(DatabaseFields::ImageRelations)));
}

void CoreDB::removeImageRelation(const ImageRelation& relation)
{
    removeImageRelation(relation.subjectId, relation.objectId, relation.type);
}

QList<qlonglong> CoreDB::removeAllImageRelationsTo(qlonglong objectId, DatabaseRelation::Type type) const
{
    QList<qlonglong> affected = getImagesRelatingTo(objectId, type);

    if (affected.isEmpty())
    {
        return affected;
    }

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations WHERE object=? AND type=?;"),
                   objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << affected << objectId,
                                          DatabaseFields::Set(DatabaseFields::ImageRelations)));

    return affected;
}

QList<qlonglong> CoreDB::removeAllImageRelationsFrom(qlonglong subjectId, DatabaseRelation::Type type) const
{
    QList<qlonglong> affected = getImagesRelatedFrom(subjectId, type);

    if (affected.isEmpty())
    {
        return affected;
    }

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations WHERE subject=? AND type=?;"),
                   subjectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << affected << subjectId,
                                          DatabaseFields::Set(DatabaseFields::ImageRelations)));

    return affected;
}

QList<qlonglong> CoreDB::getImagesRelatedFrom(qlonglong subjectId, DatabaseRelation::Type type) const
{
    return getRelatedImages(subjectId, true, type, false);
}

QVector<QList<qlonglong> > CoreDB::getImagesRelatedFrom(const QList<qlonglong>& subjectIds, DatabaseRelation::Type type) const
{
    return getRelatedImages(subjectIds, true, type, false);
}

bool CoreDB::hasImagesRelatedFrom(qlonglong subjectId, DatabaseRelation::Type type) const
{
    // returns 0 or 1 item in list

    return !getRelatedImages(subjectId, true, type, true).isEmpty();
}

QList<qlonglong> CoreDB::getImagesRelatingTo(qlonglong objectId, DatabaseRelation::Type type) const
{
    return getRelatedImages(objectId, false, type, false);
}

QVector<QList<qlonglong> > CoreDB::getImagesRelatingTo(const QList<qlonglong>& objectIds, DatabaseRelation::Type type) const
{
    return getRelatedImages(objectIds, false, type, false);
}

bool CoreDB::hasImagesRelatingTo(qlonglong objectId, DatabaseRelation::Type type) const
{
    // returns 0 or 1 item in list

    return !getRelatedImages(objectId, false, type, true).isEmpty();
}

QList<qlonglong> CoreDB::getRelatedImages(qlonglong id, bool fromOrTo, DatabaseRelation::Type type, bool boolean) const
{
    QString sql     = d->constructRelatedImagesSQL(fromOrTo, type, boolean);
    QSqlQuery query = d->db->prepareQuery(sql);

    return d->execRelatedImagesQuery(query, id, type);
}

QVector<QList<qlonglong> > CoreDB::getRelatedImages(QList<qlonglong> ids,
                                                    bool fromOrTo, DatabaseRelation::Type type, bool boolean) const
{
    if (ids.isEmpty())
    {
        return QVector<QList<qlonglong> >();
    }

    QVector<QList<qlonglong> > result(ids.size());

    QString sql     = d->constructRelatedImagesSQL(fromOrTo, type, boolean);
    QSqlQuery query = d->db->prepareQuery(sql);

    for (int i = 0 ; i < ids.size() ; ++i)
    {
        result[i] = d->execRelatedImagesQuery(query, ids[i], type);
    }

    return result;
}

QList<QPair<qlonglong, qlonglong> > CoreDB::getRelationCloud(qlonglong imageId, DatabaseRelation::Type type) const
{
    QSet<qlonglong> todo, done;
    QSet<QPair<qlonglong, qlonglong> > pairs;
    todo << imageId;

    QString sql = QString::fromUtf8("SELECT subject, object FROM ImageRelations "
                                    "INNER JOIN Images AS SubjectImages "
                                    "ON ImageRelations.subject=SubjectImages.id "
                                    " INNER JOIN Images AS ObjectImages "
                                    " ON ImageRelations.object=ObjectImages.id "
                                    "  WHERE (subject=? OR object=?) %1 "
                                    "   AND SubjectImages.status<3 "
                                    "   AND ObjectImages.status<3;");

    if (type == DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString());
    }
    else
    {
        sql = sql.arg(QString::fromUtf8("AND type=?"));
    }

    QSqlQuery query = d->db->prepareQuery(sql);

    QVariantList values;
    qlonglong    subject, object;

    while (!todo.isEmpty())
    {
        qlonglong id = *todo.begin();
        todo.erase(todo.begin());
        done << id;

        if (type == DatabaseRelation::UndefinedType)
        {
            d->db->execSql(query, id, id, &values);
        }
        else
        {
            d->db->execSql(query, id, id, type, &values);
        }

        for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
        {
            subject = (*it).toLongLong();
            ++it;
            object  = (*it).toLongLong();
            ++it;

            pairs << qMakePair(subject, object);

            if (!done.contains(subject))
            {
                todo << subject;
            }

            if (!done.contains(object))
            {
                todo << object;
            }
        }
    }

    return pairs.values();
}

QList<qlonglong> CoreDB::getOneRelatedImageEach(const QList<qlonglong>& ids, DatabaseRelation::Type type) const
{
    QString sql = QString::fromUtf8("SELECT subject, object FROM ImageRelations "
                                    "INNER JOIN Images AS SubjectImages "
                                    "ON ImageRelations.subject=SubjectImages.id "
                                    " INNER JOIN Images AS ObjectImages "
                                    " ON ImageRelations.object=ObjectImages.id "
                                    "  WHERE ( (subject=? AND ObjectImages.status<3) "
                                    "  OR (object=? AND SubjectImages.status<3) ) "
                                    "   %1 LIMIT 1;");

    if (type == DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString());
    }
    else
    {
        sql = sql.arg(QString::fromUtf8("AND type=?"));
    }

    QSqlQuery query = d->db->prepareQuery(sql);
    QSet<qlonglong> result;
    QVariantList    values;

    for (const qlonglong& id : std::as_const(ids))
    {
        if (type == DatabaseRelation::UndefinedType)
        {
            d->db->execSql(query, id, id, &values);
        }
        else
        {
            d->db->execSql(query, id, id, type, &values);
        }

        if (values.size() != 2)
        {
            continue;
        }

        // one of subject and object is the given id, the other our result

        if (values.first() != id)
        {
            result << values.first().toLongLong();
        }
        else
        {
            result << values.last().toLongLong();
        }
    }

    return result.values();
}

QList<qlonglong> CoreDB::getRelatedImagesToByType(DatabaseRelation::Type type) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT object FROM ImageRelations "
                                     "INNER JOIN Images AS SubjectImages "
                                     "ON ImageRelations.subject=SubjectImages.id "
                                     " INNER JOIN Images AS ObjectImages "
                                     " ON ImageRelations.object=ObjectImages.id "
                                     "  WHERE type=? "
                                     "   AND SubjectImages.status<3 "
                                     "   AND ObjectImages.status<3;"),
                   (int)type, &values);

    QList<qlonglong> imageIds;

    if (values.isEmpty())
    {
        return imageIds;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        imageIds << (*it).toLongLong();
    }

    return imageIds;
}

QStringList CoreDB::getItemsURLsWithTag(int tagId) const
{
    QVariantList values;
    QVariantList boundValues;

    QString query(QString::fromUtf8("SELECT DISTINCT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
                                    "LEFT JOIN ImageTags ON Images.id=ImageTags.imageid "
                                    "INNER JOIN Albums ON Albums.id=Images.album "
                                    " WHERE Images.status=1 AND Images.category=1 AND "));

    if (
        (tagId == TagsCache::instance()->tagForPickLabel(NoPickLabel)) ||
        (tagId == TagsCache::instance()->tagForColorLabel(NoColorLabel))
       )
    {
        query += QString::fromUtf8("( ImageTags.tagid=? OR ImageTags.tagid "
                                   "NOT BETWEEN ? AND ? OR ImageTags.tagid IS NULL );");
        boundValues << tagId;

        if (tagId == TagsCache::instance()->tagForPickLabel(NoPickLabel))
        {
            boundValues << TagsCache::instance()->tagForPickLabel(FirstPickLabel);
            boundValues << TagsCache::instance()->tagForPickLabel(LastPickLabel);
        }
        else
        {
            boundValues << TagsCache::instance()->tagForColorLabel(FirstColorLabel);
            boundValues << TagsCache::instance()->tagForColorLabel(LastColorLabel);
        }
    }
    else
    {
        query += QString::fromUtf8("ImageTags.tagid=?;");
        boundValues << tagId;
    }

    d->db->execSql(query, boundValues, &values);

    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QStringList CoreDB::getDirtyOrMissingFaceImageUrls() const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
                                     "LEFT JOIN ImageScannedMatrix ON Images.id=ImageScannedMatrix.imageid "
                                     "INNER JOIN Albums ON Albums.id=Images.album "
                                     " WHERE Images.status=1 AND Images.category=1 AND "
                                     " ( ImageScannedMatrix.imageid IS NULL "
                                     " OR Images.modificationDate != ImageScannedMatrix.modificationDate "
                                     " OR Images.uniqueHash != ImageScannedMatrix.uniqueHash );"),
                   &values);

    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath  = (*it).toString();
        ++it;
        name          = (*it).toString();
        ++it;

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QList<ItemScanInfo> CoreDB::getIdenticalFiles(qlonglong id) const
{
    if (!id)
    {
        return QList<ItemScanInfo>();
    }

    QVariantList values;

    // retrieve unique hash and file size

    d->db->execSql(QString::fromUtf8("SELECT uniqueHash, fileSize FROM Images WHERE id=?;"),
                   id, &values);

    if (values.isEmpty())
    {
        return QList<ItemScanInfo>();
    }

    QString uniqueHash = values.at(0).toString();
    qlonglong fileSize = values.at(1).toLongLong();

    return getIdenticalFiles(uniqueHash, fileSize, id);
}

QList<ItemScanInfo> CoreDB::getIdenticalFiles(const QString& uniqueHash, qlonglong fileSize, qlonglong sourceId) const
{
    // enforce validity

    if (uniqueHash.isEmpty() || (fileSize <= 0))
    {
        return QList<ItemScanInfo>();
    }

    QVariantList values;

    // find items with same fingerprint

    d->db->execSql(QString::fromUtf8("SELECT id, album, name, status, category, modificationDate, fileSize "
                                     "FROM Images WHERE fileSize=? AND uniqueHash=? AND album IS NOT NULL;"),
                   fileSize, uniqueHash, &values);

    QList<ItemScanInfo> list;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        ItemScanInfo info;

        info.id               = (*it).toLongLong();
        ++it;
        info.albumID          = (*it).toInt();
        ++it;
        info.itemName         = (*it).toString();
        ++it;
        info.status           = (DatabaseItem::Status)(*it).toInt();
        ++it;
        info.category         = (DatabaseItem::Category)(*it).toInt();
        ++it;
        info.modificationDate = asDateTimeUTC((*it).toDateTime());
        ++it;
        info.fileSize         = (*it).toLongLong();
        ++it;

        // exclude one source id from list

        if (sourceId == info.id)
        {
            continue;
        }

        // same for all here, per definition

        info.uniqueHash       = uniqueHash;

        list << info;
    }

    return list;
}

QStringList CoreDB::imagesFieldList(DatabaseFields::Images fields)
{
    // adds no spaces at beginning or end

    QStringList list;

    if (fields & DatabaseFields::Album)
    {
        list << QLatin1String("album");
    }

    if (fields & DatabaseFields::Name)
    {
        list << QLatin1String("name");
    }

    if (fields & DatabaseFields::Status)
    {
        list << QLatin1String("status");
    }

    if (fields & DatabaseFields::Category)
    {
        list << QLatin1String("category");
    }

    if (fields & DatabaseFields::ModificationDate)
    {
        list << QLatin1String("modificationDate");
    }

    if (fields & DatabaseFields::FileSize)
    {
        list << QLatin1String("fileSize");
    }

    if (fields & DatabaseFields::UniqueHash)
    {
        list << QLatin1String("uniqueHash");
    }

    if (fields & DatabaseFields::ManualOrder)
    {
        list << QLatin1String("manualOrder");
    }

    return list;
}

QStringList CoreDB::imageInformationFieldList(DatabaseFields::ItemInformation fields)
{
    // adds no spaces at beginning or end

    QStringList list;

    if (fields & DatabaseFields::Rating)
    {
        list << QLatin1String("rating");
    }

    if (fields & DatabaseFields::CreationDate)
    {
        list << QLatin1String("creationDate");
    }

    if (fields & DatabaseFields::DigitizationDate)
    {
        list << QLatin1String("digitizationDate");
    }

    if (fields & DatabaseFields::Orientation)
    {
        list << QLatin1String("orientation");
    }

    if (fields & DatabaseFields::Width)
    {
        list << QLatin1String("width");
    }

    if (fields & DatabaseFields::Height)
    {
        list << QLatin1String("height");
    }

    if (fields & DatabaseFields::Format)
    {
        list << QLatin1String("format");
    }

    if (fields & DatabaseFields::ColorDepth)
    {
        list << QLatin1String("colorDepth");
    }

    if (fields & DatabaseFields::ColorModel)
    {
        list << QLatin1String("colorModel");
    }

    return list;
}

QStringList CoreDB::videoMetadataFieldList(DatabaseFields::VideoMetadata fields)
{
    // adds no spaces at beginning or end

    QStringList list;

    if (fields & DatabaseFields::AspectRatio)
    {
        list << QLatin1String("aspectRatio");
    }

    if (fields & DatabaseFields::AudioBitRate)
    {
        list << QLatin1String("audioBitRate");
    }

    if (fields & DatabaseFields::AudioChannelType)
    {
        list << QLatin1String("audioChannelType");
    }

    if (fields & DatabaseFields::AudioCodec)
    {
        list << QLatin1String("audioCompressor");
    }

    if (fields & DatabaseFields::Duration)
    {
        list << QLatin1String("duration");
    }

    if (fields & DatabaseFields::FrameRate)
    {
        list << QLatin1String("frameRate");
    }

    if (fields & DatabaseFields::VideoCodec)
    {
        list << QLatin1String("videoCodec");
    }

    return list;
}

QStringList CoreDB::imageMetadataFieldList(DatabaseFields::ImageMetadata fields)
{
    // adds no spaces at beginning or end

    QStringList list;

    if (fields & DatabaseFields::Make)
    {
        list << QLatin1String("make");
    }

    if (fields & DatabaseFields::Model)
    {
        list << QLatin1String("model");
    }

    if (fields & DatabaseFields::Lens)
    {
        list << QLatin1String("lens");
    }

    if (fields & DatabaseFields::Aperture)
    {
        list << QLatin1String("aperture");
    }

    if (fields & DatabaseFields::FocalLength)
    {
        list << QLatin1String("focalLength");
    }

    if (fields & DatabaseFields::FocalLength35)
    {
        list << QLatin1String("focalLength35");
    }

    if (fields & DatabaseFields::ExposureTime)
    {
        list << QLatin1String("exposureTime");
    }

    if (fields & DatabaseFields::ExposureProgram)
    {
        list << QLatin1String("exposureProgram");
    }

    if (fields & DatabaseFields::ExposureMode)
    {
        list << QLatin1String("exposureMode");
    }

    if (fields & DatabaseFields::Sensitivity)
    {
        list << QLatin1String("sensitivity");
    }

    if (fields & DatabaseFields::FlashMode)
    {
        list << QLatin1String("flash");
    }

    if (fields & DatabaseFields::WhiteBalance)
    {
        list << QLatin1String("whiteBalance");
    }

    if (fields & DatabaseFields::WhiteBalanceColorTemperature)
    {
        list << QLatin1String("whiteBalanceColorTemperature");
    }

    if (fields & DatabaseFields::MeteringMode)
    {
        list << QLatin1String("meteringMode");
    }

    if (fields & DatabaseFields::SubjectDistance)
    {
        list << QLatin1String("subjectDistance");
    }

    if (fields & DatabaseFields::SubjectDistanceCategory)
    {
        list << QLatin1String("subjectDistanceCategory");
    }

    return list;
}

QStringList CoreDB::imagePositionsFieldList(DatabaseFields::ItemPositions fields)
{
    // adds no spaces at beginning or end

    QStringList list;

    if (fields & DatabaseFields::Latitude)
    {
        list << QLatin1String("latitude");
    }

    if (fields & DatabaseFields::LatitudeNumber)
    {
        list << QLatin1String("latitudeNumber");
    }

    if (fields & DatabaseFields::Longitude)
    {
        list << QLatin1String("longitude");
    }

    if (fields & DatabaseFields::LongitudeNumber)
    {
        list << QLatin1String("longitudeNumber");
    }

    if (fields & DatabaseFields::Altitude)
    {
        list << QLatin1String("altitude");
    }

    if (fields & DatabaseFields::PositionOrientation)
    {
        list << QLatin1String("orientation");
    }

    if (fields & DatabaseFields::PositionTilt)
    {
        list << QLatin1String("tilt");
    }

    if (fields & DatabaseFields::PositionRoll)
    {
        list << QLatin1String("roll");
    }

    if (fields & DatabaseFields::PositionAccuracy)
    {
        list << QLatin1String("accuracy");
    }

    if (fields & DatabaseFields::PositionDescription)
    {
        list << QLatin1String("description");
    }

    return list;
}

QStringList CoreDB::imageCommentsFieldList(DatabaseFields::ItemComments fields)
{
    // adds no spaces at beginning or end

    QStringList list;

    if (fields & DatabaseFields::CommentType)
    {
        list << QLatin1String("type");
    }

    if (fields & DatabaseFields::CommentLanguage)
    {
        list << QLatin1String("language");
    }

    if (fields & DatabaseFields::CommentAuthor)
    {
        list << QLatin1String("author");
    }

    if (fields & DatabaseFields::CommentDate)
    {
        list << QLatin1String("date");
    }

    if (fields & DatabaseFields::Comment)
    {
        list << QLatin1String("comment");
    }

    return list;
}

void CoreDB::addBoundValuePlaceholders(QString& query, int count)
{
    // adds no spaces at beginning or end

    QString questionMarks;
    questionMarks.reserve(count * 2);
    QString questionMark(QString::fromUtf8("?,"));

    for (int i = 0 ; i < count ; ++i)
    {
        questionMarks += questionMark;
    }

    // remove last ','

    questionMarks.chop(1);

    query += questionMarks;
}

int CoreDB::findInDownloadHistory(const QString& identifier, const QString& name, qlonglong fileSize, const QDateTime& date) const
{
    QVariantList values;
    QVariantList boundValues;
    boundValues << identifier << name << fileSize
                << d->db->asDBDateTime(date.addSecs(-2))
                << d->db->asDBDateTime(date.addSecs(2));

    d->db->execSql(QString::fromUtf8("SELECT id FROM DownloadHistory "
                                     " WHERE identifier=? AND filename=? "
                                     " AND filesize=? AND (filedate>? "
                                     " AND filedate<?);"),
                   boundValues, &values);

    if (values.isEmpty())
    {
        return -1;
    }

    return values.first().toInt();
}

int CoreDB::addToDownloadHistory(const QString& identifier, const QString& name, qlonglong fileSize, const QDateTime& date) const
{
    QVariant id;
    d->db->execSql(QString::fromUtf8("REPLACE INTO DownloadHistory "
                                     "(identifier, filename, filesize, filedate) "
                                     " VALUES (?,?,?,?);"),
                   identifier, name, fileSize, d->db->asDBDateTime(date), nullptr, &id);

    return id.toInt();
}

void CoreDB::addItemTag(qlonglong imageID, int tagID, bool newTag)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageTags (imageid, tagid) "
                                     "VALUES(?, ?);"),
                   imageID, tagID);

    d->db->recordChangeset(ImageTagChangeset(imageID, tagID, ImageTagChangeset::Added));

    // don't save pick or color tags

    if (!newTag || TagsCache::instance()->isInternalTag(tagID))
    {
        return;
    }

    // move current tag to front

    d->recentlyAssignedTags.removeAll(tagID);
    d->recentlyAssignedTags.prepend(tagID);

    if (d->recentlyAssignedTags.size() > 10)
    {
        d->recentlyAssignedTags.removeLast();
    }
}

void CoreDB::addItemTag(int albumID, const QString& name, int tagID)
{
    // easier because of attributes watch

    addItemTag(getImageId(albumID, name), tagID);
}

void CoreDB::addTagsToItems(const QList<qlonglong>& imageIDs, const QList<int>& tagIDs)
{
    if (imageIDs.isEmpty() || tagIDs.isEmpty())
    {
        return;
    }

    QSqlQuery query = d->db->prepareQuery(QString::fromUtf8("REPLACE INTO ImageTags (imageid, tagid) "
                                                                   "VALUES(?, ?);"));
    QVariantList images;
    QVariantList tags;

    for (const qlonglong& imageid : std::as_const(imageIDs))
    {
        for (int tagid : std::as_const(tagIDs))
        {
            images << imageid;
            tags   << tagid;
        }
    }

    query.addBindValue(images);
    query.addBindValue(tags);
    d->db->execBatch(query);
    d->db->recordChangeset(ImageTagChangeset(imageIDs, tagIDs, ImageTagChangeset::Added));
}

QList<int> CoreDB::getRecentlyAssignedTags() const
{
    return d->recentlyAssignedTags;
}

void CoreDB::removeItemTag(qlonglong imageID, int tagID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageTags "
                                     "WHERE imageID=? AND tagid=?;"),
                   imageID, tagID);

    d->db->recordChangeset(ImageTagChangeset(imageID, tagID, ImageTagChangeset::Removed));
}

void CoreDB::removeItemAllTags(qlonglong imageID, const QList<int>& currentTagIds)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageTags "
                                     "WHERE imageID=?;"),
                   imageID);

    d->db->recordChangeset(ImageTagChangeset(imageID, currentTagIds, ImageTagChangeset::RemovedAll));
}

void CoreDB::removeTagsFromItems(const QList<qlonglong>& imageIDs, const QList<int>& tagIDs)
{
    if (imageIDs.isEmpty() || tagIDs.isEmpty())
    {
        return;
    }

    QSqlQuery query = d->db->prepareQuery(QString::fromUtf8("DELETE FROM ImageTags WHERE imageID=? AND tagid=?;"));
    QVariantList images;
    QVariantList tags;

    for (const qlonglong& imageid : std::as_const(imageIDs))
    {
        for (int tagid : std::as_const(tagIDs))
        {
            images << imageid;
            tags   << tagid;
        }
    }

    query.addBindValue(images);
    query.addBindValue(tags);
    d->db->execBatch(query);
    d->db->recordChangeset(ImageTagChangeset(imageIDs, tagIDs, ImageTagChangeset::Removed));
}

QStringList CoreDB::getItemNamesInAlbum(int albumID, bool recursive) const
{
    QVariantList values;

    if (recursive)
    {
        int rootId = getAlbumRootId(albumID);
        QString path = getAlbumRelativePath(albumID);
        d->db->execSql(QString::fromUtf8("SELECT Images.name FROM Images WHERE Images.album IN "
                                         " (SELECT DISTINCT id FROM Albums "
                                         "  WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?));"),
                       rootId, path, path == QLatin1String("/") ? QLatin1String("/%")
                                                                : QString(path + QLatin1String("/%")), &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT name FROM Images "
                                         "WHERE album=?;"),
                       albumID, &values);
    }

    QStringList names;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        names << it->toString();
    }

    return names;
}

qlonglong CoreDB::getItemFromAlbum(int albumID, const QString& fileName) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                     "WHERE album=? AND name=?;"),
                   albumID, fileName, &values);

    if (values.isEmpty())
    {
        return -1;
    }

    return values.first().toLongLong();
}

QMap<QDateTime, int> CoreDB::getAllCreationDates() const
{
    QVariantList         values;
    QMap<QDateTime, int> dateNumberMap;

    d->db->execSql(QString::fromUtf8("SELECT creationDate, COUNT(*) FROM ImageInformation "
                                     "INNER JOIN Images ON Images.id=ImageInformation.imageid "
                                     " WHERE Images.status=1 GROUP BY creationDate;"),
                   &values);

    int       count;
    QDateTime dateTime;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        dateTime = asDateTimeUTC((*it).toDateTime());
        ++it;
        count    = (*it).toInt();
        ++it;

        dateNumberMap.insert(dateTime, count);
    }

    return dateNumberMap;
}

QList<qlonglong> CoreDB::getObsoleteItemIds() const
{
   QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                     "WHERE status=? OR album "
                                     " NOT IN (SELECT id FROM Albums);"),
                   DatabaseItem::Status::Obsolete, &values);

    QList<qlonglong> imageIds;

    for (const QVariant& object : std::as_const(values))
    {
        imageIds << object.toLongLong();
    }

    return imageIds;
}

QDateTime CoreDB::getAlbumModificationDate(int albumID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT modificationDate FROM Albums "
                                     " WHERE id=?;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return QDateTime();
    }

    QDateTime dateTime = asDateTimeUTC(values.first().toDateTime());

    return dateTime;
}

QMap<QString, QDateTime> CoreDB::getAlbumModificationMap(int albumRootId) const
{
    QVariantList             values;
    QMap<QString, QDateTime> pathDateMap;

    d->db->execSql(QString::fromUtf8("SELECT relativePath, modificationDate FROM Albums "
                                     " WHERE albumRoot=?;"),
                   albumRootId, &values);

    QDateTime dateTime;
    QString   relativePath;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        relativePath = (*it).toString();
        ++it;
        dateTime     = asDateTimeUTC((*it).toDateTime());
        ++it;

        pathDateMap.insert(relativePath, dateTime);
    }

    return pathDateMap;

}

QPair<int, int> CoreDB::getNumberOfAllItemsAndAlbums(int albumID) const
{
    int items    = 0;
    int albums   = 0;
    QVariantList values;

    int rootId   = getAlbumRootId(albumID);
    QString path = getAlbumRelativePath(albumID);
    d->db->execSql(QString::fromUtf8("SELECT COUNT(*) FROM Images WHERE Images.album IN "
                                     " (SELECT DISTINCT id FROM Albums "
                                     "  WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?));"),
                   rootId, path, path == QLatin1String("/") ? QLatin1String("/%")
                                                            : QString(path + QLatin1String("/%")), &values);

    if (!values.isEmpty())
    {
        items = values.first().toInt();
    }

    values.clear();

    d->db->execSql(QString::fromUtf8("SELECT DISTINCT COUNT(*) FROM Albums "
                                     " WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?);"),
                   rootId, path, path == QLatin1String("/") ? QLatin1String("/%")
                                                            : QString(path + QLatin1String("/%")), &values);

    if (!values.isEmpty())
    {
        albums = values.first().toInt();
    }

    return qMakePair(items, albums);
}

int CoreDB::getNumberOfItemsInAlbum(int albumID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT COUNT(*) FROM Images "
                                     "WHERE album=?;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return 0;
    }

    return values.first().toInt();
}

QHash<int, int> CoreDB::getNumberOfImagesInAlbums() const
{
    QVariantList    values, allAbumIDs;
    QHash<int, int> albumsStatHash;
    int             albumID, count;

    // initialize allAbumIDs with all existing albums from db to prevent
    // wrong album image counters

    d->db->execSql(QString::fromUtf8("SELECT id FROM Albums;"),
                   &allAbumIDs);

    for (QList<QVariant>::const_iterator it = allAbumIDs.constBegin() ; it != allAbumIDs.constEnd() ; ++it)
    {
        albumID = (*it).toInt();
        albumsStatHash.insert(albumID, 0);
    }

    d->db->execSql(QString::fromUtf8("SELECT album, COUNT(*) FROM Images "
                                     "WHERE Images.status=1 GROUP BY album;"),
                   &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        albumID = (*it).toInt();
        ++it;
        count   = (*it).toInt();
        ++it;

        albumsStatHash[albumID] = count;
    }

    return albumsStatHash;
}

QHash<int, int> CoreDB::getNumberOfImagesInTags() const
{
    QVariantList    values, allTagIDs;
    QVariantList    boundValues;
    QHash<int, int> tagsStatHash;
    int             tagID, count;

    // initialize allTagIDs with all existing tags from db to prevent
    // wrong tag counters

    d->db->execSql(QString::fromUtf8("SELECT id FROM Tags;"),
                   &allTagIDs);

    for (QList<QVariant>::const_iterator it = allTagIDs.constBegin() ; it != allTagIDs.constEnd() ; ++it)
    {
        tagID = (*it).toInt();
        tagsStatHash.insert(tagID, 0);
    }

    QString sql = QString::fromUtf8("SELECT tagid, COUNT(*) FROM ImageTags "
                                    "LEFT JOIN Images ON Images.id=ImageTags.imageid "
                                    " WHERE Images.status=1");
    sql += manualTagVisibilitySql(QLatin1String("ImageTags.imageid"),
                                  &boundValues);
    sql += QLatin1String(" GROUP BY tagid;");
    d->db->execSql(sql, boundValues, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        tagID = (*it).toInt();
        ++it;
        count = (*it).toInt();
        ++it;

        tagsStatHash[tagID] = count;
    }

    return tagsStatHash;
}

QHash<int, int> CoreDB::getNumberOfImagesInTagProperties(const QString& property) const
{
    QVariantList    values;
    QVariantList    boundValues { property };
    QHash<int, int> tagsStatHash;
    int             tagID, count;

    QString sql = QString::fromUtf8("SELECT tagid, COUNT(*) FROM ImageTagProperties "
                                    "LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                                    " WHERE ImageTagProperties.property=? AND Images.status=1");
    sql += manualTagVisibilitySql(QLatin1String("ImageTagProperties.imageid"),
                                  &boundValues);
    sql += QLatin1String(" GROUP BY tagid;");
    d->db->execSql(sql, boundValues, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        tagID = (*it).toInt();
        ++it;
        count = (*it).toInt();
        ++it;

        tagsStatHash[tagID] = count;
    }

    return tagsStatHash;
}

int CoreDB::getNumberOfImagesInTagProperties(int tagId, const QString& property) const
{
    QVariantList values;
    QVariantList boundValues { property, tagId };

    QString sql = QString::fromUtf8("SELECT COUNT(*) FROM ImageTagProperties "
                                    "LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                                    " WHERE ImageTagProperties.property=? AND Images.status=1 "
                                    " AND ImageTagProperties.tagid=?");
    sql += manualTagVisibilitySql(QLatin1String("ImageTagProperties.imageid"),
                                  &boundValues);
    sql += QLatin1Char(';');
    d->db->execSql(sql, boundValues, &values);

    if (values.isEmpty())
    {
        return 0;
    }

    return values.first().toInt();
}

QList<qlonglong> CoreDB::getImagesWithImageTagProperty(int tagId, const QString& property) const
{
    QVariantList     values;
    QVariantList     boundValues { property, tagId };
    QList<qlonglong> imageIds;

    QString sql = QString::fromUtf8("SELECT DISTINCT Images.id FROM ImageTagProperties "
                                    "LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                                    " WHERE ImageTagProperties.property=? AND Images.status=1 "
                                    " AND ImageTagProperties.tagid=?");
    sql += manualTagVisibilitySql(QLatin1String("ImageTagProperties.imageid"),
                                  &boundValues);
    sql += QLatin1Char(';');
    d->db->execSql(sql, boundValues, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        imageIds.append((*it).toLongLong());
    }

    return imageIds;
}

QList<qlonglong> CoreDB::getImagesWithProperty(const QString& property) const
{
    QVariantList     values;
    QList<qlonglong> imageIds;

    d->db->execSql(QString::fromUtf8("SELECT DISTINCT Images.id FROM ImageTagProperties "
                                     "LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                                     " WHERE ImageTagProperties.property=? AND Images.status=1;"),
                   property, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        imageIds.append((*it).toInt());
    }

    return imageIds;
}

QMap<QString, int> CoreDB::getFormatStatistics() const
{
    return getFormatStatistics(DatabaseItem::UndefinedCategory);
}

QMap<QString, int> CoreDB::getFormatStatistics(DatabaseItem::Category category) const
{
    QMap<QString, int>  map;

    QString queryString = QString::fromUtf8("SELECT COUNT(*), II.format "
                                            "FROM ImageInformation AS II "
                                            "INNER JOIN Images ON II.imageid=Images.id "
                                            " WHERE Images.status=1 ");

    if (category != DatabaseItem::UndefinedCategory)
    {
        queryString.append(QString::fromUtf8("AND Images.category=%1 ").arg(category));
    }

    queryString.append(QString::fromUtf8("GROUP BY II.format;"));
    qCDebug(DIGIKAM_DATABASE_LOG) << queryString;

    QSqlQuery query = d->db->prepareQuery(queryString);

    if (d->db->exec(query))
    {
        while (query.next())
        {
            QString quantity = query.value(0).toString();
            QString format   = query.value(1).toString();

            if (format.isEmpty())
            {
                continue;
            }

            map[format] = quantity.isEmpty() ? 0 : quantity.toInt();
        }
    }

    return map;
}

QStringList CoreDB::getListFromImageMetadata(DatabaseFields::ImageMetadata field) const
{
    QStringList  list;
    QVariantList values;
    QStringList  fieldName = imageMetadataFieldList(field);

    if (fieldName.count() != 1)
    {
        return list;
    }

    QString sql = QString::fromUtf8("SELECT DISTINCT %1 FROM ImageMetadata "
                                    "INNER JOIN Images ON imageid=Images.id "
                                    " WHERE Images.status=1;");

    sql = sql.arg(fieldName.first());
    d->db->execSql(sql, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        if (!it->isNull())
        {
            list << it->toString();
        }
    }

    return list;
}

int CoreDB::getAlbumForPath(int albumRootId, const QString& folder, bool create) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Albums WHERE albumRoot=? AND relativePath=?;"),
                   albumRootId, folder, &values);

    int albumID = -1;

    if (values.isEmpty())
    {
        if (create)
        {
            albumID = addAlbum(albumRootId, folder, QString(), QDate::currentDate(), QString());
        }
    }
    else
    {
        albumID = values.first().toInt();
    }

    return albumID;
}

QList<int> CoreDB::getAlbumAndSubalbumsForPath(int albumRootId, const QString& relativePath) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id, relativePath FROM Albums "
                                     "WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?);"),
                   albumRootId, relativePath,
                   (relativePath == QLatin1String("/") ? QLatin1String("/%")
                                                       : QString(relativePath + QLatin1String("/%"))), &values);

    int id;
    QList<int> albumIds;
    QString    albumRelativePath;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        id                = (*it).toInt();
        ++it;
        albumRelativePath = (*it).toString();
        ++it;

        // bug #223050: The LIKE operator is case insensitive

        if (albumRelativePath.startsWith(relativePath))
        {
            albumIds << id;
        }
    }

    return albumIds;
}

QList<int> CoreDB::getAlbumsOnAlbumRoot(int albumRootId) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Albums WHERE albumRoot=?;"),
                   albumRootId, &values);

    QList<int> albumIds;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        albumIds << (*it).toInt();
    }

    return albumIds;
}

qlonglong CoreDB::addItem(int albumID, const QString& name,
                          DatabaseItem::Status status,
                          DatabaseItem::Category category,
                          const QDateTime& modificationDate,
                          qlonglong fileSize,
                          const QString& uniqueHash) const
{
    QVariantList boundValues;
    boundValues << albumID << name << (int)status << (int)category
                << d->db->asDBDateTime(modificationDate) << fileSize << uniqueHash;

    QVariant id;
    d->db->execSql(QString::fromUtf8("REPLACE INTO Images "
                                     "( album, name, status, category, modificationDate, fileSize, uniqueHash ) "
                                     " VALUES (?,?,?,?,?,?,?);"),
                   boundValues, nullptr, &id);

    if (id.isNull())
    {
        return -1;
    }

    d->db->recordChangeset(ImageChangeset(id.toLongLong(), DatabaseFields::Set(DatabaseFields::ImagesAll)));
    d->db->recordChangeset(CollectionImageChangeset(id.toLongLong(), albumID,
                                                    CollectionImageChangeset::Added));
    return id.toLongLong();
}

void CoreDB::updateItem(qlonglong imageID, DatabaseItem::Category category,
                        const QDateTime& modificationDate,
                        qlonglong fileSize, const QString& uniqueHash)
{
    QVariantList boundValues;
    boundValues << (int)category << d->db->asDBDateTime(modificationDate) << fileSize << uniqueHash << imageID;

    d->db->execSql(QString::fromUtf8("UPDATE Images SET category=?, modificationDate=?, fileSize=?, uniqueHash=? "
                                     "WHERE id=?;"),
                   boundValues);

    d->db->recordChangeset(ImageChangeset(imageID,
                                          DatabaseFields::Set(DatabaseFields::Category         |
                                                              DatabaseFields::ModificationDate |
                                                              DatabaseFields::FileSize         |
                                                              DatabaseFields::UniqueHash)));
}

void CoreDB::setItemStatus(qlonglong imageID, DatabaseItem::Status status)
{
    QVariantList boundValues;
    boundValues << (int)status << imageID;
    d->db->execSql(QString::fromUtf8("UPDATE Images SET status=? WHERE id=?;"),
                   boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::Status)));
}

void CoreDB::setItemAlbum(qlonglong imageID, qlonglong album)
{
    QVariantList boundValues;
    boundValues << album << imageID;
    d->db->execSql(QString::fromUtf8("UPDATE Images SET album=? WHERE id=?;"),
                   boundValues);

    // record that the image was assigned a new album

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::Album)));

    // also record that the collection was changed by adding an image to an album.

    d->db->recordChangeset(CollectionImageChangeset(imageID, album, CollectionImageChangeset::Added));
}

void CoreDB::setItemManualOrder(qlonglong imageID, qlonglong value)
{
    QVariantList boundValues;
    boundValues << value << imageID;
    d->db->execSql(QString::fromUtf8("UPDATE Images SET manualOrder=? WHERE id=?;"),
                   boundValues);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::ManualOrder)));
}

void CoreDB::setItemModificationDate(qlonglong imageID, const QDateTime& modificationDate)
{
    QVariantList boundValues;
    boundValues << d->db->asDBDateTime(modificationDate) << imageID;
    d->db->execSql(QString::fromUtf8("UPDATE Images SET modificationDate=? WHERE id=?;"),
                   boundValues);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::ModificationDate)));
}

void CoreDB::renameItem(qlonglong imageID, const QString& newName)
{
    d->db->execSql(QString::fromUtf8("UPDATE Images SET name=? WHERE id=?;"),
                   newName, imageID);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Set(DatabaseFields::Name)));
}

int CoreDB::getItemAlbum(qlonglong imageID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT album FROM Images WHERE id=?;"),
                   imageID, &values);

    if (values.isEmpty())
    {
        return 1;
    }

    return values.first().toInt();
}

QString CoreDB::getItemName(qlonglong imageID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT name FROM Images WHERE id=?;"),
                   imageID, &values);

    if (values.isEmpty())
    {
        return QString();
    }

    return values.first().toString();
}

QStringList CoreDB::getItemURLsInAlbum(int albumID, ItemSortOrder sortOrder) const
{
    QVariantList values;

    int albumRootId = getAlbumRootId(albumID);

    if (albumRootId == -1)
    {
        return QStringList();
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(albumRootId);

    if (albumRootPath.isNull())
    {
        return QStringList();
    }

    QMap<QString, QVariant> bindingMap;
    bindingMap.insert(QString::fromUtf8(":albumID"), albumID);

    switch (sortOrder)
    {
        case ByItemName:
        {
            d->db->execDBAction(d->db->getDBAction(QLatin1String("getItemURLsInAlbumByItemName")),
                                bindingMap, &values);
            break;
        }

        case ByItemPath:
        {
            // Don't collate on the path - this is to maintain the same behavior
            // that happens when sort order is "By Path"

            d->db->execDBAction(d->db->getDBAction(QLatin1String("getItemURLsInAlbumByItemPath")),
                                bindingMap, &values);
            break;
        }

        case ByItemDate:
        {
            d->db->execDBAction(d->db->getDBAction(QLatin1String("getItemURLsInAlbumByItemDate")),
                                bindingMap, &values);
            break;
        }

        case ByItemRating:
        {
            d->db->execDBAction(d->db->getDBAction(QLatin1String("getItemURLsInAlbumByItemRating")),
                                bindingMap, &values);
            break;
        }

        case NoItemSorting:
        default:
        {
            d->db->execDBAction(d->db->getDBAction(QLatin1String("getItemURLsInAlbumNoItemSorting")),
                                bindingMap, &values);
            break;
        }
    }

    QStringList urls;
    QString     relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        relativePath = (*it).toString();
        ++it;
        name         = (*it).toString();
        ++it;

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QList<qlonglong> CoreDB::getItemIDsInAlbum(int albumID) const
{
    QList<qlonglong> itemIDs;
    QVariantList     values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images WHERE album=?;"),
                   albumID, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

QMap<qlonglong, QString> CoreDB::getItemIDsAndURLsInAlbum(int albumID) const
{
    int albumRootId = getAlbumRootId(albumID);

    if (albumRootId == -1)
    {
        return QMap<qlonglong, QString>();
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(albumRootId);

    if (albumRootPath.isNull())
    {
        return QMap<qlonglong, QString>();
    }

    QMap<qlonglong, QString> itemsMap;
    QVariantList             values;

    d->db->execSql(QString::fromUtf8("SELECT Images.id, Albums.relativePath, Images.name "
                                     "FROM Images "
                                     " INNER JOIN Albums ON Albums.id=Images.album "
                                     "  WHERE Albums.id=?;"),
                   albumID, &values);

    qlonglong id;
    QString   path;
    QString   relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        id           = (*it).toLongLong();
        ++it;
        relativePath = (*it).toString();
        ++it;
        name         = (*it).toString();
        ++it;

        if (relativePath == QLatin1String("/"))
        {
            path = albumRootPath + relativePath + name;
        }
        else
        {
            path = albumRootPath + relativePath + QLatin1Char('/') + name;
        }

        itemsMap.insert(id, path);
    };

    return itemsMap;
}

QList<qlonglong> CoreDB::getAllItems() const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images;"),
                   &values);

    QList<qlonglong> items;

    for (const QVariant& item : std::as_const(values))
    {
        items << item.toLongLong();
    }

    return items;
}

QHash<qlonglong, QPair<int, int> > CoreDB::getAllItemsWithAlbum() const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT Images.id, Albums.albumRoot, Albums.id FROM Images "
                                     "INNER JOIN Albums ON Albums.id=Images.album "
                                     " WHERE Images.status<3;"),
                   &values);

    QHash<qlonglong, QPair<int, int> > itemAlbumHash;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        qlonglong id  = (*it).toLongLong();
        ++it;
        int albumRoot = (*it).toInt();
        ++it;
        int album     = (*it).toInt();
        ++it;

        itemAlbumHash[id] = qMakePair(albumRoot, album);
    }

    return itemAlbumHash;
}

QList<ItemScanInfo> CoreDB::getItemScanInfos(int albumID) const
{
    QList<ItemScanInfo> list;

    QString sql = QString::fromUtf8("SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash "
                                    "FROM Images WHERE album=?;");

    QSqlQuery query = d->db->prepareQuery(sql);
    query.addBindValue(albumID);

    if (d->db->exec(query))
    {
        while (query.next())
        {
            ItemScanInfo info;

            info.id               = query.value(0).toLongLong();
            info.albumID          = query.value(1).toInt();
            info.itemName         = query.value(2).toString();
            info.status           = (DatabaseItem::Status)query.value(3).toInt();
            info.category         = (DatabaseItem::Category)query.value(4).toInt();
            info.modificationDate = asDateTimeUTC(query.value(5).toDateTime());
            info.fileSize         = query.value(6).toLongLong();
            info.uniqueHash       = query.value(7).toString();

            list << info;
        }
    }

    return list;
}

ItemScanInfo CoreDB::getItemScanInfo(qlonglong imageID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash "
                                     "FROM Images WHERE id=?;"),
                   imageID, &values);

    ItemScanInfo info;

    if (!values.isEmpty())
    {
        QList<QVariant>::const_iterator it = values.constBegin();

        info.id               = (*it).toLongLong();
        ++it;
        info.albumID          = (*it).toInt();
        ++it;
        info.itemName         = (*it).toString();
        ++it;
        info.status           = (DatabaseItem::Status)(*it).toInt();
        ++it;
        info.category         = (DatabaseItem::Category)(*it).toInt();
        ++it;
        info.modificationDate = asDateTimeUTC((*it).toDateTime());
        ++it;
        info.fileSize         = (*it).toLongLong();
        ++it;
        info.uniqueHash       = (*it).toString();
        ++it;
    }

    return info;
}

QStringList CoreDB::getItemURLsInTag(int tagID, bool recursive) const
{
    QVariantList            values;
    QMap<QString, QVariant> bindingMap;

    bindingMap.insert(QString::fromUtf8(":tagID"),  tagID);
    bindingMap.insert(QString::fromUtf8(":tagID2"), tagID);

    if (recursive)
    {
        d->db->execDBAction(d->db->getDBAction(QLatin1String("GetItemURLsInTagRecursive")), bindingMap, &values);
    }
    else
    {
        d->db->execDBAction(d->db->getDBAction(QLatin1String("GetItemURLsInTag")), bindingMap, &values);
    }

    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; )
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath  = (*it).toString();
        ++it;
        name          = (*it).toString();
        ++it;

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QList<qlonglong> CoreDB::getItemIDsInTag(int tagID, bool recursive) const
{
    QVariantList            values;
    QList<qlonglong>        itemIDs;
    QMap<QString, QVariant> parameters;

    parameters.insert(QString::fromUtf8(":tagPID"), tagID);
    parameters.insert(QString::fromUtf8(":tagID"),  tagID);

    if (recursive)
    {
        d->db->execDBAction(d->db->getDBAction(QLatin1String("getItemIDsInTagRecursive")), parameters, &values);
    }
    else
    {
        d->db->execDBAction(d->db->getDBAction(QLatin1String("getItemIDsInTag")), parameters, &values);
    }

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

qlonglong CoreDB::getFirstItemWithFaceTag(int tagId) const
{
    QVariantList values;
    QVariantList boundValues { tagId, ImageTagPropertyName::tagRegion() };

    QString sql = QString::fromUtf8("SELECT imageid FROM ImageTagProperties "
                                    "LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                                    " WHERE tagid=? AND property=? AND Images.status=1");
    sql += manualTagVisibilitySql(QLatin1String("ImageTagProperties.imageid"),
                                  &boundValues);
    sql += QLatin1String(" LIMIT 1;");
    d->db->execSql(sql, boundValues, &values);

    if (values.isEmpty())
    {
        return -1;
    }

    return values.first().toLongLong();
}

QString CoreDB::getAlbumRelativePath(int albumID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT relativePath FROM Albums WHERE id=?;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return QString();
    }

    return values.first().toString();
}

int CoreDB::getAlbumRootId(int albumID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT albumRoot FROM Albums WHERE id=?;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return -1;
    }

    return values.first().toInt();
}

QDate CoreDB::getAlbumLowestDate(int albumID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT MIN(creationDate) FROM ImageInformation "
                                     "INNER JOIN Images ON Images.id=ImageInformation.imageid "
                                     " WHERE Images.album=? GROUP BY Images.album;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return QDate();
    }

    QDateTime albumDateTime = asDateTimeUTC(values.first().toDateTime());

    return albumDateTime.date();
}

QDate CoreDB::getAlbumHighestDate(int albumID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT MAX(creationDate) FROM ImageInformation "
                                     "INNER JOIN Images ON Images.id=ImageInformation.imageid "
                                     " WHERE Images.album=? GROUP BY Images.album;"),
                   albumID , &values);

    if (values.isEmpty())
    {
        return QDate();
    }

    QDateTime albumDateTime = asDateTimeUTC(values.first().toDateTime());

    return albumDateTime.date();
}

QDate CoreDB::getAlbumAverageDate(int albumID) const
{
    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT creationDate FROM ImageInformation "
                                     "INNER JOIN Images ON Images.id=ImageInformation.imageid "
                                     " WHERE Images.album=?;"),
                   albumID , &values);

    QList<QDate> dates;

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ; ++it)
    {
        QDateTime itemDateTime = asDateTimeUTC((*it).toDateTime());

        if (itemDateTime.isValid())
        {
            dates << itemDateTime.date();
        }
    }

    if (dates.isEmpty())
    {
        return QDate();
    }

    qint64 julianDays = 0;

    for (const QDate& date : std::as_const(dates))
    {
        // cppcheck-suppress useStlAlgorithm
        julianDays += date.toJulianDay();
    }

    return QDate::fromJulianDay(julianDays / dates.size());
}

void CoreDB::deleteItem(int albumID, const QString& file)
{
    qlonglong imageId = getImageId(albumID, file);

    if (imageId == -1)
    {
        return;
    }

    d->db->execSql(QString::fromUtf8("DELETE FROM Images WHERE id=?;"),
                   imageId);

    d->db->recordChangeset(CollectionImageChangeset(imageId, albumID, CollectionImageChangeset::Deleted));
}

void CoreDB::deleteItem(qlonglong imageId)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Images WHERE id=? AND album IS NULL;"),
                   imageId);
}

void CoreDB::deleteObsoleteItem(qlonglong imageId)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Images WHERE id=?;"),
                   imageId);
}

void CoreDB::removeItemsFromAlbum(int albumID, const QList<qlonglong>& ids_forInformation)
{
    d->db->execSql(QString::fromUtf8("UPDATE Images SET status=?, album=NULL WHERE album=?;"),
                   (int)DatabaseItem::Trashed, albumID);

    d->db->recordChangeset(ImageChangeset(ids_forInformation, DatabaseFields::Set(DatabaseFields::Status)));
    d->db->recordChangeset(CollectionImageChangeset(ids_forInformation, albumID, CollectionImageChangeset::RemovedAll));
}

void CoreDB::removeItems(const QList<qlonglong>& itemIDs, const QList<int>& albumIDs)
{
    QSqlQuery query = d->db->prepareQuery(QString::fromUtf8("UPDATE Images SET status=?, album=NULL WHERE id=?;"));

    QVariantList imageIds;
    QVariantList status;

    for (const qlonglong& id : std::as_const(itemIDs))
    {
        status << (int)DatabaseItem::Trashed;
        imageIds << id;
    }

    query.addBindValue(status);
    query.addBindValue(imageIds);
    d->db->execBatch(query);

    d->db->recordChangeset(ImageChangeset(itemIDs, DatabaseFields::Set(DatabaseFields::Status)));
    d->db->recordChangeset(CollectionImageChangeset(itemIDs, albumIDs, CollectionImageChangeset::Removed));
}

void CoreDB::removeItemsPermanently(const QList<qlonglong>& itemIDs, const QList<int>& albumIDs)
{
    QSqlQuery query = d->db->prepareQuery(QString::fromUtf8("UPDATE Images SET status=?, album=NULL WHERE id=?;"));

    QVariantList imageIds;
    QVariantList status;

    for (const qlonglong& id : std::as_const(itemIDs))
    {
        status   << (int)DatabaseItem::Obsolete;
        imageIds << id;
    }

    query.addBindValue(status);
    query.addBindValue(imageIds);
    d->db->execBatch(query);

    d->db->recordChangeset(ImageChangeset(itemIDs, DatabaseFields::Set(DatabaseFields::Status)));
    d->db->recordChangeset(CollectionImageChangeset(itemIDs, albumIDs, CollectionImageChangeset::Removed));
}

void CoreDB::deleteRemovedItems()
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Images WHERE status=?;"),
                   (int)DatabaseItem::Obsolete);

    d->db->recordChangeset(CollectionImageChangeset(QList<qlonglong>(), QList<int>(), CollectionImageChangeset::RemovedDeleted));
}

void CoreDB::renameAlbum(int albumID, int newAlbumRoot, const QString& newRelativePath)
{
    int albumRoot        = getAlbumRootId(albumID);
    QString relativePath = getAlbumRelativePath(albumID);

    if ((relativePath == newRelativePath) && (albumRoot == newAlbumRoot))
    {
        return;
    }

    // first delete any stale albums left behind at the destination of renaming

    QMap<QString, QVariant> parameters;
    parameters.insert(QString::fromUtf8(":albumRoot"),    newAlbumRoot);
    parameters.insert(QString::fromUtf8(":relativePath"), newRelativePath);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumRootPath")), parameters))
    {
        return;
    }

    // now update the album

    d->db->execSql(QString::fromUtf8("UPDATE Albums SET albumRoot=?, relativePath=? WHERE id=? AND albumRoot=?;"),
                   newAlbumRoot, newRelativePath, albumID, albumRoot);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Renamed));
}

void CoreDB::setTagName(int tagID, const QString& name)
{
    d->db->execSql(QString::fromUtf8("UPDATE Tags SET name=? WHERE id=?;"),
                   name, tagID);
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Renamed));
}

void CoreDB::moveItem(int srcAlbumID, const QString& srcName,
                      int dstAlbumID, const QString& dstName)
{
    // find id of src image

    qlonglong imageId = getImageId(srcAlbumID, srcName);

    if (imageId == -1)
    {
        return;
    }

    // first delete any stale database entries (for destination) if any

    deleteItem(dstAlbumID, dstName);

    d->db->execSql(QString::fromUtf8("UPDATE Images SET album=?, name=? "
                                     "WHERE id=?;"),
                   dstAlbumID, dstName, imageId);

    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::Set(DatabaseFields::Album)));
    d->db->recordChangeset(CollectionImageChangeset(imageId, dstAlbumID, CollectionImageChangeset::Added));
    d->db->recordChangeset(CollectionImageChangeset(imageId, srcAlbumID, CollectionImageChangeset::Moved));
    d->db->recordChangeset(CollectionImageChangeset(imageId, srcAlbumID, CollectionImageChangeset::Removed));
}

qlonglong CoreDB::copyItem(int srcAlbumID, const QString& srcName,
                           int dstAlbumID, const QString& dstName)
{
    // find id of src image

    qlonglong srcId = getImageId(srcAlbumID, srcName);

    if ((srcId == -1) || (dstAlbumID == -1) || dstName.isEmpty())
    {
        return -1;
    }

    // check for src == dest

    if ((srcAlbumID == dstAlbumID) && (srcName == dstName))
    {
        return srcId;
    }

    // first delete any stale database entries if any

    deleteItem(dstAlbumID, dstName);

    // copy entry in Images table

    QVariant id;
    d->db->execSql(QString::fromUtf8("INSERT INTO Images "
                                     "( album, name, status, category, modificationDate, fileSize, uniqueHash, manualOrder ) "
                                     " SELECT ?, ?, status, category, modificationDate, fileSize, uniqueHash, manualOrder "
                                     " FROM Images WHERE id=?;"),
                   dstAlbumID, dstName, srcId, nullptr, &id);

    if (id.isNull())
    {
        return -1;
    }

    d->db->recordChangeset(ImageChangeset(id.toLongLong(), DatabaseFields::Set(DatabaseFields::ImagesAll)));
    d->db->recordChangeset(CollectionImageChangeset(id.toLongLong(), dstAlbumID, CollectionImageChangeset::Added));
    d->db->recordChangeset(CollectionImageChangeset(id.toLongLong(), srcAlbumID, CollectionImageChangeset::Copied));

    // copy all other tables

    copyImageAttributes(srcId, id.toLongLong());

    return id.toLongLong();
}

void CoreDB::copyImageAttributes(qlonglong srcId, qlonglong dstId)
{
    // Go through all image-specific tables and copy the entries

    DatabaseFields::Set fields;

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageInformation "
                                     "(imageid, rating, creationDate, digitizationDate, orientation, "
                                     " width, height, format, colorDepth, colorModel) "
                                     "SELECT ?, rating, creationDate, digitizationDate, orientation, "
                                     " width, height, format, colorDepth, colorModel "
                                     "FROM ImageInformation WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ItemInformationAll;

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageMetadata "
                                     "(imageid, make, model, lens, aperture, focalLength, focalLength35, "
                                     " exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                     " whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                                     "SELECT ?, make, model, lens, aperture, focalLength, focalLength35, "
                                     " exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                     " whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                                     "FROM ImageMetadata WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageMetadataAll;

    d->db->execSql(QString::fromUtf8("REPLACE INTO VideoMetadata "
                                     "(imageid, aspectRatio, audioBitRate, audioChannelType, audioCompressor, duration, "
                                     " frameRate, videoCodec) "
                                     "SELECT ?, aspectRatio, audioBitRate, audioChannelType, audioCompressor, duration, "
                                     " frameRate, videoCodec "
                                     "FROM VideoMetadata WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::VideoMetadataAll;

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImagePositions "
                                     "(imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                                     " altitude, orientation, tilt, roll, accuracy, description) "
                                     "SELECT ?, latitude, latitudeNumber, longitude, longitudeNumber, "
                                     " altitude, orientation, tilt, roll, accuracy, description "
                                     "FROM ImagePositions WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ItemPositionsAll;

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageComments "
                                     "(imageid, type, language, author, date, comment) "
                                     "SELECT ?, type, language, author, date, comment "
                                     "FROM ImageComments WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ItemCommentsAll;

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageCopyright "
                                     "(imageid, property, value, extraValue) "
                                     "SELECT ?, property, value, extraValue "
                                     "FROM ImageCopyright WHERE imageid=?;"),
                   dstId, srcId);

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageHistory "
                                     "(imageid, uuid, history) "
                                     "SELECT ?, uuid, history "
                                     "FROM ImageHistory WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageHistoryInfoAll;

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageRelations "
                                    "(subject, object, type) "
                                    "SELECT ?, object, type "
                                    "FROM ImageRelations WHERE subject=?;"),
                   dstId, srcId);
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageRelations "
                                    "(subject, object, type) "
                                    "SELECT subject, ?, type "
                                    "FROM ImageRelations WHERE object=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageRelations;

    d->db->recordChangeset(ImageChangeset(dstId, fields));

    copyImageTags(srcId, dstId);
    copyImageProperties(srcId, dstId);
}

void CoreDB::copyImageProperties(qlonglong srcId, qlonglong dstId)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageProperties "
                                    "(imageid, property, value) "
                                    "SELECT ?, property, value "
                                    "FROM ImageProperties WHERE imageid=?;"),
                   dstId, srcId);
}

void CoreDB::copyImageTags(qlonglong srcId, qlonglong dstId)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageTags "
                                     "(imageid, tagid) "
                                     "SELECT ?, tagid "
                                     "FROM ImageTags WHERE imageid=?;"),
                   dstId, srcId);

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageTagProperties "
                                     "(imageid, tagid, property, value) "
                                     "SELECT ?, tagid, property, value "
                                     "FROM ImageTagProperties WHERE imageid=?;"),
                   dstId, srcId);

    // leave empty tag list for now

    d->db->recordChangeset(ImageTagChangeset(dstId, QList<int>(),
                                             ImageTagChangeset::Added));

    d->db->recordChangeset(ImageTagChangeset(dstId, QList<int>(),
                                             ImageTagChangeset::PropertiesChanged));
}

bool CoreDB::copyAlbumProperties(int srcAlbumID, int dstAlbumID) const
{
    if (srcAlbumID == dstAlbumID)
    {
        return true;
    }

    QVariantList values;

    d->db->execSql(QString::fromUtf8("SELECT date, caption, collection, icon "
                                     "FROM Albums WHERE id=?;"),
                   srcAlbumID, &values);

    if (values.isEmpty())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << " src album ID " << srcAlbumID << " does not exist";

        return false;
    }

    QVariantList boundValues;
    boundValues << values.at(0) << values.at(1) << values.at(2) << values.at(3);
    boundValues << dstAlbumID;

    d->db->execSql(QString::fromUtf8("UPDATE Albums SET date=?, caption=?, "
                                     "collection=?, icon=? WHERE id=?;"),
                   boundValues);
    return true;
}

QVariantList CoreDB::getImageIdsFromArea(qreal lat1, qreal lat2, qreal lng1, qreal lng2, int /*sortMode*/,
                                            const QString& /*sortBy*/) const
{
    QVariantList values;
    QVariantList boundValues;
    boundValues << lat1 << lat2 << lng1 << lng2;

    d->db->execSql(QString::fromUtf8("Select ImageInformation.imageid, ImageInformation.rating, "
                                     "ImagePositions.latitudeNumber, ImagePositions.longitudeNumber "
                                     "FROM ImageInformation INNER JOIN ImagePositions "
                                     " ON ImageInformation.imageid = ImagePositions.imageid "
                                     "  WHERE (ImagePositions.latitudeNumber>? AND ImagePositions.latitudeNumber<?) "
                                     "  AND (ImagePositions.longitudeNumber>? AND ImagePositions.longitudeNumber<?);"),
                   boundValues, &values);

    return values;
}

bool CoreDB::integrityCheck() const
{
    QVariantList values;

    d->db->execDBAction(d->db->getDBAction(QLatin1String("checkCoreDbIntegrity")), &values);

    switch (d->db->databaseType())
    {
        case BdEngineBackend::DbType::SQLite:
        {
            // For SQLite the integrity check returns a single row with one string column "ok" on success and multiple rows on error.

            bool result = (
                           (values.size() == 1) &&
                           (values.first().toString().toLower().compare(QLatin1String("ok")) == 0)
                          );

            if (!result && !values.isEmpty())
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Failed integrity check for SQLite core database:"
                                                << values.first().toString();
            }

            return result;
        }

        case BdEngineBackend::DbType::MySQL:
        {
            // For MySQL, for every checked table, the table name, operation (check), message type (status) and the message text (ok on success)
            // are returned. So we check if there are four elements and if yes, whether the fourth element is "ok".
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << "MySQL check returned " << values.size() << " rows";
*/
            if ((values.size() % 4) != 0)
            {
                return false;
            }

            QString tableName, operation, messageType, messageText;

            for (QList<QVariant>::iterator it = values.begin() ; it != values.end() ; )
            {
                tableName   = (*it).toString();
                ++it;
                operation   = (*it).toString();
                ++it;
                messageType = (*it).toString();
                ++it;
                messageText = (*it).toString();
                ++it;

                Q_UNUSED(operation);

                if (!messageText.isEmpty())
                {
                    if ((messageType.toLower().compare(QLatin1String("note")) == 0))
                    {
                        qCWarning(DIGIKAM_DATABASE_LOG) << "Failed integrity check for table "
                                                        << tableName << ". Reason:" << messageText;
                    }

                    if (
                        (messageType.toLower().compare(QLatin1String("status")) == 0) &&
                        (messageText.toLower().compare(QLatin1String("ok"))     != 0)
                       )
                    {
                        return false;
                    }
                }
                else
                {
/*
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Passed integrity check for table " << tableName;
*/
                }
            }

            // No error conditions. Db passed the integrity check.

            return true;
        }

        default:
        {
            return false;
        }
    }
}

void CoreDB::vacuum()
{
    DatabaseFields::Set fields;

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageInformation "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageProperties "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImagePositions "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageComments "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageMetadata "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM VideoMetadata "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageHistory "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations "
                                     "WHERE subject NOT IN (SELECT id FROM Images);"));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations "
                                     "WHERE object NOT IN (SELECT id FROM Images);"));

    fields |= DatabaseFields::ImagesAll;
    fields |= DatabaseFields::ImageRelations;
    fields |= DatabaseFields::ItemCommentsAll;
    fields |= DatabaseFields::ImageMetadataAll;
    fields |= DatabaseFields::VideoMetadataAll;
    fields |= DatabaseFields::ItemPositionsAll;

    d->db->recordChangeset(ImageChangeset(0, fields));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageTags "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->recordChangeset(ImageTagChangeset(0, QList<int>(), ImageTagChangeset::RemovedAll));

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties "
                                     "WHERE imageid NOT IN (SELECT id FROM Images);"));

    d->db->recordChangeset(ImageTagChangeset(0, QList<int>(), ImageTagChangeset::PropertiesChanged));

    d->db->execDBAction(d->db->getDBAction(QLatin1String("vacuumCoreDB")));
}

void CoreDB::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->recentlyAssignedTags = group.readEntry(d->configRecentlyUsedTags, QList<int>());
}

void CoreDB::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configRecentlyUsedTags, d->recentlyAssignedTags);
}

} // namespace Digikam
