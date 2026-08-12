/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyportableimport.h"

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QUuid>

// C++ includes

#include <algorithm>
#include <memory>

// Local includes

#include "privacycasualarchive.h"
#include "privacystrongrecoverymanifest.h"

namespace Digikam
{

namespace
{

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() &&
            (uuid == parsed.toString(QUuid::WithoutBraces)));
}

bool isSafeOriginalName(const QString& name)
{
    if (name.isEmpty() || (name == QLatin1String(".")) ||
        (name == QLatin1String("..")) ||
        name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\')) ||
        name.contains(QChar::Null))
    {
        return false;
    }

    return true;
}

bool isSafePublicRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) ||
        path.contains(QLatin1Char('\0')) ||
        path.contains(QLatin1Char('\\')))
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

bool isSafeContainerRelativePath(const QString& path)
{
    return isSafePublicRelativePath(path);
}

bool hashFile(const QString& path, QByteArray* const digest,
              qlonglong* const size)
{
    QFile file(path);

    if (!digest || !size || !file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QCryptographicHash hasher(QCryptographicHash::Sha256);
    QByteArray buffer(1024 * 1024, Qt::Uninitialized);
    qlonglong total = 0;

    while (true)
    {
        const qint64 count = file.read(buffer.data(), buffer.size());

        if (count < 0)
        {
            return false;
        }

        if (count == 0)
        {
            break;
        }

        hasher.addData(QByteArrayView(buffer.constData(), count));
        total += count;
    }

    *digest = hasher.result();
    *size = total;
    return true;
}

PrivacyPortableImportAuthenticationResult failure(
    PrivacyPortableImportAuthenticationStatus status,
    const QString& detail)
{
    PrivacyPortableImportAuthenticationResult result;
    result.status = status;
    result.detail = detail;
    return result;
}

} // namespace

bool PrivacyPortableImportAssetFact::isValid() const
{
    const bool casualPath =
        (protectedRelativePath ==
         PrivacyCasualArchiveEngine::expectedMemberPath(
             role, ordinal, originalName));
    const bool strongPath =
        protectedRelativePath.startsWith(QLatin1String("originals/")) &&
        protectedRelativePath.endsWith(
            QLatin1String("/") + QString::number(ordinal) +
            QLatin1Char('-') + originalName);

    return ((role > 0) && (ordinal >= 0) &&
            isSafeOriginalName(originalName) &&
            isSafePublicRelativePath(publicRelativePath) &&
            (casualPath || strongPath) &&
            (hashAlgorithm == QLatin1String("sha256")) &&
            (originalSha256.size() == 32) && (originalSize >= 0) &&
            (portableAttributes.size() <= 64 * 1024) &&
            ((unixMode & 0170000) == 0100000));
}

bool PrivacyPortableImportItemFact::isValid() const
{
    const bool validKind =
        ((containerKind == PrivacyContainerKind::CasualArchive) ||
         (containerKind == PrivacyContainerKind::StrongObject));
    const bool validSource =
        ((containerKind == PrivacyContainerKind::CasualArchive) &&
         QDir::isAbsolutePath(sourceAbsolutePath) &&
         isSafeContainerRelativePath(containerRelativePath)) ||
        ((containerKind == PrivacyContainerKind::StrongObject) &&
         sourceAbsolutePath.isEmpty() &&
         containerRelativePath.startsWith(QLatin1String("originals/")));

    return (isCanonicalUuid(itemUuid) &&
            isCanonicalUuid(containerUuid) &&
            validKind && validSource &&
            isSafePublicRelativePath(proxyRelativePath) &&
            QDir::isAbsolutePath(publicRootPath) &&
            (containerSize >= 0) && (containerSha256.size() == 32) &&
            !assets.isEmpty());
}

