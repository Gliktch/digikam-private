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
    bool setCategoryTagVisibilityMode(const QString& uuid,
                                      PrivacyTagVisibilityMode mode,
                                      bool categoryAuthenticationVerified) const;

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

    bool loadSnapshot(QList<PrivacyCategory>* categories,
                      QList<PrivacyItem>* items) const;
    bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const;
};

} // namespace Digikam
