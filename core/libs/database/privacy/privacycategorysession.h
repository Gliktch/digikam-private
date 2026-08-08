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

#include <memory>

// Qt includes

#include <QMutex>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "privacygocryptfsadapter.h"
#include "privacypassword.h"
#include "privacyruntime.h"
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
    LockFailed
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
        PrivacySecretLifetimeObserver* secretObserver = nullptr);
    ~PrivacyCategorySessionCoordinator();

    PrivacyCategorySessionResult createCategory(
        const PrivacyCategoryCreateRequest& request,
        const QString& passwordText);
    PrivacyCategorySessionResult unlockCategory(
        const QString& categoryUuid,
        const QString& passwordText);
    PrivacyCategorySessionResult lockCategory(const QString& categoryUuid);
    QList<PrivacyCategorySessionResult> lockAllCategories();

    bool ownsSecret(const QString& categoryUuid) const;

private:

    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Digikam
