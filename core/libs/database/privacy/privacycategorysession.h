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

// C++ includes

#include <functional>
#include <memory>

// Qt includes

#include <QMutex>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "privacygocryptfsadapter.h"
#include "privacypassword.h"
#include "privacyruntime.h"
#include "privacytransactionjournal.h"
#include "privacytypes.h"

namespace Digikam
{

enum class PrivacyCategorySessionStatus
{
    Created,
    Unlocked,
    UnlockedStoreOffline,
    Locked,
    AlreadyUnlocked,
    AlreadyLocked,
    InvalidRequest,
    InvalidPassword,
    Conflict,
    CategoryNotActive,
    TransactionBlocked,
    StoreOffline,
    StoreIdentityMismatch,
    AuthenticationFailed,
    StoreFailure,
    PublicationFailedRecoveryRequired,
    StrongRecoveryRequired,
    Canceled,
    LockFailed,
    FreshAuthenticationVerified,
    CategoryLocked
};

enum class PrivacyCategoryOperationStatus
{
    Completed,
    InvalidRequest,
    CategoryLocked,
    TransactionBlocked
};

class DIGIKAM_DATABASE_EXPORT PrivacyCategorySessionResult
{
public:

    bool succeeded() const;
    bool recoveryRequired() const;

public:

    PrivacyCategorySessionStatus status = PrivacyCategorySessionStatus::InvalidRequest;
    PrivacyPasswordError passwordError = PrivacyPasswordError::None;
    PrivacyGocryptfsError storeError = PrivacyGocryptfsError::None;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCategoryCreateRequest
{
public:

    QString categoryUuid;
    QString storeUuid;
    QString transactionUuid;
    QString name;
    PrivacyBackend backend = PrivacyBackend::Casual;
    PrivacyPresentationMode presentationMode = PrivacyPresentationMode::Generic;
    PrivacyUnlockedThumbnailMode unlockedThumbnailMode =
        PrivacyUnlockedThumbnailMode::FocusedClear;
    PrivacyTagVisibilityMode tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
    PrivacyStorageRoot storageRoot;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCategorySessionRepository
{
public:

    virtual ~PrivacyCategorySessionRepository() = default;

    virtual bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const = 0;
    virtual bool beginCreation(const PrivacyCategory& category,
                               const PrivacyStorageRoot& root,
                               const PrivacyStore& store,
                               const PrivacyTransaction& transaction,
                               const PrivacyTransactionJournal& journal) = 0;
    virtual bool publishCreation(const PrivacyCategory& category,
                                 const PrivacyCredential& credential,
                                 const PrivacyStore& store,
                                 const QList<PrivacyStoreBinding>& bindings,
                                 const PrivacyTransaction& transaction) = 0;
    virtual bool compareAndUpdateCreationJournal(
        const PrivacyTransactionJournal& journal,
        int expectedStage) = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCoreDbCategorySessionRepository final
    : public PrivacyCategorySessionRepository
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const override;
    bool beginCreation(const PrivacyCategory& category,
                       const PrivacyStorageRoot& root,
                       const PrivacyStore& store,
                       const PrivacyTransaction& transaction,
                       const PrivacyTransactionJournal& journal) override;
    bool publishCreation(const PrivacyCategory& category,
                         const PrivacyCredential& credential,
                         const PrivacyStore& store,
                         const QList<PrivacyStoreBinding>& bindings,
                         const PrivacyTransaction& transaction) override;
    bool compareAndUpdateCreationJournal(
        const PrivacyTransactionJournal& journal,
        int expectedStage) override;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCategoryCreationJournalPersistence
{
public:

    virtual ~PrivacyCategoryCreationJournalPersistence() = default;

    virtual bool createOrLoadExact(const PrivacyStorageRoot& root,
                                   PrivacyJournalRecord* record,
                                   bool allowCreate,
                                   QByteArray* publishedSha256,
                                   PrivacyJournalError* error) = 0;
    virtual bool compareAndUpdateExact(const PrivacyStorageRoot& root,
                                       const PrivacyJournalRecord& record,
                                       const QByteArray& expectedCurrentSha256,
                                       QByteArray* publishedSha256,
                                       PrivacyJournalError* error) = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyFilesystemCategoryCreationJournalPersistence final
    : public PrivacyCategoryCreationJournalPersistence
{
public:

    bool createOrLoadExact(const PrivacyStorageRoot& root,
                           PrivacyJournalRecord* record,
                           bool allowCreate,
                           QByteArray* publishedSha256,
                           PrivacyJournalError* error) override;
    bool compareAndUpdateExact(const PrivacyStorageRoot& root,
                               const PrivacyJournalRecord& record,
                               const QByteArray& expectedCurrentSha256,
                               QByteArray* publishedSha256,
                               PrivacyJournalError* error) override;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCategoryStoreLease
{
public:

    virtual ~PrivacyCategoryStoreLease() = default;
    virtual bool isActive() = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCategoryStoreBackend
{
public:

    virtual ~PrivacyCategoryStoreBackend() = default;

    virtual bool createOrResume(const PrivacyStorageRoot& root,
                                const PrivacyStore& store,
                                const QString& temporaryCipherRelativePath,
                                const PrivacyPassword& password,
                                const QByteArray& sentinel,
                                PrivacyGocryptfsEnvelope* envelope,
                                PrivacyGocryptfsError* error) = 0;
    virtual bool validateEnvelope(const PrivacyGocryptfsEnvelope& envelope,
                                  const PrivacyPassword& password,
                                  PrivacyGocryptfsError* error) = 0;
    virtual std::unique_ptr<PrivacyCategoryStoreLease> unlock(
        const PrivacyStorageRoot& root,
        const PrivacyStore& store,
        const PrivacyGocryptfsEnvelope& envelope,
        const PrivacyPassword& password,
        const QByteArray& sentinel,
        PrivacyGocryptfsError* error) = 0;
    virtual bool lock(std::unique_ptr<PrivacyCategoryStoreLease>& lease,
                      PrivacyGocryptfsError* error) = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacySecretLifetimeObserver
{
public:

    virtual ~PrivacySecretLifetimeObserver() = default;
    virtual void secretRetained(const QString& categoryUuid) = 0;
    virtual void secretReleased(const QString& categoryUuid) = 0;
};

/**
 * Owns authenticated category secrets and category-store mount leases. Durable
 * creation publication and runtime unlock publication are fail-closed seams;
 * plaintext passwords are never placed in a persistent record.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyCategorySessionCoordinator
{
public:

    PrivacyCategorySessionCoordinator(
        PrivacyCategorySessionRepository& repository,
        PrivacyCategoryStoreBackend& storeBackend,
        const PrivacyRootVerifier& rootVerifier,
        PrivacyRuntimeCoordinator& runtime,
        PrivacySecretLifetimeObserver* secretObserver = nullptr,
        PrivacyCategoryCreationJournalPersistence* creationJournal = nullptr);
    ~PrivacyCategorySessionCoordinator();

    PrivacyCategorySessionResult createCategory(
        const PrivacyCategoryCreateRequest& request,
        const QString& passwordText);
    PrivacyCategorySessionResult unlockCategory(
        const QString& categoryUuid,
        const QString& passwordText);
    PrivacyCategorySessionResult lockCategory(const QString& categoryUuid);
    QList<PrivacyCategorySessionResult> lockAllCategories();

    /**
     * Runs one synchronous protected-item operation while the category session
     * remains unlocked. Category lock and lock-all wait for the callback to
     * return; competing create/unlock/item operations fail with
     * TransactionBlocked. The callback runs outside the coordinator mutex.
     *
     * A callback must not call createCategory(), unlockCategory(),
     * lockCategory(), or lockAllCategories() on this coordinator. Such
     * re-entry is rejected rather than allowed to wait on itself.
     */
    PrivacyCategoryOperationStatus runWhileUnlocked(
        const QString& categoryUuid,
        const std::function<void()>& operation);

    /**
     * As runWhileUnlocked(), while borrowing the retained normalized password
     * for the callback only. The reference must not be retained or returned.
     * This is the no-redundant-authentication Protect boundary; Unprotect must
     * continue to use a separately supplied freshly entered password.
     */
    PrivacyCategoryOperationStatus runWithUnlockedSecret(
        const QString& categoryUuid,
        const std::function<void(const PrivacyPassword&)>& operation);

    /**
     * Runs one synchronous protected-item operation with a newly entered,
     * independently verified category password. The category may be locked;
     * this operation never creates or replaces a retained category session or
     * mounts its store. The temporary normalized secret is lent to the
     * callback only.
     *
     * Category lock and lock-all wait until authentication, the callback and
     * temporary-secret destruction have completed. Callback exceptions are
     * propagated after releasing the operation barrier safely.
     */
    PrivacyCategorySessionResult runWithFreshlyAuthenticatedSecret(
        const QString& categoryUuid,
        const QString& passwordText,
        const std::function<void(const PrivacyPassword&)>& operation,
        const QString& allowedActiveItemTransactionUuid = QString());

    bool ownsSecret(const QString& categoryUuid) const;

private:

    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Digikam
