/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacytypes.h"

// Qt includes

#include <algorithm>
#include <QDir>
#include <QSet>
#include <QStringList>
#include <QUuid>

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

bool isOptionalCanonicalUuid(const QString& uuid)
{
    return (uuid.isEmpty() || isCanonicalUuid(uuid));
}

bool isValidRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) || path.contains(QLatin1Char('\0')))
    {
        return false;
    }

    const QStringList parts = path.split(QLatin1Char('/'));

    for (const QString& part : parts)
    {
        if (part.isEmpty() || (part == QLatin1String(".")) || (part == QLatin1String("..")))
        {
            return false;
        }
    }

    return true;
}

} // namespace

bool PrivacyCategory::isValid() const
{
    const bool validBackend = ((backend == PrivacyBackend::Casual) ||
                               (backend == PrivacyBackend::Strong));
    const bool validPresentation = ((presentationMode == PrivacyPresentationMode::Blur) ||
                                    (presentationMode == PrivacyPresentationMode::Generic));
    const bool validUnlockedThumbnailMode =
        ((unlockedThumbnailMode == PrivacyUnlockedThumbnailMode::AlwaysOpaque) ||
         (unlockedThumbnailMode == PrivacyUnlockedThumbnailMode::FocusedClear) ||
         (unlockedThumbnailMode == PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked));
    const bool validTagVisibilityMode =
        ((tagVisibilityMode == PrivacyTagVisibilityMode::UnlockedOnly) ||
         (tagVisibilityMode == PrivacyTagVisibilityMode::AlwaysVisible));
    const bool validLifecycle = ((lifecycleState == PrivacyCategoryLifecycleState::Creating)           ||
                                 (lifecycleState == PrivacyCategoryLifecycleState::Active)             ||
                                 (lifecycleState == PrivacyCategoryLifecycleState::TransactionBlocked) ||
                                 (lifecycleState == PrivacyCategoryLifecycleState::Error));

    return (isCanonicalUuid(uuid)             &&
            !name.trimmed().isEmpty()         &&
            isCanonicalUuid(recoverySetUuid)  &&
            validBackend                      &&
            validPresentation                 &&
            validUnlockedThumbnailMode        &&
            validTagVisibilityMode            &&
            validLifecycle                    &&
            (currentCredentialGeneration >= 0) &&
            (schemaVersion > 0)               &&
            createdAt.isValid());
}

bool PrivacyCredential::isValid() const
{
    return (isCanonicalUuid(categoryUuid)                     &&
            (generation >= 0)                                &&
            (encodingVersion == QLatin1String("utf8-nfc-v1")) &&
            !envelopeFormat.isEmpty()                        &&
            !envelopeBlob.isEmpty()                          &&
            !envelopeHashAlgorithm.isEmpty()                 &&
            !envelopeHash.isEmpty()                          &&
            (recoveryMode >= 0)                              &&
            (recoveryState >= 0)                             &&
            (recoveryRecordVersion > 0)                      &&
            isOptionalCanonicalUuid(recoveryDocumentUuid)    &&
            createdAt.isValid());
}

bool PrivacyStorageRoot::isValid() const
{
    const bool validKind = (((kind == PrivacyStorageRootKind::AlbumRoot) && (albumRootId > 0)) ||
                            ((kind == PrivacyStorageRootKind::ManagedStoreRoot) && (albumRootId < 0)));
    const bool validMarker = (kind == PrivacyStorageRootKind::ManagedStoreRoot)
                           ? isCanonicalUuid(markerUuid)
                           : isOptionalCanonicalUuid(markerUuid);

    return (isCanonicalUuid(uuid)               &&
            validKind                           &&
            !configuredPath.isEmpty()           &&
            (identityVersion > 0)               &&
            !identityData.isEmpty()             &&
            validMarker                         &&
            (schemaVersion > 0)                 &&
            createdAt.isValid());
}

