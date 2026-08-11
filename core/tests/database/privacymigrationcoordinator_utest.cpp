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

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

// C++ includes

#include <algorithm>

#ifdef Q_OS_UNIX

#   include <fcntl.h>
#   include <sys/stat.h>
#   include <unistd.h>

#endif

// Local includes

#include "privacymigrationcoordinator.h"
#include "privacypublicrecoverylocator.h"

using namespace Digikam;

namespace
{

const QString SourceCategoryUuid =
    QLatin1String("10000000-0000-0000-0000-000000000001");
const QString TargetCategoryUuid =
    QLatin1String("20000000-0000-0000-0000-000000000002");
const QString RootUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QString ManagedRootUuid =
    QLatin1String("40000000-0000-0000-0000-000000000001");
const QString StoreUuid = QLatin1String("50000000-0000-0000-0000-000000000001");
const QString ItemUuid = QLatin1String("60000000-0000-0000-0000-000000000001");
const QString ContainerUuid = QLatin1String("70000000-0000-0000-0000-000000000001");

class VerifiedRoot final : public PrivacyRootVerifier
{
public:

    PrivacyRootRuntimeState verify(const PrivacyStorageRoot&) const override
    {
        return PrivacyRootRuntimeState::VerifiedAvailable;
    }
};

class VerifiedIntegrity final : public PrivacyRootIntegrityInspector
{
public:

    PrivacyRootInspectionResult inspect(
        const PrivacyStorageRoot& root,
        const PrivacyRepositorySnapshot&) const override
    {
        PrivacyRootInspectionResult result;
        result.disposition = PrivacyIntegrityDisposition::Verified;
        result.summary.rootUuid = root.uuid;
        return result;
    }
};

class FakeCache final : public PrivacyStillItemCacheGate
{
public:

    bool begin(qlonglong imageId, const QString& path, bool protecting,
               bool aliasInventoryComplete) override
    {
        const QString key = QString::number(imageId) + path +
                            QString::number(protecting);

        if (!active.isEmpty() && (active != key))
        {
            return false;
        }

        active = key;
        beginPaths << path;
        beginAliasEvidence << aliasInventoryComplete;
        return true;
    }

    bool finish(qlonglong imageId, const QString& path, bool protecting,
                bool publicStateVerifiedOrLater) override
    {
        const QString key = QString::number(imageId) + path +
                            QString::number(protecting);

        if (!active.isEmpty() && (active != key))
        {
            return false;
        }

        if (!publicStateVerifiedOrLater)
        {
            return false;
        }

        active.clear();
        finishPaths << path;
        finishJournalEvidence << publicStateVerifiedOrLater;
        return true;
    }

