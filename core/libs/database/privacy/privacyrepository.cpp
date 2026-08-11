/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyrepository.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QHash>
#include <QSet>

// C++ includes

#include <utility>

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "privacycontracts.h"
#include "privacytransactionjournal.h"

namespace Digikam
{

namespace
{

QString normalizedUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return parsed.isNull() ? QString() : parsed.toString(QUuid::WithoutBraces);
}

template <typename T>
bool allRecordsValid(const QList<T>& records)
{
    for (const T& record : records)
    {
        if (!record.isValid())
        {
            return false;
        }
    }

    return true;
}

QString generationKey(const QString& categoryUuid, qlonglong generation)
{
    return categoryUuid + QLatin1Char(':') + QString::number(generation);
}

} // namespace

bool PrivacyAlbumRootRegistrationResult::succeeded() const
{
    return ((status == PrivacyAlbumRootRegistrationStatus::Created) ||
            (status == PrivacyAlbumRootRegistrationStatus::Existing));
}

bool PrivacyRepository::createCategory(const PrivacyCategory& category) const
{
    PrivacyCategory normalized = category;
    normalized.uuid             = normalizedUuid(category.uuid);
    normalized.name             = category.name.trimmed();

    if (!normalized.createdAt.isValid())
    {
        normalized.createdAt = QDateTime::currentDateTimeUtc();
    }

    if (!normalized.isValid())
    {
        return false;
    }

    CoreDbAccess access;

    return access.db()->insertPrivacyCategory(normalized);
}

PrivacyCategory PrivacyRepository::category(const QString& uuid) const
{
    const QString normalized = normalizedUuid(uuid);

    if (normalized.isEmpty())
    {
        return PrivacyCategory();
    }

    CoreDbAccess access;

    return access.db()->getPrivacyCategory(normalized);
}

bool PrivacyRepository::setCategoryUnlockedThumbnailMode(
    const QString& uuid,
    PrivacyUnlockedThumbnailMode mode,
    bool categoryAuthenticationVerified) const
{
    const QString normalized = normalizedUuid(uuid);

    if (normalized.isEmpty() || !categoryAuthenticationVerified ||
        ((mode != PrivacyUnlockedThumbnailMode::AlwaysOpaque) &&
         (mode != PrivacyUnlockedThumbnailMode::FocusedClear) &&
         (mode != PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked)))
    {
        return false;
    }

    CoreDbAccess access;

    return access.db()->updatePrivacyCategoryUnlockedThumbnailMode(normalized, mode);
}

bool PrivacyRepository::setCategoryTagVisibilityMode(
    const QString& uuid,
    PrivacyTagVisibilityMode mode,
    bool categoryAuthenticationVerified) const
{
    const QString normalized = normalizedUuid(uuid);

    if (normalized.isEmpty() ||
        ((mode != PrivacyTagVisibilityMode::UnlockedOnly) &&
         (mode != PrivacyTagVisibilityMode::AlwaysVisible)) ||
        ((mode == PrivacyTagVisibilityMode::AlwaysVisible) &&
         !categoryAuthenticationVerified))
    {
        return false;
    }

    CoreDbAccess access;

    return access.db()->updatePrivacyCategoryTagVisibilityMode(normalized, mode);
}

bool PrivacyRepository::addCredential(const PrivacyCredential& credential) const
{
    PrivacyCredential normalized = credential;
    normalized.categoryUuid       = normalizedUuid(credential.categoryUuid);
    normalized.recoveryDocumentUuid = credential.recoveryDocumentUuid.isEmpty()
                                    ? QString() : normalizedUuid(credential.recoveryDocumentUuid);

    if (!normalized.createdAt.isValid())
    {
        normalized.createdAt = QDateTime::currentDateTimeUtc();
    }

    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyCredential(normalized);
}

bool PrivacyRepository::addStorageRoot(const PrivacyStorageRoot& root) const
{
    PrivacyStorageRoot normalized = root;
    normalized.uuid               = normalizedUuid(root.uuid);
    normalized.markerUuid         = root.markerUuid.isEmpty() ? QString() : normalizedUuid(root.markerUuid);

    if (!normalized.createdAt.isValid())
    {
        normalized.createdAt = QDateTime::currentDateTimeUtc();
    }

    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyStorageRoot(normalized);
}

