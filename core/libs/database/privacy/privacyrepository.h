/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "privacytypes.h"

namespace Digikam
{

enum class PrivacyAlbumRootRegistrationStatus
{
    Created,
    Existing,
    Offline,
    IdentityMismatch,
    Conflict,
    StorageFailure
};

class DIGIKAM_DATABASE_EXPORT PrivacyAlbumRootRegistrationResult
{
public:

    bool succeeded() const;

public:

    PrivacyAlbumRootRegistrationStatus status =
        PrivacyAlbumRootRegistrationStatus::StorageFailure;
    PrivacyStorageRoot root;
};

class DIGIKAM_DATABASE_EXPORT PrivacyRepository
{
public:

    PrivacyRepository() = default;

    bool createCategory(const PrivacyCategory& category) const;
    PrivacyCategory category(const QString& uuid) const;
    bool setCategoryUnlockedThumbnailMode(
        const QString& uuid,
        PrivacyUnlockedThumbnailMode mode,
        bool categoryAuthenticationVerified) const;
    bool setCategoryTagVisibilityMode(const QString& uuid,
                                      PrivacyTagVisibilityMode mode,
                                      bool categoryAuthenticationVerified) const;
    bool updateContainerCredentialGeneration(const QString& containerUuid,
                                             qlonglong generation,
                                             qlonglong expectedGeneration) const;

    bool addCredential(const PrivacyCredential& credential) const;
    bool addStorageRoot(const PrivacyStorageRoot& root) const;
    PrivacyAlbumRootRegistrationResult ensureAlbumRoot(
        int albumRootId,
        const QString& configuredPath,
        const QString& collectionIdentifier) const;
    bool removeUnreferencedAlbumRoot(const QString& uuid, bool* absent) const;
    bool pruneUnreferencedAlbumRoots(QStringList* removedUuids = nullptr) const;
    bool addStore(const PrivacyStore& store) const;
    bool addStoreBinding(const PrivacyStoreBinding& binding) const;

    bool mapItem(const PrivacyItem& item) const;
    PrivacyItem itemForImageId(qlonglong imageId) const;

    bool addContainer(const PrivacyContainer& container) const;
    bool addAsset(const PrivacyAsset& asset) const;
    bool addDerivative(const PrivacyDerivative& derivative) const;
    bool addTransaction(const PrivacyTransaction& transaction) const;
    bool activeTransactions(QList<PrivacyTransaction>* transactions) const;
    bool compareAndUpdateTransaction(const PrivacyTransaction& transaction,
                                     PrivacyTransactionState expectedState,
                                     qlonglong expectedGeneration) const;
    bool addTransactionJournal(const PrivacyTransactionJournal& journal) const;
    bool compareAndUpdateTransactionJournal(const PrivacyTransactionJournal& journal,
                                            int expectedStage) const;
    bool beginCategoryCreation(const PrivacyCategory& category,
                               const PrivacyStorageRoot& root,
                               const PrivacyStore& store,
                               const PrivacyTransaction& transaction,
                               const PrivacyTransactionJournal& journal) const;
    bool publishCategoryCreation(const PrivacyCategory& category,
                                 const PrivacyCredential& credential,
                                 const PrivacyStore& store,
                                 const QList<PrivacyStoreBinding>& bindings,
                                 const PrivacyTransaction& transaction) const;
    bool beginItemProtection(const PrivacyItem& item,
                             const PrivacyTransaction& transaction,
                             const PrivacyTransactionJournal& journal) const;
    bool publishItemProtection(const PrivacyItem& item,
                               const PrivacyContainer& container,
                               const QList<PrivacyAsset>& assets,
                               const PrivacyTransaction& transaction) const;
    bool beginItemUnprotection(const PrivacyTransaction& transaction,
                               const PrivacyTransactionJournal& journal) const;
    bool beginCompatibilityUnlock(const PrivacyTransaction& transaction,
                                  const PrivacyTransactionJournal& journal) const;
    bool beginExternalCheckout(const PrivacyTransaction& transaction,
                               const PrivacyTransactionJournal& journal) const;
    bool beginPasswordRewrap(const PrivacyTransaction& transaction,
                             const PrivacyTransactionJournal& journal) const;
    bool publishPasswordRewrap(const QString& categoryUuid,
                               qlonglong categoryGeneration,
                               const PrivacyCredential& credential,
                               const QString& storeUuid,
                               qlonglong storeGeneration,
                               const PrivacyTransaction& transaction,
                               PrivacyTransactionState expectedState,
                               qlonglong expectedGeneration) const;
    bool publishItemUnprotection(qlonglong imageId,
                                 const QString& itemUuid,
                                 const QString& categoryUuid,
                                 qlonglong expectedItemGeneration,
                                 const QString& priorProtectTransactionUuid,
                                 const PrivacyTransaction& transaction) const;
    bool finalizeItemUnprotection(const QString& transactionUuid,
                                  const QString& categoryUuid) const;

    bool loadSnapshot(QList<PrivacyCategory>* categories,
                      QList<PrivacyItem>* items) const;
    bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const;
    /** Loads only active transactions and their journals for startup/runtime
     * recovery while retaining the complete current category/item graph. */
    bool loadRuntimeSnapshot(PrivacyRepositorySnapshot* snapshot) const;
};

} // namespace Digikam
