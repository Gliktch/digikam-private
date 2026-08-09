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

// Qt includes

#include <QSharedPointer>
#include <QScopedPointer>
#include <QSize>

// Local includes

#include "digikam_export.h"
#include "privacyprotectpreflight.h"
#include "privacyruntime.h"
#include "privacytransactionjournal.h"
#include "privacypassword.h"

namespace Digikam
{

enum class PrivacyStillItemTransactionStatus
{
    Protected,
    Unprotected,
    InvalidRequest,
    PreflightRejected,
    AcknowledgementRequired,
    AssociatedAssetSetUnsupported,
    CategoryUnavailable,
    AuthenticationRequired,
    RootUnavailable,
    SourceChanged,
    ArchiveFailure,
    ProxyFailure,
    JournalFailure,
    PersistenceFailure,
    PublicTransitionFailure,
    RuntimePublicationFailure,
    CacheTransitionFailure,
    CleanupPending,
    RecoveryRequired,
    FaultInjected
};

class DIGIKAM_DATABASE_EXPORT PrivacyStillItemTransactionResult
{
public:

    bool succeeded() const;

public:

    PrivacyStillItemTransactionStatus status =
        PrivacyStillItemTransactionStatus::InvalidRequest;
    QString transactionUuid;
    QString itemUuid;
    QString detail;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStillProtectRequest
{
public:

    qlonglong imageId = -1;
    QString categoryUuid;
    QString itemUuid;
    QString containerUuid;
    QString transactionUuid;
    PrivacyProtectPreflightResult preflight;
    bool associatedAssetsAcknowledged = false;
    PrivacyStorageRoot publicRoot;
    PrivacyJournalRootExpectation rootExpectation;
    QSize originalPixelSize;
    QDateTime originalCreationDate;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStillUnprotectRequest
{
public:

    qlonglong imageId = -1;
    QString categoryUuid;
    QString transactionUuid;
    PrivacyStorageRoot publicRoot;
    PrivacyJournalRootExpectation rootExpectation;
    // Set only by a boundary that independently verified the supplied
    // password. This authorizes Unprotect without retaining or publishing an
    // unlocked category session.
    bool freshAuthenticationConfirmed = false;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStillItemPersistence
{
public:

    virtual ~PrivacyStillItemPersistence() = default;

    virtual bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const = 0;
    virtual bool beginProtection(const PrivacyItem& item,
                                 const PrivacyTransaction& transaction,
                                 const PrivacyTransactionJournal& journal) = 0;
    virtual bool publishProtection(const PrivacyItem& item,
                                   const PrivacyContainer& container,
                                   const QList<PrivacyAsset>& assets,
                                   const PrivacyTransaction& transaction) = 0;
    virtual bool beginUnprotection(const PrivacyTransaction& transaction,
                                   const PrivacyTransactionJournal& journal) = 0;
    virtual bool publishUnprotection(qlonglong imageId,
                                     const QString& itemUuid,
                                     const QString& categoryUuid,
                                     qlonglong expectedItemGeneration,
                                     const QString& priorProtectTransactionUuid,
                                     const PrivacyTransaction& transaction) = 0;
    virtual bool finalizeUnprotection(const QString& transactionUuid,
                                      const QString& categoryUuid) = 0;
    virtual bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) = 0;
    virtual bool compareAndUpdateJournal(
        const PrivacyTransactionJournal& journal,
        int expectedStage) = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCoreDbStillItemPersistence final
    : public PrivacyStillItemPersistence
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const override;
    bool beginProtection(const PrivacyItem& item,
                         const PrivacyTransaction& transaction,
                         const PrivacyTransactionJournal& journal) override;
    bool publishProtection(const PrivacyItem& item,
                           const PrivacyContainer& container,
                           const QList<PrivacyAsset>& assets,
                           const PrivacyTransaction& transaction) override;
    bool beginUnprotection(const PrivacyTransaction& transaction,
                           const PrivacyTransactionJournal& journal) override;
    bool publishUnprotection(qlonglong imageId,
                             const QString& itemUuid,
                             const QString& categoryUuid,
                             qlonglong expectedItemGeneration,
                             const QString& priorProtectTransactionUuid,
                             const PrivacyTransaction& transaction) override;
    bool finalizeUnprotection(const QString& transactionUuid,
                              const QString& categoryUuid) override;
    bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) override;
    bool compareAndUpdateJournal(const PrivacyTransactionJournal& journal,
                                 int expectedStage) override;
};

/** Database code cannot depend on ThreadImageIO. The application adapter owns
 * the existing PrivacyCacheTransition token and keeps it active until finish. */
class DIGIKAM_DATABASE_EXPORT PrivacyStillItemCacheGate
{
public:

