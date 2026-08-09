/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

// C++ includes

#include <functional>

// Local includes

#include "privacycontracts.h"
#include "privacycategorysessionowner.h"
#include "privacyruntime.h"
#include "privacyscangate.h"

using namespace Digikam;

namespace
{

const QString categoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
const QString itemUuid = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString otherItemUuid = QLatin1String("20000000-0000-0000-0000-000000000002");
const QString rootUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QString otherRootUuid = QLatin1String("30000000-0000-0000-0000-000000000002");
const QString markerUuid = QLatin1String("40000000-0000-0000-0000-000000000001");

class FakeRootVerifier final : public PrivacyRootVerifier
{
public:

    PrivacyRootRuntimeState verify(const PrivacyStorageRoot& root) const override
    {
        if (onVerify)
        {
            onVerify();
        }

        return states.value(root.uuid, PrivacyRootRuntimeState::IdentityMismatch);
    }

public:

    QHash<QString, PrivacyRootRuntimeState> states;
    std::function<void()> onVerify;
};

class FakeRecovery final : public PrivacyTransactionRecovery
{
public:

    PrivacyRecoveryDisposition recoverRoot(
        const PrivacyStorageRoot&,
        const PrivacyTransaction&,
        const QList<PrivacyTransactionJournal>&) const override
    {
        ++callCount;

        if (onRecover)
        {
            onRecover();
        }

        return disposition;
    }

public:

    PrivacyRecoveryDisposition disposition = PrivacyRecoveryDisposition::Recovered;
    mutable int callCount = 0;
    std::function<void()> onRecover;
};

class FakeIntegrityInspector final : public PrivacyRootIntegrityInspector
{
public:

    PrivacyRootInspectionResult inspect(const PrivacyStorageRoot& root,
                                        const PrivacyRepositorySnapshot&) const override
    {
        ++callCount;

        if (onInspect)
        {
            onInspect();
        }

        PrivacyRootInspectionResult result;
        result.disposition = disposition;
        result.summary = summary;
        result.summary.rootUuid = root.uuid;

        return result;
    }

public:

    PrivacyIntegrityDisposition disposition = PrivacyIntegrityDisposition::Verified;
    PrivacyRootIntegritySummary summary;
    std::function<void()> onInspect;
    mutable int callCount = 0;
};

PrivacyCategory makeCategory()
{
    PrivacyCategory category;
    category.uuid                        = categoryUuid;
    category.name                        = QLatin1String("Synthetic category");
    category.lifecycleState              = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt                   = QDateTime::currentDateTimeUtc();

    return category;
}

PrivacyStorageRoot makeRoot(const QString& uuid, int albumRootId)
{
    PrivacyStorageRoot root;
    root.uuid            = uuid;
    root.kind            = PrivacyStorageRootKind::AlbumRoot;
    root.albumRootId     = albumRootId;
    root.configuredPath  = QLatin1String("/synthetic/collection");
    root.identityVersion = 1;
    root.identityData    = PrivacyRootIdentityCodec::encodeAlbumRootV1(
                               albumRootId, QLatin1String("synthetic-volume"));
    root.createdAt       = QDateTime::currentDateTimeUtc();

    return root;
}

PrivacyStorageRoot makeManagedRoot(const QString& path)
{
    PrivacyStorageRoot root;
    root.uuid            = rootUuid;
    root.kind            = PrivacyStorageRootKind::ManagedStoreRoot;
    root.albumRootId     = -1;
    root.configuredPath  = path;
    root.identityVersion = 1;
    root.identityData    = PrivacyRootIdentityCodec::encodeManagedRootV1(markerUuid);
    root.markerUuid      = markerUuid;
    root.createdAt       = QDateTime::currentDateTimeUtc();

    return root;
}

bool writeManagedRootMarker(const PrivacyStorageRoot& root,
                            const QString& writtenMarkerUuid = markerUuid)
{
    const QString markerPath = QDir(root.configuredPath).filePath(
                                       PrivacyRootIdentityCodec::managedRootMarkerRelativePathV1());
    const QString markerDirectory = QFileInfo(markerPath).absolutePath();

    if (!QDir().mkpath(markerDirectory) ||
        !QFile::setPermissions(markerDirectory,
                               QFileDevice::ReadOwner |
                               QFileDevice::WriteOwner |
                               QFileDevice::ExeOwner))
    {
        return false;
    }

    QFile marker(markerPath);

    if (!marker.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    if (marker.write(PrivacyRootIdentityCodec::encodeManagedRootMarkerV1(
                         root.uuid, writtenMarkerUuid)) <= 0)
    {
        return false;
    }

    marker.close();

    return QFile::setPermissions(markerPath,
                                 QFileDevice::ReadOwner |
                                 QFileDevice::WriteOwner);
}

bool writeSizedFile(const QString& path, int size)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
    {
        return false;
    }

    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    return (file.write(QByteArray(size, 'x')) == size);
}

PrivacyItem makeItem()
{
    PrivacyItem item;
    item.imageId          = 42;
    item.uuid             = itemUuid;
    item.categoryUuid     = categoryUuid;
    item.expectedProxySize = 77;
    item.generation       = 3;

    return item;
}

PrivacyItem makePublishableItem()
{
    PrivacyItem item = makeItem();
    item.originalHash     = QLatin1String("original-hash");
    item.originalSize     = 100;
    item.expectedProxyHash = QLatin1String("proxy-hash");
    item.expectedProxySize = 55;

    return item;
}

PrivacyAsset makeAsset(int role, qlonglong proxySize, const QString& publicRootUuid = rootUuid)
{
    PrivacyAsset asset;
    asset.itemUuid                 = itemUuid;
    asset.role                     = role;
    asset.ordinal                  = 0;
    asset.originalName             = (role == 1) ? QLatin1String("item.jpg")
                                                  : QLatin1String("item.xmp");
    asset.publicRootUuid           = publicRootUuid;
    asset.publicRelativePath       = (role == 1) ? QLatin1String("album/item.jpg")
                                                  : QLatin1String("album/item.xmp");
    asset.containerUuid            = QLatin1String("50000000-0000-0000-0000-000000000001");
    asset.protectedRelativePath    = (role == 1) ? QLatin1String("members/item.jpg")
                                                  : QLatin1String("members/item.xmp");
    asset.hashAlgorithm            = QLatin1String("sha256");
    asset.originalHash             = QLatin1String("original-hash");
    asset.originalSize             = 100;
    asset.proxyHashAlgorithm       = QLatin1String("sha256");
    asset.proxyHash                = QLatin1String("proxy-hash");
    asset.proxySize                = proxySize;
    asset.proxyPresentationVersion = 1;
    asset.proxyGeneration          = 3;

    return asset;
}

PrivacyContainer makeContainer(const QString& uuid,
                               const QString& relativePath,
                               qlonglong protectedSize)
{
    PrivacyContainer container;
    container.uuid                   = uuid;
    container.itemUuid               = itemUuid;
    container.kind                   = PrivacyContainerKind::CasualArchive;
    container.rootUuid               = rootUuid;
    container.objectRelativePath     = relativePath;
    container.protectedSize          = protectedSize;
    container.protectedHashAlgorithm = QLatin1String("sha256");
    container.protectedHash          = QLatin1String("synthetic-hash");
    container.formatVersion          = 1;
    container.credentialGeneration   = 1;
    container.state                  = PrivacyContainerState::Verified;
    container.createdAt              = QDateTime::currentDateTimeUtc();
    container.updatedAt              = container.createdAt;

    return container;
}

PrivacyTransaction makeCompatibilityTransaction()
{
    PrivacyTransaction transaction;
    transaction.uuid                 = QLatin1String("60000000-0000-0000-0000-000000000001");
    transaction.categoryUuid         = categoryUuid;
    transaction.itemUuid             = itemUuid;
    transaction.type                 = PrivacyTransactionType::CompatibilityUnlock;
    transaction.state                = PrivacyTransactionState::Exposed;
    transaction.generation           = 1;
    transaction.payloadFormatVersion = 1;
    transaction.createdAt            = QDateTime::currentDateTimeUtc();
    transaction.updatedAt            = transaction.createdAt;

    return transaction;
}

PrivacyTransactionJournal makeJournal()
{
    PrivacyTransactionJournal journal;
    journal.transactionUuid      = QLatin1String("60000000-0000-0000-0000-000000000001");
    journal.rootUuid             = rootUuid;
    journal.journalRelativePath  = QLatin1String("journals/compatibility.cbor");
    journal.journalFormatVersion = 1;
    journal.stage                = 1;
    journal.updatedAt            = QDateTime::currentDateTimeUtc();

    return journal;
}

PrivacyRepositorySnapshot makeSnapshot()
{
    PrivacyRepositorySnapshot snapshot;
    snapshot.categories   << makeCategory();
    snapshot.storageRoots << makeRoot(rootUuid, 9);
    snapshot.items        << makeItem();
    snapshot.assets       << makeAsset(PrivacyAsset::PrimaryMediaRole, 55)
                          << makeAsset(2, 999);
    snapshot.containers   << makeContainer(
        QLatin1String("50000000-0000-0000-0000-000000000001"),
        QLatin1String("album/item.jpg.digikam-private.zip"), 100);

    return snapshot;
}

PrivacyScanRequest makeScanRequest(int albumRootId, qlonglong size)
{
    PrivacyScanRequest request;
    request.albumRootId = albumRootId;
    request.imageId     = 42;
    request.absolutePath = QLatin1String("/synthetic/collection/album/item.jpg");
    request.pathExists   = true;
    request.byteSize     = size;

    return request;
}

} // namespace

class PrivacyRuntimeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testActionContracts();
    void testRootIdentityCodec();
    void testManagedRootMarkerVerification();
    void testScanGateFailsClosedWithoutProvider();
    void testExpectedProxyAndCanonicalAsset();
    void testOfflineAndMismatchedRoot();
    void testCompatibilityTransactionRecovery();
    void testReconnectRecoveryPipeline();
    void testRootIntegritySummary();
    void testStartupIssueSuppressionIsNarrow();
    void testRecoveryEpochCompareAndPublish();
    void testProductionStateProviders();
    void testCategorySessionOwnerShutdown();
    void testManualTagVisibilityProvider();
    void testAnalysisProvider();
    void testMixedRootAndConflictingMappings();
    void testDynamicAlbumRootRegistration();
    void testDynamicProtectedItemPublication();
    void testRootEpochTransition();
    void testTransactionStates();
};

void PrivacyRuntimeTest::testActionContracts()
{
    PrivacyActionItem logical;
    logical.imageId    = 42;
    logical.publicPath = QLatin1String("/synthetic/collection/item.jpg");

    PrivacyActionRequest request;
    request.actionKind        = PrivacyActionKind::Export;
    request.consumerIdentity  = QLatin1String("synthetic-exporter");
    request.items             << logical;
    request.requestedSource   = PrivacyRequestedSource::WritableCheckout;
    request.mutationPolicy    = PrivacyMutationPolicy::MayCreateOutputs;
    QVERIFY(request.isValid());

    PrivacyLeaseToken lease;
    lease.uuid            = QLatin1String("70000000-0000-0000-0000-000000000001");
    lease.itemUuid        = itemUuid;
    lease.itemGeneration  = 3;
    lease.categoryEpoch   = 1;
    lease.publicRootEpoch = 1;
    lease.storeRootEpoch  = 0;
    QVERIFY(lease.isValid());

    PreparedPrivacyItem allowed;
    allowed.logicalItem = logical;
    allowed.physicalPath = QLatin1String("/run/user/1000/private-checkout/item.jpg");
    allowed.disposition = PrivacyPreparedDisposition::Allowed;
    allowed.lease       = lease;
    QVERIFY(allowed.isValid());

    PreparedPrivacyItem ordinary;
    ordinary.logicalItem  = logical;
    ordinary.physicalPath = logical.publicPath;
    ordinary.disposition  = PrivacyPreparedDisposition::UnprotectedPassThrough;
    QVERIFY(ordinary.isValid());

    PreparedPrivacyItem excluded;
    excluded.logicalItem            = logical;
    excluded.logicalItem.imageId    = 43;
    excluded.logicalItem.publicPath = QLatin1String("/synthetic/collection/excluded.jpg");
    excluded.disposition            = PrivacyPreparedDisposition::Excluded;
    QVERIFY(excluded.isValid());

    PreparedPrivacySelection partial;
    partial.disposition = PrivacyPreparedDisposition::Allowed;
    partial.items       << allowed << excluded;
    QVERIFY(partial.isValid());

    PreparedPrivacySelection excludedAggregate = partial;
    excludedAggregate.disposition = PrivacyPreparedDisposition::Excluded;
    QVERIFY(!excludedAggregate.isValid());

    PreparedPrivacySelection canceledAggregate = partial;
    canceledAggregate.disposition = PrivacyPreparedDisposition::Canceled;
    QVERIFY(!canceledAggregate.isValid());

    PreparedPrivacySelection ordinarySelection;
    ordinarySelection.disposition = PrivacyPreparedDisposition::Allowed;
    ordinarySelection.items       << ordinary;
    QVERIFY(ordinarySelection.isValid());
}

void PrivacyRuntimeTest::testRootIdentityCodec()
{
    const QByteArray identity = PrivacyRootIdentityCodec::encodeAlbumRootV1(
                                    9, QLatin1String("synthetic-volume"));
    QVERIFY(!identity.isEmpty());
    QVERIFY(PrivacyRootIdentityCodec::matchesAlbumRootV1(
                identity, 9, QLatin1String("synthetic-volume")));
    QVERIFY(!PrivacyRootIdentityCodec::matchesAlbumRootV1(
                 identity, 10, QLatin1String("synthetic-volume")));
    QVERIFY(!PrivacyRootIdentityCodec::matchesAlbumRootV1(
                 identity, 9, QLatin1String("other-volume")));

    const QByteArray managedIdentity = PrivacyRootIdentityCodec::encodeManagedRootV1(
                                           markerUuid,
                                           QLatin1String("filesystem-uuid-v1:synthetic"));
    QVERIFY(PrivacyRootIdentityCodec::matchesManagedRootV1(
                managedIdentity, markerUuid,
                QLatin1String("filesystem-uuid-v1:synthetic")));
    QVERIFY(!PrivacyRootIdentityCodec::matchesManagedRootV1(
                 managedIdentity, markerUuid,
                 QLatin1String("filesystem-uuid-v1:other")));
}

