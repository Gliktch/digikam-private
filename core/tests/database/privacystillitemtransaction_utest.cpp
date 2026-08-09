/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include <sys/stat.h>
#include <fcntl.h>

#include "privacycontracts.h"
#include "privacyprocessrunner.h"
#include "privacyproxygenerator.h"
#include "privacypublictransition.h"
#include "privacystillitemtransaction.h"
#include "privacyvideoproxygenerator.h"

using namespace Digikam;

namespace
{

const QString CategoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
const QString RootUuid = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString ItemUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QString ContainerUuid = QLatin1String("40000000-0000-0000-0000-000000000001");
const QString ProtectUuid = QLatin1String("50000000-0000-0000-0000-000000000001");
const QString UnprotectUuid = QLatin1String("60000000-0000-0000-0000-000000000001");
const QString CompatibilityUnlockUuid =
    QLatin1String("70000000-0000-0000-0000-000000000001");
const QString CompatibilityGroupUuid =
    QLatin1String("90000000-0000-0000-0000-000000000001");

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

class FakePersistence final : public PrivacyStillItemPersistence
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
                                     candidate.isActive()));
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

                if ((transaction.state == PrivacyTransactionState::Prepared) &&
                    afterPrepared)
                {
                    afterPrepared(transaction);
                }

                return true;
            }
        }

        return false;
    }

public:

    PrivacyRepositorySnapshot snapshot;
    std::function<void(const PrivacyTransaction&)> afterPrepared;
};

PrivacyCategory category()
{
    PrivacyCategory value;
    value.uuid = CategoryUuid;
    value.name = QLatin1String("Synthetic stills");
    value.backend = PrivacyBackend::Casual;
    value.presentationMode = PrivacyPresentationMode::Generic;
    value.lifecycleState = PrivacyCategoryLifecycleState::Active;
    value.currentCredentialGeneration = 1;
    value.createdAt = QDateTime::currentDateTimeUtc();
    return value;
}

bool populateSyntheticRequest(QTemporaryDir& directory,
                              const QString& relativePath,
                              const QSize& pixelSize,
                              QString* const sourcePath,
                              PrivacyStorageRoot* const root,
                              PrivacyJournalRootExpectation* const expectation,
                              PrivacyStillProtectRequest* const protect)
{
    if (!sourcePath || !root || !expectation || !protect ||
        !directory.isValid() || relativePath.isEmpty() || !pixelSize.isValid())
    {
        return false;
    }

    *sourcePath = QDir(directory.path()).filePath(relativePath);

    if (!QFileInfo(*sourcePath).isFile() ||
        (::chmod(QFile::encodeName(*sourcePath).constData(), 0640) != 0))
    {
        return false;
    }

    struct stat rootStat = {};
    struct stat sourceStat = {};

    if ((::stat(QFile::encodeName(directory.path()).constData(), &rootStat) != 0) ||
        (::stat(QFile::encodeName(*sourcePath).constData(), &sourceStat) != 0))
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

    PrivacyInventoryAsset inventoryAsset;
    inventoryAsset.role = PrivacyInventoryAssetRole::PrimaryMedia;
    inventoryAsset.ordinal = 0;
    inventoryAsset.location.root.uuid = RootUuid;
    inventoryAsset.location.root.absolutePath = directory.path();
    inventoryAsset.location.relativePath = relativePath;
    inventoryAsset.evidence.type = PrivacyInventoryFileType::Regular;
    inventoryAsset.evidence.identityComplete = true;
    inventoryAsset.evidence.deviceId = static_cast<quint64>(sourceStat.st_dev);
    inventoryAsset.evidence.inode = static_cast<quint64>(sourceStat.st_ino);
    inventoryAsset.evidence.linkCount = static_cast<quint64>(sourceStat.st_nlink);
    inventoryAsset.evidence.byteSize = sourceStat.st_size;
    PrivacyAssetInventoryBridgeItemResult bridgeItem;
    bridgeItem.imageId = 42;
    bridgeItem.inventory.status = PrivacyInventoryStatus::Ready;
    bridgeItem.inventory.requiredAssets << inventoryAsset;
    protect->imageId = 42;
    protect->categoryUuid = CategoryUuid;
    protect->itemUuid = ItemUuid;
    protect->containerUuid = ContainerUuid;
    protect->transactionUuid = ProtectUuid;
    protect->preflight.bridge.status = PrivacyInventoryStatus::Ready;
    protect->preflight.bridge.items << bridgeItem;
    protect->associatedAssetsAcknowledged = true;
    protect->publicRoot = *root;
    protect->rootExpectation = *expectation;
    protect->originalPixelSize = pixelSize;
    protect->originalCreationDate = QFileInfo(*sourcePath).birthTime();
    return true;
}

bool prepareSyntheticStill(QTemporaryDir& directory,
                           QString* const sourcePath,
                           PrivacyStorageRoot* const root,
                           PrivacyJournalRootExpectation* const expectation,
                           PrivacyStillProtectRequest* const protect)
{
    if (!sourcePath || !directory.isValid())
    {
        return false;
    }

    const QString relativePath = QLatin1String("album/synthetic.jpg");
    *sourcePath = QDir(directory.path()).filePath(relativePath);

    if (!QDir().mkpath(QFileInfo(*sourcePath).absolutePath()))
    {
        return false;
    }

    QImage source(24, 16, QImage::Format_RGB32);
    source.fill(Qt::red);

    return (source.save(*sourcePath, "JPEG") &&
            populateSyntheticRequest(directory, relativePath, source.size(),
                                     sourcePath, root, expectation, protect));
}

bool prepareSyntheticVideo(QTemporaryDir& directory,
                           const PrivacyVideoToolPaths& tools,
                           QString* const sourcePath,
                           PrivacyStorageRoot* const root,
                           PrivacyJournalRootExpectation* const expectation,
                           PrivacyStillProtectRequest* const protect)
{
    if (!tools.isValid() || !sourcePath || !directory.isValid())
    {
        return false;
    }

    const QString relativePath = QLatin1String("album/synthetic.mkv");
    *sourcePath = QDir(directory.path()).filePath(relativePath);

    if (!QDir().mkpath(QFileInfo(*sourcePath).absolutePath()))
    {
        return false;
    }

    PrivacyProcessSpec spec;
    spec.program = tools.ffmpeg;
    spec.arguments = {
        QLatin1String("-nostdin"), QLatin1String("-hide_banner"),
        QLatin1String("-loglevel"), QLatin1String("error"),
        QLatin1String("-f"), QLatin1String("lavfi"),
        QLatin1String("-i"), QLatin1String("testsrc2=s=160x96:r=2"),
        QLatin1String("-t"), QLatin1String("2"),
        QLatin1String("-g"), QLatin1String("2"),
        QLatin1String("-c:v"), QLatin1String("libx264"),
        QLatin1String("-pix_fmt"), QLatin1String("yuv420p"),
        QLatin1String("-y"), *sourcePath
    };
    spec.environment = QProcessEnvironment::systemEnvironment();
    spec.finishTimeoutMs = 30000;
    spec.maximumStdout = 1024;
    spec.maximumStderr = 16384;
    spec.sensitiveOutput = true;
    QProcessPrivacyProcessRunner runner;
    PrivacyProcessResult result = runner.run(spec, {});

    return (result.succeeded() &&
            populateSyntheticRequest(directory, relativePath, QSize(160, 96),
                                     sourcePath, root, expectation, protect));
}

bool prepareCompatibilityExposure(
    QTemporaryDir& directory, const PrivacyPassword& password,
    QString* const sourcePath, PrivacyStorageRoot* const root,
    PrivacyJournalRootExpectation* const expectation,
    PrivacyStillProtectRequest* const protect,
    FakePersistence* const persistence,
    PrivacyRuntimeCoordinator* const runtime, FakeCache* const cache,
    QByteArray* const originalBytes, QByteArray* const proxyBytes)
{
    if (!sourcePath || !root || !expectation || !protect || !persistence ||
        !runtime || !cache || !originalBytes || !proxyBytes ||
        !prepareSyntheticStill(directory, sourcePath, root, expectation,
                               protect))
    {
        return false;
    }

    QFile source(*sourcePath);

    if (!source.open(QIODevice::ReadOnly))
    {
        return false;
    }

    *originalBytes = source.readAll();
    source.close();
    persistence->snapshot.categories << category();
    persistence->snapshot.storageRoots << *root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    runtime->initialize(persistence->snapshot, verifier, {}, inspector);

    if (!runtime->setCategoryUnlocked(CategoryUuid, true))
    {
        return false;
    }

    PrivacyStillItemTransactionEngine engine(*persistence, *runtime, *cache);

    if (engine.protect(*protect, password).status !=
        PrivacyStillItemTransactionStatus::Protected)
    {
        return false;
    }

    if (!source.open(QIODevice::ReadOnly))
    {
        return false;
    }

    *proxyBytes = source.readAll();
    source.close();
    PrivacyCompatibilityUnlockRequest unlock;
    unlock.imageId = protect->imageId;
    unlock.categoryUuid = CategoryUuid;
    unlock.itemUuid = ItemUuid;
    unlock.transactionUuid = CompatibilityUnlockUuid;
    unlock.groupUuid = CompatibilityGroupUuid;
    unlock.publicRoot = *root;
    unlock.rootExpectation = *expectation;
    return (engine.compatibilityUnlock(unlock, password).status ==
            PrivacyStillItemTransactionStatus::CompatibilityUnlocked);
}

} // namespace

class PrivacyStillItemTransactionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void protectFaultReplay_data();
    void protectFaultReplay();
    void protectUnprotectAndReplayFinalCleanup_data();
    void protectUnprotectAndReplayFinalCleanup();
    void associatedAssetsProtectUnprotectRoundTrip();
    void videoPreparedReplayRetainsExactProxy();
    void compatibilityGuardRelock();
    void compatibilityGuardArmFailureCancels();
    void compatibilityDetachedGuardParentDeath();
    void compatibilityPublicTransitionRecovery();
    void rejectsUnsafeReplayInputs();
};

void PrivacyStillItemTransactionTest::protectFaultReplay_data()
{
    QTest::addColumn<int>("faultPoint");
    const QList<PrivacyStillItemFaultPoint> points = {
        PrivacyStillItemFaultPoint::AfterDatabaseBegin,
        PrivacyStillItemFaultPoint::AfterFilesystemJournal,
        PrivacyStillItemFaultPoint::AfterPreparedPayload,
        PrivacyStillItemFaultPoint::AfterReplacementStageCreated,
        PrivacyStillItemFaultPoint::AfterStagesPrepared,
        PrivacyStillItemFaultPoint::AfterArchivePublished,
        PrivacyStillItemFaultPoint::AfterProtectedCopyJournal,
        PrivacyStillItemFaultPoint::AfterPublicTransition,
        PrivacyStillItemFaultPoint::AfterCompleteJournal,
        PrivacyStillItemFaultPoint::AfterProtectedStageCleanup,
        PrivacyStillItemFaultPoint::AfterDatabasePublication,
        PrivacyStillItemFaultPoint::AfterRuntimePublication
    };

    for (const PrivacyStillItemFaultPoint point : points)
    {
        QTest::newRow(qPrintable(QString::number(static_cast<int>(point))))
            << static_cast<int>(point);
    }
}

void PrivacyStillItemTransactionTest::protectFaultReplay()
{
    QFETCH(int, faultPoint);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString relativePath = QLatin1String("album/synthetic.jpg");
    const QString sourcePath = QDir(directory.path()).filePath(relativePath);
    QVERIFY(QDir().mkpath(QFileInfo(sourcePath).absolutePath()));
    QImage source(24, 16, QImage::Format_RGB32);
    source.fill(Qt::red);
    QVERIFY(source.save(sourcePath, "JPEG"));
    QVERIFY(::chmod(QFile::encodeName(sourcePath).constData(), 0640) == 0);
    struct timespec originalTimes[2] = {};
    originalTimes[0].tv_nsec = UTIME_OMIT;
    originalTimes[1].tv_sec = 1700000000;
    originalTimes[1].tv_nsec = 123000000;
    QVERIFY(::utimensat(AT_FDCWD, QFile::encodeName(sourcePath).constData(),
                        originalTimes, 0) == 0);

    struct stat rootStat = {};
    struct stat sourceStat = {};
    QVERIFY(::stat(QFile::encodeName(directory.path()).constData(), &rootStat) == 0);
    QVERIFY(::stat(QFile::encodeName(sourcePath).constData(), &sourceStat) == 0);

    PrivacyStorageRoot root;
    root.uuid = RootUuid;
    root.kind = PrivacyStorageRootKind::AlbumRoot;
    root.albumRootId = 1;
    root.configuredPath = directory.path();
    root.identityVersion = 1;
    root.identityData = PrivacyRootIdentityCodec::encodeAlbumRootV1(
        1, QLatin1String("synthetic-volume"));
    root.createdAt = QDateTime::currentDateTimeUtc();
    PrivacyJournalRootExpectation expectation;
    expectation.rootUuid = RootUuid;
    expectation.device = static_cast<quint64>(rootStat.st_dev);
    expectation.inode = static_cast<quint64>(rootStat.st_ino);
    expectation.identitySha256 = QCryptographicHash::hash(
        root.identityData, QCryptographicHash::Sha256);

    FakePersistence persistence;
    persistence.snapshot.categories << category();
    persistence.snapshot.storageRoots << root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(persistence.snapshot, verifier, {}, inspector);
    QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);

    PrivacyInventoryAsset inventoryAsset;
    inventoryAsset.role = PrivacyInventoryAssetRole::PrimaryMedia;
    inventoryAsset.ordinal = 0;
    inventoryAsset.location.root.uuid = RootUuid;
    inventoryAsset.location.root.absolutePath = directory.path();
    inventoryAsset.location.relativePath = relativePath;
    inventoryAsset.evidence.type = PrivacyInventoryFileType::Regular;
    inventoryAsset.evidence.identityComplete = true;
    inventoryAsset.evidence.deviceId = static_cast<quint64>(sourceStat.st_dev);
    inventoryAsset.evidence.inode = static_cast<quint64>(sourceStat.st_ino);
    inventoryAsset.evidence.linkCount = static_cast<quint64>(sourceStat.st_nlink);
    inventoryAsset.evidence.byteSize = sourceStat.st_size;
    PrivacyAssetInventoryBridgeItemResult bridgeItem;
    bridgeItem.imageId = 42;
    bridgeItem.inventory.status = PrivacyInventoryStatus::Ready;
    bridgeItem.inventory.requiredAssets << inventoryAsset;
    PrivacyStillProtectRequest protect;
    protect.imageId = 42;
    protect.categoryUuid = CategoryUuid;
    protect.itemUuid = ItemUuid;
    protect.containerUuid = ContainerUuid;
    protect.transactionUuid = ProtectUuid;
    protect.preflight.bridge.status = PrivacyInventoryStatus::Ready;
    protect.preflight.bridge.items << bridgeItem;
    protect.associatedAssetsAcknowledged = true;
    protect.publicRoot = root;
    protect.rootExpectation = expectation;
    protect.originalPixelSize = source.size();
    protect.originalCreationDate = QFileInfo(sourcePath).birthTime();
    engine.setFaultHook([faultPoint](PrivacyStillItemFaultPoint point)
    {
        return (static_cast<int>(point) == faultPoint);
    });
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QLatin1String("synthetic passphrase"));
    const PrivacyStillItemTransactionResult faultResult = engine.protect(
        protect, password);
    QVERIFY2(faultResult.status == PrivacyStillItemTransactionStatus::FaultInjected,
             qPrintable(faultResult.detail));

    PrivacyRuntimeCoordinator restartedRuntime;
    restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
    FakeCache coldCache;
    PrivacyStillItemTransactionEngine restarted(
        persistence, restartedRuntime, coldCache);
    PrivacyStillItemTransactionResult replayResult = restarted.recover(
        root, ProtectUuid);

    if ((faultPoint == static_cast<int>(
             PrivacyStillItemFaultPoint::AfterDatabaseBegin)) ||
        (faultPoint == static_cast<int>(
             PrivacyStillItemFaultPoint::AfterFilesystemJournal)))
    {
        QCOMPARE(replayResult.status,
                 PrivacyStillItemTransactionStatus::AuthenticationRequired);
        QVERIFY(!restartedRuntime.isCategoryUnlocked(CategoryUuid));
        const PrivacyPassword invalidPassword =
            PrivacyPassword::fromUnicode(QString());
        QCOMPARE(restarted.resumeAuthenticated(
                     root, ProtectUuid, invalidPassword, false).status,
                 PrivacyStillItemTransactionStatus::InvalidRequest);
        replayResult = restarted.resumeAuthenticated(
            root, ProtectUuid, password, false);
        QVERIFY(!restartedRuntime.isCategoryUnlocked(CategoryUuid));
    }

    QVERIFY2(replayResult.status == PrivacyStillItemTransactionStatus::Protected,
             qPrintable(replayResult.detail));
    QCOMPARE(persistence.snapshot.items.size(), 1);
    QCOMPARE(persistence.snapshot.containers.size(), 1);
    QCOMPARE(persistence.snapshot.assets.size(), 1);
    QCOMPARE(persistence.snapshot.transactions.size(), 1);
    QCOMPARE(persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Complete);
    QVERIFY(restartedRuntime.hasProtectedItem(
        persistence.snapshot.items.constFirst(),
        persistence.snapshot.containers.constFirst(),
        persistence.snapshot.assets));
    QVERIFY(QFileInfo::exists(sourcePath + QLatin1String(".digikam-private.zip")));
    const QString displacedPath = QFileInfo(sourcePath).absolutePath() +
        QLatin1Char('/') + PrivacyPublicTransitionEngine::expectedStageFileName(
            ProtectUuid, PrivacyAsset::PrimaryMediaRole, 0);
    QVERIFY(!QFileInfo::exists(displacedPath));
}

void PrivacyStillItemTransactionTest::protectUnprotectAndReplayFinalCleanup_data()
{
    QTest::addColumn<int>("faultPoint");
    const QList<PrivacyStillItemFaultPoint> points = {
        PrivacyStillItemFaultPoint::AfterDatabaseBegin,
        PrivacyStillItemFaultPoint::AfterFilesystemJournal,
        PrivacyStillItemFaultPoint::AfterReplacementStageCreated,
        PrivacyStillItemFaultPoint::AfterStagesPrepared,
        PrivacyStillItemFaultPoint::AfterProtectedCopyJournal,
        PrivacyStillItemFaultPoint::AfterPublicTransition,
        PrivacyStillItemFaultPoint::AfterCompleteJournal,
        PrivacyStillItemFaultPoint::AfterUnprotectDatabaseTeardown,
        PrivacyStillItemFaultPoint::AfterUnprotectRuntimeRemoval,
        PrivacyStillItemFaultPoint::AfterArchiveCleanup
    };

    for (const PrivacyStillItemFaultPoint point : points)
    {
        QTest::newRow(qPrintable(QString::number(static_cast<int>(point))))
            << static_cast<int>(point);
    }
}