bool PrivacyStore::isValid() const
{
    const bool validLifecycle = ((lifecycleState == PrivacyStoreLifecycleState::Creating)  ||
                                 (lifecycleState == PrivacyStoreLifecycleState::Active)    ||
                                 (lifecycleState == PrivacyStoreLifecycleState::Migrating) ||
                                 (lifecycleState == PrivacyStoreLifecycleState::Error));

    const bool configInsideCipher =
        configRelativePath.startsWith(cipherRelativePath + QLatin1Char('/'));

    return (isCanonicalUuid(uuid)              &&
            isCanonicalUuid(categoryUuid)      &&
            isCanonicalUuid(rootUuid)          &&
            !format.isEmpty()                  &&
            (formatVersion > 0)                &&
            isValidRelativePath(cipherRelativePath) &&
            isValidRelativePath(configRelativePath) &&
            configInsideCipher                  &&
            (configGeneration >= -1)           &&
            validLifecycle                     &&
            (schemaVersion > 0)                &&
            createdAt.isValid());
}

bool PrivacyStoreBinding::isValid() const
{
    const bool validRole = ((role == PrivacyStoreRole::CredentialAuthority) ||
                            (role == PrivacyStoreRole::Originals)           ||
                            (role == PrivacyStoreRole::Derivatives));

    return (isCanonicalUuid(categoryUuid) &&
            validRole                     &&
            isCanonicalUuid(storeUuid)    &&
            (schemaVersion > 0));
}

bool PrivacyContainer::isValid() const
{
    const bool validKind = ((kind == PrivacyContainerKind::CasualArchive) ||
                            (kind == PrivacyContainerKind::StrongObject));
    const bool validState = ((state == PrivacyContainerState::Creating)  ||
                             (state == PrivacyContainerState::Verified)  ||
                             (state == PrivacyContainerState::Rewriting) ||
                             (state == PrivacyContainerState::Error));
    const bool rootLocation = (isCanonicalUuid(rootUuid) && storeUuid.isEmpty());
    const bool storeLocation = (rootUuid.isEmpty() && isCanonicalUuid(storeUuid));
    const bool validLocation = (((kind == PrivacyContainerKind::CasualArchive) && rootLocation) ||
                                ((kind == PrivacyContainerKind::StrongObject) && storeLocation));
    const bool validObjectPath =
        (((kind == PrivacyContainerKind::CasualArchive) &&
          objectRelativePath.endsWith(QLatin1String(".digikam-private.zip"))) ||
         ((kind == PrivacyContainerKind::StrongObject) &&
          objectRelativePath.startsWith(QLatin1String("originals/"))));

    return (isCanonicalUuid(uuid)              &&
            isCanonicalUuid(itemUuid)          &&
            validKind                          &&
            validLocation                      &&
            isValidRelativePath(objectRelativePath) &&
            validObjectPath                    &&
            (protectedSize >= 0)               &&
            !protectedHashAlgorithm.isEmpty()  &&
            !protectedHash.isEmpty()           &&
            (formatVersion > 0)                &&
            (credentialGeneration >= 0)        &&
            validState                         &&
            createdAt.isValid()                &&
            updatedAt.isValid());
}

bool PrivacyAsset::isValid() const
{
    const bool noProxy = (proxyHashAlgorithm.isEmpty() && proxyHash.isEmpty() &&
                          (proxySize < 0) && (proxyPresentationVersion == 0) &&
                          (proxyGeneration < 0));
    const bool validProxy = (!proxyHashAlgorithm.isEmpty() && !proxyHash.isEmpty() &&
                             (proxySize >= 0) && (proxyPresentationVersion > 0) &&
                             (proxyGeneration >= 0));

    return (isCanonicalUuid(itemUuid)             &&
            (role > 0)                            &&
            (ordinal >= 0)                        &&
            !originalName.isEmpty()               &&
            isCanonicalUuid(publicRootUuid)       &&
            isValidRelativePath(publicRelativePath) &&
            isCanonicalUuid(containerUuid)        &&
            isValidRelativePath(protectedRelativePath) &&
            !hashAlgorithm.isEmpty()              &&
            !originalHash.isEmpty()               &&
            (originalSize >= 0)                   &&
            (noProxy || validProxy));
}