void PrivacyRuntimeTest::testManagedRootMarkerVerification()
{
    QTemporaryDir temporaryRoot;
    QVERIFY(temporaryRoot.isValid());

    PrivacyStorageRoot root = makeManagedRoot(temporaryRoot.path());

    const QSharedPointer<const PrivacyRootVerifier> verifier =
        createDefaultPrivacyRootVerifier();
    QCOMPARE(verifier->verify(root), PrivacyRootRuntimeState::IdentityMismatch);

    QVERIFY(writeManagedRootMarker(root));
    QCOMPARE(verifier->verify(root), PrivacyRootRuntimeState::VerifiedAvailable);

    QVERIFY(writeManagedRootMarker(
                root, QLatin1String("40000000-0000-0000-0000-000000000002")));
    QCOMPARE(verifier->verify(root), PrivacyRootRuntimeState::IdentityMismatch);

    const QString markerPath = QDir(root.configuredPath).filePath(
                                       PrivacyRootIdentityCodec::managedRootMarkerRelativePathV1());
    QVERIFY(QFile::remove(markerPath));
    const QString outsideMarker = QDir(root.configuredPath).filePath(
                                      QLatin1String("outside-marker.json"));
    QVERIFY(writeSizedFile(outsideMarker, 32));
    QVERIFY(QFile::link(outsideMarker, markerPath));
    QCOMPARE(verifier->verify(root), PrivacyRootRuntimeState::IdentityMismatch);

    QVERIFY(QFile::remove(markerPath));
    QVERIFY(writeSizedFile(markerPath, 4097));
    QCOMPARE(verifier->verify(root), PrivacyRootRuntimeState::IdentityMismatch);

    root.configuredPath = QDir(temporaryRoot.path()).filePath(QLatin1String("offline"));
    QCOMPARE(verifier->verify(root), PrivacyRootRuntimeState::Offline);

    root.configuredPath = temporaryRoot.path();
    QVERIFY(writeManagedRootMarker(root));
    root.identityData = PrivacyRootIdentityCodec::encodeManagedRootV1(
                            markerUuid, QLatin1String("filesystem-uuid-v1:wrong"));
    QCOMPARE(verifier->verify(root), PrivacyRootRuntimeState::IdentityMismatch);
}

void PrivacyRuntimeTest::testScanGateFailsClosedWithoutProvider()
{
    PrivacyScanGate::resetProvider();
    QCOMPARE(PrivacyScanGate::evaluate(PrivacyScanRequest()),
             PrivacyScanDisposition::RootRecovering);
    QVERIFY(PrivacyScanGate::hasDeferredRoots());
}

void PrivacyRuntimeTest::testExpectedProxyAndCanonicalAsset()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);

    PrivacyRuntimeCoordinator runtime;
    PrivacyStartupReport report = runtime.initialize(makeSnapshot(), verifier, {}, {});
    QCOMPARE(report.state, PrivacyStartupState::Degraded);
    QCOMPARE(runtime.evaluate(makeScanRequest(9, 55)),
             PrivacyScanDisposition::RootRecovering);
    QCOMPARE(runtime.publicSourceDisposition(42), PrivacyPublicSourceDisposition::Denied);

    report = runtime.initialize(makeSnapshot(), verifier, {}, integrity);
    QCOMPARE(report.state, PrivacyStartupState::Ready);
    QVERIFY(runtime.rootContainsProtectedItems(9));
    QCOMPARE(runtime.publicSourceDisposition(42), PrivacyPublicSourceDisposition::LockedProxy);
    QVERIFY(!runtime.unregisterUnreferencedAlbumRoot(rootUuid));
    QVERIFY(!runtime.publicSourceCacheNamespace(42).isEmpty());
    QCOMPARE(runtime.expectedPublicProxySize(42), 55LL);
    QCOMPARE(runtime.publicSourceDisposition(999), PrivacyPublicSourceDisposition::Unprotected);
    QCOMPARE(runtime.evaluate(makeScanRequest(9, 55)),
             PrivacyScanDisposition::ProtectedProxyExpected);
    QCOMPARE(runtime.evaluate(makeScanRequest(9, 999)),
             PrivacyScanDisposition::PrivacyInspectionRequired);

    PrivacyRepositorySnapshot wrongRootSnapshot = makeSnapshot();
    wrongRootSnapshot.storageRoots << makeRoot(otherRootUuid, 10);
    verifier->states.insert(otherRootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    runtime.initialize(wrongRootSnapshot, verifier, {}, integrity);
    QCOMPARE(runtime.evaluate(makeScanRequest(10, 55)),
             PrivacyScanDisposition::PrivacyInspectionRequired);
}

void PrivacyRuntimeTest::testOfflineAndMismatchedRoot()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::Offline);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);

    PrivacyRuntimeCoordinator runtime;
    PrivacyStartupReport report = runtime.initialize(makeSnapshot(), verifier, {}, integrity);
    QCOMPARE(report.state, PrivacyStartupState::Degraded);
    QCOMPARE(report.offlineRootCount, 1);
    QCOMPARE(report.roots.size(), 1);
    QCOMPARE(report.roots.constFirst().protectedItemCount, 1);
    QCOMPARE(report.roots.constFirst().state, PrivacyRootRuntimeState::Offline);
    QCOMPARE(runtime.evaluate(makeScanRequest(9, 55)), PrivacyScanDisposition::RootOffline);
    QVERIFY(runtime.hasDeferredRoots());

    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::IdentityMismatch);
    report = runtime.initialize(makeSnapshot(), verifier, {}, integrity);
    QCOMPARE(report.mismatchedRootCount, 1);
    QVERIFY(report.roots.constFirst().identityMismatch);
    QCOMPARE(runtime.evaluate(makeScanRequest(9, 55)),
             PrivacyScanDisposition::RootIdentityMismatch);
    PrivacyActionItemState actionState;
    QVERIFY(runtime.stateForItem(42, &actionState));
    QCOMPARE(actionState.publicRootState, PrivacyRootRuntimeState::IdentityMismatch);
    QVERIFY(!actionState.proxyReady);
}