void PrivacyStillItemTransactionTest::protectUnprotectAndReplayFinalCleanup()
{
    QFETCH(int, faultPoint);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString relativePath = QLatin1String("album/synthetic.jpg");
    const QString sourcePath = QDir(directory.path()).filePath(relativePath);
    QVERIFY(QDir().mkpath(QFileInfo(sourcePath).absolutePath()));
    QImage source(24, 16, QImage::Format_RGB32);
    source.fill(Qt::red);
    QVERIFY(source.save(sourcePath, "JPEG"));
    QVERIFY(::chmod(QFile::encodeName(sourcePath).constData(), 0640) == 0);
    struct timespec originalTimes[2] = {};
    originalTimes[0].tv_nsec = UTIME_OMIT;
    originalTimes[1].tv_sec = 1700000000;
    originalTimes[1].tv_nsec = 123000000;
    QVERIFY(::utimensat(AT_FDCWD, QFile::encodeName(sourcePath).constData(),
                        originalTimes, 0) == 0);
    QFile sourceFile(sourcePath);
    QVERIFY(sourceFile.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = sourceFile.readAll();
    sourceFile.close();

    struct stat rootStat = {};
    struct stat sourceStat = {};
    QVERIFY(::stat(QFile::encodeName(directory.path()).constData(), &rootStat) == 0);
    QVERIFY(::stat(QFile::encodeName(sourcePath).constData(), &sourceStat) == 0);

    PrivacyStorageRoot root;
    root.uuid = RootUuid;
    root.kind = PrivacyStorageRootKind::AlbumRoot;
    root.albumRootId = 1;
    root.configuredPath = directory.path();
    root.identityVersion = 1;
    root.identityData = PrivacyRootIdentityCodec::encodeAlbumRootV1(
        1, QLatin1String("synthetic-volume"));
    root.createdAt = QDateTime::currentDateTimeUtc();
    PrivacyJournalRootExpectation expectation;
    expectation.rootUuid = RootUuid;
    expectation.device = static_cast<quint64>(rootStat.st_dev);
    expectation.inode = static_cast<quint64>(rootStat.st_ino);
    expectation.identitySha256 = QCryptographicHash::hash(
        root.identityData, QCryptographicHash::Sha256);

    FakePersistence persistence;
    persistence.snapshot.categories << category();
    persistence.snapshot.storageRoots << root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(persistence.snapshot, verifier, {}, inspector);
    QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);

    PrivacyInventoryAsset inventoryAsset;
    inventoryAsset.role = PrivacyInventoryAssetRole::PrimaryMedia;
    inventoryAsset.ordinal = 0;
    inventoryAsset.location.root.uuid = RootUuid;
    inventoryAsset.location.root.absolutePath = directory.path();
    inventoryAsset.location.relativePath = relativePath;
    inventoryAsset.evidence.type = PrivacyInventoryFileType::Regular;
    inventoryAsset.evidence.identityComplete = true;
    inventoryAsset.evidence.deviceId = static_cast<quint64>(sourceStat.st_dev);
    inventoryAsset.evidence.inode = static_cast<quint64>(sourceStat.st_ino);
    inventoryAsset.evidence.linkCount = static_cast<quint64>(sourceStat.st_nlink);
    inventoryAsset.evidence.byteSize = sourceStat.st_size;
    PrivacyAssetInventoryBridgeItemResult bridgeItem;
    bridgeItem.imageId = 42;
    bridgeItem.inventory.status = PrivacyInventoryStatus::Ready;
    bridgeItem.inventory.requiredAssets << inventoryAsset;
    PrivacyStillProtectRequest protect;
    protect.imageId = 42;
    protect.categoryUuid = CategoryUuid;
    protect.itemUuid = ItemUuid;
    protect.containerUuid = ContainerUuid;
    protect.transactionUuid = ProtectUuid;
    protect.preflight.bridge.status = PrivacyInventoryStatus::Ready;
    protect.preflight.bridge.items << bridgeItem;
    protect.associatedAssetsAcknowledged = true;
    protect.publicRoot = root;
    protect.rootExpectation = expectation;
    protect.originalPixelSize = source.size();
    protect.originalCreationDate = QFileInfo(sourcePath).birthTime();
    const PrivacyPassword protectPassword = PrivacyPassword::fromUnicode(
        QLatin1String("synthetic passphrase"));
    const PrivacyStillItemTransactionResult protectResult = engine.protect(
        protect, protectPassword);
    QVERIFY2(protectResult.status == PrivacyStillItemTransactionStatus::Protected,
             qPrintable(protectResult.detail));
    QCOMPARE(cache.beginPaths, QStringList({ sourcePath }));
    QCOMPARE(cache.finishPaths, QStringList({ sourcePath }));
    QCOMPARE(cache.beginAliasEvidence, QList<bool>({ true }));
    QCOMPARE(cache.finishJournalEvidence, QList<bool>({ true }));
    QVERIFY(QFileInfo::exists(sourcePath + QLatin1String(".digikam-private.zip")));
    QVERIFY(runtime.hasProtectedItem(persistence.snapshot.items.constFirst(),
                                     persistence.snapshot.containers.constFirst(),
                                     persistence.snapshot.assets));

    // Fresh authentication is sufficient for Unprotect. It must not require
    // or leave a retained category session behind.
    QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, false));
    QVERIFY(!runtime.isCategoryUnlocked(CategoryUuid));

    PrivacyStillUnprotectRequest unprotect;
    unprotect.imageId = 42;
    unprotect.categoryUuid = CategoryUuid;
    unprotect.transactionUuid = UnprotectUuid;
    unprotect.publicRoot = root;
    unprotect.rootExpectation = expectation;
    unprotect.freshAuthenticationConfirmed = true;
    engine.setFaultHook([faultPoint](PrivacyStillItemFaultPoint point)
    {
        return (static_cast<int>(point) == faultPoint);
    });
    const PrivacyPassword unprotectPassword = PrivacyPassword::fromUnicode(
        QLatin1String("synthetic passphrase"));
    QCOMPARE(engine.unprotect(unprotect, unprotectPassword).status,
             PrivacyStillItemTransactionStatus::FaultInjected);

    if (faultPoint == static_cast<int>(
            PrivacyStillItemFaultPoint::AfterArchiveCleanup))
    {
        QVERIFY(!QFileInfo::exists(
            sourcePath + QLatin1String(".digikam-private.zip")));
    }

    PrivacyRuntimeCoordinator restartedRuntime;
    restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
    FakeCache coldCache;
    PrivacyStillItemTransactionEngine restarted(
        persistence, restartedRuntime, coldCache);
    const PrivacyPassword replayPassword = PrivacyPassword::fromUnicode(
        QLatin1String("synthetic passphrase"));
    PrivacyStillItemTransactionResult replayResult = restarted.recover(
        root, UnprotectUuid);

    if ((faultPoint == static_cast<int>(
             PrivacyStillItemFaultPoint::AfterDatabaseBegin)) ||
        (faultPoint == static_cast<int>(
             PrivacyStillItemFaultPoint::AfterFilesystemJournal)))
    {
        QCOMPARE(replayResult.status,
                 PrivacyStillItemTransactionStatus::AuthenticationRequired);
        QVERIFY(!restartedRuntime.isCategoryUnlocked(CategoryUuid));
        QCOMPARE(restarted.resumeAuthenticated(
                     root, UnprotectUuid, replayPassword, false).status,
                 PrivacyStillItemTransactionStatus::AuthenticationRequired);
        QCOMPARE(persistence.snapshot.transactions.constLast().state,
                 PrivacyTransactionState::Created);
        replayResult = restarted.resumeAuthenticated(
            root, UnprotectUuid, replayPassword, true);
        QVERIFY(!restartedRuntime.isCategoryUnlocked(CategoryUuid));
    }

    QVERIFY2(replayResult.status ==
             PrivacyStillItemTransactionStatus::Unprotected,
             qPrintable(replayResult.detail));
    QVERIFY(persistence.snapshot.items.isEmpty());
    QVERIFY(persistence.snapshot.transactions.isEmpty());
    QVERIFY(sourceFile.open(QIODevice::ReadOnly));
    QCOMPARE(sourceFile.readAll(), originalBytes);
    sourceFile.close();
    struct stat restoredStat = {};
    QVERIFY(::stat(QFile::encodeName(sourcePath).constData(), &restoredStat) == 0);
    QCOMPARE(restoredStat.st_mode & 07777, mode_t(0640));

    for (const QString& path : cache.beginPaths + cache.finishPaths +
                               coldCache.beginPaths + coldCache.finishPaths)
    {
        QVERIFY2(QDir::isAbsolutePath(path), qPrintable(path));
        QCOMPARE(QDir::cleanPath(path), QDir::cleanPath(sourcePath));
    }

    for (bool evidence : coldCache.finishJournalEvidence)
    {
        QVERIFY(evidence);
    }

    for (bool aliasEvidence : coldCache.beginAliasEvidence)
    {
        QVERIFY(!aliasEvidence);
    }
    QCOMPARE(restoredStat.st_mtim.tv_sec, time_t(1700000000));
    QCOMPARE(restoredStat.st_mtim.tv_nsec, long(123000000));
}