    QString active;
    QStringList beginPaths;
    QStringList finishPaths;
    QList<bool> beginAliasEvidence;
    QList<bool> finishJournalEvidence;
};

class FakePersistence final : public PrivacyStillItemPersistence,
                              public PrivacyMigrationPersistence
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* output) const override
    {
        *output = snapshot;
        return true;
    }

    bool beginProtection(const PrivacyItem& item,
                         const PrivacyTransaction& transaction,
                         const PrivacyTransactionJournal& journal) override
    {
        snapshot.items << item;
        snapshot.transactions << transaction;
        snapshot.transactionJournals << journal;
        return true;
    }

    bool publishProtection(const PrivacyItem& item,
                           const PrivacyContainer& container,
                           const QList<PrivacyAsset>& assets,
                           const PrivacyTransaction& transaction) override
    {
        for (PrivacyItem& existing : snapshot.items)
        {
            if (existing.uuid == item.uuid)
            {
                existing = item;
            }
        }

        snapshot.containers << container;
        snapshot.assets << assets;
        return replaceTransaction(transaction, PrivacyTransactionState::Prepared,
                                  1);
    }

    bool beginUnprotection(const PrivacyTransaction& transaction,
                           const PrivacyTransactionJournal& journal) override
    {
        snapshot.transactions << transaction;
        snapshot.transactionJournals << journal;
        return true;
    }

    bool beginCompatibilityUnlock(
        const PrivacyTransaction& transaction,
        const PrivacyTransactionJournal& journal) override
    {
        if (std::any_of(snapshot.transactions.cbegin(),
                        snapshot.transactions.cend(),
                        [&transaction](const PrivacyTransaction& candidate)
                        {
                            return ((candidate.uuid == transaction.uuid) ||
                                    ((candidate.itemUuid == transaction.itemUuid) &&
                                     candidate.isActive() &&
                                     (candidate.type !=
                                      PrivacyTransactionType::MigrateBackend)));
                        }))
        {
            return false;
        }

        snapshot.transactions << transaction;
        snapshot.transactionJournals << journal;
        return true;
    }

    bool publishUnprotection(qlonglong imageId, const QString& itemUuid,
                             const QString&, qlonglong,
                             const QString& priorProtectTransactionUuid,
                             const PrivacyTransaction& transaction) override
    {
        for (PrivacyTransaction& existing : snapshot.transactions)
        {
            if (existing.uuid == transaction.uuid)
            {
                if ((existing.state != PrivacyTransactionState::Prepared) ||
                    (existing.generation != 1))
                {
                    return false;
                }

                existing = transaction;
                existing.itemUuid.clear();
            }
        }

        snapshot.items.removeIf([&](const PrivacyItem& item)
        {
            return ((item.imageId == imageId) && (item.uuid == itemUuid));
        });
        snapshot.containers.removeIf([&](const PrivacyContainer& container)
        {
            return (container.itemUuid == itemUuid);
        });
        snapshot.assets.removeIf([&](const PrivacyAsset& asset)
        {
            return (asset.itemUuid == itemUuid);
        });
        snapshot.transactionJournals.removeIf(
            [&](const PrivacyTransactionJournal& journal)
            {
                return (journal.transactionUuid == priorProtectTransactionUuid);
            });
        snapshot.transactions.removeIf([&](const PrivacyTransaction& candidate)
        {
            return (candidate.uuid == priorProtectTransactionUuid);
        });
        return true;
    }

    bool finalizeUnprotection(const QString& transactionUuid,
                              const QString&) override
    {
        snapshot.transactionJournals.removeIf(
            [&](const PrivacyTransactionJournal& journal)
            {
                return (journal.transactionUuid == transactionUuid);
            });
        snapshot.transactions.removeIf([&](const PrivacyTransaction& transaction)
        {
            return (transaction.uuid == transactionUuid);
        });
        return true;
    }

    bool compareAndUpdateTransaction(const PrivacyTransaction& transaction,
                                     PrivacyTransactionState expectedState,
                                     qlonglong expectedGeneration) override
    {
        return replaceTransaction(transaction, expectedState,
                                  expectedGeneration);
    }

    bool compareAndUpdateJournal(const PrivacyTransactionJournal& journal,
                                 int expectedStage) override
    {
        for (PrivacyTransactionJournal& existing : snapshot.transactionJournals)
        {
            if ((existing.transactionUuid == journal.transactionUuid) &&
                (existing.rootUuid == journal.rootUuid))
            {
                if (existing.stage != expectedStage)
                {
                    return false;
                }

                existing = journal;
                return true;
            }
        }

        return false;
    }

    bool beginMigration(const PrivacyTransaction& transaction,
                        const PrivacyTransactionJournal& journal) override
    {
        if (std::any_of(snapshot.transactions.cbegin(),
                        snapshot.transactions.cend(),
                        [&transaction](const PrivacyTransaction& candidate)
                        {
                            return ((candidate.uuid == transaction.uuid) ||
                                    ((candidate.itemUuid == transaction.itemUuid) &&
                                     candidate.isActive() &&
                                     (candidate.type !=
                                      PrivacyTransactionType::CompatibilityUnlock)));
                        }))
        {
            return false;
        }

        snapshot.transactions << transaction;
        snapshot.transactionJournals << journal;
        return true;
    }

    bool publishMigration(const PrivacyItem& item,
                          const PrivacyContainer& container,
                          const QList<PrivacyAsset>& assets,
                          const QString& sourceContainerUuid,
                          const PrivacyTransaction& transaction,
                          PrivacyTransactionState expectedState,
                          qlonglong expectedGeneration) override
    {
        for (PrivacyItem& existing : snapshot.items)
        {
            if (existing.uuid == item.uuid)
            {
                existing = item;
            }
        }

        snapshot.containers << container;
        snapshot.assets << assets;
        snapshot.containers.removeIf(
            [&sourceContainerUuid](const PrivacyContainer& candidate)
            {
                return (candidate.uuid == sourceContainerUuid);
            });
        snapshot.assets.removeIf(
            [&sourceContainerUuid](const PrivacyAsset& candidate)
            {
                return (candidate.containerUuid == sourceContainerUuid);
            });
        return replaceTransaction(transaction, expectedState,
                                  expectedGeneration);
    }

    bool removeContainerAndAssets(const QString& containerUuid,
                                  const QString& itemUuid) override
    {
        snapshot.containers.removeIf(
            [&containerUuid](const PrivacyContainer& candidate)
            {
                return (candidate.uuid == containerUuid);
            });
        snapshot.assets.removeIf(
            [&containerUuid](const PrivacyAsset& candidate)
            {
                return (candidate.containerUuid == containerUuid);
            });
        Q_UNUSED(itemUuid);
        return true;
    }

    bool replaceTransaction(const PrivacyTransaction& transaction,
                            PrivacyTransactionState state,
                            qlonglong generation)
    {
        for (PrivacyTransaction& existing : snapshot.transactions)
        {
            if (existing.uuid == transaction.uuid)
            {
                if ((existing.state != state) ||
                    (existing.generation != generation))
                {
                    return false;
                }

                existing = transaction;
                return true;
            }
        }

        return false;
    }

    PrivacyRepositorySnapshot snapshot;
};

