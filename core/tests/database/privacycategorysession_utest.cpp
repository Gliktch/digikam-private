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
#include <stdexcept>
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

    bool setCategoryUnlockedThumbnailMode(
        const QString& categoryUuid,
        PrivacyUnlockedThumbnailMode mode) override
    {
        ++thumbnailModeUpdateCalls;

        if (failThumbnailModeUpdate)
        {
            return false;
        }

        for (PrivacyCategory& category : snapshot.categories)
        {
            if (category.uuid == categoryUuid)
            {
                category.unlockedThumbnailMode = mode;
                return true;
            }
        }

        return false;
    }

    bool setCategoryTagVisibilityMode(
        const QString& categoryUuid,
        PrivacyTagVisibilityMode mode) override
    {
        ++tagVisibilityUpdateCalls;

        if (failTagVisibilityUpdate)
        {
            return false;
        }

        for (PrivacyCategory& category : snapshot.categories)
        {
            if (category.uuid == categoryUuid)
            {
                category.tagVisibilityMode = mode;
                return true;
            }
        }

        return false;
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
        bool rootPresent = false;

        for (const PrivacyStorageRoot& existing : std::as_const(snapshot.storageRoots))
        {
            if (existing.uuid == root.uuid)
            {
                rootPresent = true;
            }
        }

        if (!rootPresent)
        {
            snapshot.storageRoots << root;
        }
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

    bool compareAndUpdateCreationJournal(
        const PrivacyTransactionJournal& journal, int expectedStage) override
    {
        ++journalUpdateCalls;

        if (failCompleteJournalUpdateOnce &&
            (journal.stage == static_cast<int>(PrivacyJournalStage::Complete)))
        {
            failCompleteJournalUpdateOnce = false;
            return false;
        }

        for (PrivacyTransactionJournal& existing : snapshot.transactionJournals)
        {
            if ((existing.transactionUuid == journal.transactionUuid) &&
                (existing.rootUuid == journal.rootUuid) &&
                (existing.stage == expectedStage))
            {
                existing = journal;
                return true;
            }
        }

        return false;
    }

    mutable std::atomic<int> loadCalls { 0 };
    std::atomic<int> beginCalls { 0 };
    std::atomic<int> publishCalls { 0 };
    std::atomic<int> journalUpdateCalls { 0 };
    std::atomic<int> thumbnailModeUpdateCalls { 0 };
    std::atomic<int> tagVisibilityUpdateCalls { 0 };
    bool failBegin = false;
    bool failPublish = false;
    bool failCompleteJournalUpdateOnce = false;
    bool failThumbnailModeUpdate = false;
    bool failTagVisibilityUpdate = false;
    PrivacyRepositorySnapshot snapshot;
};