void PrivacyStillItemTransactionTest::associatedAssetsProtectUnprotectRoundTrip()
{
    QTemporaryDir directory;
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    PrivacyStillProtectRequest protect;
    QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                  &expectation, &protect));

    QFile source(sourcePath);
    QVERIFY(source.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = source.readAll();
    source.close();

    const QString sidecarPath = sourcePath + QLatin1String(".xmp");
    const QByteArray sidecarBytes = QByteArrayLiteral(
        "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\">synthetic</x:xmpmeta>\n");
    QFile sidecar(sidecarPath);
    QVERIFY(sidecar.open(QIODevice::WriteOnly | QIODevice::NewOnly));
    QCOMPARE(sidecar.write(sidecarBytes), qint64(sidecarBytes.size()));
    sidecar.close();
    QVERIFY(::chmod(QFile::encodeName(sidecarPath).constData(), 0600) == 0);
    const QString sidecarAliasPath = sidecarPath + QLatin1String(".alias");
    QVERIFY(::link(QFile::encodeName(sidecarPath).constData(),
                   QFile::encodeName(sidecarAliasPath).constData()) == 0);

    struct timespec primaryTimes[2] = {};
    primaryTimes[0].tv_nsec = UTIME_OMIT;
    primaryTimes[1].tv_sec = 1700000100;
    primaryTimes[1].tv_nsec = 111000000;
    QVERIFY(::utimensat(AT_FDCWD, QFile::encodeName(sourcePath).constData(),
                        primaryTimes, 0) == 0);
    struct timespec sidecarTimes[2] = {};
    sidecarTimes[0].tv_nsec = UTIME_OMIT;
    sidecarTimes[1].tv_sec = 1700000200;
    sidecarTimes[1].tv_nsec = 222000000;
    QVERIFY(::utimensat(AT_FDCWD, QFile::encodeName(sidecarPath).constData(),
                        sidecarTimes, 0) == 0);

    struct stat sidecarStat = {};
    QVERIFY(::stat(QFile::encodeName(sidecarPath).constData(), &sidecarStat) == 0);
    PrivacyInventoryAsset sidecarAsset;
    sidecarAsset.role = PrivacyInventoryAssetRole::XmpSidecar;
    sidecarAsset.ordinal = 0;
    sidecarAsset.location.root.uuid = RootUuid;
    sidecarAsset.location.root.absolutePath = directory.path();
    sidecarAsset.location.relativePath =
        QDir(directory.path()).relativeFilePath(sidecarPath);
    sidecarAsset.evidence.type = PrivacyInventoryFileType::Regular;
    sidecarAsset.evidence.identityComplete = true;
    sidecarAsset.evidence.deviceId = static_cast<quint64>(sidecarStat.st_dev);
    sidecarAsset.evidence.inode = static_cast<quint64>(sidecarStat.st_ino);
    sidecarAsset.evidence.linkCount = static_cast<quint64>(sidecarStat.st_nlink);
    sidecarAsset.evidence.byteSize = sidecarStat.st_size;
    protect.preflight.bridge.items[0].inventory.requiredAssets << sidecarAsset;

    FakePersistence persistence;
    persistence.snapshot.categories << category();
    persistence.snapshot.storageRoots << root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(persistence.snapshot, verifier, {}, inspector);
    QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QLatin1String("synthetic passphrase"));

    const PrivacyStillItemTransactionResult protectedResult = engine.protect(
        protect, password);
    QVERIFY2(protectedResult.status ==
             PrivacyStillItemTransactionStatus::Protected,
             qPrintable(protectedResult.detail));
    QCOMPARE(persistence.snapshot.assets.size(), 2);
    QVERIFY(QFileInfo::exists(sourcePath));
    QVERIFY(!QFileInfo::exists(sidecarPath));
    QVERIFY(QFileInfo::exists(sidecarAliasPath));
    QFile sidecarAlias(sidecarAliasPath);
    QVERIFY(sidecarAlias.open(QIODevice::ReadOnly));
    QCOMPARE(sidecarAlias.readAll(), sidecarBytes);
    sidecarAlias.close();
    QVERIFY(QFileInfo::exists(sourcePath +
                              QLatin1String(".digikam-private.zip")));
    QVERIFY(source.open(QIODevice::ReadOnly));
    QVERIFY(source.readAll() != originalBytes);
    source.close();

    QCOMPARE(persistence.snapshot.transactions.size(), 1);
    const QByteArray exactPayload =
        persistence.snapshot.transactions[0].payloadData;
    QJsonDocument payloadDocument = QJsonDocument::fromJson(exactPayload);
    QVERIFY(payloadDocument.isObject());
    QJsonObject payloadObject = payloadDocument.object();
    QJsonObject mismatchedPathObject = payloadObject;
    mismatchedPathObject.insert(
        QLatin1String("archiveStageRelativePath"),
        QLatin1String("album/unrelated.digikam-private-stage.zip"));
    persistence.snapshot.transactions[0].payloadData =
        QJsonDocument(mismatchedPathObject).toJson(QJsonDocument::Compact);
    QCOMPARE(engine.recover(root, ProtectUuid).status,
             PrivacyStillItemTransactionStatus::RecoveryRequired);
    persistence.snapshot.transactions[0].payloadData = exactPayload;

    PrivacyJournalRecord mismatchedContainerRecord;
    const QByteArray encodedJournal = QByteArray::fromBase64(
        payloadObject.value(QLatin1String("journal")).toString().toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    QVERIFY(PrivacyTransactionJournalCodec::decode(
        encodedJournal, &mismatchedContainerRecord));
    QCOMPARE(mismatchedContainerRecord.assets.size(), 2);
    mismatchedContainerRecord.assets[1].containerRelativePath =
        QLatin1String("album/unrelated.digikam-private.zip");
    const QByteArray mismatchedJournal =
        PrivacyTransactionJournalCodec::encode(mismatchedContainerRecord);
    QVERIFY(!mismatchedJournal.isEmpty());
    payloadObject.insert(QLatin1String("journal"),
                         QString::fromLatin1(mismatchedJournal.toBase64()));
    persistence.snapshot.transactions[0].payloadData =
        QJsonDocument(payloadObject).toJson(QJsonDocument::Compact);
    QCOMPARE(engine.recover(root, ProtectUuid).status,
             PrivacyStillItemTransactionStatus::RecoveryRequired);
    persistence.snapshot.transactions[0].payloadData = exactPayload;

    bool foundPrimary = false;
    bool foundSidecar = false;

    for (const PrivacyAsset& asset : std::as_const(persistence.snapshot.assets))
    {
        if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
            (asset.ordinal == 0))
        {
            foundPrimary = true;
            QVERIFY(asset.proxySize >= 0);
        }
        else if ((asset.role ==
                  static_cast<int>(PrivacyInventoryAssetRole::XmpSidecar)) &&
                 (asset.ordinal == 0))
        {
            foundSidecar = true;
            QCOMPARE(asset.publicRelativePath,
                     QDir(directory.path()).relativeFilePath(sidecarPath));
            QCOMPARE(asset.proxySize, qlonglong(-1));
        }

        const QString displacedPath = QFileInfo(sourcePath).absolutePath() +
            QLatin1Char('/') +
            PrivacyPublicTransitionEngine::expectedStageFileName(
                ProtectUuid, asset.role, asset.ordinal);
        QVERIFY2(!QFileInfo::exists(displacedPath), qPrintable(displacedPath));
    }

    QVERIFY(foundPrimary);
    QVERIFY(foundSidecar);
    QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, false));

    PrivacyStillUnprotectRequest unprotect;
    unprotect.imageId = protect.imageId;
    unprotect.categoryUuid = protect.categoryUuid;
    unprotect.transactionUuid = UnprotectUuid;
    unprotect.publicRoot = root;
    unprotect.rootExpectation = expectation;
    unprotect.freshAuthenticationConfirmed = true;
    const PrivacyStillItemTransactionResult unprotectedResult =
        engine.unprotect(unprotect, password);
    QVERIFY2(unprotectedResult.status ==
             PrivacyStillItemTransactionStatus::Unprotected,
             qPrintable(unprotectedResult.detail));

    QVERIFY(source.open(QIODevice::ReadOnly));
    QCOMPARE(source.readAll(), originalBytes);
    source.close();
    QVERIFY(sidecar.open(QIODevice::ReadOnly));
    QCOMPARE(sidecar.readAll(), sidecarBytes);
    sidecar.close();
    QVERIFY(sidecarAlias.open(QIODevice::ReadOnly));
    QCOMPARE(sidecarAlias.readAll(), sidecarBytes);
    sidecarAlias.close();
    QVERIFY(!QFileInfo::exists(sourcePath +
                               QLatin1String(".digikam-private.zip")));
    QVERIFY(persistence.snapshot.items.isEmpty());
    QVERIFY(persistence.snapshot.assets.isEmpty());
    QVERIFY(persistence.snapshot.containers.isEmpty());
    QVERIFY(persistence.snapshot.transactions.isEmpty());

    struct stat restoredPrimary = {};
    struct stat restoredSidecar = {};
    QVERIFY(::stat(QFile::encodeName(sourcePath).constData(),
                   &restoredPrimary) == 0);
    QVERIFY(::stat(QFile::encodeName(sidecarPath).constData(),
                   &restoredSidecar) == 0);
    QCOMPARE(restoredPrimary.st_mode & 07777, mode_t(0640));
    QCOMPARE(restoredPrimary.st_mtim.tv_sec, time_t(1700000100));
    QCOMPARE(restoredPrimary.st_mtim.tv_nsec, long(111000000));
    QCOMPARE(restoredSidecar.st_mode & 07777, mode_t(0600));
    QCOMPARE(restoredSidecar.st_mtim.tv_sec, time_t(1700000200));
    QCOMPARE(restoredSidecar.st_mtim.tv_nsec, long(222000000));
}