PrivacyCategory makeCategory(const QString& uuid, const QString& name,
                             PrivacyBackend backend)
{
    PrivacyCategory category;
    category.uuid = uuid;
    category.name = name;
    category.recoverySetUuid =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    category.backend = backend;
    category.presentationMode = PrivacyPresentationMode::Generic;
    category.unlockedThumbnailMode = PrivacyUnlockedThumbnailMode::FocusedClear;
    category.tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt = QDateTime::currentDateTimeUtc();
    return category;
}

bool writeFile(const QString& path, const QByteArray& bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(bytes) == bytes.size()) && file.flush();
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

bool prepareSyntheticOriginal(QTemporaryDir& directory, QString* const sourcePath,
                              PrivacyStorageRoot* const root,
                              PrivacyJournalRootExpectation* const expectation,
                              QByteArray* const originalBytes)
{
    if (!directory.isValid() || !sourcePath || !root || !expectation ||
        !originalBytes)
    {
        return false;
    }

    *sourcePath = QDir(directory.path()).filePath(
        QLatin1String("album/photo.jpg"));

    if (!QDir().mkpath(QFileInfo(*sourcePath).absolutePath()))
    {
        return false;
    }

    QImage source(24, 16, QImage::Format_RGB32);
    source.fill(Qt::red);

    if (!source.save(*sourcePath, "JPEG") ||
        (::chmod(QFile::encodeName(*sourcePath).constData(), 0640) != 0))
    {
        return false;
    }

    QFile file(*sourcePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    *originalBytes = file.readAll();
    file.close();

    struct stat rootStat = {};
    struct stat sourceStat = {};

    if ((::stat(QFile::encodeName(directory.path()).constData(),
                &rootStat) != 0) ||
        (::stat(QFile::encodeName(*sourcePath).constData(),
                &sourceStat) != 0))
    {
        return false;
    }

    root->uuid = RootUuid;
    root->kind = PrivacyStorageRootKind::AlbumRoot;
    root->albumRootId = 1;
    root->configuredPath = directory.path();
    root->identityVersion = 1;
    root->identityData = PrivacyRootIdentityCodec::encodeAlbumRootV1(
        1, QLatin1String("synthetic-volume"));
    root->createdAt = QDateTime::currentDateTimeUtc();
    expectation->rootUuid = RootUuid;
    expectation->device = static_cast<quint64>(rootStat.st_dev);
    expectation->inode = static_cast<quint64>(rootStat.st_ino);
    expectation->identitySha256 = QCryptographicHash::hash(
        root->identityData, QCryptographicHash::Sha256);
    return true;
}

PrivacyStillProtectRequest makeProtectRequest(
    const QString& sourcePath, const PrivacyStorageRoot& root,
    const PrivacyJournalRootExpectation& expectation,
    const QString& categoryUuid, const QByteArray& originalBytes)
{
    struct stat sourceStat = {};

    if (::stat(QFile::encodeName(sourcePath).constData(),
               &sourceStat) != 0)
    {
        return {};
    }

    PrivacyInventoryAsset inventoryAsset;
    inventoryAsset.role = PrivacyInventoryAssetRole::PrimaryMedia;
    inventoryAsset.ordinal = 0;
    inventoryAsset.location.root.uuid = RootUuid;
    inventoryAsset.location.root.absolutePath = root.configuredPath;
    inventoryAsset.location.relativePath =
        QLatin1String("album/photo.jpg");
    inventoryAsset.evidence.type = PrivacyInventoryFileType::Regular;
    inventoryAsset.evidence.identityComplete = true;
    inventoryAsset.evidence.deviceId =
        static_cast<quint64>(sourceStat.st_dev);
    inventoryAsset.evidence.inode =
        static_cast<quint64>(sourceStat.st_ino);
    inventoryAsset.evidence.linkCount =
        static_cast<quint64>(sourceStat.st_nlink);
    inventoryAsset.evidence.byteSize =
        static_cast<qlonglong>(sourceStat.st_size);
    PrivacyAssetInventoryBridgeItemResult bridgeItem;
    bridgeItem.imageId = 42;
    bridgeItem.inventory.status = PrivacyInventoryStatus::Ready;
    bridgeItem.inventory.requiredAssets << inventoryAsset;

    PrivacyStillProtectRequest protect;
    protect.imageId = 42;
    protect.categoryUuid = categoryUuid;
    protect.itemUuid = ItemUuid;
    protect.containerUuid = ContainerUuid;
    protect.transactionUuid = QUuid::createUuid().toString(
        QUuid::WithoutBraces);
    protect.preflight.bridge.status = PrivacyInventoryStatus::Ready;
    protect.preflight.bridge.items << bridgeItem;
    protect.associatedAssetsAcknowledged = true;
    protect.publicRoot = root;
    protect.rootExpectation = expectation;
    protect.originalPixelSize = QSize(24, 16);
    protect.originalCreationDate = QFileInfo(sourcePath).birthTime();
    Q_UNUSED(originalBytes);
    return protect;
}

PrivacyMigrationAssetInput primaryInput()
{
    PrivacyMigrationAssetInput input;
    input.role = PrivacyAsset::PrimaryMediaRole;
    input.ordinal = 0;
    input.publicRelativePath = QLatin1String("album/photo.jpg");
    input.originalName = QLatin1String("photo.jpg");
    return input;
}

std::shared_ptr<const PrivacyPassword> password(const QString& value)
{
    return std::make_shared<const PrivacyPassword>(
        PrivacyPassword::fromUnicode(value));
}

void addStrongTarget(FakePersistence* persistence,
                     const QString& managedRootPath)
{
    PrivacyStore store;
    store.uuid = StoreUuid;
    store.categoryUuid = TargetCategoryUuid;
    store.rootUuid = ManagedRootUuid;
    store.format = QLatin1String("gocryptfs");
    store.formatVersion = 2;
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/") +
                               StoreUuid;
    store.configRelativePath = store.cipherRelativePath +
                               QLatin1String("/gocryptfs.conf");
    store.configGeneration = 1;
    store.lifecycleState = PrivacyStoreLifecycleState::Active;
    store.createdAt = QDateTime::currentDateTimeUtc();
    persistence->snapshot.stores << store;

    PrivacyStoreBinding originals;
    originals.categoryUuid = TargetCategoryUuid;
    originals.role = PrivacyStoreRole::Originals;
    originals.storeUuid = StoreUuid;
    persistence->snapshot.storeBindings << originals;

    PrivacyStorageRoot managed;
    managed.uuid = ManagedRootUuid;
    managed.kind = PrivacyStorageRootKind::ManagedStoreRoot;
    managed.albumRootId = -1;
    managed.configuredPath = managedRootPath;
    managed.identityVersion = 1;
    managed.identityData = QByteArray("synthetic-managed-root");
    managed.markerUuid = QLatin1String("80000000-0000-0000-0000-000000000001");
    managed.createdAt = QDateTime::currentDateTimeUtc();
    persistence->snapshot.storageRoots << managed;
}

} // namespace

class PrivacyMigrationCoordinatorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testUnprotectedToProtected();
    void testProtectedToUnprotected();
    void testCasualToStrongRetiresSource();
    void testInterruptedCompatibilityRollsBackOnRecovery();
};

void PrivacyMigrationCoordinatorTest::testUnprotectedToProtected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    QByteArray originalBytes;
    QVERIFY(prepareSyntheticOriginal(directory, &sourcePath, &root,
                                     &expectation, &originalBytes));

    FakePersistence persistence;
    persistence.snapshot.categories << makeCategory(
        TargetCategoryUuid, QLatin1String("Target"),
        PrivacyBackend::Casual);
    persistence.snapshot.storageRoots << root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    QCOMPARE(runtime.initialize(persistence.snapshot, verifier, {},
                                inspector).state,
             PrivacyStartupState::Ready);
    QVERIFY(runtime.setCategoryUnlocked(TargetCategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine stillEngine(
        persistence, runtime, cache);
    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyMigrationCoordinator coordinator(
        persistence, stillEngine, runtime, archiveEngine);

    PrivacyMigrationRequest request;
    request.imageId = 42;
    request.itemUuid = ItemUuid;
    request.targetCategoryUuid = TargetCategoryUuid;
    request.targetBackend = PrivacyBackend::Casual;
    request.publicRoot = root;
    request.rootExpectation = expectation;
    request.assets << primaryInput();
    request.targetPassword = password(QLatin1String("target-secret"));

    const PrivacyMigrationBatchResult batch =
        coordinator.migrateBatch({ request });
    QVERIFY2(batch.succeeded(), qPrintable(batch.items.constFirst().detail));
    QCOMPARE(batch.succeededCount, 1);
    QCOMPARE(persistence.snapshot.items.size(), 1);
    QCOMPARE(persistence.snapshot.items.constFirst().categoryUuid,
             TargetCategoryUuid);
    QCOMPARE(persistence.snapshot.containers.size(), 1);
    QCOMPARE(persistence.snapshot.containers.constFirst().kind,
             PrivacyContainerKind::CasualArchive);
    QVERIFY(readFile(sourcePath) != originalBytes);
    QVERIFY(QFileInfo::exists(sourcePath +
                              QLatin1String(".digikam-private.zip")));
}

void PrivacyMigrationCoordinatorTest::testProtectedToUnprotected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    QByteArray originalBytes;
    QVERIFY(prepareSyntheticOriginal(directory, &sourcePath, &root,
                                     &expectation, &originalBytes));

    FakePersistence persistence;
    persistence.snapshot.categories << makeCategory(
        SourceCategoryUuid, QLatin1String("Source"),
        PrivacyBackend::Casual);
    persistence.snapshot.storageRoots << root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    QCOMPARE(runtime.initialize(persistence.snapshot, verifier, {},
                                inspector).state,
             PrivacyStartupState::Ready);
    QVERIFY(runtime.setCategoryUnlocked(SourceCategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine stillEngine(
        persistence, runtime, cache);
    const std::shared_ptr<const PrivacyPassword> sourcePassword =
        password(QLatin1String("source-secret"));
    PrivacyStillProtectRequest protectRequest = makeProtectRequest(
        sourcePath, root, expectation, SourceCategoryUuid, originalBytes);
    QCOMPARE(stillEngine.protect(protectRequest, *sourcePassword).status,
             PrivacyStillItemTransactionStatus::Protected);

    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyMigrationCoordinator coordinator(
        persistence, stillEngine, runtime, archiveEngine);
    PrivacyMigrationRequest request;
    request.imageId = 42;
    request.itemUuid = ItemUuid;
    request.sourceCategoryUuid = SourceCategoryUuid;
    request.publicRoot = root;
    request.rootExpectation = expectation;
    request.assets << primaryInput();
    request.sourcePassword = sourcePassword;

    const PrivacyMigrationBatchResult batch =
        coordinator.migrateBatch({ request });
    QVERIFY2(batch.succeeded(), qPrintable(batch.items.constFirst().detail));
    QVERIFY(persistence.snapshot.items.isEmpty());
    QVERIFY(persistence.snapshot.containers.isEmpty());
    QVERIFY(persistence.snapshot.assets.isEmpty());
    QCOMPARE(readFile(sourcePath), originalBytes);
    QVERIFY(!QFileInfo::exists(sourcePath +
                               QLatin1String(".digikam-private.zip")));
}

void PrivacyMigrationCoordinatorTest::testCasualToStrongRetiresSource()
{
    QTemporaryDir directory;
    QTemporaryDir vaultDirectory;
    QVERIFY(directory.isValid());
    QVERIFY(vaultDirectory.isValid());
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    QByteArray originalBytes;
    QVERIFY(prepareSyntheticOriginal(directory, &sourcePath, &root,
                                     &expectation, &originalBytes));

    FakePersistence persistence;
    persistence.snapshot.categories << makeCategory(
        SourceCategoryUuid, QLatin1String("Source"),
        PrivacyBackend::Casual);
    persistence.snapshot.categories << makeCategory(
        TargetCategoryUuid, QLatin1String("Target"),
        PrivacyBackend::Strong);
    persistence.snapshot.storageRoots << root;
    addStrongTarget(&persistence, vaultDirectory.path());
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    QCOMPARE(runtime.initialize(persistence.snapshot, verifier, {},
                                inspector).state,
             PrivacyStartupState::Ready);
    QVERIFY(runtime.setCategoryUnlocked(SourceCategoryUuid, true));
    QVERIFY(runtime.setCategoryUnlocked(TargetCategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine stillEngine(
        persistence, runtime, cache);
    const std::shared_ptr<const PrivacyPassword> sourcePassword =
        password(QLatin1String("source-secret"));
    const std::shared_ptr<const PrivacyPassword> targetPassword =
        password(QLatin1String("target-secret"));
    PrivacyStillProtectRequest protectRequest = makeProtectRequest(
        sourcePath, root, expectation, SourceCategoryUuid, originalBytes);
    QCOMPARE(stillEngine.protect(protectRequest, *sourcePassword).status,
             PrivacyStillItemTransactionStatus::Protected);
    const QString sourceArchive = sourcePath +
                                  QLatin1String(".digikam-private.zip");
    QVERIFY(QFileInfo::exists(sourceArchive));

    QList<PrivacyPublicRecoveryLocatorEntry> locatorEntries;
    PrivacyPublicRecoveryLocatorError locatorError =
        PrivacyPublicRecoveryLocatorError::None;
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        directory.path(), &locatorEntries, &locatorError));
    QCOMPARE(locatorEntries.size(), 1);
    const QString sourceRecoverySetUuid =
        locatorEntries.constFirst().recoverySetUuid;
    QCOMPARE(locatorEntries.constFirst().backend, PrivacyBackend::Casual);

    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyMigrationCoordinator coordinator(
        persistence, stillEngine, runtime, archiveEngine);
    PrivacyMigrationRequest request;
    request.imageId = 42;
    request.itemUuid = ItemUuid;
    request.sourceCategoryUuid = SourceCategoryUuid;
    request.targetCategoryUuid = TargetCategoryUuid;
    request.targetBackend = PrivacyBackend::Strong;
    request.publicRoot = root;
    request.rootExpectation = expectation;
    request.assets << primaryInput();
    request.targetVaultPlaintextRoot = vaultDirectory.path();
    request.targetStrongStoreUuid = StoreUuid;
    request.sourcePassword = sourcePassword;
    request.targetPassword = targetPassword;

    const PrivacyMigrationBatchResult batch =
        coordinator.migrateBatch({ request });
    QVERIFY2(batch.succeeded(), qPrintable(batch.items.constFirst().detail));
    QCOMPARE(persistence.snapshot.items.size(), 1);
    QCOMPARE(persistence.snapshot.items.constFirst().categoryUuid,
             TargetCategoryUuid);
    QCOMPARE(persistence.snapshot.containers.size(), 1);
    QCOMPARE(persistence.snapshot.containers.constFirst().kind,
             PrivacyContainerKind::StrongObject);
    QCOMPARE(persistence.snapshot.containers.constFirst().storeUuid,
             StoreUuid);
    QVERIFY(readFile(sourcePath) != originalBytes);
    QVERIFY(!QFileInfo::exists(sourceArchive));
    QFile vaultObject(QDir(vaultDirectory.path()).filePath(
        QLatin1String("originals/") +
        persistence.snapshot.containers.constFirst().uuid +
        QLatin1String("/0-photo.jpg")));
    QVERIFY(vaultObject.open(QIODevice::ReadOnly));
    QCOMPARE(vaultObject.readAll(), originalBytes);
    vaultObject.close();
    QCOMPARE(persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Complete);
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        directory.path(), &locatorEntries, &locatorError));
    QCOMPARE(locatorEntries.size(), 1);
    QVERIFY(locatorEntries.constFirst().recoverySetUuid !=
            sourceRecoverySetUuid);
    QCOMPARE(locatorEntries.constFirst().backend, PrivacyBackend::Strong);

    QString targetRecoverySetUuid;

    for (const PrivacyCategory& candidate :
         std::as_const(persistence.snapshot.categories))
    {
        if (candidate.uuid == TargetCategoryUuid)
        {
            targetRecoverySetUuid = candidate.recoverySetUuid;
            break;
        }
    }

    QVERIFY(!targetRecoverySetUuid.isEmpty());
    QCOMPARE(locatorEntries.constFirst().recoverySetUuid,
             targetRecoverySetUuid);
}

