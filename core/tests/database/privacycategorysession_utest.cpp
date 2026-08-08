/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C++ includes

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

// Qt includes

#include <QCryptographicHash>
#include <QSemaphore>
#include <QTest>

// Local includes

#include "privacycategorysession.h"

using namespace Digikam;

namespace
{

const QString CategoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
const QString StoreUuid = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString RootUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QString MarkerUuid = QLatin1String("40000000-0000-0000-0000-000000000001");
const QString TransactionUuid = QLatin1String("50000000-0000-0000-0000-000000000001");
const QByteArray OpaqueConfig("synthetic opaque gocryptfs config");

QString configHash()
{
    return QString::fromLatin1(
        QCryptographicHash::hash(OpaqueConfig, QCryptographicHash::Sha256).toHex());
}

PrivacyStorageRoot makeRoot()
{
    PrivacyStorageRoot root;
    root.uuid = RootUuid;
    root.kind = PrivacyStorageRootKind::ManagedStoreRoot;
    root.configuredPath = QLatin1String("/synthetic/managed-store");
    root.identityVersion = 1;
    root.identityData = QByteArray("synthetic-root-identity");
    root.markerUuid = MarkerUuid;
    root.createdAt = QDateTime::currentDateTimeUtc();

    return root;
}

PrivacyCategory makeCategory()
{
    PrivacyCategory category;
    category.uuid = CategoryUuid;
    category.name = QLatin1String("Synthetic category");
    category.backend = PrivacyBackend::Casual;
    category.presentationMode = PrivacyPresentationMode::Generic;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt = QDateTime::currentDateTimeUtc();

    return category;
}

PrivacyStore makeStore()
{
    PrivacyStore store;
    store.uuid = StoreUuid;
    store.categoryUuid = CategoryUuid;
    store.rootUuid = RootUuid;
    store.format = QLatin1String("gocryptfs");
    store.formatVersion = 2;
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/") + StoreUuid;
    store.configRelativePath = store.cipherRelativePath +
                               QLatin1String("/gocryptfs.conf");
    store.configGeneration = 1;
    store.lifecycleState = PrivacyStoreLifecycleState::Active;
    store.createdAt = QDateTime::currentDateTimeUtc();

    return store;
}

PrivacyRepositorySnapshot makeActiveSnapshot()
{
    PrivacyRepositorySnapshot snapshot;
    snapshot.categories << makeCategory();

    PrivacyCredential credential;
    credential.categoryUuid = CategoryUuid;
    credential.generation = 1;
    credential.encodingVersion = QLatin1String("utf8-nfc-v1");
    credential.envelopeFormat = QLatin1String("gocryptfs-config-v2");
    credential.envelopeBlob = OpaqueConfig;
    credential.envelopeHashAlgorithm = QLatin1String("sha256");
    credential.envelopeHash = configHash();
    credential.createdAt = QDateTime::currentDateTimeUtc();
    snapshot.credentials << credential;
    snapshot.storageRoots << makeRoot();
    snapshot.stores << makeStore();

    for (const PrivacyStoreRole role : { PrivacyStoreRole::CredentialAuthority,
                                         PrivacyStoreRole::Derivatives })
    {
        PrivacyStoreBinding binding;
        binding.categoryUuid = CategoryUuid;
        binding.role = role;
        binding.storeUuid = StoreUuid;
        snapshot.storeBindings << binding;
    }

    return snapshot;
}

PrivacyCategoryCreateRequest makeCreateRequest()
{
    PrivacyCategoryCreateRequest request;
    request.categoryUuid = CategoryUuid;
    request.storeUuid = StoreUuid;
    request.transactionUuid = TransactionUuid;
    request.name = QLatin1String("Synthetic category");
    request.storageRoot = makeRoot();

    return request;
}

class FakeRootVerifier final : public PrivacyRootVerifier
{
public:

    PrivacyRootRuntimeState verify(const PrivacyStorageRoot&) const override
    {
        ++calls;
        return state;
    }