void PrivacyStillItemTransactionTest::videoPreparedReplayRetainsExactProxy()
{
    const PrivacyVideoToolPaths tools = PrivacyVideoToolPaths::discover();

    if (!tools.isValid())
    {
        QSKIP("ffmpeg and ffprobe are not installed in this test environment");
    }

    QTemporaryDir directory;
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    PrivacyStillProtectRequest protect;
    QVERIFY(prepareSyntheticVideo(directory, tools, &sourcePath, &root,
                                  &expectation, &protect));
    QFile source(sourcePath);
    QVERIFY(source.open(QIODevice::ReadOnly));
    const QByteArray originalBytes = source.readAll();
    source.close();

    FakePersistence persistence;
    persistence.snapshot.categories << category();
    persistence.snapshot.storageRoots << root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(persistence.snapshot, verifier, {}, inspector);
    QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
    engine.setFaultHook([](PrivacyStillItemFaultPoint point)
    {
        return (point == PrivacyStillItemFaultPoint::AfterPreparedPayload);
    });
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QLatin1String("synthetic passphrase"));
    QCOMPARE(engine.protect(protect, password).status,
             PrivacyStillItemTransactionStatus::FaultInjected);
    QCOMPARE(persistence.snapshot.transactions.size(), 1);
    QCOMPARE(persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Prepared);
    QVERIFY(!persistence.snapshot.transactions.constFirst().payloadData.isEmpty());

    PrivacyRuntimeCoordinator restartedRuntime;
    restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
    FakeCache restartedCache;
    PrivacyStillItemTransactionEngine restarted(
        persistence, restartedRuntime, restartedCache);
    const PrivacyStillItemTransactionResult recovered = restarted.recover(
        root, ProtectUuid);
    QVERIFY2(recovered.status == PrivacyStillItemTransactionStatus::Protected,
             qPrintable(recovered.detail));
    QCOMPARE(persistence.snapshot.transactions.size(), 1);
    QCOMPARE(persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Complete);
    const QJsonDocument completedPayload = QJsonDocument::fromJson(
        persistence.snapshot.transactions.constFirst().payloadData);
    QVERIFY(completedPayload.isObject());
    QVERIFY(completedPayload.object()
                .value(QLatin1String("preparedProxyBytes"))
                .toString().isEmpty());
    QVERIFY(QFileInfo::exists(sourcePath +
                              QLatin1String(".digikam-private.zip")));
    QVERIFY(source.open(QIODevice::ReadOnly));
    QVERIFY(source.readAll() != originalBytes);
    source.close();

    PrivacyStillUnprotectRequest unprotect;
    unprotect.imageId = protect.imageId;
    unprotect.categoryUuid = protect.categoryUuid;
    unprotect.transactionUuid = UnprotectUuid;
    unprotect.publicRoot = root;
    unprotect.rootExpectation = expectation;
    unprotect.freshAuthenticationConfirmed = true;
    const PrivacyStillItemTransactionResult restored = restarted.unprotect(
        unprotect, password);
    QVERIFY2(restored.status == PrivacyStillItemTransactionStatus::Unprotected,
             qPrintable(restored.detail));
    QVERIFY(source.open(QIODevice::ReadOnly));
    QCOMPARE(source.readAll(), originalBytes);
}


void PrivacyStillItemTransactionTest::compatibilityGuardRelock()
{
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QString::fromUtf8("compatibility guard synthetic password"));
    QVERIFY(password.isValid());

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        FakePersistence persistence;
        PrivacyRuntimeCoordinator runtime;
        FakeCache cache;
        QByteArray originalBytes;
        QByteArray proxyBytes;
        QVERIFY(prepareCompatibilityExposure(
            directory, password, &sourcePath, &root, &expectation, &protect,
            &persistence, &runtime, &cache, &originalBytes, &proxyBytes));
        QFile exposed(sourcePath);
        QVERIFY(exposed.open(QIODevice::ReadOnly));
        QCOMPARE(exposed.readAll(), originalBytes);
        exposed.close();

        const PrivacyStillItemTransactionResult guarded =
            PrivacyCompatibilityExposureGuardEngine::relock(
                root, expectation, CompatibilityUnlockUuid);
        QVERIFY2(guarded.status ==
                     PrivacyStillItemTransactionStatus::CompatibilityRelocked,
                 qPrintable(guarded.detail));
        QVERIFY(exposed.open(QIODevice::ReadOnly));
        QCOMPARE(exposed.readAll(), proxyBytes);
        exposed.close();

        const auto transactionIt = std::find_if(
            persistence.snapshot.transactions.cbegin(),
            persistence.snapshot.transactions.cend(),
            [](const PrivacyTransaction& transaction)
            {
                return (transaction.uuid == CompatibilityUnlockUuid);
            });
        QVERIFY(transactionIt != persistence.snapshot.transactions.cend());
        QCOMPARE(transactionIt->state, PrivacyTransactionState::Exposed);

        PrivacyJournalError journalError = PrivacyJournalError::None;
        QString detail;
        std::unique_ptr<PrivacyTransactionJournalStore> store =
            PrivacyTransactionJournalStore::open(
                root.configuredPath, expectation, &journalError, &detail);
        QVERIFY2(store, qPrintable(detail));
        const PrivacyJournalLoadResult loaded = store->load(
            CompatibilityUnlockUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::Loaded);
        QVERIFY(loaded.authoritative);
        QCOMPARE(loaded.record.stage, PrivacyJournalStage::Complete);
        const QString stagedOriginal = QDir(root.configuredPath).filePath(
            loaded.record.assets.constFirst().stagedRelativePath);
        QVERIFY(!QFileInfo::exists(stagedOriginal));

        const PrivacyStillItemTransactionResult repeated =
            PrivacyCompatibilityExposureGuardEngine::relock(
                root, expectation, CompatibilityUnlockUuid);
        QVERIFY2(repeated.status ==
                     PrivacyStillItemTransactionStatus::CompatibilityRelocked,
                 qPrintable(repeated.detail));

        QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
        QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
        PrivacyRuntimeCoordinator restartedRuntime;
        restartedRuntime.initialize(persistence.snapshot, verifier, {},
                                    inspector);
        FakeCache restartedCache;
        PrivacyStillItemTransactionEngine restarted(
            persistence, restartedRuntime, restartedCache);
        const PrivacyStillItemTransactionResult synchronized =
            restarted.recover(root, CompatibilityUnlockUuid);
        QVERIFY2(synchronized.status ==
                     PrivacyStillItemTransactionStatus::CompatibilityRelocked,
                 qPrintable(synchronized.detail));
        const auto completedIt = std::find_if(
            persistence.snapshot.transactions.cbegin(),
            persistence.snapshot.transactions.cend(),
            [](const PrivacyTransaction& transaction)
            {
                return (transaction.uuid == CompatibilityUnlockUuid);
            });
        QVERIFY(completedIt != persistence.snapshot.transactions.cend());
        QCOMPARE(completedIt->state, PrivacyTransactionState::Complete);
        QCOMPARE(restartedRuntime.publicSourceDisposition(protect.imageId),
                 PrivacyPublicSourceDisposition::Denied);
    }

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        FakePersistence persistence;
        PrivacyRuntimeCoordinator runtime;
        FakeCache cache;
        QByteArray originalBytes;
        QByteArray proxyBytes;
        QVERIFY(prepareCompatibilityExposure(
            directory, password, &sourcePath, &root, &expectation, &protect,
            &persistence, &runtime, &cache, &originalBytes, &proxyBytes));
        QFile changed(sourcePath);
        const QByteArray changedBytes(
            "externally changed before guarded relock\n");
        QVERIFY(changed.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(changed.write(changedBytes),
                 static_cast<qint64>(changedBytes.size()));
        changed.close();
        const PrivacyStillItemTransactionResult guarded =
            PrivacyCompatibilityExposureGuardEngine::relock(
                root, expectation, CompatibilityUnlockUuid);
        QVERIFY2(guarded.status ==
                     PrivacyStillItemTransactionStatus::ReconciliationRequired,
                 qPrintable(guarded.detail));
        QVERIFY(changed.open(QIODevice::ReadOnly));
        QCOMPARE(changed.readAll(), changedBytes);
        changed.close();

        PrivacyJournalError journalError = PrivacyJournalError::None;
        QString detail;
        std::unique_ptr<PrivacyTransactionJournalStore> store =
            PrivacyTransactionJournalStore::open(
                root.configuredPath, expectation, &journalError, &detail);
        QVERIFY2(store, qPrintable(detail));
        const PrivacyJournalLoadResult loaded = store->load(
            CompatibilityUnlockUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::Loaded);
        QVERIFY(loaded.authoritative);
        QCOMPARE(loaded.record.stage,
                 PrivacyJournalStage::ReconciliationRequired);

        QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
        QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
        PrivacyRuntimeCoordinator restartedRuntime;
        restartedRuntime.initialize(persistence.snapshot, verifier, {},
                                    inspector);
        FakeCache restartedCache;
        PrivacyStillItemTransactionEngine restarted(
            persistence, restartedRuntime, restartedCache);
        const PrivacyStillItemTransactionResult recovered = restarted.recover(
            root, CompatibilityUnlockUuid);
        QVERIFY2(recovered.status ==
                     PrivacyStillItemTransactionStatus::ReconciliationRequired,
                 qPrintable(recovered.detail));
        const auto pendingIt = std::find_if(
            persistence.snapshot.transactions.cbegin(),
            persistence.snapshot.transactions.cend(),
            [](const PrivacyTransaction& transaction)
            {
                return (transaction.uuid == CompatibilityUnlockUuid);
            });
        QVERIFY(pendingIt != persistence.snapshot.transactions.cend());
        QCOMPARE(pendingIt->state,
                 PrivacyTransactionState::NeedsReconciliation);
        QVERIFY(changed.open(QIODevice::ReadOnly));
        QCOMPARE(changed.readAll(), changedBytes);
        changed.close();
    }
}