bool PrivacyDerivative::isValid() const
{
    const bool validKind = ((kind == PrivacyDerivativeKind::ClearThumbnail) ||
                            (kind == PrivacyDerivativeKind::BlurredPresentation));

    return (isCanonicalUuid(itemUuid)             &&
            validKind                             &&
            (ordinal >= 0)                        &&
            isCanonicalUuid(storeUuid)            &&
            isValidRelativePath(protectedRelativePath) &&
            protectedRelativePath.startsWith(QLatin1String("derivatives/")) &&
            !sourceHashAlgorithm.isEmpty()        &&
            !sourceOriginalHash.isEmpty()         &&
            !derivativeFormat.isEmpty()           &&
            !derivativeHashAlgorithm.isEmpty()    &&
            !derivativeHash.isEmpty()             &&
            (derivativeSize >= 0)                 &&
            (presentationVersion > 0)             &&
            (generation >= 0)                     &&
            createdAt.isValid());
}

bool PrivacyTransaction::isValid() const
{
    const bool validType = ((type == PrivacyTransactionType::ProtectItem)          ||
                            (type == PrivacyTransactionType::UnprotectItem)        ||
                            (type == PrivacyTransactionType::ChangePassword)       ||
                            (type == PrivacyTransactionType::MigrateBackend)       ||
                            (type == PrivacyTransactionType::CompatibilityUnlock)  ||
                            (type == PrivacyTransactionType::ExternalCheckout)     ||
                            (type == PrivacyTransactionType::DeleteProtectedItem)  ||
                            (type == PrivacyTransactionType::ChangePresentation)     ||
                            (type == PrivacyTransactionType::CreateCategory));

    const bool validState = ((state == PrivacyTransactionState::Created)             ||
                             (state == PrivacyTransactionState::Prepared)            ||
                             (state == PrivacyTransactionState::Applying)            ||
                             (state == PrivacyTransactionState::Exposed)             ||
                             (state == PrivacyTransactionState::Relocking)           ||
                             (state == PrivacyTransactionState::NeedsReconciliation) ||
                             (state == PrivacyTransactionState::Complete)            ||
                             (state == PrivacyTransactionState::Error));

    return (isCanonicalUuid(uuid)                &&
            isCanonicalUuid(categoryUuid)        &&
            isOptionalCanonicalUuid(itemUuid)    &&
            validType                            &&
            validState                           &&
            (generation >= 0)                    &&
            (fromCredentialGeneration >= -1)     &&
            (toCredentialGeneration >= -1)       &&
            (payloadFormatVersion > 0)           &&
            createdAt.isValid()                  &&
            updatedAt.isValid());
}

bool PrivacyTransaction::isActive() const
{
    return (isValid() && (state != PrivacyTransactionState::Complete));
}

bool PrivacyTransactionJournal::isValid() const
{
    const bool noHash = (expectedHashAlgorithm.isEmpty() && expectedJournalHash.isEmpty());
    const bool validHash = (!expectedHashAlgorithm.isEmpty() && !expectedJournalHash.isEmpty());

    return (isCanonicalUuid(transactionUuid)        &&
            isCanonicalUuid(rootUuid)               &&
            isValidRelativePath(journalRelativePath) &&
            (journalFormatVersion > 0)              &&
            (stage >= 0)                            &&
            (noHash || validHash)                   &&
            updatedAt.isValid());
}

bool PrivacyItem::isValid() const
{
    return ((imageId > 0)                    &&
            isCanonicalUuid(uuid)            &&
            isCanonicalUuid(categoryUuid)    &&
            (presentationVersion > 0)        &&
            (generation >= 0)                &&
            (transactionState >= 0));
}