void PrivacyMigrationCoordinatorTest::
    testInterruptedCompatibilityRollsBackOnRecovery()
{
    QTemporaryDir directory;
    QTemporaryDir vaultDirectory;
    QVERIFY(directory.isValid());
    QVERIFY(vaultDirectory.isValid());
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    QByteArray originalBytes;
    QVERIFY(prepareSyntheticOriginal(directory, &sourcePath, &root,
                                     &expectation, &originalBytes));

    FakePersistence persistence;
    persistence.snapshot.categories << makeCategory(
        SourceCategoryUuid, QLatin1String("Source"),
        PrivacyBackend::Casual);
    persistence.snapshot.categories << makeCategory(
        TargetCategoryUuid, QLatin1String("Target"),
        PrivacyBackend::Strong);
    persistence.snapshot.storageRoots << root;
    addStrongTarget(&persistence, vaultDirectory.path());
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    QCOMPARE(runtime.initialize(persistence.snapshot, verifier, {},
                                inspector).state,
             PrivacyStartupState::Ready);
    QVERIFY(runtime.setCategoryUnlocked(SourceCategoryUuid, true));
    QVERIFY(runtime.setCategoryUnlocked(TargetCategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine stillEngine(
        persistence, runtime, cache);
    const std::shared_ptr<const PrivacyPassword> sourcePassword =
        password(QLatin1String("source-secret"));
    const std::shared_ptr<const PrivacyPassword> targetPassword =
        password(QLatin1String("target-secret"));
    PrivacyStillProtectRequest protectRequest = makeProtectRequest(
        sourcePath, root, expectation, SourceCategoryUuid, originalBytes);
    QCOMPARE(stillEngine.protect(protectRequest, *sourcePassword).status,
             PrivacyStillItemTransactionStatus::Protected);
    const QByteArray sourceProxy = readFile(sourcePath);
    QVERIFY(sourceProxy != originalBytes);

    stillEngine.setFaultHook([](PrivacyStillItemFaultPoint point)
    {
        return (point ==
                PrivacyStillItemFaultPoint::AfterCompatibilityUnlockPublicTransition);
    });
    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyMigrationCoordinator coordinator(
        persistence, stillEngine, runtime, archiveEngine);
    PrivacyMigrationRequest request;
    request.imageId = 42;
    request.itemUuid = ItemUuid;
    request.sourceCategoryUuid = SourceCategoryUuid;
    request.targetCategoryUuid = TargetCategoryUuid;
    request.targetBackend = PrivacyBackend::Strong;
    request.publicRoot = root;
    request.rootExpectation = expectation;
    request.assets << primaryInput();
    request.targetVaultPlaintextRoot = vaultDirectory.path();
    request.targetStrongStoreUuid = StoreUuid;
    request.sourcePassword = sourcePassword;
    request.targetPassword = targetPassword;

    const PrivacyMigrationBatchResult failed =
        coordinator.migrateBatch({ request });
    QVERIFY(!failed.succeeded());
    QCOMPARE(failed.items.constFirst().status,
             PrivacyMigrationStatus::Failed);
    QVERIFY(QFileInfo::exists(sourcePath +
                              QLatin1String(".digikam-private.zip")));

    stillEngine.setFaultHook({});
    const PrivacyMigrationBatchResult recovered =
        coordinator.recover({ request });
    QCOMPARE(recovered.items.constFirst().status,
             PrivacyMigrationStatus::RolledBack);
    QCOMPARE(readFile(sourcePath), sourceProxy);
    QVERIFY(QFileInfo::exists(sourcePath +
                              QLatin1String(".digikam-private.zip")));
    const PrivacyTransaction* migrationTransaction = nullptr;

    for (const PrivacyTransaction& transaction :
         persistence.snapshot.transactions)
    {
        if (transaction.type == PrivacyTransactionType::MigrateBackend)
        {
            migrationTransaction = &transaction;
            break;
        }
    }

    QVERIFY(migrationTransaction);
    QCOMPARE(migrationTransaction->state, PrivacyTransactionState::Error);
}

QTEST_GUILESS_MAIN(PrivacyMigrationCoordinatorTest)

#include "privacymigrationcoordinator_utest.moc"