class FakeCreationJournalPersistence final
    : public PrivacyCategoryCreationJournalPersistence
{
public:

    bool createOrLoadExact(const PrivacyStorageRoot&, PrivacyJournalRecord* record,
                           bool allowCreate,
                           QByteArray* publishedSha256,
                           PrivacyJournalError* error) override
    {
        ++createCalls;

        if (!record || !publishedSha256 || failCreate)
        {
            if (error)
            {
                *error = PrivacyJournalError::PublicationConflict;
            }

            return false;
        }

        record->rootDevice = 101;
        record->rootInode = 202;
        const QByteArray bytes = PrivacyTransactionJournalCodec::encode(*record);

        if (bytes.isEmpty() || (currentBytes.isEmpty() && !allowCreate) ||
            (!currentBytes.isEmpty() && (currentBytes != bytes)))
        {
            if (error)
            {
                *error = PrivacyJournalError::PublicationConflict;
            }

            return false;
        }

        currentBytes = bytes;
        currentHash = PrivacyTransactionJournalCodec::sha256(bytes);
        *publishedSha256 = currentHash;
        return true;
    }

    bool compareAndUpdateExact(const PrivacyStorageRoot&,
                               const PrivacyJournalRecord& record,
                               const QByteArray& expectedCurrentSha256,
                               QByteArray* publishedSha256,
                               PrivacyJournalError* error) override
    {
        ++updateCalls;

        if (!publishedSha256 || failUpdate ||
            (expectedCurrentSha256 != currentHash))
        {
            if (error)
            {
                *error = PrivacyJournalError::StaleComparison;
            }

            return false;
        }

        const QByteArray bytes = PrivacyTransactionJournalCodec::encode(record);

        if (bytes.isEmpty())
        {
            return false;
        }

        currentBytes = bytes;
        currentHash = PrivacyTransactionJournalCodec::sha256(bytes);
        *publishedSha256 = currentHash;
        return true;
    }

    std::atomic<int> createCalls { 0 };
    std::atomic<int> updateCalls { 0 };
    bool failCreate = false;
    bool failUpdate = false;
    QByteArray currentBytes;
    QByteArray currentHash;
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

    QString plaintextRoot() const override
    {
        return QLatin1String("/synthetic/category-store");
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
        journalReadyAtCreate.store(!journalCreateCounter ||
                                   (journalCreateCounter->load() > 0));
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

        if (blockValidate)
        {
            validateEntered.release();
            validateContinue.acquire();
        }

        bool matches = false;
        password.withStdinLine([&matches](const QByteArray& line)
        {
            matches = (line == QByteArray("secret\n")) ||
                      (line == QByteArray("fresh secret\n"));
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
    std::atomic<bool> journalReadyAtCreate { false };
    std::atomic<int>* journalCreateCounter = nullptr;
    bool failCreate = false;
    bool failLock = false;
    bool blockCreate = false;
    bool blockValidate = false;
    bool blockUnlock = false;
    bool blockLock = false;
    QSemaphore createEntered;
    QSemaphore createContinue;
    QSemaphore validateEntered;
    QSemaphore validateContinue;
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
    void testCreationJournalPrecedesMutationAndPublishesRuntimeState();
    void testCreationReverifiesPreexistingOfflineRuntimeRoot();
    void testRuntimeCreationPublicationRejectsExtraRolesAndDuplicateRoots();
    void testMismatchedCreationJournalReplayFailsClosed();
    void testCompletedDatabaseCreationReconcilesIdempotently();
    void testMissingJournalAfterDatabaseBeginResumesExactly();
    void testFilesystemCompleteBeforeDatabaseJournalCasResumesExactly();
    void testFilesystemCompleteRejectsMismatchedDatabasePredecessorHash();
    void testFailedCreationReleasesBarrier();
    void testLockAllWaitsForInflightCreation();
    void testLockCancelsInflightUnlock();
    void testLockAllCancelsInflightUnlock();
    void testItemOperationBorrowsSecretAndSerializesLock();
    void testFreshAuthenticationWhileLockedDoesNotCreateSession();
    void testFreshAuthenticationAllowsOnlyNamedItemTransaction();
    void testFreshAuthenticationDoesNotReplaceRetainedSecret();
    void testFreshAuthenticationRejectsWrongPasswordAndRecoversFromException();
    void testFreshAuthenticationSerializesLock();
    void testThumbnailModeUpdateAuthenticatesAndCompensatesFailure();
    void testTagVisibilityUpdateReusesUnlockedAuthentication();
    void testTagVisibilityUpdateRequiresAuthenticationAndCompensatesFailure();
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

void PrivacyCategorySessionTest::testCreationJournalPrecedesMutationAndPublishesRuntimeState()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    FakeCreationJournalPersistence creationJournal;
    backend.journalCreateCounter = &creationJournal.createCalls;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Created);
    QVERIFY(backend.journalReadyAtCreate.load());
    QCOMPARE(repository.journalUpdateCalls.load(), 2);
    QCOMPARE(creationJournal.updateCalls.load(), 1);
    QVERIFY(runtime.categoryEpoch(CategoryUuid) > 0);
    QCOMPARE(runtime.rootState(RootUuid),
             PrivacyRootRuntimeState::VerifiedAvailable);
    QCOMPARE(repository.snapshot.transactionJournals.size(), 1);
    QCOMPARE(repository.snapshot.transactionJournals.first().stage,
             static_cast<int>(PrivacyJournalStage::Complete));
}

void PrivacyCategorySessionTest::testCreationReverifiesPreexistingOfflineRuntimeRoot()
{
    FakeRepository repository;
    repository.snapshot.storageRoots << makeRoot();
    FakeStoreBackend backend;
    FakeCreationJournalPersistence creationJournal;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    verifier->state = PrivacyRootRuntimeState::Offline;
    PrivacyRuntimeCoordinator runtime;
    PrivacyRepositorySnapshot runtimeSnapshot;
    runtimeSnapshot.storageRoots << makeRoot();
    QCOMPARE(runtime.initialize(runtimeSnapshot, verifier, {}, {}).state,
             PrivacyStartupState::Degraded);
    QCOMPARE(runtime.rootState(RootUuid), PrivacyRootRuntimeState::Offline);
    verifier->state = PrivacyRootRuntimeState::VerifiedAvailable;
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Created);
    QCOMPARE(runtime.rootState(RootUuid),
             PrivacyRootRuntimeState::VerifiedAvailable);
    QCOMPARE(runtime.report().state, PrivacyStartupState::Ready);
}

void PrivacyCategorySessionTest::testRuntimeCreationPublicationRejectsExtraRolesAndDuplicateRoots()
{
    const PrivacyRepositorySnapshot active = makeActiveSnapshot();
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator extraRoleRuntime;
    QCOMPARE(extraRoleRuntime.initialize({}, verifier, {}, {}).state,
             PrivacyStartupState::Ready);
    QList<PrivacyStoreBinding> extraBindings = active.storeBindings;
    PrivacyStoreBinding extra;
    extra.categoryUuid = CategoryUuid;
    extra.role = PrivacyStoreRole::Originals;
    extra.storeUuid = StoreUuid;
    extraBindings << extra;
    QVERIFY(!extraRoleRuntime.publishCategoryCreation(
        active.categories.first(), active.credentials.first(),
        active.storageRoots.first(), active.stores.first(), extraBindings));
    QCOMPARE(extraRoleRuntime.categoryEpoch(CategoryUuid), quint64(0));

    PrivacyRuntimeCoordinator duplicateRootRuntime;
    PrivacyRepositorySnapshot duplicateRoots;
    duplicateRoots.storageRoots << active.storageRoots.first()
                                << active.storageRoots.first();
    QCOMPARE(duplicateRootRuntime.initialize(duplicateRoots, verifier, {}, {}).state,
             PrivacyStartupState::Degraded);
    QVERIFY(!duplicateRootRuntime.publishCategoryCreation(
        active.categories.first(), active.credentials.first(),
        active.storageRoots.first(), active.stores.first(),
        active.storeBindings));
    QCOMPARE(duplicateRootRuntime.categoryEpoch(CategoryUuid), quint64(0));
}

void PrivacyCategorySessionTest::testMismatchedCreationJournalReplayFailsClosed()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    backend.failCreate = true;
    FakeCreationJournalPersistence creationJournal;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::StoreFailure);
    QCOMPARE(backend.createCalls.load(), 1);
    creationJournal.currentBytes.append('x');
    backend.failCreate = false;
    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Conflict);
    QCOMPARE(backend.createCalls.load(), 1);
}