    mutable std::atomic<int> calls { 0 };
    PrivacyRootRuntimeState state = PrivacyRootRuntimeState::VerifiedAvailable;
};

class FakeRepository final : public PrivacyCategorySessionRepository
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* target) const override
    {
        ++loadCalls;

        if (!target)
        {
            return false;
        }

        *target = snapshot;
        return true;
    }

    bool beginCreation(const PrivacyCategory& category,
                       const PrivacyStorageRoot& root,
                       const PrivacyStore& store,
                       const PrivacyTransaction& transaction,
                       const PrivacyTransactionJournal& journal) override
    {
        ++beginCalls;

        if (failBegin)
        {
            return false;
        }

        for (const PrivacyCategory& existing : std::as_const(snapshot.categories))
        {
            if ((existing.uuid == category.uuid) ||
                (existing.name.compare(category.name, Qt::CaseInsensitive) == 0))
            {
                return false;
            }
        }

        snapshot.categories << category;
        snapshot.storageRoots << root;
        snapshot.stores << store;
        snapshot.transactions << transaction;
        snapshot.transactionJournals << journal;
        return true;
    }

    bool publishCreation(const PrivacyCategory& category,
                         const PrivacyCredential& credential,
                         const PrivacyStore& store,
                         const QList<PrivacyStoreBinding>& bindings,
                         const PrivacyTransaction& transaction) override
    {
        ++publishCalls;

        if (failPublish)
        {
            return false;
        }

        for (PrivacyCategory& existing : snapshot.categories)
        {
            if (existing.uuid == category.uuid)
            {
                existing = category;
            }
        }

        for (PrivacyStore& existing : snapshot.stores)
        {
            if (existing.uuid == store.uuid)
            {
                existing = store;
            }
        }

        for (PrivacyTransaction& existing : snapshot.transactions)
        {
            if (existing.uuid == transaction.uuid)
            {
                existing = transaction;
            }
        }

        snapshot.credentials << credential;
        snapshot.storeBindings << bindings;
        return true;
    }

    mutable std::atomic<int> loadCalls { 0 };
    std::atomic<int> beginCalls { 0 };
    std::atomic<int> publishCalls { 0 };
    bool failBegin = false;
    bool failPublish = false;
    PrivacyRepositorySnapshot snapshot;
};

class FakeLease final : public PrivacyCategoryStoreLease
{
public:

    explicit FakeLease(std::atomic<int>* destructionCounter)
        : destroyed(destructionCounter)
    {
    }

    ~FakeLease() override
    {
        if (destroyed)
        {
            ++(*destroyed);
        }
    }

    bool isActive() override
    {
        return active;
    }

    bool active = true;
    std::atomic<int>* destroyed = nullptr;
};

class FakeStoreBackend final : public PrivacyCategoryStoreBackend
{
public:

    bool createOrResume(const PrivacyStorageRoot&, const PrivacyStore&,
                        const QString&, const PrivacyPassword&,
                        const QByteArray&, PrivacyGocryptfsEnvelope* envelope,
                        PrivacyGocryptfsError* error) override
    {
        ++createCalls;
        createEntered.release();

        if (blockCreate)
        {
            createContinue.acquire();
        }

        if (failCreate)
        {
            if (error)
            {
                *error = PrivacyGocryptfsError::ProcessFailed;
            }

            return false;
        }

        *envelope = PrivacyGocryptfsEnvelope::fromOpaqueConfig(
            QLatin1String("gocryptfs-config-v2"), OpaqueConfig, error);
        return envelope->isValid();
    }

    bool validateEnvelope(const PrivacyGocryptfsEnvelope& envelope,
                          const PrivacyPassword& password,
                          PrivacyGocryptfsError* error) override
    {
        ++validateCalls;
        bool matches = false;
        password.withStdinLine([&matches](const QByteArray& line)
        {
            matches = (line == QByteArray("secret\n"));
            return true;
        });

        if (!matches || !envelope.isValid())
        {
            if (error)
            {
                *error = PrivacyGocryptfsError::ProcessFailed;
            }

            return false;
        }

        return true;
    }

    std::unique_ptr<PrivacyCategoryStoreLease> unlock(
        const PrivacyStorageRoot&, const PrivacyStore&,
        const PrivacyGocryptfsEnvelope&, const PrivacyPassword&,
        const QByteArray&, PrivacyGocryptfsError*) override
    {
        ++unlockCalls;
        unlockEntered.release();

        if (blockUnlock)
        {
            unlockContinue.acquire();
        }

        return std::make_unique<FakeLease>(&leaseDestructions);
    }

    bool lock(std::unique_ptr<PrivacyCategoryStoreLease>& lease,
              PrivacyGocryptfsError* error) override
    {
        ++lockCalls;
        lockEntered.release();

        if (blockLock)
        {
            lockContinue.acquire();
        }

        if (failLock)
        {
            if (error)
            {
                *error = PrivacyGocryptfsError::UnmountFailed;
            }

            return false;
        }

        if (FakeLease* const fake = dynamic_cast<FakeLease*>(lease.get()))
        {
            fake->active = false;
        }

        lease.reset();
        return true;
    }

    std::atomic<int> createCalls { 0 };
    std::atomic<int> validateCalls { 0 };
    std::atomic<int> unlockCalls { 0 };
    std::atomic<int> lockCalls { 0 };
    std::atomic<int> leaseDestructions { 0 };
    bool failCreate = false;
    bool failLock = false;
    bool blockCreate = false;
    bool blockUnlock = false;
    bool blockLock = false;
    QSemaphore createEntered;
    QSemaphore createContinue;
    QSemaphore unlockEntered;
    QSemaphore unlockContinue;
    QSemaphore lockEntered;
    QSemaphore lockContinue;
};