    virtual ~PrivacyStillItemCacheGate() = default;
    /** Both operations are idempotent for one image/path/direction. Protect
     * begin requires exact legacy-primary alias-completeness evidence. A
     * tokenless cold-replay finish requires authoritative evidence that the
     * journal reached PublicStateVerified or later. */
    virtual bool begin(qlonglong imageId, const QString& logicalPath,
                       bool protecting,
                       bool legacyPrimaryAliasInventoryComplete) = 0;
    virtual bool finish(qlonglong imageId, const QString& logicalPath,
                        bool protecting,
                        bool publicStateVerifiedOrLater) = 0;
};

enum class PrivacyStillItemFaultPoint
{
    AfterDatabaseBegin,
    AfterFilesystemJournal,
    AfterPreparedPayload,
    AfterReplacementStageCreated,
    AfterStagesPrepared,
    AfterArchivePublished,
    AfterProtectedCopyJournal,
    AfterPublicTransition,
    AfterCompleteJournal,
    AfterDatabasePublication,
    AfterRuntimePublication,
    AfterUnprotectDatabaseTeardown,
    AfterUnprotectRuntimeRemoval,
    AfterArchiveCleanup
};

/** One complete Casual, single-primary-media transaction. Multi-asset sets are
 * rejected before mutation until batch namespace publication exists. */
class DIGIKAM_DATABASE_EXPORT PrivacyStillItemTransactionEngine
{
public:

    using FaultHook = std::function<bool(PrivacyStillItemFaultPoint)>;

    PrivacyStillItemTransactionEngine(
        PrivacyStillItemPersistence& persistence,
        PrivacyRuntimeCoordinator& runtime,
        PrivacyStillItemCacheGate& cacheGate);
    ~PrivacyStillItemTransactionEngine();

    void setFaultHook(const FaultHook& hook);
    PrivacyStillItemTransactionResult protect(
        const PrivacyStillProtectRequest& request,
        const PrivacyPassword& password);
    PrivacyStillItemTransactionResult unprotect(
        const PrivacyStillUnprotectRequest& request,
        const PrivacyPassword& password);

    /**
     * Resumes one exact durable transaction without a retained password.
     * Secret-dependent phases return AuthenticationRequired without mutation;
     * fresh transactions can never enter through this recovery boundary.
     */
    PrivacyStillItemTransactionResult recover(
        const PrivacyStorageRoot& publicRoot,
        const QString& transactionUuid);

    /**
     * Resumes one exact durable transaction with a caller-verified category
     * password. Only a Created transaction consumes the password; later states
     * retain the password-free recovery rules used by recover(). Unprotect
     * requires an independently verified fresh authentication.
     */
    PrivacyStillItemTransactionResult resumeAuthenticated(
        const PrivacyStorageRoot& publicRoot,
        const QString& transactionUuid,
        const PrivacyPassword& verifiedPassword,
        bool freshAuthenticationConfirmed);

private:

    PrivacyStillItemTransactionResult recoverInternal(
        const PrivacyStorageRoot& publicRoot,
        const QString& transactionUuid,
        const PrivacyPassword* verifiedPassword,
        bool freshAuthenticationConfirmed);

    class Private;
    QScopedPointer<Private> d;
};

} // namespace Digikam