void PrivacyCategorySessionTest::testCompletedDatabaseCreationReconcilesIdempotently()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    FakeCreationJournalPersistence creationJournal;
    creationJournal.failUpdate = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired);
    QCOMPARE(backend.createCalls.load(), 1);
    creationJournal.failUpdate = false;
    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Created);
    QCOMPARE(backend.createCalls.load(), 1);
    QCOMPARE(repository.snapshot.transactionJournals.first().stage,
             static_cast<int>(PrivacyJournalStage::Complete));
    QVERIFY(runtime.categoryEpoch(CategoryUuid) > 0);
}

void PrivacyCategorySessionTest::testMissingJournalAfterDatabaseBeginResumesExactly()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    FakeCreationJournalPersistence creationJournal;
    creationJournal.failCreate = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Conflict);
    QCOMPARE(backend.createCalls.load(), 0);
    QVERIFY(repository.snapshot.transactionJournals.first().expectedJournalHash.isEmpty());
    creationJournal.failCreate = false;
    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Created);
    QCOMPARE(backend.createCalls.load(), 1);
}

void PrivacyCategorySessionTest::testFilesystemCompleteBeforeDatabaseJournalCasResumesExactly()
{
    FakeRepository repository;
    repository.failCompleteJournalUpdateOnce = true;
    FakeStoreBackend backend;
    FakeCreationJournalPersistence creationJournal;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired);
    QCOMPARE(repository.snapshot.transactionJournals.first().stage,
             static_cast<int>(PrivacyJournalStage::Created));
    PrivacyJournalRecord completeRecord;
    PrivacyJournalError decodeError = PrivacyJournalError::None;
    QString decodeDetail;
    QVERIFY2(PrivacyTransactionJournalCodec::decode(creationJournal.currentBytes,
                                                     &completeRecord,
                                                     &decodeError,
                                                     &decodeDetail),
             qPrintable(decodeDetail));
    QCOMPARE(completeRecord.stage, PrivacyJournalStage::Complete);
    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Created);
    QCOMPARE(backend.createCalls.load(), 1);
    QCOMPARE(repository.snapshot.transactionJournals.first().stage,
             static_cast<int>(PrivacyJournalStage::Complete));
}