void PrivacyRuntimeTest::testCompatibilityTransactionRecovery()
{
    PrivacyRepositorySnapshot snapshot = makeSnapshot();
    snapshot.transactions       << makeCompatibilityTransaction();
    snapshot.transactionJournals << makeJournal();

    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);

    PrivacyRuntimeCoordinator runtime;
    PrivacyStartupReport report = runtime.initialize(snapshot, verifier, {}, integrity);
    QCOMPARE(report.unresolvedTransactionCount, 1);
    QCOMPARE(runtime.evaluate(makeScanRequest(9, 55)),
             PrivacyScanDisposition::CompatibilityOriginalExposed);
    PrivacyActionItemState actionState;
    QVERIFY(runtime.stateForItem(42, &actionState));
    QVERIFY(actionState.unresolvedTransaction);
    PrivacyLeaseCurrentState leaseState;
    QVERIFY(runtime.currentState(itemUuid, &leaseState));
    QVERIFY(leaseState.unresolvedTransaction);

    const QSharedPointer<FakeRecovery> recovery(new FakeRecovery);
    bool recoveryObservedPublishedRuntime = false;
    recovery->onRecover = [&]()
    {
        PrivacyActionItemState recoveringState;
        recoveryObservedPublishedRuntime =
            runtime.stateForItem(42, &recoveringState) &&
            recoveringState.unresolvedTransaction &&
            (runtime.rootState(rootUuid) ==
             PrivacyRootRuntimeState::Recovering);
    };
    report = runtime.initialize(snapshot, verifier, recovery, integrity);
    QVERIFY(recoveryObservedPublishedRuntime);
    QCOMPARE(integrity->callCount, 1);
    QCOMPARE(report.state, PrivacyStartupState::Ready);
    QCOMPARE(report.unresolvedTransactionCount, 0);
    QCOMPARE(runtime.evaluate(makeScanRequest(9, 55)),
             PrivacyScanDisposition::ProtectedProxyExpected);
    QVERIFY(runtime.stateForItem(42, &actionState));
    QVERIFY(!actionState.unresolvedTransaction);
}

void PrivacyRuntimeTest::testReconnectRecoveryPipeline()
{
    PrivacyRepositorySnapshot snapshot = makeSnapshot();
    snapshot.transactions        << makeCompatibilityTransaction();
    snapshot.transactionJournals << makeJournal();

    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeRecovery> recovery(new FakeRecovery);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);

    PrivacyRuntimeCoordinator runtime;
    recovery->disposition = PrivacyRecoveryDisposition::Deferred;
    PrivacyStartupReport report = runtime.initialize(snapshot, verifier, recovery,
                                                      integrity);
    QCOMPARE(report.state, PrivacyStartupState::Degraded);

    recovery->disposition = PrivacyRecoveryDisposition::Recovered;
    recovery->onRecover = [&runtime]()
    {
        QCOMPARE(runtime.rootSummary(rootUuid).unresolvedTransactionCount, 1);
        QCOMPARE(runtime.rootSummary(rootUuid).compatibilityExposureCount, 1);
        QVERIFY(runtime.publishRootState(
            rootUuid, PrivacyRootRuntimeState::VerifiedAvailable));
    };

    QVERIFY(runtime.publishRootState(rootUuid, PrivacyRootRuntimeState::Offline));
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::Offline);
    QCOMPARE(runtime.recoverRoot(rootUuid), PrivacyRootRecoveryResult::PublishedVerified);
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::VerifiedAvailable);
    QVERIFY(recovery->callCount >= 2);

    recovery->disposition = PrivacyRecoveryDisposition::Deferred;
    recovery->onRecover = {};
    report = runtime.initialize(snapshot, verifier, recovery, integrity);
    QCOMPARE(report.state, PrivacyStartupState::Degraded);
    QCOMPARE(runtime.recoverRoot(rootUuid), PrivacyRootRecoveryResult::Deferred);
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::Recovering);
    QCOMPARE(runtime.rootSummary(rootUuid).unresolvedTransactionCount, 1);
    QCOMPARE(runtime.rootSummary(rootUuid).compatibilityExposureCount, 1);

    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::IdentityMismatch);
    QCOMPARE(runtime.recoverRoot(rootUuid),
             PrivacyRootRecoveryResult::PublishedIdentityMismatch);
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::IdentityMismatch);

    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::Offline);
    QCOMPARE(runtime.recoverRoot(rootUuid), PrivacyRootRecoveryResult::PublishedOffline);
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::Offline);
}

void PrivacyRuntimeTest::testRootIntegritySummary()
{
    QTemporaryDir temporaryRoot;
    QVERIFY(temporaryRoot.isValid());

    const PrivacyStorageRoot root = makeManagedRoot(temporaryRoot.path());
    PrivacyRepositorySnapshot snapshot;
    snapshot.categories << makeCategory();
    snapshot.storageRoots << root;
    snapshot.items << makeItem();

    PrivacyAsset missingProxy = makeAsset(PrivacyAsset::PrimaryMediaRole, 55);
    missingProxy.publicRelativePath = QLatin1String("album/missing.jpg");
    PrivacyAsset changedProxy = makeAsset(2, 12);
    changedProxy.ordinal = 1;
    changedProxy.publicRelativePath = QLatin1String("album/changed.xmp");
    PrivacyAsset sameSizeProxy = makeAsset(3, 8);
    sameSizeProxy.ordinal = 2;
    sameSizeProxy.publicRelativePath = QLatin1String("album/same-size.jpg");
    snapshot.assets << missingProxy << changedProxy << sameSizeProxy;

    snapshot.containers << makeContainer(
        QLatin1String("50000000-0000-0000-0000-000000000001"),
        QLatin1String("album/missing.jpg.digikam-private.zip"), 100);
    snapshot.containers << makeContainer(
        QLatin1String("50000000-0000-0000-0000-000000000002"),
        QLatin1String("album/changed.jpg.digikam-private.zip"), 20);

    QVERIFY(writeSizedFile(QDir(temporaryRoot.path()).filePath(changedProxy.publicRelativePath), 7));
    const QString sameSizePath = QDir(temporaryRoot.path()).filePath(
                                     sameSizeProxy.publicRelativePath);
    QVERIFY(writeSizedFile(sameSizePath, 8));
    QFile sameSizeFile(sameSizePath);
    QVERIFY(sameSizeFile.open(QIODevice::ReadWrite));
    QVERIFY(sameSizeFile.setFileTime(QDateTime::currentDateTimeUtc().addDays(-1),
                                     QFileDevice::FileModificationTime));
    sameSizeFile.close();
    QVERIFY(writeSizedFile(QDir(temporaryRoot.path()).filePath(
                               QLatin1String("album/changed.jpg.digikam-private.zip")), 9));

    const QSharedPointer<const PrivacyRootIntegrityInspector> inspector =
        createDefaultPrivacyRootIntegrityInspector();
    const PrivacyRootInspectionResult result = inspector->inspect(root, snapshot);
    QCOMPARE(result.disposition, PrivacyIntegrityDisposition::Verified);
    QCOMPARE(result.summary.protectedItemCount, 1);
    QCOMPARE(result.summary.missingProxyCount, 1);
    QCOMPARE(result.summary.changedProxySizeCount, 1);
    QCOMPARE(result.summary.missingProtectedObjectCount, 1);
    QCOMPARE(result.summary.changedProtectedObjectSizeCount, 1);

    // Equal byte size is intentionally sufficient for the startup pass;
    // metadata/wrapper validation remains an on-display operation.
    QCOMPARE(result.summary.unexpectedPublicAssetCount, 0);

    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    PrivacyRuntimeCoordinator runtime;
    const PrivacyStartupReport report = runtime.initialize(snapshot, verifier, {}, inspector);
    QCOMPARE(report.state, PrivacyStartupState::Ready);
    QCOMPARE(report.roots.size(), 1);
    QCOMPARE(report.roots.constFirst().configuredPath, root.configuredPath);
    QCOMPARE(report.roots.constFirst().missingProxyCount, 1);
    QCOMPARE(report.roots.constFirst().changedProxySizeCount, 1);
    QCOMPARE(report.roots.constFirst().missingProtectedObjectCount, 1);
    QCOMPARE(report.roots.constFirst().changedProtectedObjectSizeCount, 1);

    PrivacyActionItemState actionState;
    QVERIFY(runtime.stateForItem(42, &actionState));
    QCOMPARE(actionState.publicRootState, PrivacyRootRuntimeState::VerifiedAvailable);
    QCOMPARE(actionState.originalRootState, PrivacyRootRuntimeState::VerifiedAvailable);
    QVERIFY(!actionState.proxyReady);
    QVERIFY(!actionState.originalReady);
}