void PrivacyStillItemTransactionTest::compatibilityGuardArmFailureCancels()
{
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QString::fromUtf8("guard arm failure synthetic password"));
    QVERIFY(password.isValid());
    QTemporaryDir directory;
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    PrivacyStillProtectRequest protect;
    QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root, &expectation,
                                  &protect));
    FakePersistence persistence;
    persistence.snapshot.categories << category();
    persistence.snapshot.storageRoots << root;
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);
    PrivacyRuntimeCoordinator runtime;
    runtime.initialize(persistence.snapshot, verifier, {}, inspector);
    QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
    FakeCache cache;
    PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
    QCOMPARE(engine.protect(protect, password).status,
             PrivacyStillItemTransactionStatus::Protected);
    QFile publicFile(sourcePath);
    QVERIFY(publicFile.open(QIODevice::ReadOnly));
    const QByteArray proxyBytes = publicFile.readAll();
    publicFile.close();
    bool hookCalled = false;
    PrivacyJournalStage observedStage =
        static_cast<PrivacyJournalStage>(0);
    engine.setCompatibilityGuardArmHook(
        [&hookCalled, &observedStage](
            const PrivacyStorageRoot& hookRoot,
            const PrivacyJournalRootExpectation& hookExpectation,
            const QString& transactionUuid, QString* const detail)
        {
            hookCalled = true;
            PrivacyJournalError journalError = PrivacyJournalError::None;
            std::unique_ptr<PrivacyTransactionJournalStore> store =
                PrivacyTransactionJournalStore::open(
                    hookRoot.configuredPath, hookExpectation,
                    &journalError, detail);

            if (store)
            {
                const PrivacyJournalLoadResult loaded = store->load(
                    transactionUuid);

                if ((loaded.disposition ==
                     PrivacyJournalLoadDisposition::Loaded) &&
                    loaded.authoritative && loaded.hasRecord)
                {
                    observedStage = loaded.record.stage;
                }
            }

            if (detail)
            {
                *detail = QStringLiteral("synthetic guard launch failure");
            }

            return false;
        });
    PrivacyCompatibilityUnlockRequest unlock;
    unlock.imageId = protect.imageId;
    unlock.categoryUuid = CategoryUuid;
    unlock.itemUuid = ItemUuid;
    unlock.transactionUuid = CompatibilityUnlockUuid;
    unlock.groupUuid = CompatibilityGroupUuid;
    unlock.publicRoot = root;
    unlock.rootExpectation = expectation;
    const PrivacyStillItemTransactionResult result =
        engine.compatibilityUnlock(unlock, password);
    QVERIFY2(result.status ==
                 PrivacyStillItemTransactionStatus::RecoveryRequired,
             qPrintable(result.detail));
    QVERIFY(hookCalled);
    QCOMPARE(observedStage, PrivacyJournalStage::ProtectedCopyVerified);
    QVERIFY(publicFile.open(QIODevice::ReadOnly));
    QCOMPARE(publicFile.readAll(), proxyBytes);
    publicFile.close();
    const auto transactionIt = std::find_if(
        persistence.snapshot.transactions.cbegin(),
        persistence.snapshot.transactions.cend(),
        [](const PrivacyTransaction& transaction)
        {
            return (transaction.uuid == CompatibilityUnlockUuid);
        });
    QVERIFY(transactionIt != persistence.snapshot.transactions.cend());
    QCOMPARE(transactionIt->state, PrivacyTransactionState::Complete);
    QCOMPARE(runtime.publicSourceDisposition(protect.imageId),
             PrivacyPublicSourceDisposition::LockedProxy);

    PrivacyJournalError journalError = PrivacyJournalError::None;
    QString detail;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(
            root.configuredPath, expectation, &journalError, &detail);
    QVERIFY2(store, qPrintable(detail));
    const PrivacyJournalLoadResult loaded = store->load(
        CompatibilityUnlockUuid);
    QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::Loaded);
    QVERIFY(loaded.authoritative);
    QCOMPARE(loaded.record.stage, PrivacyJournalStage::Complete);
    QVERIFY(!QFileInfo::exists(QDir(root.configuredPath).filePath(
        loaded.record.assets.constFirst().stagedRelativePath)));

    QTemporaryDir preExposureDirectory;
    const QString guardProgram = QDir(QCoreApplication::applicationDirPath())
                                     .filePath(QStringLiteral(
                                         "digikam-private-guard"));
    QVERIFY2(QFileInfo(guardProgram).isExecutable(),
             qPrintable(guardProgram));
    QString preExposurePath;
    PrivacyStorageRoot preExposureRoot;
    PrivacyJournalRootExpectation preExposureExpectation;
    PrivacyStillProtectRequest preExposureProtect;
    QVERIFY(prepareSyntheticStill(
        preExposureDirectory, &preExposurePath, &preExposureRoot,
        &preExposureExpectation, &preExposureProtect));
    FakePersistence preExposurePersistence;
    preExposurePersistence.snapshot.categories << category();
    preExposurePersistence.snapshot.storageRoots << preExposureRoot;
    QSharedPointer<VerifiedRoot> preExposureVerifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> preExposureInspector(
        new VerifiedIntegrity);
    PrivacyRuntimeCoordinator preExposureRuntime;
    preExposureRuntime.initialize(preExposurePersistence.snapshot,
                                  preExposureVerifier, {},
                                  preExposureInspector);
    QVERIFY(preExposureRuntime.setCategoryUnlocked(CategoryUuid, true));
    FakeCache preExposureCache;
    PrivacyStillItemTransactionEngine preExposureEngine(
        preExposurePersistence, preExposureRuntime, preExposureCache);
    QCOMPARE(preExposureEngine.protect(preExposureProtect, password).status,
             PrivacyStillItemTransactionStatus::Protected);
    QFile preExposurePublic(preExposurePath);
    QVERIFY(preExposurePublic.open(QIODevice::ReadOnly));
    const QByteArray preExposureProxyBytes = preExposurePublic.readAll();
    preExposurePublic.close();
    preExposureEngine.setFaultHook([](PrivacyStillItemFaultPoint point)
    {
        return (point == PrivacyStillItemFaultPoint::
                         AfterCompatibilityUnlockApplying);
    });
    PrivacyCompatibilityUnlockRequest preExposureUnlock;
    preExposureUnlock.imageId = preExposureProtect.imageId;
    preExposureUnlock.categoryUuid = CategoryUuid;
    preExposureUnlock.itemUuid = ItemUuid;
    preExposureUnlock.transactionUuid = CompatibilityUnlockUuid;
    preExposureUnlock.groupUuid = CompatibilityGroupUuid;
    preExposureUnlock.publicRoot = preExposureRoot;
    preExposureUnlock.rootExpectation = preExposureExpectation;
    QCOMPARE(preExposureEngine.compatibilityUnlock(
                 preExposureUnlock, password).status,
             PrivacyStillItemTransactionStatus::FaultInjected);
    std::unique_ptr<PrivacyTransactionJournalStore> preExposureStore =
        PrivacyTransactionJournalStore::open(
            preExposureRoot.configuredPath, preExposureExpectation,
            &journalError, &detail);
    QVERIFY2(preExposureStore, qPrintable(detail));
    PrivacyJournalLoadResult preExposureLoaded = preExposureStore->load(
        CompatibilityUnlockUuid);
    QCOMPARE(preExposureLoaded.disposition,
             PrivacyJournalLoadDisposition::Loaded);
    QCOMPARE(preExposureLoaded.record.stage,
             PrivacyJournalStage::ProtectedCopyVerified);

    QProcess preExposureParent;
    preExposureParent.start(QStringLiteral("/bin/sleep"),
                            { QStringLiteral("30") });
    QVERIFY(preExposureParent.waitForStarted(5000));
    QProcess preExposureGuard;
    QTemporaryFile preExposureReady(
        QDir(preExposureDirectory.path()).filePath(
            QStringLiteral("guard-ready-XXXXXX")));
    QVERIFY(preExposureReady.open());
    QVERIFY(preExposureReady.setPermissions(QFileDevice::ReadOwner |
                                             QFileDevice::WriteOwner));
    const QString preExposureReadyPath = preExposureReady.fileName();
    const QString preExposureReadyToken =
        QLatin1String("a0000000-0000-0000-0000-000000000001");
    preExposureReady.close();
    preExposureGuard.setProgram(guardProgram);
    preExposureGuard.setArguments({
        QStringLiteral("--parent-pid"),
        QString::number(preExposureParent.processId()),
        QStringLiteral("--root-path"), preExposureRoot.configuredPath,
        QStringLiteral("--root-uuid"), preExposureRoot.uuid,
        QStringLiteral("--root-marker-uuid"), preExposureRoot.markerUuid,
        QStringLiteral("--root-identity"),
        QString::fromLatin1(preExposureRoot.identityData.toBase64()),
        QStringLiteral("--album-root-id"),
        QString::number(preExposureRoot.albumRootId),
        QStringLiteral("--root-device"),
        QString::number(preExposureExpectation.device),
        QStringLiteral("--root-inode"),
        QString::number(preExposureExpectation.inode),
        QStringLiteral("--ready-file"), preExposureReadyPath,
        QStringLiteral("--ready-token"), preExposureReadyToken,
        QStringLiteral("--transaction-uuid"), CompatibilityUnlockUuid
    });
    preExposureGuard.start();
    QVERIFY2(preExposureGuard.waitForStarted(5000),
             qPrintable(preExposureGuard.errorString()));
    bool preExposureAcknowledged = false;

    for (int attempt = 0 ; attempt < 500 ; ++attempt)
    {
        QFile acknowledgement(preExposureReadyPath);

        if (acknowledgement.open(QIODevice::ReadOnly) &&
            (acknowledgement.readAll() == preExposureReadyToken.toUtf8()))
        {
            preExposureAcknowledged = true;
            break;
        }

        QTest::qWait(10);
    }

    QVERIFY(preExposureAcknowledged);
    QVERIFY(preExposurePublic.open(QIODevice::ReadOnly));
    QCOMPARE(preExposurePublic.readAll(), preExposureProxyBytes);
    preExposurePublic.close();
    preExposureParent.kill();
    QVERIFY(preExposureParent.waitForFinished(5000));
    QVERIFY2(preExposureGuard.waitForFinished(15000),
             qPrintable(preExposureGuard.errorString()));
    QCOMPARE(preExposureGuard.exitStatus(), QProcess::NormalExit);
    QCOMPARE(preExposureGuard.exitCode(), 0);
    QVERIFY(preExposurePublic.open(QIODevice::ReadOnly));
    QCOMPARE(preExposurePublic.readAll(), preExposureProxyBytes);
    preExposurePublic.close();
    preExposureLoaded = preExposureStore->load(CompatibilityUnlockUuid);
    QCOMPARE(preExposureLoaded.disposition,
             PrivacyJournalLoadDisposition::Loaded);
    QCOMPARE(preExposureLoaded.record.stage, PrivacyJournalStage::Complete);
    QVERIFY(!QFileInfo::exists(QDir(preExposureRoot.configuredPath).filePath(
        preExposureLoaded.record.assets.constFirst().stagedRelativePath)));
}