class ReentrantObserver final : public PrivacySecretLifetimeObserver
{
public:

    void secretRetained(const QString& categoryUuid) override
    {
        ++retained;
        retainedSawOwned.store(coordinator && coordinator->ownsSecret(categoryUuid));
    }

    void secretReleased(const QString& categoryUuid) override
    {
        ++released;
        releasedSawOwned.store(coordinator && coordinator->ownsSecret(categoryUuid));
        releasedSawLeaseDestroyed.store(!leaseDestructions ||
                                        (leaseDestructions->load() > 0));
    }

    PrivacyCategorySessionCoordinator* coordinator = nullptr;
    std::atomic<int> retained { 0 };
    std::atomic<int> released { 0 };
    std::atomic<bool> retainedSawOwned { false };
    std::atomic<bool> releasedSawOwned { true };
    std::atomic<bool> releasedSawLeaseDestroyed { false };
    std::atomic<int>* leaseDestructions = nullptr;
};

void initializeRuntime(PrivacyRuntimeCoordinator* runtime,
                       const PrivacyRepositorySnapshot& snapshot,
                       const QSharedPointer<FakeRootVerifier>& verifier)
{
    PrivacyRepositorySnapshot runtimeSnapshot;
    runtimeSnapshot.categories = snapshot.categories;

    QCOMPARE(runtime->initialize(runtimeSnapshot, verifier, {}, {}).state,
             PrivacyStartupState::Ready);
}

} // namespace

class PrivacyCategorySessionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testStrongRejectedBeforeMutation();
    void testFailedCreationReleasesBarrier();
    void testLockAllWaitsForInflightCreation();
    void testLockCancelsInflightUnlock();
    void testLockAllCancelsInflightUnlock();
    void testCallbacksAndBlockingTeardownAreOutOfLock();
    void testDestructorForcesFailedLockTeardown();
};

void PrivacyCategorySessionTest::testStrongRejectedBeforeMutation()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    PrivacyCategoryCreateRequest request = makeCreateRequest();
    request.backend = PrivacyBackend::Strong;

    const PrivacyCategorySessionResult result =
        coordinator.createCategory(request, QLatin1String("secret"));
    QCOMPARE(result.status, PrivacyCategorySessionStatus::StrongRecoveryRequired);
    QCOMPARE(repository.beginCalls.load(), 0);
    QCOMPARE(repository.publishCalls.load(), 0);
    QCOMPARE(backend.createCalls.load(), 0);
    QCOMPARE(verifier->calls.load(), 0);
}

void PrivacyCategorySessionTest::testFailedCreationReleasesBarrier()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    backend.failCreate = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(), QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::StoreFailure);
    backend.failCreate = false;
    QCOMPARE(coordinator.createCategory(makeCreateRequest(), QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Created);
    QCOMPARE(backend.createCalls.load(), 2);
    QCOMPARE(repository.publishCalls.load(), 1);
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testLockAllWaitsForInflightCreation()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    backend.blockCreate = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    PrivacyCategorySessionResult createResult;
    std::atomic<bool> lockAllFinished { false };
    QSemaphore lockAllStarted;

    std::thread createThread([&]()
    {
        createResult = coordinator.createCategory(makeCreateRequest(),
                                                  QLatin1String("secret"));
    });
    const bool createEntered = backend.createEntered.tryAcquire(1, 2000);

    if (!createEntered)
    {
        backend.createContinue.release();
        createThread.join();
        QVERIFY(createEntered);
    }

    std::thread lockThread([&]()
    {
        lockAllStarted.release();
        coordinator.lockAllCategories();
        lockAllFinished.store(true);
    });
    const bool lockAllWasStarted = lockAllStarted.tryAcquire(1, 2000);

    if (!lockAllWasStarted)
    {
        backend.createContinue.release();
        createThread.join();
        lockThread.join();
        QVERIFY(lockAllWasStarted);
    }

    QTest::qWait(30);
    const bool escapedBarrier = lockAllFinished.load();
    backend.createContinue.release();
    createThread.join();
    lockThread.join();

    QVERIFY(!escapedBarrier);
    QCOMPARE(createResult.status, PrivacyCategorySessionStatus::Created);
    QVERIFY(lockAllFinished.load());
}