void PrivacyRuntimeTest::testStartupIssueSuppressionIsNarrow()
{
    PrivacyStartupReport report;
    report.state = PrivacyStartupState::Ready;
    PrivacyRootIntegritySummary root;
    root.rootUuid = rootUuid;
    root.configuredPath = QLatin1String("/synthetic/collection");
    root.state = PrivacyRootRuntimeState::VerifiedAvailable;
    report.roots << root;
    QVERIFY(!report.hasReportableIssues(false));
    QVERIFY(!report.hasOnlyProxySizeIssues());

    report.roots[0].changedProxySizeCount = 2;
    QVERIFY(report.hasReportableIssues(false));
    QVERIFY(report.hasOnlyProxySizeIssues());
    QVERIFY(!report.hasReportableIssues(true));

    report.roots[0].missingProxyCount = 1;
    QVERIFY(report.hasReportableIssues(true));
    QVERIFY(!report.hasOnlyProxySizeIssues());

    report.roots[0].missingProxyCount = 0;
    report.roots[0].state = PrivacyRootRuntimeState::Offline;
    report.state = PrivacyStartupState::Degraded;
    report.offlineRootCount = 1;
    QVERIFY(report.hasReportableIssues(true));
    QVERIFY(!report.hasOnlyProxySizeIssues());

    report.roots[0].state = PrivacyRootRuntimeState::VerifiedAvailable;
    report.state = PrivacyStartupState::Ready;
    report.offlineRootCount = 0;
    report.diagnostics << QLatin1String("synthetic recovery diagnostic");
    QVERIFY(report.hasReportableIssues(true));
    QVERIFY(!report.hasOnlyProxySizeIssues());
}

void PrivacyRuntimeTest::testRecoveryEpochCompareAndPublish()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);

    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(makeSnapshot(), verifier, {}, integrity);

    integrity->onInspect = [&runtime]()
    {
        QVERIFY(runtime.publishRootState(rootUuid, PrivacyRootRuntimeState::Offline));
    };

    QCOMPARE(runtime.recoverRoot(rootUuid), PrivacyRootRecoveryResult::StaleEpoch);
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::Offline);
}

void PrivacyRuntimeTest::testProductionStateProviders()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);
    const QSharedPointer<PrivacyRuntimeCoordinator> runtime(new PrivacyRuntimeCoordinator);
    QCOMPARE(runtime->initialize(makeSnapshot(), verifier, {}, integrity).state,
             PrivacyStartupState::Ready);

    PrivacyActionItemState actionState;
    QVERIFY(runtime->stateForItem(42, &actionState));
    QVERIFY(actionState.isValid());
    QVERIFY(actionState.protectedItem);
    QCOMPARE(actionState.access, PrivacyItemAccess::Locked);
    QCOMPARE(actionState.publicRootState, PrivacyRootRuntimeState::VerifiedAvailable);
    QVERIFY(actionState.proxyReady);
    QVERIFY(actionState.originalReady);

    PrivacyActionRequest proxyRequest;
    proxyRequest.actionKind = PrivacyActionKind::Preview;
    proxyRequest.consumerIdentity = QLatin1String("synthetic-runtime-provider");
    proxyRequest.requestedSource = PrivacyRequestedSource::PublicProxy;
    proxyRequest.mutationPolicy = PrivacyMutationPolicy::ReadOnly;
    PrivacyActionItem logicalItem;
    logicalItem.imageId = 42;
    logicalItem.publicPath = QLatin1String("/synthetic/untrusted-input-path.jpg");
    proxyRequest.items << logicalItem;
    const PrivacyActionPolicyResult policyResult =
        PrivacyActionPolicy::classify(proxyRequest, *runtime);
    QVERIFY(policyResult.isImmediatelyReady());
    QCOMPARE(policyResult.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::ReadyWithProxy);

    PrivacyActionItemState ordinaryState;
    QVERIFY(runtime->stateForItem(999, &ordinaryState));
    QVERIFY(ordinaryState.isValid());
    QVERIFY(!ordinaryState.protectedItem);

    PrivacyLeaseCurrentState leaseState;
    QVERIFY(runtime->currentState(itemUuid, &leaseState));
    QVERIFY(leaseState.isValid());
    QVERIFY(!leaseState.categoryUnlocked);
    QVERIFY(leaseState.publicRootAvailable);
    QVERIFY(leaseState.storeRootAvailable);

    const QSharedPointer<const PrivacyLeaseStateProvider> leaseProvider = runtime;
    PrivacyLeaseRegistry registry(leaseProvider);
    const PrivacyLeaseToken lockedProxyLease = registry.issue(itemUuid, false);
    QVERIFY(lockedProxyLease.isValid());
    QVERIFY(!registry.issue(itemUuid, true).isValid());

    QVERIFY(runtime->setCategoryUnlocked(categoryUuid, true));
    QCOMPARE(registry.validate(lockedProxyLease), PrivacyLeaseValidation::StateChanged);
    const PrivacyLeaseToken originalLease = registry.issue(itemUuid, true);
    QVERIFY(originalLease.isValid());

    QVERIFY(runtime->setCategoryUnlocked(categoryUuid, false));
    QCOMPARE(registry.validate(originalLease), PrivacyLeaseValidation::StateChanged);
    QVERIFY(runtime->stateForItem(42, &actionState));
    QCOMPARE(actionState.access, PrivacyItemAccess::Locked);

    QVERIFY(runtime->setCategoryUnlocked(categoryUuid, true));
    const PrivacyLeaseToken beforeDisconnect = registry.issue(itemUuid, true);
    QVERIFY(beforeDisconnect.isValid());
    QVERIFY(runtime->publishRootState(rootUuid, PrivacyRootRuntimeState::Offline));
    QCOMPARE(registry.validate(beforeDisconnect), PrivacyLeaseValidation::RootUnavailable);
    QVERIFY(runtime->stateForItem(42, &actionState));
    QCOMPARE(actionState.publicRootState, PrivacyRootRuntimeState::Offline);
    QVERIFY(!actionState.proxyReady);

    QCOMPARE(runtime->recoverRoot(rootUuid), PrivacyRootRecoveryResult::PublishedVerified);
    QCOMPARE(registry.validate(beforeDisconnect), PrivacyLeaseValidation::StateChanged);
    QVERIFY(runtime->stateForItem(42, &actionState));
    QCOMPARE(actionState.publicRootState, PrivacyRootRuntimeState::VerifiedAvailable);
    QVERIFY(actionState.proxyReady);
    const PrivacyLeaseToken beforeCommit = registry.issue(itemUuid, true);
    QVERIFY(beforeCommit.isValid());
    QVERIFY(runtime->compareAndSetItemGeneration(42, 3, 4));
    QVERIFY(!runtime->compareAndSetItemGeneration(42, 3, 5));
    QCOMPARE(registry.validate(beforeCommit), PrivacyLeaseValidation::StateChanged);

    QVERIFY(runtime->stateForItem(42, &actionState));
    QCOMPARE(actionState.itemGeneration, 4LL);
    QVERIFY(!runtime->currentState(otherItemUuid, &leaseState));

    const PrivacyLeaseToken beforeRuntimeReset = registry.issue(itemUuid, true);
    QVERIFY(beforeRuntimeReset.isValid());
    runtime->reset();
    QCOMPARE(registry.validate(beforeRuntimeReset), PrivacyLeaseValidation::StateChanged);
    QVERIFY(!runtime->stateForItem(42, &actionState));
}