PrivacyAlbumRootRegistrationResult PrivacyRepository::ensureAlbumRoot(
    int albumRootId,
    const QString& configuredPath,
    const QString& collectionIdentifier) const
{
    PrivacyAlbumRootRegistrationResult result;
    const QString cleanPath = QDir::cleanPath(configuredPath);
    const QFileInfo pathInfo(cleanPath);

    if ((albumRootId <= 0) || collectionIdentifier.isEmpty() ||
        collectionIdentifier.contains(QChar::Null) ||
        !QDir::isAbsolutePath(cleanPath) || (cleanPath == QLatin1String("/")) ||
        !pathInfo.isDir() || (pathInfo.canonicalFilePath() != cleanPath))
    {
        result.status = PrivacyAlbumRootRegistrationStatus::Offline;
        return result;
    }

    CoreDbAccess access;
    QList<PrivacyStorageRoot> roots;

    if (!access.db()->getPrivacyStorageRoots(&roots))
    {
        return result;
    }

    const auto classifyExisting = [albumRootId, &collectionIdentifier, &result]
                                  (const QList<PrivacyStorageRoot>& records)
    {
        int matches = 0;

        for (const PrivacyStorageRoot& root : records)
        {
            if ((root.kind != PrivacyStorageRootKind::AlbumRoot) ||
                (root.albumRootId != albumRootId))
            {
                continue;
            }

            ++matches;
            result.root = root;
        }

        if (matches > 1)
        {
            result.status = PrivacyAlbumRootRegistrationStatus::Conflict;
            return true;
        }

        if (matches == 1)
        {
            if (!result.root.isValid())
            {
                result.status = PrivacyAlbumRootRegistrationStatus::Conflict;
                return true;
            }

            result.status = (result.root.identityVersion == 1) &&
                            PrivacyRootIdentityCodec::matchesAlbumRootV1(
                                result.root.identityData, albumRootId,
                                collectionIdentifier)
                          ? PrivacyAlbumRootRegistrationStatus::Existing
                          : PrivacyAlbumRootRegistrationStatus::IdentityMismatch;
            return true;
        }

        return false;
    };

    if (classifyExisting(roots))
    {
        return result;
    }

    PrivacyStorageRoot candidate;
    candidate.uuid            = QUuid::createUuid().toString(QUuid::WithoutBraces);
    candidate.kind            = PrivacyStorageRootKind::AlbumRoot;
    candidate.albumRootId     = albumRootId;
    candidate.configuredPath  = cleanPath;
    candidate.identityVersion = 1;
    candidate.identityData    = PrivacyRootIdentityCodec::encodeAlbumRootV1(
                                    albumRootId, collectionIdentifier);
    candidate.createdAt       = QDateTime::currentDateTimeUtc();
    bool created = false;

    if (candidate.isValid() &&
        access.db()->ensurePrivacyAlbumRoot(candidate, &result.root, &created))
    {
        if (!result.root.isValid())
        {
            result.status = PrivacyAlbumRootRegistrationStatus::Conflict;
        }
        else if ((result.root.identityVersion != 1) ||
                 !PrivacyRootIdentityCodec::matchesAlbumRootV1(
                result.root.identityData, albumRootId, collectionIdentifier))
        {
            result.status = PrivacyAlbumRootRegistrationStatus::IdentityMismatch;
        }
        else
        {
            result.status = created ? PrivacyAlbumRootRegistrationStatus::Created
                                    : PrivacyAlbumRootRegistrationStatus::Existing;
        }

        return result;
    }

    // A concurrent process may have won the unique album-root insertion.
    // Re-read once and converge on its durable UUID when the identity agrees.

    roots.clear();

    if (access.db()->getPrivacyStorageRoots(&roots) && classifyExisting(roots))
    {
        return result;
    }

    result.root = PrivacyStorageRoot();
    result.status = PrivacyAlbumRootRegistrationStatus::StorageFailure;

    return result;
}