bool PrivacyPortableImportCandidate::isValid() const
{
    const bool validCredential =
        hasCredential
            ? (isCanonicalUuid(storeUuid) &&
               isCanonicalUuid(managedStoreMarkerRootUuid) &&
               isCanonicalUuid(managedStoreMarkerMarkerUuid) &&
               QDir::isAbsolutePath(managedStoreRootPath) &&
               isSafeContainerRelativePath(cipherRelativePath) &&
               !credentialEnvelopeFormat.isEmpty() &&
               !credentialEnvelopeBlob.isEmpty())
            : (storeUuid.isEmpty() &&
               managedStoreRootPath.isEmpty() &&
               managedStoreMarkerRootUuid.isEmpty() &&
               managedStoreMarkerMarkerUuid.isEmpty() &&
               cipherRelativePath.isEmpty() &&
               credentialEnvelopeFormat.isEmpty() &&
               credentialEnvelopeBlob.isEmpty());

    return (isCanonicalUuid(recoverySetUuid) &&
            ((backend == PrivacyBackend::Casual) ||
             (backend == PrivacyBackend::Strong)) &&
            isCanonicalUuid(categoryUuid) && validCredential &&
            (presentationMode > 0) &&
            (unlockedThumbnailMode > 0) &&
            (tagVisibilityMode > 0) &&
            !items.isEmpty());
}

class Q_DECL_HIDDEN PrivacyGocryptfsPortableStoreInspector::Private
{
public:

    struct ActiveInspection
    {
        QString workspaceRoot;
        PrivacyGocryptfsStoreLayout layout;
        std::unique_ptr<PrivacyGocryptfsMountLease> lease;
    };

    Private(PrivacyProcessRunner& runner,
            const PrivacyMountStateProbe& mountProbe,
            PrivacyGocryptfsToolPaths toolPaths,
            QString workspaceRoot)
        : m_runner(runner),
          m_mountProbe(mountProbe),
          m_toolPaths(std::move(toolPaths)),
          m_workspaceRoot(std::move(workspaceRoot))
    {
    }

    PrivacyProcessRunner&          m_runner;
    const PrivacyMountStateProbe&  m_mountProbe;
    PrivacyGocryptfsToolPaths      m_toolPaths;
    QString                        m_workspaceRoot;
    QMap<QString, std::shared_ptr<ActiveInspection>> m_active;
};

PrivacyGocryptfsPortableStoreInspector::PrivacyGocryptfsPortableStoreInspector(
    PrivacyProcessRunner& runner,
    const PrivacyMountStateProbe& mountProbe,
    PrivacyGocryptfsToolPaths toolPaths,
    QString workspaceRoot)
    : d(new Private(runner, mountProbe, std::move(toolPaths),
                    std::move(workspaceRoot)))
{
}

PrivacyGocryptfsPortableStoreInspector::~PrivacyGocryptfsPortableStoreInspector()
{
    const QList<QString> keys = d->m_active.keys();

    for (const QString& key : keys)
    {
        PrivacyPortableStoreInspection inspection;
        inspection.valid = true;
        inspection.plaintextRoot = key;
        QString error;
        release(inspection, &error);
    }
}