void PrivacyRuntimeTest::testCategorySessionOwnerShutdown()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);
    const QSharedPointer<PrivacyRuntimeCoordinator> runtime(new PrivacyRuntimeCoordinator);
    QCOMPARE(runtime->initialize(makeSnapshot(), verifier, {}, integrity).state,
             PrivacyStartupState::Ready);

    QVERIFY(!PrivacyCategorySessionOwner::create({}, verifier));
    QVERIFY(!PrivacyCategorySessionOwner::create(runtime, {}));

    const QSharedPointer<PrivacyCategorySessionOwner> owner =
        PrivacyCategorySessionOwner::create(runtime, verifier);
    QVERIFY(owner);
    QCOMPARE(owner->runWhileUnlocked(categoryUuid, [] {}),
             PrivacyCategoryOperationStatus::CategoryLocked);

    owner->shutdown();
    QCOMPARE(owner->runWhileUnlocked(categoryUuid, [] {}),
             PrivacyCategoryOperationStatus::TransactionBlocked);
    QCOMPARE(owner->unlockCategory(categoryUuid, QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::TransactionBlocked);
    QCOMPARE(owner->lockAllCategories().constFirst().status,
             PrivacyCategorySessionStatus::TransactionBlocked);
    QVERIFY(!owner->ownsSecret(categoryUuid));
    owner->shutdown();
}

void PrivacyRuntimeTest::testManualTagVisibilityProvider()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);
    const QSharedPointer<PrivacyRuntimeCoordinator> runtime(new PrivacyRuntimeCoordinator);

    QVERIFY(!runtime->mayAccessManualTags(42));
    QCOMPARE(runtime->initialize(makeSnapshot(), verifier, {}, integrity).state,
             PrivacyStartupState::Ready);

    const QSharedPointer<const PrivacyManualTagVisibilityProvider> provider = runtime;
    QVERIFY(!provider->mayAccessManualTags(42));
    QVERIFY(provider->mayAccessManualTags(999));
    QVERIFY(!provider->mayAccessManualTags(-1));

    QVERIFY(!runtime->setCategoryTagVisibilityMode(
        categoryUuid, PrivacyTagVisibilityMode::AlwaysVisible, false));
    QVERIFY(!provider->mayAccessManualTags(42));
    QVERIFY(runtime->setCategoryTagVisibilityMode(
        categoryUuid, PrivacyTagVisibilityMode::AlwaysVisible, true));
    QVERIFY(provider->mayAccessManualTags(42));

    QVERIFY(runtime->setCategoryTagVisibilityMode(
        categoryUuid, PrivacyTagVisibilityMode::UnlockedOnly, false));
    QVERIFY(!provider->mayAccessManualTags(42));
    QVERIFY(runtime->setCategoryUnlocked(categoryUuid, true));
    QVERIFY(provider->mayAccessManualTags(42));
    QVERIFY(!runtime->setCategoryTagVisibilityMode(
        categoryUuid, static_cast<PrivacyTagVisibilityMode>(99), true));

    runtime->reset();
    QVERIFY(!provider->mayAccessManualTags(42));

    PrivacyRepositorySnapshot invalid = makeSnapshot();
    invalid.categories[0].tagVisibilityMode = static_cast<PrivacyTagVisibilityMode>(99);
    QCOMPARE(runtime->initialize(invalid, verifier, {}, integrity).state,
             PrivacyStartupState::Ready);
    QVERIFY(!provider->mayAccessManualTags(42));
}

void PrivacyRuntimeTest::testAnalysisProvider()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);
    PrivacyRuntimeCoordinator runtime;

    QCOMPARE(runtime.analysisDisposition(42), PrivacyAnalysisDisposition::Unavailable);
    QCOMPARE(runtime.initialize(makeSnapshot(), verifier, {}, integrity).state,
             PrivacyStartupState::Ready);
    QCOMPARE(runtime.analysisDisposition(42),
             PrivacyAnalysisDisposition::ProtectedExcluded);
    QCOMPARE(runtime.analysisDisposition(999), PrivacyAnalysisDisposition::Allowed);
    QCOMPARE(runtime.analysisDisposition(-1), PrivacyAnalysisDisposition::Unavailable);

    QVERIFY(runtime.setCategoryUnlocked(categoryUuid, true));
    QCOMPARE(runtime.analysisDisposition(42),
             PrivacyAnalysisDisposition::ProtectedExcluded);
    QVERIFY(runtime.setCategoryTagVisibilityMode(
        categoryUuid, PrivacyTagVisibilityMode::AlwaysVisible, true));
    QCOMPARE(runtime.analysisDisposition(42),
             PrivacyAnalysisDisposition::ProtectedExcluded);

    PrivacyRepositorySnapshot exposed = makeSnapshot();
    exposed.transactions << makeCompatibilityTransaction();
    exposed.transactionJournals << makeJournal();
    runtime.initialize(exposed, verifier, {}, integrity);
    QCOMPARE(runtime.analysisDisposition(42),
             PrivacyAnalysisDisposition::ProtectedExcluded);
}