void PrivacyCategorySessionTest::testLockCancelsInflightUnlock()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    backend.blockUnlock = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    PrivacyCategorySessionResult unlockResult;
    PrivacyCategorySessionResult lockResult;
    QSemaphore lockStarted;

    std::thread unlockThread([&]()
    {
        unlockResult = coordinator.unlockCategory(CategoryUuid,
                                                  QLatin1String("secret"));
    });
    const bool unlockEntered = backend.unlockEntered.tryAcquire(1, 2000);

    if (!unlockEntered)
    {
        backend.unlockContinue.release();
        unlockThread.join();
        QVERIFY(unlockEntered);
    }

    std::thread lockThread([&]()
    {
        lockStarted.release();
        lockResult = coordinator.lockCategory(CategoryUuid);
    });
    const bool lockWasStarted = lockStarted.tryAcquire(1, 2000);

    if (!lockWasStarted)
    {
        backend.unlockContinue.release();
        unlockThread.join();
        lockThread.join();
        QVERIFY(lockWasStarted);
    }

    QTest::qWait(30);
    backend.unlockContinue.release();
    unlockThread.join();
    lockThread.join();

    QCOMPARE(unlockResult.status, PrivacyCategorySessionStatus::Canceled);
    QCOMPARE(lockResult.status, PrivacyCategorySessionStatus::AlreadyLocked);
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testLockAllCancelsInflightUnlock()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    backend.blockUnlock = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    PrivacyCategorySessionResult unlockResult;
    QList<PrivacyCategorySessionResult> lockResults;
    QSemaphore lockAllStarted;

    std::thread unlockThread([&]()
    {
        unlockResult = coordinator.unlockCategory(CategoryUuid,
                                                  QLatin1String("secret"));
    });
    const bool unlockEntered = backend.unlockEntered.tryAcquire(1, 2000);

    if (!unlockEntered)
    {
        backend.unlockContinue.release();
        unlockThread.join();
        QVERIFY(unlockEntered);
    }

    std::thread lockThread([&]()
    {
        lockAllStarted.release();
        lockResults = coordinator.lockAllCategories();
    });
    const bool lockAllWasStarted = lockAllStarted.tryAcquire(1, 2000);

    if (!lockAllWasStarted)
    {
        backend.unlockContinue.release();
        unlockThread.join();
        lockThread.join();
        QVERIFY(lockAllWasStarted);
    }

    QTest::qWait(30);
    backend.unlockContinue.release();
    unlockThread.join();
    lockThread.join();

    QCOMPARE(unlockResult.status, PrivacyCategorySessionStatus::Canceled);
    QVERIFY(lockResults.isEmpty());
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testCallbacksAndBlockingTeardownAreOutOfLock()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    ReentrantObserver observer;
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, &observer);
    observer.coordinator = &coordinator;
    observer.leaseDestructions = &backend.leaseDestructions;

    QCOMPARE(coordinator.unlockCategory(CategoryUuid, QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Unlocked);
    QCOMPARE(observer.retained.load(), 1);
    QVERIFY(observer.retainedSawOwned.load());

    backend.blockLock = true;
    PrivacyCategorySessionResult lockResult;
    std::thread lockThread([&]()
    {
        lockResult = coordinator.lockCategory(CategoryUuid);
    });
    const bool lockEntered = backend.lockEntered.tryAcquire(1, 2000);

    if (!lockEntered)
    {
        backend.lockContinue.release();
        lockThread.join();
        QVERIFY(lockEntered);
    }

    // A blocked unmount must not hold the coordinator mutex.
    QVERIFY(coordinator.ownsSecret(CategoryUuid));
    backend.lockContinue.release();
    lockThread.join();

    QCOMPARE(lockResult.status, PrivacyCategorySessionStatus::Locked);
    QCOMPARE(observer.released.load(), 1);
    QVERIFY(!observer.releasedSawOwned.load());
    QVERIFY(observer.releasedSawLeaseDestroyed.load());
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testDestructorForcesFailedLockTeardown()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    ReentrantObserver observer;
    observer.leaseDestructions = &backend.leaseDestructions;

    {
        PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                      runtime, &observer);
        observer.coordinator = &coordinator;
        QCOMPARE(coordinator.unlockCategory(CategoryUuid,
                                            QLatin1String("secret")).status,
                 PrivacyCategorySessionStatus::Unlocked);
        backend.failLock = true;
    }

    observer.coordinator = nullptr;
    QCOMPARE(backend.lockCalls.load(), 1);
    QCOMPARE(backend.leaseDestructions.load(), 1);
    QCOMPARE(observer.retained.load(), 1);
    QCOMPARE(observer.released.load(), 1);
    QVERIFY(!observer.releasedSawOwned.load());
    QVERIFY(observer.releasedSawLeaseDestroyed.load());
}

QTEST_GUILESS_MAIN(PrivacyCategorySessionTest)

#include "privacycategorysession_utest.moc"
