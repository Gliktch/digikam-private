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
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
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

} // namespace

class PrivacyStillItemTransactionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void protectFaultReplay_data();
    void protectFaultReplay();
    void protectUnprotectAndReplayFinalCleanup_data();
    void protectUnprotectAndReplayFinalCleanup();
    void videoPreparedReplayRetainsExactProxy();
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