bool PrivacyRepository::removeUnreferencedAlbumRoot(const QString& uuid,
                                                    bool* const absent) const
{
    const QString normalized = normalizedUuid(uuid);

    if (!absent || normalized.isEmpty())
    {
        return false;
    }

    CoreDbAccess access;

    return access.db()->removeUnreferencedPrivacyAlbumRoot(normalized, absent);
}

bool PrivacyRepository::pruneUnreferencedAlbumRoots(
    QStringList* const removedUuids) const
{
    if (removedUuids)
    {
        removedUuids->clear();
    }

    CoreDbAccess access;
    QList<PrivacyStorageRoot> roots;

    if (!access.db()->getPrivacyStorageRoots(&roots))
    {
        return false;
    }

    for (const PrivacyStorageRoot& root : std::as_const(roots))
    {
        if (root.kind != PrivacyStorageRootKind::AlbumRoot)
        {
            continue;
        }

        bool absent = false;

        if (!access.db()->removeUnreferencedPrivacyAlbumRoot(root.uuid, &absent))
        {
            return false;
        }

        if (absent && removedUuids)
        {
            removedUuids->append(root.uuid);
        }
    }

    return true;
}

bool PrivacyRepository::addStore(const PrivacyStore& store) const
{
    PrivacyStore normalized = store;
    normalized.uuid         = normalizedUuid(store.uuid);
    normalized.categoryUuid = normalizedUuid(store.categoryUuid);
    normalized.rootUuid     = normalizedUuid(store.rootUuid);

    if (!normalized.createdAt.isValid())
    {
        normalized.createdAt = QDateTime::currentDateTimeUtc();
    }

    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyStore(normalized);
}

bool PrivacyRepository::addStoreBinding(const PrivacyStoreBinding& binding) const
{
    PrivacyStoreBinding normalized = binding;
    normalized.categoryUuid        = normalizedUuid(binding.categoryUuid);
    normalized.storeUuid           = normalizedUuid(binding.storeUuid);
    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyStoreBinding(normalized);
}

bool PrivacyRepository::mapItem(const PrivacyItem& item) const
{
    PrivacyItem normalized = item;
    normalized.uuid         = normalizedUuid(item.uuid);
    normalized.categoryUuid = normalizedUuid(item.categoryUuid);

    if (!normalized.isValid())
    {
        return false;
    }

    CoreDbAccess access;

    return access.db()->insertPrivacyItem(normalized);
}

PrivacyItem PrivacyRepository::itemForImageId(qlonglong imageId) const
{
    if (imageId <= 0)
    {
        return PrivacyItem();
    }

    CoreDbAccess access;

    return access.db()->getPrivacyItem(imageId);
}

bool PrivacyRepository::addContainer(const PrivacyContainer& container) const
{
    PrivacyContainer normalized = container;
    normalized.uuid             = normalizedUuid(container.uuid);
    normalized.itemUuid         = normalizedUuid(container.itemUuid);
    normalized.rootUuid         = container.rootUuid.isEmpty() ? QString() : normalizedUuid(container.rootUuid);
    normalized.storeUuid        = container.storeUuid.isEmpty() ? QString() : normalizedUuid(container.storeUuid);

    if (!normalized.createdAt.isValid())
    {
        normalized.createdAt = QDateTime::currentDateTimeUtc();
    }

    if (!normalized.updatedAt.isValid())
    {
        normalized.updatedAt = normalized.createdAt;
    }

    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyContainer(normalized);
}

bool PrivacyRepository::addAsset(const PrivacyAsset& asset) const
{
    PrivacyAsset normalized       = asset;
    normalized.itemUuid           = normalizedUuid(asset.itemUuid);
    normalized.publicRootUuid     = normalizedUuid(asset.publicRootUuid);
    normalized.containerUuid      = normalizedUuid(asset.containerUuid);
    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyAsset(normalized);
}

bool PrivacyRepository::addDerivative(const PrivacyDerivative& derivative) const
{
    PrivacyDerivative normalized = derivative;
    normalized.itemUuid          = normalizedUuid(derivative.itemUuid);
    normalized.storeUuid         = normalizedUuid(derivative.storeUuid);

    if (!normalized.createdAt.isValid())
    {
        normalized.createdAt = QDateTime::currentDateTimeUtc();
    }

    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyDerivative(normalized);
}