void PrivacyCategorySessionTest::testFilesystemCompleteRejectsMismatchedDatabasePredecessorHash()
{
    FakeRepository repository;
    repository.failCompleteJournalUpdateOnce = true;
    FakeStoreBackend backend;
    FakeCreationJournalPersistence creationJournal;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired);
    repository.snapshot.transactionJournals.first().expectedJournalHash =
        QString(64, QLatin1Char('0'));
    QCOMPARE(coordinator.createCategory(makeCreateRequest(),
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Conflict);
    QCOMPARE(backend.createCalls.load(), 1);
    QCOMPARE(runtime.categoryEpoch(CategoryUuid), quint64(0));
    QCOMPARE(repository.snapshot.transactionJournals.first().stage,
             static_cast<int>(PrivacyJournalStage::Created));
}

void PrivacyCategorySessionTest::testFailedCreationReleasesBarrier()
{
    FakeRepository repository;
    FakeStoreBackend backend;
    FakeCreationJournalPersistence creationJournal;
    backend.failCreate = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);

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
    FakeCreationJournalPersistence creationJournal;
    backend.blockCreate = true;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, {}, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime, nullptr,
                                                  &creationJournal);
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

void PrivacyCategorySessionTest::testItemOperationBorrowsSecretAndSerializesLock()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);

    QCOMPARE(coordinator.runWhileUnlocked(CategoryUuid, [] {}),
             PrivacyCategoryOperationStatus::CategoryLocked);
    QCOMPARE(coordinator.unlockCategory(CategoryUuid,
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Unlocked);

    bool secretMatched = false;
    PrivacyCategorySessionStatus reentrantLockStatus =
        PrivacyCategorySessionStatus::InvalidRequest;
    QList<PrivacyCategorySessionResult> reentrantLockAllResults;
    QCOMPARE(coordinator.runWithUnlockedSecret(
                 CategoryUuid,
                 [&](const PrivacyPassword& password)
                 {
                     password.withStdinLine([&](const QByteArray& line)
                     {
                         secretMatched = (line == QByteArray("secret\n"));
                         return true;
                     });
                     reentrantLockStatus = coordinator.lockCategory(CategoryUuid).status;
                     reentrantLockAllResults = coordinator.lockAllCategories();
                 }),
             PrivacyCategoryOperationStatus::Completed);
    QVERIFY(secretMatched);
    QCOMPARE(reentrantLockStatus,
             PrivacyCategorySessionStatus::TransactionBlocked);
    QCOMPARE(reentrantLockAllResults.size(), 1);
    QCOMPARE(reentrantLockAllResults.constFirst().status,
             PrivacyCategorySessionStatus::TransactionBlocked);
    QVERIFY(coordinator.ownsSecret(CategoryUuid));

    bool storeSecretMatched = false;
    QString borrowedStoreRoot;
    QCOMPARE(coordinator.runWithUnlockedStore(
                 CategoryUuid,
                 [&](const PrivacyPassword& password, const QString& root)
                 {
                     password.withStdinLine([&](const QByteArray& line)
                     {
                         storeSecretMatched = (line == QByteArray("secret\n"));
                         return true;
                     });
                     borrowedStoreRoot = root;
                 }),
             PrivacyCategoryOperationStatus::Completed);
    QVERIFY(storeSecretMatched);
    QCOMPARE(borrowedStoreRoot,
             QLatin1String("/synthetic/category-store"));

    bool exceptionObserved = false;

    try
    {
        coordinator.runWhileUnlocked(CategoryUuid, []
        {
            throw std::runtime_error("synthetic callback failure");
        });
    }
    catch (const std::runtime_error&)
    {
        exceptionObserved = true;
    }

    QVERIFY(exceptionObserved);
    QCOMPARE(coordinator.runWhileUnlocked(CategoryUuid, [] {}),
             PrivacyCategoryOperationStatus::Completed);

    QSemaphore operationEntered;
    QSemaphore operationContinue;
    std::atomic<bool> lockFinished { false };
    PrivacyCategoryOperationStatus operationStatus =
        PrivacyCategoryOperationStatus::InvalidRequest;
    PrivacyCategorySessionResult lockResult;

    std::thread operationThread([&]()
    {
        operationStatus = coordinator.runWhileUnlocked(CategoryUuid, [&]()
        {
            operationEntered.release();
            operationContinue.acquire();
        });
    });

    const bool entered = operationEntered.tryAcquire(1, 2000);

    if (!entered)
    {
        operationContinue.release();
        operationThread.join();
        QVERIFY(entered);
    }

    QCOMPARE(coordinator.runWhileUnlocked(CategoryUuid, [] {}),
             PrivacyCategoryOperationStatus::TransactionBlocked);

    std::thread lockThread([&]()
    {
        lockResult = coordinator.lockCategory(CategoryUuid);
        lockFinished.store(true);
    });
    QTest::qWait(30);
    QVERIFY(!lockFinished.load());
    operationContinue.release();
    operationThread.join();
    lockThread.join();

    QCOMPARE(operationStatus, PrivacyCategoryOperationStatus::Completed);
    QCOMPARE(lockResult.status, PrivacyCategorySessionStatus::Locked);
    QVERIFY(lockFinished.load());
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testFreshAuthenticationWhileLockedDoesNotCreateSession()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    bool callbackCalled = false;

    const PrivacyCategorySessionResult result =
        coordinator.runWithFreshlyAuthenticatedSecret(
            CategoryUuid, QLatin1String("secret"),
            [&](const PrivacyPassword&)
            {
                callbackCalled = true;
            });

    QCOMPARE(result.status,
             PrivacyCategorySessionStatus::FreshAuthenticationVerified);
    QVERIFY(result.succeeded());
    QVERIFY(callbackCalled);
    QCOMPARE(backend.validateCalls.load(), 1);
    QCOMPARE(backend.unlockCalls.load(), 0);
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
    QVERIFY(!runtime.isCategoryUnlocked(CategoryUuid));
}

