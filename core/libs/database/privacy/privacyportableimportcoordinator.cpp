/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyportableimportcoordinator.h"

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>

// Local includes

#include "coredbaccess.h"
#include "coredb.h"
#include "privacycontracts.h"
#include "privacyrepository.h"

namespace Digikam
{

namespace
{

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

PrivacyPortableImportGroupResult groupResult(
    const PrivacyPortableDiscoveryGroup& group,
    PrivacyPortableImportAuthenticationStatus status,
    const QString& detail,
    bool committed = false)
{
    PrivacyPortableImportGroupResult result;
    result.recoverySetUuid = group.recoverySetUuid;
    result.backend = group.backend;
    result.status = status;
    result.detail = detail;
    result.committed = committed;
    return result;
}

} // namespace

bool PrivacyCoreDbPortableImportCommitTarget::ensureAlbumRoot(
    int albumRootId, const QString& configuredPath,
    const QString& collectionIdentifier, PrivacyStorageRoot* const persisted)
{
    if (!persisted)
    {
        return false;
    }

    const PrivacyAlbumRootRegistrationResult result =
        PrivacyRepository().ensureAlbumRoot(
            albumRootId, configuredPath, collectionIdentifier);

    if (!result.succeeded())
    {
        return false;
    }

    *persisted = result.root;
    return true;
}

bool PrivacyCoreDbPortableImportCommitTarget::publish(
    const PrivacyPortableImportPublication& publication)
{
    CoreDbAccess access;
    return access.db()->publishPrivacyPortableImport(publication);
}

bool PrivacyPortableImportCoordinator::buildPublication(
    const PrivacyPortableImportCandidate& candidate,
    const QHash<QString, int>& albumRootIdsByPath,
    const QString& defaultCategoryName,
    PrivacyPortableImportPublication* const publication,
    QString* const detail)
{
    if (!publication || !detail || !candidate.isValid())
    {
        return false;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const qlonglong generation = candidate.hasCredential ? 1 : 0;
    const QString categoryName =
        !candidate.categoryName.isEmpty()
            ? candidate.categoryName
            : (!defaultCategoryName.isEmpty()
                   ? defaultCategoryName
                   : QStringLiteral("Imported private media"));

    PrivacyPortableImportPublication result;
    result.category.uuid = candidate.categoryUuid;
    result.category.name = categoryName;
    result.category.recoverySetUuid = candidate.recoverySetUuid;
    result.category.backend = candidate.backend;
    result.category.presentationMode =
        static_cast<PrivacyPresentationMode>(candidate.presentationMode);
    result.category.unlockedThumbnailMode =
        static_cast<PrivacyUnlockedThumbnailMode>(
            candidate.unlockedThumbnailMode);
    result.category.tagVisibilityMode =
        static_cast<PrivacyTagVisibilityMode>(candidate.tagVisibilityMode);
    result.category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    result.category.currentCredentialGeneration = generation;
    result.category.schemaVersion = 1;
    result.category.createdAt = now;
    result.hasCredential = candidate.hasCredential;

    if (candidate.hasCredential)
    {
        result.category.currentCredentialGeneration = 1;
        result.credential.categoryUuid = candidate.categoryUuid;
        result.credential.generation = 1;
        result.credential.encodingVersion = QLatin1String("utf8-nfc-v1");
        result.credential.envelopeFormat =
            candidate.credentialEnvelopeFormat;
        result.credential.envelopeBlob = candidate.credentialEnvelopeBlob;
        result.credential.envelopeHashAlgorithm = QLatin1String("sha256");
        result.credential.envelopeHash = QString::fromLatin1(
            QCryptographicHash::hash(candidate.credentialEnvelopeBlob,
                                     QCryptographicHash::Sha256).toHex());
        result.credential.recoveryRecordVersion = 1;
        result.credential.createdAt = now;

        result.managedStoreRoot.uuid = candidate.managedStoreMarkerRootUuid;
        result.managedStoreRoot.kind = PrivacyStorageRootKind::ManagedStoreRoot;
        result.managedStoreRoot.albumRootId = -1;
        result.managedStoreRoot.configuredPath =
            candidate.managedStoreRootPath;
        result.managedStoreRoot.identityVersion = 1;
        result.managedStoreRoot.identityData =
            PrivacyRootIdentityCodec::encodeManagedRootV1(
                candidate.managedStoreMarkerMarkerUuid);
        result.managedStoreRoot.markerUuid =
            candidate.managedStoreMarkerMarkerUuid;
        result.managedStoreRoot.createdAt = now;

        result.store.uuid = candidate.storeUuid;
        result.store.categoryUuid = candidate.categoryUuid;
        result.store.rootUuid = result.managedStoreRoot.uuid;
        result.store.format = QLatin1String("gocryptfs");
        result.store.formatVersion = 2;
        result.store.cipherRelativePath = candidate.cipherRelativePath;
        result.store.configRelativePath =
            candidate.cipherRelativePath + QLatin1String("/gocryptfs.conf");
        result.store.configGeneration = 1;
        result.store.lifecycleState = PrivacyStoreLifecycleState::Active;
        result.store.schemaVersion = 1;
        result.store.createdAt = now;

        for (const PrivacyStoreRole role :
             { PrivacyStoreRole::CredentialAuthority,
               PrivacyStoreRole::Derivatives,
               PrivacyStoreRole::Originals })
        {
            PrivacyStoreBinding binding;
            binding.categoryUuid = candidate.categoryUuid;
            binding.role = role;
            binding.storeUuid = candidate.storeUuid;
            binding.schemaVersion = 1;
            result.storeBindings << binding;
        }
    }

    QHash<QString, PrivacyStorageRoot> rootsByPath;
    QHash<QString, int> albumRootIds;

    for (const PrivacyPortableImportItemFact& item : candidate.items)
    {
        if (!rootsByPath.contains(item.publicRootPath))
        {
            const auto albumRootIdIt =
                albumRootIdsByPath.constFind(item.publicRootPath);

            if (albumRootIdIt == albumRootIdsByPath.constEnd())
            {
                *detail = QStringLiteral(
                    "no registered album root for the imported proxy location");
                return false;
            }

            PrivacyStorageRoot persisted;

            if (!m_commitTarget.ensureAlbumRoot(
                    albumRootIdIt.value(), item.publicRootPath,
                    QLatin1String("privacy-import"), &persisted))
            {
                *detail = QStringLiteral(
                    "the public collection root could not be registered");
                return false;
            }

            rootsByPath.insert(item.publicRootPath, persisted);
            albumRootIds.insert(item.publicRootPath, albumRootIdIt.value());
        }
    }

    result.albumRoots = rootsByPath.values();

    for (const PrivacyPortableImportItemFact& item : candidate.items)
    {
        const PrivacyStorageRoot root =
            rootsByPath.value(item.publicRootPath);
        const QString proxyAbsolutePath =
            QDir(item.publicRootPath).filePath(item.proxyRelativePath);
        QByteArray proxySha256;
        qlonglong proxySize = -1;

        if (!hashFile(proxyAbsolutePath, &proxySha256, &proxySize))
        {
            *detail = QStringLiteral(
                "the public proxy could not be read for import");
            return false;
        }

        const QString proxyHashHex =
            QString::fromLatin1(proxySha256.toHex());
        const QFileInfo proxyInfo(proxyAbsolutePath);
        const PrivacyPortableImportAssetFact* primary = nullptr;

        for (const PrivacyPortableImportAssetFact& asset : item.assets)
        {
            if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                primary = &asset;
                break;
            }
        }

        if (!primary)
        {
            *detail = QStringLiteral("an imported item has no primary member");
            return false;
        }

        PrivacyPortableImportImageFact imageFact;
        imageFact.albumRootId = albumRootIds.value(item.publicRootPath);
        imageFact.publicRelativePath = item.proxyRelativePath;
        imageFact.proxyHashHex = proxyHashHex;
        imageFact.proxySize = proxySize;
        imageFact.modificationDate = proxyInfo.lastModified();

        PrivacyItem privacyItem;
        privacyItem.imageId = -1;
        privacyItem.uuid = item.itemUuid;
        privacyItem.categoryUuid = candidate.categoryUuid;
        privacyItem.originalHash = QString::fromLatin1(
            primary->originalSha256.toHex());
        privacyItem.originalSize = primary->originalSize;
        privacyItem.expectedProxyHash = proxyHashHex;
        privacyItem.expectedProxySize = proxySize;
        privacyItem.presentationVersion = 1;
        privacyItem.generation = 1;
        privacyItem.transactionState =
            static_cast<int>(PrivacyTransactionState::Complete);

        PrivacyContainer container;
        container.uuid = item.containerUuid;
        container.itemUuid = item.itemUuid;
        container.kind = item.containerKind;

        if (item.containerKind == PrivacyContainerKind::CasualArchive)
        {
            container.rootUuid = root.uuid;
        }
        else
        {
            container.storeUuid = candidate.storeUuid;
        }

        container.objectRelativePath = item.containerRelativePath;
        container.protectedSize = item.containerSize;
        container.protectedHashAlgorithm = QLatin1String("sha256");
        container.protectedHash = QString::fromLatin1(
            item.containerSha256.toHex());
        container.formatVersion = 1;
        container.credentialGeneration = generation;
        container.state = PrivacyContainerState::Verified;
        container.createdAt = now;
        container.updatedAt = now;

        result.imageFacts << imageFact;
        result.items << privacyItem;
        result.containers << container;

        for (const PrivacyPortableImportAssetFact& asset : item.assets)
        {
            PrivacyAsset privacyAsset;
            privacyAsset.itemUuid = item.itemUuid;
            privacyAsset.role = asset.role;
            privacyAsset.ordinal = asset.ordinal;
            privacyAsset.originalName = asset.originalName;
            privacyAsset.publicRootUuid = root.uuid;
            privacyAsset.publicRelativePath = asset.publicRelativePath;
            privacyAsset.containerUuid = item.containerUuid;
            privacyAsset.protectedRelativePath =
                asset.protectedRelativePath;
            privacyAsset.hashAlgorithm = asset.hashAlgorithm;
            privacyAsset.originalHash = QString::fromLatin1(
                asset.originalSha256.toHex());
            privacyAsset.originalSize = asset.originalSize;
            privacyAsset.originalCreationDate = asset.creationTimeUtc;
            privacyAsset.originalModificationDate =
                asset.modificationTimeUtc;
            privacyAsset.portableAttributes = asset.portableAttributes;

            if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                privacyAsset.proxyHashAlgorithm = QLatin1String("sha256");
                privacyAsset.proxyHash = proxyHashHex;
                privacyAsset.proxySize = proxySize;
                privacyAsset.proxyPresentationVersion = 1;
                privacyAsset.proxyGeneration = 1;
            }

            result.assets << privacyAsset;
        }
    }