bool PrivacyRepository::addTransaction(const PrivacyTransaction& transaction) const
{
    PrivacyTransaction normalized = transaction;
    normalized.uuid               = normalizedUuid(transaction.uuid);
    normalized.categoryUuid       = normalizedUuid(transaction.categoryUuid);
    normalized.itemUuid           = transaction.itemUuid.isEmpty()
                                  ? QString() : normalizedUuid(transaction.itemUuid);

    if (!normalized.createdAt.isValid())
    {
        normalized.createdAt = QDateTime::currentDateTimeUtc();
    }

    if (!normalized.updatedAt.isValid())
    {
        normalized.updatedAt = normalized.createdAt;
    }

    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyTransaction(normalized);
}

bool PrivacyRepository::addTransactionJournal(const PrivacyTransactionJournal& journal) const
{
    PrivacyTransactionJournal normalized = journal;
    normalized.transactionUuid           = normalizedUuid(journal.transactionUuid);
    normalized.rootUuid                  = normalizedUuid(journal.rootUuid);

    if (!normalized.updatedAt.isValid())
    {
        normalized.updatedAt = QDateTime::currentDateTimeUtc();
    }

    CoreDbAccess access;

    return normalized.isValid() && access.db()->insertPrivacyTransactionJournal(normalized);
}

bool PrivacyRepository::activeTransactions(QList<PrivacyTransaction>* transactions) const
{
    if (!transactions)
    {
        return false;
    }

    CoreDbAccess access;
    QList<PrivacyTransaction> loaded;

    if (!access.db()->getActivePrivacyTransactions(&loaded) || !allRecordsValid(loaded))
    {
        return false;
    }

    *transactions = loaded;

    return true;
}

bool PrivacyRepository::compareAndUpdateTransaction(const PrivacyTransaction& transaction,
                                                    PrivacyTransactionState expectedState,
                                                    qlonglong expectedGeneration) const
{
    PrivacyTransaction normalized = transaction;
    normalized.uuid               = normalizedUuid(transaction.uuid);
    normalized.categoryUuid       = normalizedUuid(transaction.categoryUuid);
    normalized.itemUuid           = transaction.itemUuid.isEmpty()
                                  ? QString() : normalizedUuid(transaction.itemUuid);

    if (!normalized.updatedAt.isValid())
    {
        normalized.updatedAt = QDateTime::currentDateTimeUtc();
    }

    CoreDbAccess access;

    return (normalized.isValid() &&
            access.db()->compareAndUpdatePrivacyTransaction(normalized,
                                                            expectedState,
                                                            expectedGeneration));
}

bool PrivacyRepository::compareAndUpdateTransactionJournal(const PrivacyTransactionJournal& journal,
                                                           int expectedStage) const
{
    PrivacyTransactionJournal normalized = journal;
    normalized.transactionUuid           = normalizedUuid(journal.transactionUuid);
    normalized.rootUuid                  = normalizedUuid(journal.rootUuid);

    if (!normalized.updatedAt.isValid())
    {
        normalized.updatedAt = QDateTime::currentDateTimeUtc();
    }

    CoreDbAccess access;

    return (normalized.isValid() &&
            access.db()->compareAndUpdatePrivacyTransactionJournal(normalized, expectedStage));
}

bool PrivacyRepository::beginCategoryCreation(
    const PrivacyCategory& category,
    const PrivacyStorageRoot& root,
    const PrivacyStore& store,
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    CoreDbAccess access;

    return access.db()->beginPrivacyCategoryCreation(category, root, store,
                                                      transaction, journal);
}

bool PrivacyRepository::publishCategoryCreation(
    const PrivacyCategory& category,
    const PrivacyCredential& credential,
    const PrivacyStore& store,
    const QList<PrivacyStoreBinding>& bindings,
    const PrivacyTransaction& transaction) const
{
    CoreDbAccess access;

    return access.db()->publishPrivacyCategoryCreation(category, credential, store,
                                                        bindings, transaction);
}