void PrivacyRuntimeTest::testMixedRootAndConflictingMappings()
{
    PrivacyRepositorySnapshot snapshot = makeSnapshot();
    snapshot.storageRoots << makeRoot(otherRootUuid, 10);

    PrivacyItem otherItem = makeItem();
    otherItem.imageId = 43;
    otherItem.uuid = otherItemUuid;
    snapshot.items << otherItem;

    PrivacyAsset otherAsset = makeAsset(
        PrivacyAsset::PrimaryMediaRole, 66, rootUuid);
    otherAsset.itemUuid = otherItemUuid;
    otherAsset.originalName = QLatin1String("other.jpg");
    otherAsset.publicRelativePath = QLatin1String("album/other.jpg");
    otherAsset.containerUuid = QLatin1String("50000000-0000-0000-0000-000000000002");
    snapshot.assets << otherAsset;

    const QString otherStoreUuid =
        QLatin1String("70000000-0000-0000-0000-000000000002");
    PrivacyStore otherStore;
    otherStore.uuid = otherStoreUuid;
    otherStore.categoryUuid = categoryUuid;
    otherStore.rootUuid = otherRootUuid;
    snapshot.stores << otherStore;

    PrivacyContainer otherContainer = makeContainer(
        QLatin1String("50000000-0000-0000-0000-000000000002"),
        QLatin1String("album/other.jpg.digikam-private.zip"), 120);
    otherContainer.itemUuid = otherItemUuid;
    otherContainer.kind = PrivacyContainerKind::StrongObject;
    otherContainer.rootUuid.clear();
    otherContainer.storeUuid = otherStoreUuid;
    otherContainer.objectRelativePath = QLatin1String("originals/other.jpg");
    snapshot.containers << otherContainer;

    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    verifier->states.insert(otherRootUuid, PrivacyRootRuntimeState::Offline);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);

    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(snapshot, verifier, {}, integrity);

    PrivacyActionItemState firstState;
    PrivacyActionItemState secondState;
    QVERIFY(runtime.stateForItem(42, &firstState));
    QVERIFY(runtime.stateForItem(43, &secondState));
    QCOMPARE(firstState.publicRootState, PrivacyRootRuntimeState::VerifiedAvailable);
    QCOMPARE(secondState.publicRootState, PrivacyRootRuntimeState::VerifiedAvailable);
    QCOMPARE(secondState.originalRootState, PrivacyRootRuntimeState::Offline);
    QVERIFY(firstState.proxyReady);
    QVERIFY(secondState.proxyReady);
    QVERIFY(!secondState.originalReady);

    PrivacyLeaseCurrentState secondLeaseState;
    QVERIFY(runtime.currentState(otherItemUuid, &secondLeaseState));
    QVERIFY(secondLeaseState.publicRootAvailable);
    QVERIFY(!secondLeaseState.storeRootAvailable);

    PrivacyRepositorySnapshot conflicting = snapshot;
    PrivacyAsset duplicatePrimary = otherAsset;
    duplicatePrimary.itemUuid = itemUuid;
    duplicatePrimary.containerUuid =
        QLatin1String("50000000-0000-0000-0000-000000000001");
    conflicting.assets << duplicatePrimary;
    runtime.initialize(conflicting, verifier, {}, integrity);
    QVERIFY(!runtime.stateForItem(42, &firstState));
    QVERIFY(!runtime.mayAccessManualTags(42));
    QCOMPARE(runtime.analysisDisposition(42), PrivacyAnalysisDisposition::Unavailable);
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::Recovering);
    QCOMPARE(runtime.rootState(otherRootUuid), PrivacyRootRuntimeState::Offline);

    PrivacyLeaseCurrentState firstLeaseState;
    QVERIFY(!runtime.currentState(itemUuid, &firstLeaseState));
    QVERIFY(!runtime.currentState(
                QLatin1String("20000000-0000-0000-0000-000000000099"),
                &firstLeaseState));
}

void PrivacyRuntimeTest::testDynamicAlbumRootRegistration()
{
    const QString registeredUuid =
        QLatin1String("30000000-0000-0000-0000-000000000010");
    const QString offlineUuid =
        QLatin1String("30000000-0000-0000-0000-000000000011");
    const QString mismatchUuid =
        QLatin1String("30000000-0000-0000-0000-000000000012");
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    verifier->states.insert(registeredUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    verifier->states.insert(offlineUuid, PrivacyRootRuntimeState::Offline);
    verifier->states.insert(mismatchUuid, PrivacyRootRuntimeState::IdentityMismatch);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);
    PrivacyRuntimeCoordinator runtime;
    QCOMPARE(runtime.initialize(makeSnapshot(), verifier, {}, integrity).state,
             PrivacyStartupState::Ready);
    QVERIFY(runtime.setCategoryUnlocked(categoryUuid, true));

    const PrivacyStorageRoot registered = makeRoot(registeredUuid, 10);
    QCOMPARE(runtime.registerAlbumRoot(registered),
             PrivacyRootRecoveryResult::PublishedVerified);
    QCOMPARE(runtime.rootUuidForAlbumRootId(10), registeredUuid);
    QCOMPARE(runtime.rootState(registeredUuid),
             PrivacyRootRuntimeState::VerifiedAvailable);
    QVERIFY(!runtime.rootContainsProtectedItems(10));
    const quint64 firstEpoch = runtime.rootEpoch(registeredUuid);
    QVERIFY(firstEpoch > 0);

    PrivacyActionItemState itemState;
    QVERIFY(runtime.stateForItem(42, &itemState));
    QCOMPARE(itemState.access, PrivacyItemAccess::Unlocked);

    QCOMPARE(runtime.registerAlbumRoot(registered),
             PrivacyRootRecoveryResult::PublishedVerified);
    QVERIFY(runtime.rootEpoch(registeredUuid) > firstEpoch);
    QVERIFY(runtime.stateForItem(42, &itemState));
    QCOMPARE(itemState.access, PrivacyItemAccess::Unlocked);

    QCOMPARE(runtime.registerAlbumRoot(makeRoot(
                 QLatin1String("30000000-0000-0000-0000-000000000099"), 10)),
             PrivacyRootRecoveryResult::Deferred);
    QCOMPARE(runtime.registerAlbumRoot(makeRoot(registeredUuid, 99)),
             PrivacyRootRecoveryResult::Deferred);
    QCOMPARE(runtime.rootUuidForAlbumRootId(10), registeredUuid);
    QVERIFY(runtime.unregisterUnreferencedAlbumRoot(registeredUuid));
    QVERIFY(runtime.rootUuidForAlbumRootId(10).isEmpty());
    QCOMPARE(runtime.rootState(registeredUuid), PrivacyRootRuntimeState::Unknown);
    QVERIFY(runtime.unregisterUnreferencedAlbumRoot(registeredUuid));
    QVERIFY(runtime.stateForItem(42, &itemState));
    QCOMPARE(itemState.access, PrivacyItemAccess::Unlocked);

    QCOMPARE(runtime.registerAlbumRoot(makeRoot(offlineUuid, 11)),
             PrivacyRootRecoveryResult::PublishedOffline);
    QCOMPARE(runtime.rootState(offlineUuid), PrivacyRootRuntimeState::Offline);
    QCOMPARE(runtime.registerAlbumRoot(makeRoot(mismatchUuid, 12)),
             PrivacyRootRecoveryResult::PublishedIdentityMismatch);
    QCOMPARE(runtime.rootState(mismatchUuid),
             PrivacyRootRuntimeState::IdentityMismatch);
}