void PrivacyCategorySessionTest::testFreshAuthenticationAllowsOnlyNamedItemTransaction()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);

    PrivacyTransaction transaction;
    transaction.uuid = TransactionUuid;
    transaction.categoryUuid = CategoryUuid;
    transaction.itemUuid = QLatin1String("60000000-0000-0000-0000-000000000001");
    transaction.type = PrivacyTransactionType::ProtectItem;
    transaction.state = PrivacyTransactionState::Created;
    transaction.generation = 0;
    transaction.fromCredentialGeneration = 1;
    transaction.toCredentialGeneration = 1;
    transaction.payloadFormatVersion = 1;
    transaction.payloadData = QByteArray("synthetic item transaction");
    transaction.createdAt = QDateTime::currentDateTimeUtc();
    transaction.updatedAt = transaction.createdAt;
    QVERIFY(transaction.isValid());
    repository.snapshot.transactions << transaction;

    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    bool callbackCalled = false;
    const PrivacyCategorySessionResult wrong =
        coordinator.runWithFreshlyAuthenticatedSecret(
            CategoryUuid, QLatin1String("secret"),
            [&](const PrivacyPassword&)
            {
                callbackCalled = true;
            },
            QLatin1String("70000000-0000-0000-0000-000000000001"));
    QCOMPARE(wrong.status, PrivacyCategorySessionStatus::TransactionBlocked);
    QVERIFY(!callbackCalled);
    QCOMPARE(backend.validateCalls.load(), 0);

    const PrivacyCategorySessionResult exact =
        coordinator.runWithFreshlyAuthenticatedSecret(
            CategoryUuid, QLatin1String("secret"),
            [&](const PrivacyPassword&)
            {
                callbackCalled = true;
            },
            TransactionUuid);
    QCOMPARE(exact.status,
             PrivacyCategorySessionStatus::FreshAuthenticationVerified);
    QVERIFY(callbackCalled);
    QCOMPARE(backend.validateCalls.load(), 1);
    QCOMPARE(backend.unlockCalls.load(), 0);
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testFreshAuthenticationDoesNotReplaceRetainedSecret()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);

    QCOMPARE(coordinator.unlockCategory(CategoryUuid,
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Unlocked);
    bool freshSecretMatched = false;
    PrivacyCategorySessionStatus reentrantLockStatus =
        PrivacyCategorySessionStatus::InvalidRequest;
    const PrivacyCategorySessionResult result =
        coordinator.runWithFreshlyAuthenticatedSecret(
            CategoryUuid, QLatin1String("fresh secret"),
            [&](const PrivacyPassword& password)
            {
                password.withStdinLine([&](const QByteArray& line)
                {
                    freshSecretMatched = (line == QByteArray("fresh secret\n"));
                    return true;
                });
                reentrantLockStatus = coordinator.lockCategory(CategoryUuid).status;
            });

    QCOMPARE(result.status,
             PrivacyCategorySessionStatus::FreshAuthenticationVerified);
    QVERIFY(result.succeeded());
    QVERIFY(freshSecretMatched);
    QCOMPARE(reentrantLockStatus,
             PrivacyCategorySessionStatus::TransactionBlocked);
    QVERIFY(coordinator.ownsSecret(CategoryUuid));

    bool retainedSecretMatched = false;
    QCOMPARE(coordinator.runWithUnlockedSecret(
                 CategoryUuid,
                 [&](const PrivacyPassword& password)
                 {
                     password.withStdinLine([&](const QByteArray& line)
                     {
                         retainedSecretMatched = (line == QByteArray("secret\n"));
                         return true;
                     });
                 }),
             PrivacyCategoryOperationStatus::Completed);
    QVERIFY(retainedSecretMatched);
}