bool PrivacyPortableImportImageFact::isValid() const
{
    const QByteArray hash = proxyHashHex.toLatin1();
    const bool validHash =
        (hash.size() == 64) &&
        std::all_of(hash.cbegin(), hash.cend(),
                    [](char character)
                    {
                        return (((character >= '0') && (character <= '9')) ||
                                ((character >= 'a') && (character <= 'f')));
                    });

    return ((albumRootId > 0) &&
            isValidRelativePath(publicRelativePath) &&
            validHash && (proxySize >= 0) &&
            modificationDate.isValid());
}

bool PrivacyPortableImportPublication::isValid() const
{
    if (!category.isValid() ||
        (category.lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        items.isEmpty() || containers.isEmpty() || assets.isEmpty() ||
        (items.size() != containers.size()) ||
        (items.size() != imageFacts.size()) ||
        albumRoots.isEmpty())
    {
        return false;
    }

    if (hasCredential)
    {
        if ((category.currentCredentialGeneration < 1) ||
            !credential.isValid() ||
            (credential.categoryUuid != category.uuid) ||
            (credential.generation != category.currentCredentialGeneration) ||
            !managedStoreRoot.isValid() ||
            (managedStoreRoot.kind !=
             PrivacyStorageRootKind::ManagedStoreRoot) ||
            !store.isValid() ||
            (store.categoryUuid != category.uuid) ||
            (store.rootUuid != managedStoreRoot.uuid) ||
            (store.lifecycleState != PrivacyStoreLifecycleState::Active) ||
            (store.configGeneration != category.currentCredentialGeneration) ||
            storeBindings.isEmpty())
        {
            return false;
        }

        for (const PrivacyStoreBinding& binding : storeBindings)
        {
            if ((binding.categoryUuid != category.uuid) ||
                (binding.storeUuid != store.uuid) || !binding.isValid())
            {
                return false;
            }
        }
    }
    else
    {
        if ((category.currentCredentialGeneration != 0) ||
            managedStoreRoot.isValid() || store.isValid() ||
            !storeBindings.isEmpty())
        {
            return false;
        }
    }

    for (const PrivacyStorageRoot& root : albumRoots)
    {
        if (!root.isValid() ||
            (root.kind != PrivacyStorageRootKind::AlbumRoot))
        {
            return false;
        }
    }

    for (int i = 0 ; i < items.size() ; ++i)
    {
        const PrivacyItem& item = items.at(i);
        const PrivacyContainer& container = containers.at(i);

        if (!imageFacts.at(i).isValid() ||
            !isCanonicalUuid(item.uuid) ||
            (item.categoryUuid != category.uuid) ||
            (item.presentationVersion <= 0) || (item.generation < 0) ||
            (item.transactionState !=
             static_cast<int>(PrivacyTransactionState::Complete)) ||
            item.expectedProxyHash.isEmpty() || (item.expectedProxySize < 0) ||
            !container.isValid() || (container.itemUuid != item.uuid))
        {
            return false;
        }

        bool foundPrimary = false;

        for (const PrivacyAsset& asset : assets)
        {
            if (asset.itemUuid != item.uuid)
            {
                continue;
            }

            if (!asset.isValid() ||
                (asset.containerUuid != container.uuid))
            {
                return false;
            }

            if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                if (foundPrimary ||
                    (asset.publicRelativePath !=
                     imageFacts.at(i).publicRelativePath) ||
                    (asset.proxyHash != imageFacts.at(i).proxyHashHex) ||
                    (asset.proxySize != imageFacts.at(i).proxySize))
                {
                    return false;
                }

                foundPrimary = true;
            }
        }

        if (!foundPrimary)
        {
            return false;
        }
    }

    QSet<QString> itemUuids;
    QSet<QString> containerUuids;

    for (const PrivacyItem& item : items)
    {
        if (itemUuids.contains(item.uuid))
        {
            return false;
        }

        itemUuids.insert(item.uuid);
    }

    for (const PrivacyContainer& container : containers)
    {
        if (containerUuids.contains(container.uuid))
        {
            return false;
        }

        containerUuids.insert(container.uuid);
    }

    return true;
}

} // namespace Digikam