bool PrivacyGocryptfsPortableStoreInspector::inspect(
    const PrivacyPortableStrongStoreCandidate& store,
    const PrivacyPassword& password,
    PrivacyPortableStoreInspection* const inspection,
    QString* const error)
{
    const auto fail = [error](const QString& detail)
    {
        if (error)
        {
            *error = detail;
        }

        return false;
    };

    if (!inspection || !store.isValid() || !password.isValid() ||
        !QDir::isAbsolutePath(d->m_workspaceRoot))
    {
        return fail(QStringLiteral("invalid store inspection request"));
    }

    *inspection = PrivacyPortableStoreInspection();
    const QString workspaceRoot = QDir(d->m_workspaceRoot).filePath(
        QLatin1String("import-") +
        QUuid::createUuid().toString(QUuid::WithoutBraces));

    if (!QDir().mkpath(workspaceRoot) ||
        !QFile::setPermissions(workspaceRoot,
                               QFileDevice::ReadOwner |
                               QFileDevice::WriteOwner |
                               QFileDevice::ExeOwner))
    {
        return fail(QStringLiteral("the import inspection workspace could not be created"));
    }

    PrivacyGocryptfsStoreLayout layout;
    layout.workspaceRoot = workspaceRoot;
    layout.cipherDirectory = QFileInfo(store.configAbsolutePath).absolutePath();
    layout.mountDirectory = QDir(workspaceRoot).filePath(QLatin1String("mount"));
    layout.runtimeDirectory = QDir(workspaceRoot).filePath(QLatin1String("runtime"));
    PrivacyGocryptfsStoreHarness harness(
        d->m_runner, d->m_mountProbe, d->m_toolPaths, layout);
    PrivacyGocryptfsError harnessError = PrivacyGocryptfsError::None;

    if (!harness.checkCapabilities(&harnessError))
    {
        QDir(workspaceRoot).removeRecursively();
        return fail(QStringLiteral("the bundled gocryptfs tooling is unavailable"));
    }

    std::unique_ptr<PrivacyGocryptfsMountLease> lease =
        harness.mountStoreForInspection(password, &harnessError);

    if (!lease)
    {
        QDir(workspaceRoot).removeRecursively();

        if (harnessError == PrivacyGocryptfsError::InvalidPassword)
        {
            return fail(QStringLiteral("the store password is invalid"));
        }

        return fail(QStringLiteral("the copied store could not be mounted"));
    }

    QFile sentinelFile(harness.sentinelPath());

    if (!sentinelFile.open(QIODevice::ReadOnly) ||
        (sentinelFile.size() > 4096))
    {
        PrivacyGocryptfsError releaseError = PrivacyGocryptfsError::None;
        harness.unmountStore(*lease, &releaseError);
        QDir(workspaceRoot).removeRecursively();
        return fail(QStringLiteral("the mounted store sentinel is missing"));
    }

    QString sentinelCategory;
    QString sentinelStore;

    if (!PrivacyGocryptfsSentinelCodec::decode(
            sentinelFile.readAll(), &sentinelCategory, &sentinelStore))
    {
        PrivacyGocryptfsError releaseError = PrivacyGocryptfsError::None;
        harness.unmountStore(*lease, &releaseError);
        QDir(workspaceRoot).removeRecursively();
        return fail(QStringLiteral("the mounted store sentinel is invalid"));
    }

    Private::ActiveInspection entry;
    entry.workspaceRoot = workspaceRoot;
    entry.layout = layout;
    entry.lease = std::move(lease);
    d->m_active.insert(
        harness.mountDirectory(),
        std::make_shared<Private::ActiveInspection>(std::move(entry)));

    inspection->valid = true;
    inspection->plaintextRoot = harness.mountDirectory();
    inspection->sentinelCategoryUuid = sentinelCategory;
    inspection->sentinelStoreUuid = sentinelStore;
    return true;
}

bool PrivacyGocryptfsPortableStoreInspector::release(
    const PrivacyPortableStoreInspection& inspection,
    QString* const error)
{
    const auto fail = [error](const QString& detail)
    {
        if (error)
        {
            *error = detail;
        }

        return false;
    };

    if (!inspection.valid ||
        !d->m_active.contains(inspection.plaintextRoot))
    {
        return fail(QStringLiteral("unknown store inspection"));
    }

    const std::shared_ptr<Private::ActiveInspection> entry =
        d->m_active.take(inspection.plaintextRoot);
    PrivacyGocryptfsStoreHarness harness(
        d->m_runner, d->m_mountProbe, d->m_toolPaths, entry->layout);
    PrivacyGocryptfsError harnessError = PrivacyGocryptfsError::None;
    const bool unmounted =
        harness.unmountStore(*entry->lease, &harnessError);
    entry->lease.reset();

    if (!unmounted)
    {
        return fail(QStringLiteral("the store could not be unmounted"));
    }

    QDir(entry->workspaceRoot).removeRecursively();
    return true;
}