void PrivacyCategorySessionTest::testFreshAuthenticationRejectsWrongPasswordAndRecoversFromException()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);

    QCOMPARE(coordinator.unlockCategory(CategoryUuid,
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Unlocked);
    bool callbackCalled = false;
    const PrivacyCategorySessionResult wrong =
        coordinator.runWithFreshlyAuthenticatedSecret(
            CategoryUuid, QLatin1String("wrong"),
            [&](const PrivacyPassword&)
            {
                callbackCalled = true;
            });
    QCOMPARE(wrong.status, PrivacyCategorySessionStatus::AuthenticationFailed);
    QVERIFY(!callbackCalled);
    QVERIFY(coordinator.ownsSecret(CategoryUuid));

    bool exceptionObserved = false;

    try
    {
        coordinator.runWithFreshlyAuthenticatedSecret(
            CategoryUuid, QLatin1String("secret"),
            [](const PrivacyPassword&)
            {
                throw std::runtime_error("synthetic fresh-auth callback failure");
            });
    }
    catch (const std::runtime_error&)
    {
        exceptionObserved = true;
    }

    QVERIFY(exceptionObserved);
    QCOMPARE(coordinator.runWithFreshlyAuthenticatedSecret(
                 CategoryUuid, QLatin1String("secret"),
                 [](const PrivacyPassword&) {}).status,
             PrivacyCategorySessionStatus::FreshAuthenticationVerified);
    QVERIFY(coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testFreshAuthenticationSerializesLock()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);

    QCOMPARE(coordinator.unlockCategory(CategoryUuid,
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Unlocked);
    backend.blockValidate = true;
    PrivacyCategorySessionResult authenticationResult;
    PrivacyCategorySessionResult lockResult;
    std::atomic<bool> lockFinished { false };

    std::thread authenticationThread([&]()
    {
        authenticationResult = coordinator.runWithFreshlyAuthenticatedSecret(
            CategoryUuid, QLatin1String("secret"),
            [](const PrivacyPassword&) {});
    });
    const bool validationEntered = backend.validateEntered.tryAcquire(1, 2000);

    if (!validationEntered)
    {
        backend.validateContinue.release();
        authenticationThread.join();
        QVERIFY(validationEntered);
    }

    QCOMPARE(coordinator.runWhileUnlocked(CategoryUuid, [] {}),
             PrivacyCategoryOperationStatus::TransactionBlocked);

    std::thread lockThread([&]()
    {
        lockResult = coordinator.lockCategory(CategoryUuid);
        lockFinished.store(true);
    });
    QTest::qWait(30);
    const bool escapedBarrier = lockFinished.load();
    backend.validateContinue.release();
    authenticationThread.join();
    lockThread.join();

    QVERIFY(!escapedBarrier);
    QCOMPARE(authenticationResult.status,
             PrivacyCategorySessionStatus::FreshAuthenticationVerified);
    QCOMPARE(lockResult.status, PrivacyCategorySessionStatus::Locked);
    QVERIFY(lockFinished.load());
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testThumbnailModeUpdateAuthenticatesAndCompensatesFailure()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    const quint64 initialEpoch = runtime.categoryEpoch(CategoryUuid);

    QCOMPARE(coordinator.setCategoryUnlockedThumbnailMode(
                 CategoryUuid, PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked,
                 QLatin1String("wrong")).status,
             PrivacyCategorySessionStatus::AuthenticationFailed);
    QCOMPARE(repository.thumbnailModeUpdateCalls.load(), 0);
    QCOMPARE(runtime.categoryEpoch(CategoryUuid), initialEpoch);

    QCOMPARE(coordinator.setCategoryUnlockedThumbnailMode(
                 CategoryUuid, PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked,
                 QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::SettingsUpdated);
    QCOMPARE(repository.snapshot.categories.constFirst().unlockedThumbnailMode,
             PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked);
    QVERIFY(runtime.categoryEpoch(CategoryUuid) > initialEpoch);
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));

    const quint64 revealedEpoch = runtime.categoryEpoch(CategoryUuid);
    repository.failThumbnailModeUpdate = true;
    QCOMPARE(coordinator.setCategoryUnlockedThumbnailMode(
                 CategoryUuid, PrivacyUnlockedThumbnailMode::AlwaysOpaque,
                 QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::SettingsUpdateFailed);
    QCOMPARE(repository.snapshot.categories.constFirst().unlockedThumbnailMode,
             PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked);
    QVERIFY(runtime.categoryEpoch(CategoryUuid) >= (revealedEpoch + 2));

    QCOMPARE(coordinator.setCategoryUnlockedThumbnailMode(
                 CategoryUuid, static_cast<PrivacyUnlockedThumbnailMode>(99),
                 QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::InvalidRequest);
}

void PrivacyCategorySessionTest::testTagVisibilityUpdateReusesUnlockedAuthentication()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);

    QCOMPARE(coordinator.unlockCategory(CategoryUuid,
                                        QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::Unlocked);
    const quint64 initialEpoch = runtime.categoryEpoch(CategoryUuid);

    QCOMPARE(coordinator.setCategoryTagVisibilityMode(
                 CategoryUuid, PrivacyTagVisibilityMode::AlwaysVisible).status,
             PrivacyCategorySessionStatus::SettingsUpdated);
    QCOMPARE(repository.tagVisibilityUpdateCalls.load(), 1);
    QCOMPARE(repository.snapshot.categories.constFirst().tagVisibilityMode,
             PrivacyTagVisibilityMode::AlwaysVisible);
    QVERIFY(runtime.categoryEpoch(CategoryUuid) > initialEpoch);

    QCOMPARE(coordinator.setCategoryTagVisibilityMode(
                 CategoryUuid, PrivacyTagVisibilityMode::UnlockedOnly).status,
             PrivacyCategorySessionStatus::SettingsUpdated);
    QCOMPARE(repository.tagVisibilityUpdateCalls.load(), 2);
    QCOMPARE(repository.snapshot.categories.constFirst().tagVisibilityMode,
             PrivacyTagVisibilityMode::UnlockedOnly);
    QVERIFY(coordinator.ownsSecret(CategoryUuid));
}

void PrivacyCategorySessionTest::testTagVisibilityUpdateRequiresAuthenticationAndCompensatesFailure()
{
    FakeRepository repository;
    repository.snapshot = makeActiveSnapshot();
    FakeStoreBackend backend;
    QSharedPointer<FakeRootVerifier> verifier(new FakeRootVerifier);
    PrivacyRuntimeCoordinator runtime;
    initializeRuntime(&runtime, repository.snapshot, verifier);
    PrivacyCategorySessionCoordinator coordinator(repository, backend, *verifier,
                                                  runtime);
    const quint64 initialEpoch = runtime.categoryEpoch(CategoryUuid);

    QCOMPARE(coordinator.setCategoryTagVisibilityMode(
                 CategoryUuid, PrivacyTagVisibilityMode::AlwaysVisible,
                 QLatin1String("wrong")).status,
             PrivacyCategorySessionStatus::AuthenticationFailed);
    QCOMPARE(repository.tagVisibilityUpdateCalls.load(), 0);
    QCOMPARE(runtime.categoryEpoch(CategoryUuid), initialEpoch);

    QCOMPARE(coordinator.setCategoryTagVisibilityMode(
                 CategoryUuid, PrivacyTagVisibilityMode::AlwaysVisible,
                 QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::SettingsUpdated);
    QCOMPARE(repository.snapshot.categories.constFirst().tagVisibilityMode,
             PrivacyTagVisibilityMode::AlwaysVisible);
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));

    const quint64 exposedEpoch = runtime.categoryEpoch(CategoryUuid);
    repository.failTagVisibilityUpdate = true;
    QCOMPARE(coordinator.setCategoryTagVisibilityMode(
                 CategoryUuid, PrivacyTagVisibilityMode::UnlockedOnly,
                 QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::SettingsUpdateFailed);
    QCOMPARE(repository.snapshot.categories.constFirst().tagVisibilityMode,
             PrivacyTagVisibilityMode::AlwaysVisible);
    QVERIFY(runtime.categoryEpoch(CategoryUuid) >= (exposedEpoch + 2));
    QVERIFY(!coordinator.ownsSecret(CategoryUuid));

    QCOMPARE(coordinator.setCategoryTagVisibilityMode(
                 CategoryUuid, static_cast<PrivacyTagVisibilityMode>(99),
                 QLatin1String("secret")).status,
             PrivacyCategorySessionStatus::InvalidRequest);
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