bool PrivacyRepository::beginItemProtection(
    const PrivacyItem& item, const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    PrivacyItem normalizedItem = item;
    normalizedItem.uuid = normalizedUuid(item.uuid);
    normalizedItem.categoryUuid = normalizedUuid(item.categoryUuid);
    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    normalizedTransaction.itemUuid = normalizedUuid(transaction.itemUuid);
    PrivacyTransactionJournal normalizedJournal = journal;
    normalizedJournal.transactionUuid = normalizedUuid(journal.transactionUuid);
    normalizedJournal.rootUuid = normalizedUuid(journal.rootUuid);
    CoreDbAccess access;

    return access.db()->beginPrivacyItemProtection(normalizedItem,
                                                    normalizedTransaction,
                                                    normalizedJournal);
}

bool PrivacyRepository::publishItemProtection(
    const PrivacyItem& item, const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets,
    const PrivacyTransaction& transaction) const
{
    PrivacyItem normalizedItem = item;
    normalizedItem.uuid = normalizedUuid(item.uuid);
    normalizedItem.categoryUuid = normalizedUuid(item.categoryUuid);
    PrivacyContainer normalizedContainer = container;
    normalizedContainer.uuid = normalizedUuid(container.uuid);
    normalizedContainer.itemUuid = normalizedUuid(container.itemUuid);
    normalizedContainer.rootUuid = normalizedUuid(container.rootUuid);
    QList<PrivacyAsset> normalizedAssets = assets;

    for (PrivacyAsset& asset : normalizedAssets)
    {
        asset.itemUuid = normalizedUuid(asset.itemUuid);
        asset.publicRootUuid = normalizedUuid(asset.publicRootUuid);
        asset.containerUuid = normalizedUuid(asset.containerUuid);
    }

    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    normalizedTransaction.itemUuid = normalizedUuid(transaction.itemUuid);
    CoreDbAccess access;

    return access.db()->publishPrivacyItemProtection(normalizedItem,
                                                      normalizedContainer,
                                                      normalizedAssets,
                                                      normalizedTransaction);
}

bool PrivacyRepository::beginItemUnprotection(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    normalizedTransaction.itemUuid = normalizedUuid(transaction.itemUuid);
    PrivacyTransactionJournal normalizedJournal = journal;
    normalizedJournal.transactionUuid = normalizedUuid(journal.transactionUuid);
    normalizedJournal.rootUuid = normalizedUuid(journal.rootUuid);
    CoreDbAccess access;

    return access.db()->beginPrivacyItemUnprotection(normalizedTransaction,
                                                      normalizedJournal);
}

bool PrivacyRepository::beginCompatibilityUnlock(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    normalizedTransaction.itemUuid = normalizedUuid(transaction.itemUuid);
    PrivacyTransactionJournal normalizedJournal = journal;
    normalizedJournal.transactionUuid = normalizedUuid(journal.transactionUuid);
    normalizedJournal.rootUuid = normalizedUuid(journal.rootUuid);
    CoreDbAccess access;

    return access.db()->beginPrivacyCompatibilityUnlock(normalizedTransaction,
                                                         normalizedJournal);
}

bool PrivacyRepository::beginExternalCheckout(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    normalizedTransaction.itemUuid = normalizedUuid(transaction.itemUuid);
    PrivacyTransactionJournal normalizedJournal = journal;
    normalizedJournal.transactionUuid = normalizedUuid(journal.transactionUuid);
    normalizedJournal.rootUuid = normalizedUuid(journal.rootUuid);
    CoreDbAccess access;

    return access.db()->beginPrivacyExternalCheckout(normalizedTransaction,
                                                      normalizedJournal);
}

bool PrivacyRepository::beginPasswordRewrap(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal) const
{
    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    PrivacyTransactionJournal normalizedJournal = journal;
    normalizedJournal.transactionUuid = normalizedUuid(journal.transactionUuid);
    normalizedJournal.rootUuid = normalizedUuid(journal.rootUuid);
    CoreDbAccess access;

    return access.db()->beginPrivacyPasswordRewrap(normalizedTransaction,
                                                    normalizedJournal);
}

