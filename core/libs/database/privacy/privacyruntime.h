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

// Qt includes

#include <QHash>
#include <QReadWriteLock>
#include <QSet>
#include <QSharedPointer>
#include <QStringList>

// C++ includes

#include <functional>

// Local includes

#include "digikam_export.h"
#include "privacyactionpolicy.h"
#include "privacyanalysisgate.h"
#include "privacycontracts.h"
#include "privacyleaseregistry.h"
#include "privacyscangate.h"
#include "privacyservice.h"
#include "privacytypes.h"

namespace Digikam
{

class PrivacyCategorySessionOwner;

enum class PrivacyRecoveryDisposition
{
    Recovered = 1,
    Deferred  = 2,
    Failed    = 3
};

enum class PrivacyIntegrityDisposition
{
    Verified = 1,
    Deferred = 2,
    Failed   = 3
};

class DIGIKAM_DATABASE_EXPORT PrivacyRootVerifier
{
public:

    PrivacyRootVerifier()          = default;
    virtual ~PrivacyRootVerifier() = default;

    virtual PrivacyRootRuntimeState verify(const PrivacyStorageRoot& root) const = 0;

private:

    Q_DISABLE_COPY(PrivacyRootVerifier)
};

class DIGIKAM_DATABASE_EXPORT PrivacyTransactionRecovery
{
public:

    PrivacyTransactionRecovery()          = default;
    virtual ~PrivacyTransactionRecovery() = default;

    /// Must be idempotent. Recovered means the supplied root's durable state
    /// is reconciled and safe to inspect; it does not authorize another root.
    virtual PrivacyRecoveryDisposition recoverRoot(
        const PrivacyStorageRoot& root,
        const PrivacyTransaction& transaction,
        const QList<PrivacyTransactionJournal>& journals) const = 0;

private:

    Q_DISABLE_COPY(PrivacyTransactionRecovery)
};

class DIGIKAM_DATABASE_EXPORT PrivacyRootIntegritySummary
{
public:

    QString rootUuid;
    QString configuredPath;
    PrivacyRootRuntimeState state = PrivacyRootRuntimeState::Unknown;
    int protectedItemCount = 0;
    int missingProxyCount = 0;
    int changedProxySizeCount = 0;
    int failedProxyValidationCount = 0;
    int exposedOriginalAtProxyPathCount = 0;
    int unexpectedPublicAssetCount = 0;
    int missingProtectedObjectCount = 0;
    int changedProtectedObjectSizeCount = 0;
    int unresolvedTransactionCount = 0;
    int compatibilityExposureCount = 0;
    bool identityMismatch = false;

    bool hasReportableIssues(bool includeProxySizeChanges = true) const;
};

class DIGIKAM_DATABASE_EXPORT PrivacyRootInspectionResult
{
public:

    PrivacyIntegrityDisposition disposition = PrivacyIntegrityDisposition::Deferred;
    PrivacyRootIntegritySummary summary;
    QSet<QString> proxyIssueItemUuids;
    QSet<QString> originalIssueItemUuids;
};

class DIGIKAM_DATABASE_EXPORT PrivacyRootIntegrityInspector
{
public:

    PrivacyRootIntegrityInspector()          = default;
    virtual ~PrivacyRootIntegrityInspector() = default;

    virtual PrivacyRootInspectionResult inspect(
        const PrivacyStorageRoot& root,
        const PrivacyRepositorySnapshot& snapshot) const = 0;

private:

    Q_DISABLE_COPY(PrivacyRootIntegrityInspector)
};

enum class PrivacyStartupState
{
    Ready    = 1,
    Degraded = 2
};

enum class PrivacyPublicSourceDisposition
{
    Unprotected = 1,
    LockedProxy = 2,
    Denied      = 3
};

enum class PrivacyPublicProxyDisplayResult
{
    Verified              = 1,
    Denied                = 2,
    NewlyFailedValidation = 3,
    NewlyExposedOriginal  = 4
};

enum class PrivacyRootRecoveryResult
{
    PublishedVerified = 1,
    PublishedOffline = 2,
    PublishedIdentityMismatch = 3,
    Deferred = 4,
    StaleEpoch = 5,
    UnknownRoot = 6
};

class DIGIKAM_DATABASE_EXPORT PrivacyStartupReport
{
public:

    PrivacyStartupState state = PrivacyStartupState::Degraded;
    int verifiedRootCount = 0;
    int offlineRootCount = 0;
    int mismatchedRootCount = 0;
    int recoveringRootCount = 0;
    int unresolvedTransactionCount = 0;
    QList<PrivacyRootIntegritySummary> roots;
    QStringList diagnostics;

    bool hasOnlyProxySizeIssues() const;
    bool hasReportableIssues(bool suppressProxySizeOnly) const;
};

class DIGIKAM_DATABASE_EXPORT PrivacyRuntimeCoordinator : public PrivacyScanGateProvider,
                                                          public PrivacyActionStateProvider,
                                                          public PrivacyLeaseStateProvider,
                                                          public PrivacyManualTagVisibilityProvider,
                                                          public PrivacyAnalysisGateProvider
{
public:

    PrivacyRuntimeCoordinator();
    ~PrivacyRuntimeCoordinator() override;

    PrivacyStartupReport initialize(
        const PrivacyRepositorySnapshot& snapshot,
        const QSharedPointer<const PrivacyRootVerifier>& rootVerifier,
        const QSharedPointer<const PrivacyTransactionRecovery>& recovery,
        const QSharedPointer<const PrivacyRootIntegrityInspector>& integrityInspector);
    void reset();

    /// Snapshot of the most recent startup initialization. Root state/epoch
    /// accessors are authoritative after reconnect transitions.
    PrivacyStartupReport report() const;
    PrivacyRootRuntimeState rootState(const QString& rootUuid) const;
    quint64 rootEpoch(const QString& rootUuid) const;
    PrivacyRootIntegritySummary rootSummary(const QString& rootUuid) const;
    QString rootUuidForAlbumRootId(int albumRootId) const;
    PrivacyPublicSourceDisposition publicSourceDisposition(qlonglong imageId) const;
    QString publicSourceCacheNamespace(qlonglong imageId) const;
    qlonglong expectedPublicProxySize(qlonglong imageId) const;
    PrivacyPublicProxyDisplayResult validatePublicProxyForDisplay(
        qlonglong imageId, const QString& absolutePath);

    bool setCategoryUnlocked(const QString& categoryUuid, bool unlocked);
    bool isCategoryUnlocked(const QString& categoryUuid) const;
    bool publishCategory(const PrivacyCategory& category);
    bool publishCategoryCreation(const PrivacyCategory& category,
                                 const PrivacyCredential& credential,
                                 const PrivacyStorageRoot& root,
                                 const PrivacyStore& store,
                                 const QList<PrivacyStoreBinding>& bindings);
    /// Publishes one newly completed Casual protection set. Existing or
    /// partial item facts are rejected; use hasProtectedItem() for replay.
    bool publishProtectedItem(const PrivacyItem& item,
                              const PrivacyContainer& container,
                              const QList<PrivacyAsset>& assets);
    /// Replaces only the exact DB-begun partial item held by a cold runtime
    /// after its sole Protect transaction has durably completed.
    bool publishProtectedItemForProtectRecovery(
        const PrivacyItem& item,
        const PrivacyContainer& container,
        const QList<PrivacyAsset>& assets,
        const PrivacyTransaction& completedTransaction);
    /// True only when every supplied item/container/asset fact exactly matches
    /// the coherent live snapshot and verified roots.
    bool hasProtectedItem(const PrivacyItem& item,
                          const PrivacyContainer& container,
                          const QList<PrivacyAsset>& assets) const;
    /// Removes only an exact completed Casual protection set.
    bool removeProtectedItem(const PrivacyItem& item,
                             const PrivacyContainer& container,
                             const QList<PrivacyAsset>& assets);
    /// Removes an exact protected set during cold Unprotect replay when the
    /// supplied active transaction is the sole reason every required root is
    /// Recovering. Offline, identity-mismatched, and conflicting roots remain
    /// ineligible.
    bool removeProtectedItemForUnprotectRecovery(
        const PrivacyItem& item,
        const PrivacyContainer& container,
        const QList<PrivacyAsset>& assets,
        const QString& transactionUuid);
    quint64 categoryEpoch(const QString& categoryUuid) const;
    bool setCategoryTagVisibilityMode(const QString& categoryUuid,
                                      PrivacyTagVisibilityMode mode,
                                      bool categoryAuthenticationVerified);
    void lockAllCategories();
    bool compareAndSetItemGeneration(qlonglong imageId,
                                     qlonglong expectedGeneration,
                                     qlonglong newGeneration);

    PrivacyRootRecoveryResult registerAlbumRoot(const PrivacyStorageRoot& root);
    bool unregisterUnreferencedAlbumRoot(const QString& rootUuid);
    bool beginRootRecovery(const QString& rootUuid);
    bool publishRootState(const QString& rootUuid, PrivacyRootRuntimeState state);
    bool publishRootStateIfEpoch(const QString& rootUuid,
                                 quint64 expectedEpoch,
                                 PrivacyRootRuntimeState state);
    PrivacyRootRecoveryResult recoverRoot(const QString& rootUuid);

    bool stateForItem(qlonglong imageId,
                      PrivacyActionItemState* state) const override;
    bool currentState(const QString& itemUuid,
                      PrivacyLeaseCurrentState* state) const override;
    bool mayAccessManualTags(qlonglong imageId) const override;
    QSet<QString> visibleManualTagCategoryUuids() const override;
    PrivacyAnalysisDisposition analysisDisposition(qlonglong imageId) const override;

    PrivacyScanDisposition evaluate(const PrivacyScanRequest& request) const override;
    bool hasDeferredRoots() const override;
    bool rootContainsProtectedItems(int albumRootId) const override;

private:

    bool removeProtectedItemInternal(const PrivacyItem& item,
                                     const PrivacyContainer& container,
                                     const QList<PrivacyAsset>& assets,
                                     const QString& recoveryTransactionUuid);

    class Private;
    Private* const d = nullptr;
};

DIGIKAM_DATABASE_EXPORT QSharedPointer<const PrivacyRootVerifier>
createDefaultPrivacyRootVerifier();

DIGIKAM_DATABASE_EXPORT QSharedPointer<const PrivacyRootIntegrityInspector>
createDefaultPrivacyRootIntegrityInspector();

class DIGIKAM_DATABASE_EXPORT PrivacyStartupRecovery
{
public:

    using TransactionRecoveryFactory =
        std::function<QSharedPointer<const PrivacyTransactionRecovery>(
            PrivacyRuntimeCoordinator&)>;

    static PrivacyStartupReport run();
    static void reset();
    static void setTransactionRecoveryFactory(
        const TransactionRecoveryFactory& factory);
    static PrivacyStartupReport report();
    static QSharedPointer<PrivacyRuntimeCoordinator> coordinator();
    static QSharedPointer<PrivacyCategorySessionOwner> categorySessions();
    static QSharedPointer<const PrivacyActionStateProvider> actionStateProvider();
    static QSharedPointer<const PrivacyLeaseStateProvider> leaseStateProvider();
    static QSharedPointer<const PrivacyManualTagVisibilityProvider>
        manualTagVisibilityProvider();
};

/**
 * Process-wide manual-tag policy facade. The provider is installed during
 * normal application database startup. Its absence preserves upstream
 * behaviour for isolated database-library users and tests.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyManualTagVisibilityGate
{
public:

    static void setProvider(
        const QSharedPointer<const PrivacyManualTagVisibilityProvider>& provider);
    static void resetProvider();
    static bool isInstalled();
    static bool mayAccess(qlonglong imageId);
    static QSet<QString> visibleCategoryUuids();
    static bool queryState(QSet<QString>* visibleCategoryUuids);
};

} // namespace Digikam