void PrivacyStillItemTransactionTest::compatibilityDetachedGuardParentDeath()
{
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QString::fromUtf8("detached guard synthetic password"));
    QVERIFY(password.isValid());
    QTemporaryDir directory;
    QString sourcePath;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation expectation;
    PrivacyStillProtectRequest protect;
    FakePersistence persistence;
    PrivacyRuntimeCoordinator runtime;
    FakeCache cache;
    QByteArray originalBytes;
    QByteArray proxyBytes;
    QVERIFY(prepareCompatibilityExposure(
        directory, password, &sourcePath, &root, &expectation, &protect,
        &persistence, &runtime, &cache, &originalBytes, &proxyBytes));

    QProcess parent;
    parent.start(QStringLiteral("/bin/sleep"), { QStringLiteral("30") });
    QVERIFY(parent.waitForStarted(5000));
    QVERIFY(parent.processId() > 0);
    const QString guardProgram = QDir(QCoreApplication::applicationDirPath())
                                     .filePath(QStringLiteral(
                                         "digikam-private-guard"));
    QVERIFY2(QFileInfo(guardProgram).isExecutable(),
             qPrintable(guardProgram));
    QProcess guard;
    QTemporaryFile readyFile(QDir(directory.path()).filePath(
        QStringLiteral("guard-ready-XXXXXX")));
    QVERIFY(readyFile.open());
    QVERIFY(readyFile.setPermissions(QFileDevice::ReadOwner |
                                     QFileDevice::WriteOwner));
    const QString readyPath = readyFile.fileName();
    const QString readyToken =
        QLatin1String("a0000000-0000-0000-0000-000000000002");
    readyFile.close();
    guard.setProgram(guardProgram);
    guard.setArguments({
        QStringLiteral("--parent-pid"), QString::number(parent.processId()),
        QStringLiteral("--root-path"), root.configuredPath,
        QStringLiteral("--root-uuid"), root.uuid,
        QStringLiteral("--root-marker-uuid"), root.markerUuid,
        QStringLiteral("--root-identity"),
        QString::fromLatin1(root.identityData.toBase64()),
        QStringLiteral("--album-root-id"), QString::number(root.albumRootId),
        QStringLiteral("--root-device"), QString::number(expectation.device),
        QStringLiteral("--root-inode"), QString::number(expectation.inode),
        QStringLiteral("--ready-file"), readyPath,
        QStringLiteral("--ready-token"), readyToken,
        QStringLiteral("--transaction-uuid"), CompatibilityUnlockUuid
    });
    guard.start();
    QVERIFY2(guard.waitForStarted(5000), qPrintable(guard.errorString()));
    bool acknowledged = false;

    for (int attempt = 0 ; attempt < 500 ; ++attempt)
    {
        QFile acknowledgement(readyPath);

        if (acknowledgement.open(QIODevice::ReadOnly) &&
            (acknowledgement.readAll() == readyToken.toUtf8()))
        {
            acknowledged = true;
            break;
        }

        QTest::qWait(10);
    }

    QVERIFY(acknowledged);
    QCOMPARE(guard.state(), QProcess::Running);
    QFile publicFile(sourcePath);
    QVERIFY(publicFile.open(QIODevice::ReadOnly));
    QCOMPARE(publicFile.readAll(), originalBytes);
    publicFile.close();

    parent.kill();
    QVERIFY(parent.waitForFinished(5000));
    QVERIFY2(guard.waitForFinished(15000), qPrintable(guard.errorString()));
    QCOMPARE(guard.exitStatus(), QProcess::NormalExit);
    QCOMPARE(guard.exitCode(), 0);
    QVERIFY(publicFile.open(QIODevice::ReadOnly));
    QCOMPARE(publicFile.readAll(), proxyBytes);
    publicFile.close();

    PrivacyJournalError journalError = PrivacyJournalError::None;
    QString detail;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(
            root.configuredPath, expectation, &journalError, &detail);
    QVERIFY2(store, qPrintable(detail));
    const PrivacyJournalLoadResult loaded = store->load(
        CompatibilityUnlockUuid);
    QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::Loaded);
    QVERIFY(loaded.authoritative);
    QCOMPARE(loaded.record.stage, PrivacyJournalStage::Complete);
    QVERIFY(!QFileInfo::exists(QDir(root.configuredPath).filePath(
        loaded.record.assets.constFirst().stagedRelativePath)));
}

void PrivacyStillItemTransactionTest::compatibilityPublicTransitionRecovery()
{
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QString::fromUtf8("compatibility recovery password"));
    QVERIFY(password.isValid());
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);

    const QList<PrivacyStillItemFaultPoint> cancelledExposureFaults = {
        PrivacyStillItemFaultPoint::AfterCompatibilityUnlockDatabaseBegin,
        PrivacyStillItemFaultPoint::AfterCompatibilityUnlockStages
    };

    for (const PrivacyStillItemFaultPoint faultPoint : cancelledExposureFaults)
    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        QCOMPARE(engine.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::Protected);
        QFile proxy(sourcePath);
        QVERIFY(proxy.open(QIODevice::ReadOnly));
        const QByteArray proxyBytes = proxy.readAll();
        proxy.close();
        engine.setFaultHook([faultPoint](PrivacyStillItemFaultPoint point)
        {
            return (point == faultPoint);
        });
        PrivacyCompatibilityUnlockRequest unlock;
        unlock.imageId = protect.imageId;
        unlock.categoryUuid = CategoryUuid;
        unlock.itemUuid = ItemUuid;
        unlock.transactionUuid = CompatibilityUnlockUuid;
        unlock.groupUuid = CompatibilityGroupUuid;
        unlock.publicRoot = root;
        unlock.rootExpectation = expectation;
        QCOMPARE(engine.compatibilityUnlock(unlock, password).status,
                 PrivacyStillItemTransactionStatus::FaultInjected);

        PrivacyRuntimeCoordinator restartedRuntime;
        restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
        FakeCache restartedCache;
        PrivacyStillItemTransactionEngine restarted(
            persistence, restartedRuntime, restartedCache);
        const PrivacyStillItemTransactionResult recovered = restarted.recover(
            root, CompatibilityUnlockUuid);
        QVERIFY2(recovered.status ==
                     PrivacyStillItemTransactionStatus::CompatibilityRelocked,
                 qPrintable(recovered.detail));
        QVERIFY(proxy.open(QIODevice::ReadOnly));
        QCOMPARE(proxy.readAll(), proxyBytes);
        proxy.close();
    }

    const QList<PrivacyStillItemFaultPoint> unlockTransitionFaults = {
        PrivacyStillItemFaultPoint::AfterCompatibilityUnlockApplying,
        PrivacyStillItemFaultPoint::AfterCompatibilityUnlockPublicTransition
    };

    for (const PrivacyStillItemFaultPoint faultPoint : unlockTransitionFaults)
    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        QCOMPARE(engine.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::Protected);
        QFile proxy(sourcePath);
        QVERIFY(proxy.open(QIODevice::ReadOnly));
        const QByteArray proxyBytes = proxy.readAll();
        proxy.close();
        engine.setFaultHook([faultPoint](PrivacyStillItemFaultPoint point)
        {
            return (point == faultPoint);
        });
        PrivacyCompatibilityUnlockRequest unlock;
        unlock.imageId = protect.imageId;
        unlock.categoryUuid = CategoryUuid;
        unlock.itemUuid = ItemUuid;
        unlock.transactionUuid = CompatibilityUnlockUuid;
        unlock.groupUuid = CompatibilityGroupUuid;
        unlock.publicRoot = root;
        unlock.rootExpectation = expectation;
        QCOMPARE(engine.compatibilityUnlock(unlock, password).status,
                 PrivacyStillItemTransactionStatus::FaultInjected);

        PrivacyRuntimeCoordinator restartedRuntime;
        restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
        FakeCache restartedCache;
        PrivacyStillItemTransactionEngine restarted(
            persistence, restartedRuntime, restartedCache);
        const PrivacyStillItemTransactionResult recovered = restarted.recover(
            root, CompatibilityUnlockUuid);
        QVERIFY2(recovered.status ==
                     PrivacyStillItemTransactionStatus::CompatibilityRelocked,
                 qPrintable(recovered.detail));
        QVERIFY(proxy.open(QIODevice::ReadOnly));
        QCOMPARE(proxy.readAll(), proxyBytes);
        proxy.close();
        QCOMPARE(restartedRuntime.publicSourceDisposition(protect.imageId),
                 PrivacyPublicSourceDisposition::Denied);
    }

}