bool PrivacyRepository::publishPasswordRewrap(
    const QString& categoryUuid, qlonglong categoryGeneration,
    const PrivacyCredential& credential, const QString& storeUuid,
    qlonglong storeGeneration, const PrivacyTransaction& transaction,
    PrivacyTransactionState expectedState,
    qlonglong expectedGeneration) const
{
    PrivacyCredential normalizedCredential = credential;
    normalizedCredential.categoryUuid = normalizedUuid(credential.categoryUuid);
    normalizedCredential.recoveryDocumentUuid =
        credential.recoveryDocumentUuid.isEmpty()
            ? QString() : normalizedUuid(credential.recoveryDocumentUuid);
    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    CoreDbAccess access;

    return access.db()->publishPrivacyPasswordRewrap(
        normalizedUuid(categoryUuid), categoryGeneration,
        normalizedCredential, normalizedUuid(storeUuid), storeGeneration,
        normalizedTransaction, expectedState, expectedGeneration);
}

bool PrivacyRepository::publishItemUnprotection(
    qlonglong imageId, const QString& itemUuid, const QString& categoryUuid,
    qlonglong expectedItemGeneration,
    const QString& priorProtectTransactionUuid,
    const PrivacyTransaction& transaction) const
{
    PrivacyTransaction normalizedTransaction = transaction;
    normalizedTransaction.uuid = normalizedUuid(transaction.uuid);
    normalizedTransaction.categoryUuid = normalizedUuid(transaction.categoryUuid);
    normalizedTransaction.itemUuid = normalizedUuid(transaction.itemUuid);
    CoreDbAccess access;

    return access.db()->publishPrivacyItemUnprotection(
        imageId, normalizedUuid(itemUuid), normalizedUuid(categoryUuid),
        expectedItemGeneration, normalizedUuid(priorProtectTransactionUuid),
        normalizedTransaction);
}

bool PrivacyRepository::finalizeItemUnprotection(
    const QString& transactionUuid, const QString& categoryUuid) const
{
    CoreDbAccess access;

    return access.db()->finalizePrivacyItemUnprotection(
        normalizedUuid(transactionUuid), normalizedUuid(categoryUuid));
}

bool PrivacyRepository::loadSnapshot(QList<PrivacyCategory>* categories,
                                     QList<PrivacyItem>* items) const
{
    if (!categories || !items)
    {
        return false;
    }

    PrivacyRepositorySnapshot snapshot;

    if (!loadSnapshot(&snapshot))
    {
        return false;
    }

    *categories = snapshot.categories;
    *items      = snapshot.items;

    return true;
}