PrivacyPortableImportAuthenticationResult
PrivacyPortableImportAuthenticator::authenticateCasual(
    const PrivacyPortableDiscoveryGroup& group,
    const QList<PrivacyPortableStrongStoreCandidate>& storeCandidates,
    const PrivacyPassword& password,
    PrivacyPortableStoreInspector& inspector,
    const CancellationCheck& isCancelled)
{
    if (!password.isValid())
    {
        return failure(PrivacyPortableImportAuthenticationStatus::InvalidRequest,
                       QStringLiteral("the category password is invalid"));
    }

    if (group.backend != PrivacyBackend::Casual)
    {
        return failure(PrivacyPortableImportAuthenticationStatus::UnsupportedBackend,
                       QStringLiteral("Casual authentication cannot verify a Strong group"));
    }

    if (group.casualArchives.isEmpty())
    {
        return failure(PrivacyPortableImportAuthenticationStatus::InvalidRequest,
                       QStringLiteral("the discovery group has no Casual archives"));
    }

    PrivacyPortableImportCandidate candidate;
    candidate.recoverySetUuid = group.recoverySetUuid;
    candidate.backend = PrivacyBackend::Casual;
    candidate.hasCredential = false;
    candidate.presentationMode =
        static_cast<int>(PrivacyPresentationMode::Generic);
    candidate.unlockedThumbnailMode =
        static_cast<int>(PrivacyUnlockedThumbnailMode::FocusedClear);
    candidate.tagVisibilityMode =
        static_cast<int>(PrivacyTagVisibilityMode::UnlockedOnly);
    PrivacyCasualArchiveEngine engine;
    QSet<QString> itemUuids;

    for (const PrivacyPortableCasualArchiveCandidate& archive :
         group.casualArchives)
    {
        if (isCancelled && isCancelled())
        {
            return failure(PrivacyPortableImportAuthenticationStatus::Cancelled,
                           QStringLiteral("import authentication was cancelled"));
        }

        PrivacyCasualArchiveError archiveError =
            PrivacyCasualArchiveError::None;
        const PrivacyCasualArchiveIdentity identity =
            engine.inspectIdentity(archive.absolutePath, &archiveError);

        if (!identity.valid ||
            (identity.recoverySetUuid != group.recoverySetUuid))
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive does not carry the group recovery identity"));
        }

        PrivacyCasualArchiveManifest manifest;

        if (!engine.verifyAndReadManifest(
                archive.absolutePath, password, identity.archiveSize,
                identity.sha256, &manifest, isCancelled, &archiveError))
        {
            if ((archiveError == PrivacyCasualArchiveError::InvalidPassword) ||
                (archiveError == PrivacyCasualArchiveError::DecryptionFailed))
            {
                return failure(
                    PrivacyPortableImportAuthenticationStatus::InvalidPassword,
                    QStringLiteral("the category password does not open the archives"));
            }

            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive could not be fully verified with the password"));
        }

        if (!manifest.isValid())
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive manifest is invalid"));
        }

        if (candidate.categoryUuid.isEmpty())
        {
            candidate.categoryUuid = manifest.categoryUuid;
        }
        else if (candidate.categoryUuid != manifest.categoryUuid)
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("archives in one recovery group belong to different categories"));
        }

        if (itemUuids.contains(manifest.itemUuid))
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("two archives publish the same protected item"));
        }

        itemUuids.insert(manifest.itemUuid);
        PrivacyPortableImportItemFact item;
        item.itemUuid = manifest.itemUuid;
        item.containerUuid = manifest.containerUuid;
        item.containerKind = PrivacyContainerKind::CasualArchive;
        item.containerRelativePath = archive.relativePath;
        item.proxyRelativePath = archive.proxyRelativePath;
        item.publicRootPath = archive.rootPath;
        item.sourceAbsolutePath = archive.absolutePath;
        item.containerSize = identity.archiveSize;
        item.containerSha256 = identity.sha256;

        for (const PrivacyCasualArchiveManifestMember& member :
             manifest.members)
        {
            PrivacyPortableImportAssetFact asset;
            asset.role = member.role;
            asset.ordinal = member.ordinal;
            asset.publicRelativePath = member.publicRelativePath;
            asset.originalName = member.originalName;
            asset.protectedRelativePath = member.protectedRelativePath;
            asset.hashAlgorithm = member.hashAlgorithm;
            asset.originalSha256 = member.sha256;
            asset.originalSize = member.size;
            asset.creationTimeUtc = member.creationTimeUtc;
            asset.modificationTimeUtc = member.modificationTimeUtc;
            asset.portableAttributes = member.portableAttributes;
            asset.unixMode = member.unixMode;

            if (!asset.isValid())
            {
                return failure(
                    PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                    QStringLiteral("an archive member fact is invalid"));
            }

            item.assets << asset;
        }

        if (!item.isValid())
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive item fact is invalid"));
        }

        candidate.items << item;
    }

    std::sort(candidate.items.begin(), candidate.items.end(),
              [](const PrivacyPortableImportItemFact& left,
                 const PrivacyPortableImportItemFact& right)
              {
                  return (left.itemUuid < right.itemUuid);
              });

    if (!candidate.isValid())
    {
        return failure(
            PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
            QStringLiteral("the authenticated import candidate is invalid"));
    }

    // Link a copied store: mount candidates with the same password and match
    // the sentinel's category UUID (and cipher-directory store UUID).
    QSet<QString> seenStoreUuids;
    PrivacyPortableStrongStoreCandidate matchedStore;
    bool storeMatched = false;

    for (const PrivacyPortableStrongStoreCandidate& store :
         storeCandidates)
    {
        if (!store.isValid() || seenStoreUuids.contains(store.storeUuid))
        {
            continue;
        }

        seenStoreUuids.insert(store.storeUuid);
        PrivacyPortableStoreInspection inspection;
        QString inspectError;

        if (!inspector.inspect(store, password, &inspection, &inspectError))
        {
            continue;
        }

        const bool identityMatches =
            (inspection.sentinelCategoryUuid == candidate.categoryUuid) &&
            (inspection.sentinelStoreUuid == store.storeUuid);
        QString releaseError;
        inspector.release(inspection, &releaseError);

        if (!identityMatches)
        {
            continue;
        }

        if (storeMatched &&
            (matchedStore.configAbsolutePath != store.configAbsolutePath))
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("two different copied stores claim the same category"));
        }

        storeMatched = true;
        matchedStore = store;
    }

    if (storeMatched)
    {
        QFile configFile(matchedStore.configAbsolutePath);

        if (!configFile.open(QIODevice::ReadOnly) ||
            (configFile.size() > 1024 * 1024))
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("the matched store configuration could not be read"));
        }

        candidate.hasCredential = true;
        candidate.storeUuid = matchedStore.storeUuid;
        candidate.managedStoreRootPath = matchedStore.rootPath;
        candidate.managedStoreMarkerRootUuid =
            matchedStore.markerRootUuid;
        candidate.managedStoreMarkerMarkerUuid =
            matchedStore.markerMarkerUuid;
        candidate.cipherRelativePath = matchedStore.cipherRelativePath;
        candidate.credentialEnvelopeFormat = QLatin1String("gocryptfs-config-v2");
        candidate.credentialEnvelopeBlob = configFile.readAll();

        if (!candidate.isValid())
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("the linked store candidate is invalid"));
        }
    }

    PrivacyPortableImportAuthenticationResult result;
    result.status = PrivacyPortableImportAuthenticationStatus::Authenticated;
    result.candidate = candidate;
    return result;
}