void PrivacyStillItemTransactionTest::rejectsUnsafeReplayInputs()
{
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QLatin1String("synthetic passphrase"));
    QSharedPointer<VerifiedRoot> verifier(new VerifiedRoot);
    QSharedPointer<VerifiedIntegrity> inspector(new VerifiedIntegrity);

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        QCOMPARE(engine.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::Protected);

        PrivacyStillUnprotectRequest unprotect;
        unprotect.imageId = 42;
        unprotect.categoryUuid = CategoryUuid;
        unprotect.transactionUuid = UnprotectUuid;
        unprotect.publicRoot = root;
        unprotect.rootExpectation = expectation;
        unprotect.freshAuthenticationConfirmed = false;
        QCOMPARE(engine.unprotect(unprotect, password).status,
                 PrivacyStillItemTransactionStatus::InvalidRequest);
        QCOMPARE(persistence.snapshot.items.size(), 1);
        QCOMPARE(persistence.snapshot.containers.size(), 1);
        QVERIFY(QFileInfo::exists(
            sourcePath + QLatin1String(".digikam-private.zip")));
    }

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        const QString stagePath = QFileInfo(sourcePath).absolutePath() +
            QLatin1Char('/') +
            PrivacyPublicTransitionEngine::expectedStageFileName(
                ProtectUuid, PrivacyAsset::PrimaryMediaRole, 0);
        bool stageWritten = false;
        persistence.afterPrepared = [stagePath, &stageWritten](
            const PrivacyTransaction&)
        {
            QFile stage(stagePath);
            stageWritten = stage.open(QIODevice::WriteOnly | QIODevice::NewOnly) &&
                (stage.write(QByteArrayLiteral("mismatched-existing-stage")) ==
                 QByteArrayLiteral("mismatched-existing-stage").size()) &&
                stage.flush();
            stage.close();
            stageWritten = stageWritten &&
                (::chmod(QFile::encodeName(stagePath).constData(), 0600) == 0);
        };
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        QCOMPARE(engine.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::ProxyFailure);
        QVERIFY(stageWritten);
        QFile original(sourcePath);
        QVERIFY(original.open(QIODevice::ReadOnly));
        QVERIFY(!original.readAll().isEmpty());
    }

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        PrivacyStillProxyRequest proxyRequest;
        proxyRequest.sourcePath = sourcePath;
        proxyRequest.publicFileName = QFileInfo(sourcePath).fileName();
        proxyRequest.presentation = PrivacyStillProxyPresentation::Generic;
        const PrivacyStillProxyResult proxy =
            PrivacyStillProxyGenerator().generate(proxyRequest);
        QVERIFY(proxy.isValid());
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        const QString stagePath = QFileInfo(sourcePath).absolutePath() +
            QLatin1Char('/') +
            PrivacyPublicTransitionEngine::expectedStageFileName(
                ProtectUuid, PrivacyAsset::PrimaryMediaRole, 0);
        bool stageWritten = false;
        persistence.afterPrepared = [stagePath, proxy, &stageWritten](
            const PrivacyTransaction&)
        {
            QFile stage(stagePath);
            stageWritten = stage.open(QIODevice::WriteOnly | QIODevice::NewOnly) &&
                (stage.write(proxy.encodedBytes) == proxy.encodedBytes.size()) &&
                stage.flush();
            stage.close();
            stageWritten = stageWritten &&
                (::chmod(QFile::encodeName(stagePath).constData(), 0600) == 0);
        };
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        const PrivacyStillItemTransactionResult result =
            engine.protect(protect, password);
        QVERIFY(stageWritten);
        QVERIFY2(result.status == PrivacyStillItemTransactionStatus::Protected,
                 qPrintable(result.detail));
    }

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        engine.setFaultHook([](PrivacyStillItemFaultPoint point)
        {
            return (point ==
                    PrivacyStillItemFaultPoint::AfterProtectedCopyJournal);
        });
        QCOMPARE(engine.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::FaultInjected);

        PrivacyJournalError journalError = PrivacyJournalError::None;
        QString detail;
        std::unique_ptr<PrivacyTransactionJournalStore> store =
            PrivacyTransactionJournalStore::open(
                root.configuredPath, expectation, &journalError, &detail);
        QVERIFY2(store != nullptr, qPrintable(detail));
        const PrivacyJournalLoadResult loaded = store->load(ProtectUuid);
        QVERIFY(loaded.authoritative && loaded.hasRecord);
        PrivacyJournalRecord unbound = loaded.record;
        unbound.stage = PrivacyJournalStage::Applying;
        unbound.assets[0].proxy.sha256 = QCryptographicHash::hash(
            QByteArrayLiteral("valid-but-unbound"), QCryptographicHash::Sha256);
        QByteArray unboundHash;
        QVERIFY2(store->compareAndUpdate(unbound, loaded.sha256, &unboundHash,
                                        &journalError, &detail),
                 qPrintable(detail));

        PrivacyRuntimeCoordinator restartedRuntime;
        restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(restartedRuntime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache coldCache;
        PrivacyStillItemTransactionEngine restarted(
            persistence, restartedRuntime, coldCache);
        QCOMPARE(restarted.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::JournalFailure);
    }

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        QFile original(sourcePath);
        QVERIFY(original.open(QIODevice::ReadOnly));
        const QByteArray originalBytes = original.readAll();
        original.close();
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        QCOMPARE(engine.recover(root, UnprotectUuid).status,
                 PrivacyStillItemTransactionStatus::RecoveryRequired);
        QVERIFY(original.open(QIODevice::ReadOnly));
        QCOMPARE(original.readAll(), originalBytes);
    }

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        engine.setFaultHook([](PrivacyStillItemFaultPoint point)
        {
            return (point == PrivacyStillItemFaultPoint::AfterFilesystemJournal);
        });
        QCOMPARE(engine.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::FaultInjected);

        PrivacyStorageRoot wrongRoot = root;
        wrongRoot.configuredPath = directory.filePath(QLatin1String("other-root"));
        QVERIFY(QDir().mkpath(wrongRoot.configuredPath));
        PrivacyRuntimeCoordinator restartedRuntime;
        restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
        FakeCache coldCache;
        PrivacyStillItemTransactionEngine restarted(
            persistence, restartedRuntime, coldCache);
        QCOMPARE(restarted.recover(wrongRoot, ProtectUuid).status,
                 PrivacyStillItemTransactionStatus::RootUnavailable);
        QVERIFY(QFileInfo::exists(sourcePath));
        QVERIFY(!QFileInfo::exists(
            sourcePath + QLatin1String(".digikam-private.zip")));
    }

    {
        QTemporaryDir directory;
        QString sourcePath;
        PrivacyStorageRoot root;
        PrivacyJournalRootExpectation expectation;
        PrivacyStillProtectRequest protect;
        QVERIFY(prepareSyntheticStill(directory, &sourcePath, &root,
                                      &expectation, &protect));
        FakePersistence persistence;
        persistence.snapshot.categories << category();
        persistence.snapshot.storageRoots << root;
        PrivacyRuntimeCoordinator runtime;
        runtime.initialize(persistence.snapshot, verifier, {}, inspector);
        QVERIFY(runtime.setCategoryUnlocked(CategoryUuid, true));
        FakeCache cache;
        PrivacyStillItemTransactionEngine engine(persistence, runtime, cache);
        engine.setFaultHook([](PrivacyStillItemFaultPoint point)
        {
            return (point == PrivacyStillItemFaultPoint::AfterFilesystemJournal);
        });
        QCOMPARE(engine.protect(protect, password).status,
                 PrivacyStillItemTransactionStatus::FaultInjected);
        QCOMPARE(persistence.snapshot.transactions.size(), 1);
        QVERIFY(!persistence.snapshot.transactions[0].payloadData.isEmpty());
        persistence.snapshot.transactions[0].payloadData[0] =
            static_cast<char>(persistence.snapshot.transactions[0].payloadData[0] ^
                              0x01);

        PrivacyRuntimeCoordinator restartedRuntime;
        restartedRuntime.initialize(persistence.snapshot, verifier, {}, inspector);
        FakeCache coldCache;
        PrivacyStillItemTransactionEngine restarted(
            persistence, restartedRuntime, coldCache);
        QCOMPARE(restarted.recover(root, ProtectUuid).status,
                 PrivacyStillItemTransactionStatus::RecoveryRequired);
        QVERIFY(QFileInfo::exists(sourcePath));
        QVERIFY(!QFileInfo::exists(
            sourcePath + QLatin1String(".digikam-private.zip")));
    }
}

QTEST_MAIN(PrivacyStillItemTransactionTest)

#include "privacystillitemtransaction_utest.moc"