static bool loadSnapshotInternal(PrivacyRepositorySnapshot* snapshot,
                                 bool activeOnly)
{
    if (!snapshot)
    {
        return false;
    }

    PrivacyRepositorySnapshot loaded;
    CoreDbAccess access;

    if (!access.db()->getPrivacyCategories(&loaded.categories)                 ||
        !access.db()->getPrivacyCredentials(&loaded.credentials)               ||
        !access.db()->getPrivacyStorageRoots(&loaded.storageRoots)             ||
        !access.db()->getPrivacyStores(&loaded.stores)                         ||
        !access.db()->getPrivacyStoreBindings(&loaded.storeBindings)           ||
        !access.db()->getPrivacyItems(&loaded.items)                           ||
        !access.db()->getPrivacyContainers(&loaded.containers)                 ||
        !access.db()->getPrivacyAssets(&loaded.assets)                         ||
        !access.db()->getPrivacyDerivatives(&loaded.derivatives)               ||
        !(activeOnly
            ? access.db()->getActivePrivacyTransactions(&loaded.transactions)
            : access.db()->getPrivacyTransactions(&loaded.transactions))       ||
        !(activeOnly
            ? access.db()->getActivePrivacyTransactionJournals(
                  &loaded.transactionJournals)
            : access.db()->getPrivacyTransactionJournals(
                  &loaded.transactionJournals))                                 ||
        !allRecordsValid(loaded.categories)                                    ||
        !allRecordsValid(loaded.credentials)                                   ||
        !allRecordsValid(loaded.storageRoots)                                  ||
        !allRecordsValid(loaded.stores)                                        ||
        !allRecordsValid(loaded.storeBindings)                                 ||
        !allRecordsValid(loaded.items)                                         ||
        !allRecordsValid(loaded.containers)                                    ||
        !allRecordsValid(loaded.assets)                                        ||
        !allRecordsValid(loaded.derivatives)                                   ||
        !allRecordsValid(loaded.transactions)                                  ||
        !allRecordsValid(loaded.transactionJournals))
    {
        return false;
    }

    QHash<QString, PrivacyCategory> categories;
    QSet<QString> credentials;
    QHash<QString, PrivacyStorageRoot> roots;
    QHash<QString, PrivacyStore> stores;
    QHash<QString, QString> bindings;
    QHash<QString, QString> categoryStores;
    QHash<QString, PrivacyItem> items;
    QHash<QString, PrivacyContainer> containers;
    QHash<QString, PrivacyTransaction> transactions;

    for (const PrivacyCategory& category : std::as_const(loaded.categories))
    {
        if (categories.contains(category.uuid))
        {
            return false;
        }

        categories.insert(category.uuid, category);
    }

    for (const PrivacyCredential& credential : std::as_const(loaded.credentials))
    {
        const QString key = generationKey(credential.categoryUuid, credential.generation);

        if (!categories.contains(credential.categoryUuid) || credentials.contains(key))
        {
            return false;
        }

        credentials.insert(key);
    }

    for (const PrivacyStorageRoot& root : std::as_const(loaded.storageRoots))
    {
        if (roots.contains(root.uuid))
        {
            return false;
        }

        roots.insert(root.uuid, root);
    }

    for (const PrivacyStore& store : std::as_const(loaded.stores))
    {
        if (stores.contains(store.uuid) || !categories.contains(store.categoryUuid) ||
            !roots.contains(store.rootUuid) ||
            ((store.configGeneration >= 0) &&
             !credentials.contains(generationKey(store.categoryUuid, store.configGeneration))))
        {
            return false;
        }

        stores.insert(store.uuid, store);
    }

    for (const PrivacyStoreBinding& binding : std::as_const(loaded.storeBindings))
    {
        const QString key = binding.categoryUuid + QLatin1Char(':') +
                            QString::number(static_cast<int>(binding.role));
        const auto storeIt = stores.constFind(binding.storeUuid);

        if (bindings.contains(key) || !categories.contains(binding.categoryUuid) ||
            (storeIt == stores.constEnd()) || (storeIt->categoryUuid != binding.categoryUuid))
        {
            return false;
        }

        const auto categoryStoreIt = categoryStores.constFind(binding.categoryUuid);

        if ((categoryStoreIt != categoryStores.constEnd()) &&
            (categoryStoreIt.value() != binding.storeUuid))
        {
            return false;
        }

        bindings.insert(key, binding.storeUuid);
        categoryStores.insert(binding.categoryUuid, binding.storeUuid);
    }

    for (const PrivacyCategory& category : std::as_const(loaded.categories))
    {
        if (category.lifecycleState != PrivacyCategoryLifecycleState::Active)
        {
            continue;
        }

        const QString credentialKey = generationKey(category.uuid, category.currentCredentialGeneration);
        const QString authorityKey = category.uuid + QLatin1String(":1");
        const QString derivativeKey = category.uuid + QLatin1String(":3");
        const QString originalKey = category.uuid + QLatin1String(":2");
        const QString authorityStoreUuid = bindings.value(authorityKey);
        const auto authorityStoreIt = stores.constFind(authorityStoreUuid);

        if (!credentials.contains(credentialKey) || !bindings.contains(authorityKey) ||
            !bindings.contains(derivativeKey) ||
            (bindings.value(authorityKey) != bindings.value(derivativeKey)) ||
            (authorityStoreIt == stores.constEnd()) ||
            (authorityStoreIt->lifecycleState != PrivacyStoreLifecycleState::Active) ||
            (authorityStoreIt->configGeneration != category.currentCredentialGeneration) ||
            ((category.backend == PrivacyBackend::Casual) && bindings.contains(originalKey)) ||
            ((category.backend == PrivacyBackend::Strong) &&
             (!bindings.contains(originalKey) ||
              (bindings.value(authorityKey) != bindings.value(originalKey)))))
        {
            return false;
        }
    }

    for (const PrivacyItem& item : std::as_const(loaded.items))
    {
        if (items.contains(item.uuid) || !categories.contains(item.categoryUuid))
        {
            return false;
        }

        items.insert(item.uuid, item);
    }

    for (const PrivacyContainer& container : std::as_const(loaded.containers))
    {
        const auto itemIt = items.constFind(container.itemUuid);

        if (containers.contains(container.uuid) || (itemIt == items.constEnd()) ||
            !credentials.contains(generationKey(itemIt->categoryUuid,
                                                container.credentialGeneration)) ||
            (!container.rootUuid.isEmpty() && !roots.contains(container.rootUuid)) ||
            (!container.storeUuid.isEmpty() && !stores.contains(container.storeUuid)))
        {
            return false;
        }

        const PrivacyCategory& category = categories.value(itemIt->categoryUuid);

        if (container.kind == PrivacyContainerKind::CasualArchive)
        {
            if ((category.backend != PrivacyBackend::Casual) ||
                (roots.value(container.rootUuid).kind != PrivacyStorageRootKind::AlbumRoot))
            {
                return false;
            }
        }
        else
        {
            const PrivacyStore& store = stores.value(container.storeUuid);
            const QString originalKey = itemIt->categoryUuid + QLatin1String(":2");

            if ((category.backend != PrivacyBackend::Strong) ||
                (store.categoryUuid != itemIt->categoryUuid) ||
                (bindings.value(originalKey) != container.storeUuid))
            {
                return false;
            }
        }

        containers.insert(container.uuid, container);
    }

    for (const PrivacyAsset& asset : std::as_const(loaded.assets))
    {
        if (!items.contains(asset.itemUuid) || !roots.contains(asset.publicRootUuid) ||
            (roots.value(asset.publicRootUuid).kind != PrivacyStorageRootKind::AlbumRoot) ||
            !containers.contains(asset.containerUuid) ||
            (containers.value(asset.containerUuid).itemUuid != asset.itemUuid))
        {
            return false;
        }
    }

    for (const PrivacyDerivative& derivative : std::as_const(loaded.derivatives))
    {
        const auto itemIt = items.constFind(derivative.itemUuid);
        const auto storeIt = stores.constFind(derivative.storeUuid);

        if ((itemIt == items.constEnd()) || (storeIt == stores.constEnd()) ||
            (storeIt->categoryUuid != itemIt->categoryUuid) ||
            (bindings.value(itemIt->categoryUuid + QLatin1String(":3")) != derivative.storeUuid))
        {
            return false;
        }
    }

    for (const PrivacyTransaction& transaction : std::as_const(loaded.transactions))
    {
        if (transactions.contains(transaction.uuid) || !categories.contains(transaction.categoryUuid) ||
            ((transaction.fromCredentialGeneration >= 0) &&
             !credentials.contains(generationKey(transaction.categoryUuid,
                                                 transaction.fromCredentialGeneration))) ||
            ((transaction.toCredentialGeneration >= 0) &&
             !credentials.contains(generationKey(transaction.categoryUuid,
                                                 transaction.toCredentialGeneration))) ||
            (!transaction.itemUuid.isEmpty() &&
             (!items.contains(transaction.itemUuid) ||
              (items.value(transaction.itemUuid).categoryUuid != transaction.categoryUuid))))
        {
            return false;
        }

        transactions.insert(transaction.uuid, transaction);
    }

    for (const PrivacyTransactionJournal& journal : std::as_const(loaded.transactionJournals))
    {
        if (!transactions.contains(journal.transactionUuid) || !roots.contains(journal.rootUuid))
        {
            return false;
        }
    }

    *snapshot = loaded;

    return true;
}

bool PrivacyRepository::loadSnapshot(PrivacyRepositorySnapshot* snapshot) const
{
    return loadSnapshotInternal(snapshot, false);
}

bool PrivacyRepository::loadRuntimeSnapshot(
    PrivacyRepositorySnapshot* snapshot) const
{
    return loadSnapshotInternal(snapshot, true);
}

} // namespace Digikam