    if (!result.isValid())
    {
        *detail = QStringLiteral("the built import publication is invalid");
        return false;
    }

    *publication = result;
    return true;
}

PrivacyPortableImportCoordinatorResult
PrivacyPortableImportCoordinator::run(
    const QList<QString>& scanRoots,
    const QHash<QString, QString>& passwordsByRecoverySet,
    const QHash<QString, int>& albumRootIdsByPath,
    const QString& defaultCategoryName,
    PrivacyPortableStoreInspector& inspector,
    const CancellationCheck& isCancelled)
{
    PrivacyPortableImportCoordinatorResult result;
    const PrivacyPortableDiscoveryResult discovery =
        PrivacyPortableDiscovery::scan(scanRoots, isCancelled);
    result.issues = discovery.issues;
    result.cancelled = discovery.cancelled;

    if (result.cancelled)
    {
        return result;
    }

    QList<PrivacyPortableStrongStoreCandidate> storeCandidates;

    for (const PrivacyPortableDiscoveryGroup& group : discovery.groups)
    {
        for (const PrivacyPortableStrongStoreCandidate& store :
             group.strongStores)
        {
            storeCandidates << store;
        }
    }

    for (const PrivacyPortableDiscoveryGroup& group : discovery.groups)
    {
        if (isCancelled && isCancelled())
        {
            result.cancelled = true;
            break;
        }

        const auto passwordIt =
            passwordsByRecoverySet.constFind(group.recoverySetUuid);

        if (passwordIt == passwordsByRecoverySet.constEnd())
        {
            result.groups << groupResult(
                group, PrivacyPortableImportAuthenticationStatus::InvalidRequest,
                QStringLiteral("no password supplied for this recovery group"));
            continue;
        }

        const PrivacyPassword password =
            PrivacyPassword::fromUnicode(passwordIt.value());

        if (!password.isValid())
        {
            result.groups << groupResult(
                group, PrivacyPortableImportAuthenticationStatus::InvalidPassword,
                QStringLiteral("the supplied password is invalid"));
            continue;
        }

        PrivacyPortableImportAuthenticationResult authentication;

        if (group.backend == PrivacyBackend::Casual)
        {
            authentication = PrivacyPortableImportAuthenticator::authenticateCasual(
                group, storeCandidates, password, inspector, isCancelled);
        }
        else
        {
            authentication = PrivacyPortableImportAuthenticator::authenticateStrong(
                group, password, inspector, isCancelled);
        }

        if (!authentication.succeeded())
        {
            result.groups << groupResult(
                group, authentication.status, authentication.detail);
            continue;
        }

        PrivacyPortableImportPublication publication;
        QString detail;

        if (!buildPublication(authentication.candidate, albumRootIdsByPath,
                              defaultCategoryName, &publication, &detail) ||
            !m_commitTarget.publish(publication))
        {
            result.groups << groupResult(
                group, PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                detail.isEmpty()
                    ? QStringLiteral("the category could not be published")
                    : detail);
            continue;
        }

        result.groups << groupResult(
            group, PrivacyPortableImportAuthenticationStatus::Authenticated,
            QString(), true);
    }

    return result;
}

PrivacyPortableImportCoordinator::PrivacyPortableImportCoordinator(
    PrivacyPortableImportCommitTarget& commitTarget)
    : m_commitTarget(commitTarget)
{
}

} // namespace Digikam