PrivacyPortableImportAuthenticationResult
PrivacyPortableImportAuthenticator::authenticateStrong(
    const PrivacyPortableDiscoveryGroup& group,
    const PrivacyPassword& password,
    PrivacyPortableStoreInspector& inspector,
    const CancellationCheck& isCancelled)
{
    if (!password.isValid())
    {
        return failure(PrivacyPortableImportAuthenticationStatus::InvalidRequest,
                       QStringLiteral("the category password is invalid"));
    }

    if (group.backend != PrivacyBackend::Strong)
    {
        return failure(PrivacyPortableImportAuthenticationStatus::UnsupportedBackend,
                       QStringLiteral("Strong authentication requires a Strong discovery group"));
    }

    if (group.strongStores.isEmpty())
    {
        return failure(PrivacyPortableImportAuthenticationStatus::InvalidRequest,
                       QStringLiteral("the discovery group has no Strong store"));
    }

    if (isCancelled && isCancelled())
    {
        return failure(PrivacyPortableImportAuthenticationStatus::Cancelled,
                       QStringLiteral("import authentication was cancelled"));
    }

    const PrivacyPortableStrongStoreCandidate& store =
        group.strongStores.constFirst();

    for (const PrivacyPortableStrongStoreCandidate& other :
         group.strongStores)
    {
        if (other.configAbsolutePath != store.configAbsolutePath)
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("one recovery identity has conflicting Strong stores"));
        }
    }

    PrivacyPortableStoreInspection inspection;
    QString inspectError;

    if (!inspector.inspect(store, password, &inspection, &inspectError))
    {
        if (inspectError.contains(QLatin1String("password")))
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InvalidPassword,
                inspectError);
        }

        return failure(
            PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
            inspectError);
    }

    const auto releaseInspection = [&inspector, &inspection]()
    {
        QString releaseError;
        inspector.release(inspection, &releaseError);
    };

    if ((inspection.sentinelStoreUuid != store.storeUuid) ||
        inspection.sentinelCategoryUuid.isEmpty())
    {
        releaseInspection();
        return failure(
            PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
            QStringLiteral("the store sentinel does not match the discovered store"));
    }

    PrivacyStrongRecoveryManifest manifest;
    PrivacyStrongRecoveryManifestError manifestError =
        PrivacyStrongRecoveryManifestError::None;

    if (!PrivacyStrongRecoveryManifestStore::load(
            inspection.plaintextRoot, &manifest, &manifestError) ||
        !manifest.isValid() ||
        (manifest.storeUuid != store.storeUuid))
    {
        releaseInspection();
        return failure(
            PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
            QStringLiteral("the vault recovery manifest is invalid or mismatched"));
    }

    PrivacyPortableImportCandidate candidate;
    candidate.recoverySetUuid = store.storeUuid;
    candidate.backend = PrivacyBackend::Strong;
    candidate.categoryUuid = manifest.categoryUuid;
    candidate.categoryName = manifest.categoryName;
    candidate.presentationMode = manifest.presentationMode;
    candidate.unlockedThumbnailMode = manifest.unlockedThumbnailMode;
    candidate.tagVisibilityMode = manifest.tagVisibilityMode;
    candidate.hasCredential = true;
    candidate.storeUuid = store.storeUuid;
    candidate.managedStoreRootPath = store.rootPath;
    candidate.managedStoreMarkerRootUuid = store.markerRootUuid;
    candidate.managedStoreMarkerMarkerUuid = store.markerMarkerUuid;
    candidate.cipherRelativePath = store.cipherRelativePath;
    candidate.credentialEnvelopeFormat = QLatin1String("gocryptfs-config-v2");
    QFile configFile(store.configAbsolutePath);

    if (!configFile.open(QIODevice::ReadOnly) ||
        (configFile.size() > 1024 * 1024))
    {
        releaseInspection();
        return failure(
            PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
            QStringLiteral("the store configuration could not be read"));
    }

    candidate.credentialEnvelopeBlob = configFile.readAll();
    QSet<QString> itemUuids;

    for (const PrivacyStrongRecoveryItem& recoveryItem :
         std::as_const(manifest.items))
    {
        if (isCancelled && isCancelled())
        {
            releaseInspection();
            return failure(PrivacyPortableImportAuthenticationStatus::Cancelled,
                           QStringLiteral("import authentication was cancelled"));
        }

        if (!recoveryItem.isValid() ||
            itemUuids.contains(recoveryItem.itemUuid))
        {
            releaseInspection();
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("the vault manifest has invalid or duplicate items"));
        }

        itemUuids.insert(recoveryItem.itemUuid);
        PrivacyPortableImportItemFact item;
        item.itemUuid = recoveryItem.itemUuid;
        item.containerUuid = recoveryItem.containerUuid;
        item.containerKind = PrivacyContainerKind::StrongObject;

        if (recoveryItem.members.isEmpty())
        {
            releaseInspection();
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("a vault item has no members"));
        }

        const QString containerRelativePath =
            QFileInfo(recoveryItem.members.constFirst().vaultRelativePath)
                .path();
        item.containerRelativePath = containerRelativePath;
        item.publicRootPath = store.rootPath;
        qlonglong totalSize = 0;
        QByteArray combined;
        const PrivacyStrongRecoveryMember* primary = nullptr;

        for (const PrivacyStrongRecoveryMember& member :
             recoveryItem.members)
        {
            if (!member.isValid() ||
                (QFileInfo(member.vaultRelativePath).path() !=
                 containerRelativePath))
            {
                releaseInspection();
                return failure(
                    PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                    QStringLiteral("a vault member path is invalid"));
            }

            const QString objectPath = QDir(inspection.plaintextRoot).filePath(
                member.vaultRelativePath);
            QByteArray digest;
            qlonglong size = -1;

            if (!hashFile(objectPath, &digest, &size) ||
                (digest != QByteArray::fromHex(member.sha256Hex.toLatin1())) ||
                (size != member.size))
            {
                releaseInspection();
                return failure(
                    PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                    QStringLiteral("a vault object does not match the manifest"));
            }

            combined += digest;
            totalSize += size;
            PrivacyPortableImportAssetFact asset;
            asset.role = member.role;
            asset.ordinal = member.ordinal;
            asset.publicRelativePath = member.publicRelativePath;
            asset.originalName = member.originalName;
            asset.protectedRelativePath = member.vaultRelativePath;
            asset.hashAlgorithm = QLatin1String("sha256");
            asset.originalSha256 = digest;
            asset.originalSize = size;
            asset.creationTimeUtc = member.creationTimeUtc;
            asset.modificationTimeUtc = member.modificationTimeUtc;
            asset.portableAttributes = member.portableAttributes;
            asset.unixMode = member.unixMode;
            item.assets << asset;

            if ((member.role == PrivacyAsset::PrimaryMediaRole) &&
                (member.ordinal == 0))
            {
                primary = &member;
            }
        }

        if (!primary)
        {
            releaseInspection();
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("a vault item has no primary member"));
        }

        item.proxyRelativePath = primary->publicRelativePath;
        item.containerSize = totalSize;
        item.containerSha256 =
            QCryptographicHash::hash(combined, QCryptographicHash::Sha256);

        if (!item.isValid())
        {
            releaseInspection();
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("a vault item fact is invalid"));
        }

        candidate.items << item;
    }

    releaseInspection();
    std::sort(candidate.items.begin(), candidate.items.end(),
              [](const PrivacyPortableImportItemFact& left,
                 const PrivacyPortableImportItemFact& right)
              {
                  return (left.itemUuid < right.itemUuid);
              });

    if (!candidate.isValid())
    {
        return failure(
            PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
            QStringLiteral("the authenticated Strong import candidate is invalid"));
    }

    PrivacyPortableImportAuthenticationResult result;
    result.status = PrivacyPortableImportAuthenticationStatus::Authenticated;
    result.candidate = candidate;
    return result;
}

} // namespace Digikam