void PrivacyRuntimeTest::testDynamicProtectedItemPublication()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);

    PrivacyRepositorySnapshot snapshot;
    snapshot.categories << makeCategory();
    snapshot.storageRoots << makeRoot(rootUuid, 9);

    PrivacyRuntimeCoordinator runtime;
    QCOMPARE(runtime.initialize(snapshot, verifier, {}, {}).state,
             PrivacyStartupState::Ready);
    QVERIFY(!runtime.isCategoryUnlocked(categoryUuid));
    QVERIFY(runtime.setCategoryUnlocked(categoryUuid, true));
    QVERIFY(runtime.isCategoryUnlocked(categoryUuid));

    const PrivacyItem item = makePublishableItem();
    const PrivacyContainer container = makeContainer(
        QLatin1String("50000000-0000-0000-0000-000000000001"),
        QLatin1String("album/item.jpg.digikam-private.zip"), 100);
    const PrivacyAsset primary = makeAsset(PrivacyAsset::PrimaryMediaRole, 55);
    QVERIFY(!runtime.hasProtectedItem(item, container, { primary }));

    PrivacyAsset noPrimary = primary;
    noPrimary.role = 2;
    const quint64 beforeRejectedPublish = runtime.categoryEpoch(categoryUuid);
    QVERIFY(!runtime.publishProtectedItem(item, container, { noPrimary }));

    PrivacyContainer strongContainer = container;
    strongContainer.kind = PrivacyContainerKind::StrongObject;
    strongContainer.rootUuid.clear();
    strongContainer.storeUuid =
        QLatin1String("70000000-0000-0000-0000-000000000001");
    strongContainer.objectRelativePath = QLatin1String("originals/item.jpg");
    QVERIFY(strongContainer.isValid());
    QVERIFY(!runtime.publishProtectedItem(item, strongContainer, { primary }));
    QCOMPARE(runtime.categoryEpoch(categoryUuid), beforeRejectedPublish);

    PrivacyRepositorySnapshot partialSnapshot = snapshot;
    partialSnapshot.containers << container;
    const QSharedPointer<FakeIntegrityInspector> integrity(
        new FakeIntegrityInspector);
    PrivacyRuntimeCoordinator partialRuntime;
    QCOMPARE(partialRuntime.initialize(partialSnapshot, verifier, {}, integrity).state,
             PrivacyStartupState::Ready);
    QVERIFY(partialRuntime.setCategoryUnlocked(categoryUuid, true));
    QVERIFY(!partialRuntime.publishProtectedItem(item, container, { primary }));
    QVERIFY(!partialRuntime.hasProtectedItem(item, container, { primary }));

    const quint64 beforePublish = runtime.categoryEpoch(categoryUuid);
    QVERIFY(runtime.publishProtectedItem(item, container, { primary }));
    QVERIFY(runtime.categoryEpoch(categoryUuid) > beforePublish);
    QCOMPARE(runtime.rootSummary(rootUuid).protectedItemCount, 1);
    QVERIFY(runtime.rootContainsProtectedItems(9));
    QCOMPARE(runtime.publicSourceDisposition(item.imageId),
             PrivacyPublicSourceDisposition::LockedProxy);

    PrivacyActionItemState actionState;
    QVERIFY(runtime.stateForItem(item.imageId, &actionState));
    QVERIFY(actionState.protectedItem);
    QCOMPARE(actionState.access, PrivacyItemAccess::Unlocked);
    QVERIFY(actionState.proxyReady);
    QVERIFY(actionState.originalReady);

    PrivacyLeaseCurrentState leaseState;
    QVERIFY(runtime.currentState(item.uuid, &leaseState));
    QVERIFY(leaseState.categoryUnlocked);
    QVERIFY(runtime.hasProtectedItem(item, container, { primary }));
    QVERIFY(!runtime.publishProtectedItem(item, container, { primary }));

    PrivacyAsset mismatchedAsset = primary;
    mismatchedAsset.originalName = QLatin1String("different.jpg");
    const quint64 beforeRejectedRemoval = runtime.categoryEpoch(categoryUuid);
    QVERIFY(!runtime.hasProtectedItem(item, container, { mismatchedAsset }));
    QVERIFY(!runtime.removeProtectedItem(item, container, { mismatchedAsset }));
    QCOMPARE(runtime.categoryEpoch(categoryUuid), beforeRejectedRemoval);
    QVERIFY(runtime.stateForItem(item.imageId, &actionState));
    QVERIFY(actionState.protectedItem);

    PrivacyItem mismatchedItem = item;
    ++mismatchedItem.generation;
    QVERIFY(!runtime.hasProtectedItem(mismatchedItem, container, { primary }));
    QVERIFY(!runtime.removeProtectedItem(mismatchedItem, container, { primary }));

    QVERIFY(runtime.removeProtectedItem(item, container, { primary }));
    QVERIFY(runtime.categoryEpoch(categoryUuid) > beforeRejectedRemoval);
    QVERIFY(runtime.isCategoryUnlocked(categoryUuid));
    QCOMPARE(runtime.rootSummary(rootUuid).protectedItemCount, 0);
    QVERIFY(!runtime.rootContainsProtectedItems(9));
    QCOMPARE(runtime.publicSourceDisposition(item.imageId),
             PrivacyPublicSourceDisposition::Unprotected);
    QVERIFY(runtime.stateForItem(item.imageId, &actionState));
    QVERIFY(!actionState.protectedItem);
    QVERIFY(!runtime.currentState(item.uuid, &leaseState));
    QCOMPARE(runtime.analysisDisposition(item.imageId),
             PrivacyAnalysisDisposition::Allowed);
    QVERIFY(!runtime.hasProtectedItem(item, container, { primary }));
    QVERIFY(!runtime.removeProtectedItem(item, container, { primary }));
}

void PrivacyRuntimeTest::testRootEpochTransition()
{
    const QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->states.insert(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    const QSharedPointer<FakeIntegrityInspector> integrity(new FakeIntegrityInspector);

    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(makeSnapshot(), verifier, {}, integrity);
    const quint64 initialEpoch = runtime.rootEpoch(rootUuid);
    QVERIFY(initialEpoch > 0);
    QVERIFY(runtime.beginRootRecovery(rootUuid));
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::Recovering);
    QVERIFY(runtime.rootEpoch(rootUuid) > initialEpoch);
    QVERIFY(runtime.publishRootState(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable));
    QCOMPARE(runtime.rootState(rootUuid), PrivacyRootRuntimeState::VerifiedAvailable);
}

void PrivacyRuntimeTest::testTransactionStates()
{
    PrivacyTransaction transaction = makeCompatibilityTransaction();
    QVERIFY(transaction.isValid());
    QVERIFY(transaction.isActive());
    transaction.state = PrivacyTransactionState::Complete;
    QVERIFY(transaction.isValid());
    QVERIFY(!transaction.isActive());
    transaction.state = static_cast<PrivacyTransactionState>(99);
    QVERIFY(!transaction.isValid());
}

QTEST_GUILESS_MAIN(PrivacyRuntimeTest)

#include "privacyruntime_utest.moc"
