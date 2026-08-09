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

#include <cstring>
#include <memory>

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

// POSIX includes

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

// Local includes

#include "privacypublictransition.h"

using namespace Digikam;

namespace
{

const QString TransactionUuid = QStringLiteral("11111111-1111-4111-8111-111111111111");
const QString CategoryUuid    = QStringLiteral("22222222-2222-4222-8222-222222222222");
const QString RootUuid        = QStringLiteral("33333333-3333-4333-8333-333333333333");
const QString ItemUuid        = QStringLiteral("44444444-4444-4444-8444-444444444444");
const QString ContainerUuid   = QStringLiteral("55555555-5555-4555-8555-555555555555");

const QByteArray OriginalBytes  = QByteArrayLiteral("synthetic-original-bytes");
const QByteArray ProxyBytes     = QByteArrayLiteral("synthetic-proxy-bytes");
const QByteArray SidecarBytes   = QByteArrayLiteral("synthetic-sidecar-bytes");
const QByteArray ContainerBytes = QByteArrayLiteral("synthetic-protected-container");

QByteArray digest(const QByteArray& bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}

PrivacyJournalObjectFact fact(const QByteArray& bytes)
{
    PrivacyJournalObjectFact result;
    result.presence = PrivacyJournalExpectedPresence::Present;
    result.size     = bytes.size();
    result.linkCount = 1;
    result.sha256   = digest(bytes);
    return result;
}

PrivacyJournalObjectFact absentFact()
{
    PrivacyJournalObjectFact result;
    result.presence = PrivacyJournalExpectedPresence::Absent;
    return result;
}

bool writeNew(const QString& path, const QByteArray& bytes, mode_t mode = 0600)
{
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) ||
        (file.write(bytes) != bytes.size()) || !file.flush())
    {
        return false;
    }

    file.close();
    return (::chmod(QFile::encodeName(path).constData(), mode) == 0);
}

bool replaceBytes(const QString& path, const QByteArray& bytes)
{
    QFile file(path);
    return (file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
            (file.write(bytes) == bytes.size()) && file.flush());
}

QByteArray readBytes(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

class TransitionFixture
{
public:

    TransitionFixture()
        : root(QStringLiteral("/tmp/dpt-XXXXXX"))
    {
    }

    bool initialize(PrivacyTransactionType type = PrivacyTransactionType::ProtectItem,
                    bool publicPresent = true,
                    bool advanceToProtected = true,
                    bool adjacentContainer = true,
                    quint64 currentLinkCount = 1,
                    bool createStage = true,
                    bool identicalPublicFacts = false,
                    PrivacyJournalStage initialStage =
                        PrivacyJournalStage::Staged,
                    bool includeAbsentAssociated = false)
    {
        if (!root.isValid())
        {
            return false;
        }

        publicDirectory = root.path() + QStringLiteral("/album/nested");

        if (!QDir().mkpath(publicDirectory))
        {
            return false;
        }

        struct stat rootFacts = {};

        if (::stat(QFile::encodeName(root.path()).constData(), &rootFacts) != 0)
        {
            return false;
        }

        expectation.rootUuid       = RootUuid;
        expectation.identitySha256 = digest(QByteArrayLiteral("transition-root-v1"));
        expectation.device         = static_cast<quint64>(rootFacts.st_dev);
        expectation.inode          = static_cast<quint64>(rootFacts.st_ino);

        PrivacyJournalError journalError = PrivacyJournalError::None;
        QString detail;
        store = PrivacyTransactionJournalStore::open(root.path(), expectation,
                                                      &journalError, &detail);

        if (!store)
        {
            return false;
        }

        const QString stageFile =
            PrivacyPublicTransitionEngine::expectedStageFileName(
                TransactionUuid, 1, 0);
        publicRelativePath = QStringLiteral("album/nested/photo.jpg");
        stagedRelativePath = QStringLiteral("album/nested/") + stageFile;
        publicPath         = root.path() + QLatin1Char('/') + publicRelativePath;
        stagedPath         = root.path() + QLatin1Char('/') + stagedRelativePath;
        containerRelativePath = adjacentContainer
            ? QStringLiteral("album/nested/photo.jpg.digikam-private.zip")
            : QString();
        containerPath = containerRelativePath.isEmpty()
                      ? QString()
                      : (root.path() + QLatin1Char('/') + containerRelativePath);
        associatedPublicRelativePath = QStringLiteral(
            "album/nested/photo.jpg.xmp");
        associatedStagedRelativePath = QStringLiteral("album/nested/") +
            PrivacyPublicTransitionEngine::expectedStageFileName(
                TransactionUuid, 3, 0);
        associatedPublicPath = root.path() + QLatin1Char('/') +
                               associatedPublicRelativePath;
        associatedStagedPath = root.path() + QLatin1Char('/') +
                               associatedStagedRelativePath;

        const bool installedIsProxy =
            (type == PrivacyTransactionType::ProtectItem);
        const QByteArray stagedBytes = installedIsProxy ? ProxyBytes : OriginalBytes;
        const QByteArray currentBytes = identicalPublicFacts
                                      ? stagedBytes
                                      : (installedIsProxy
                                             ? OriginalBytes
                                             : ProxyBytes);

        if ((createStage && !writeNew(stagedPath, stagedBytes)) ||
            (adjacentContainer && !writeNew(containerPath, ContainerBytes)) ||
            (publicPresent && !writeNew(publicPath, currentBytes, 0640)) ||
            (includeAbsentAssociated &&
             !writeNew(associatedPublicPath, SidecarBytes, 0640)))
        {
            return false;
        }

        if (publicPresent && (currentLinkCount > 1))
        {
            if (currentLinkCount != 2)
            {
                return false;
            }

            publicAliasPath = publicDirectory + QStringLiteral("/acknowledged-alias.jpg");

            if (::link(QFile::encodeName(publicPath).constData(),
                       QFile::encodeName(publicAliasPath).constData()) != 0)
            {
                return false;
            }
        }

        PrivacyJournalAsset asset;
        asset.itemUuid              = ItemUuid;
        asset.containerUuid         = ContainerUuid;
        asset.role                  = 1;
        asset.ordinal               = 0;
        asset.publicRelativePath    = publicRelativePath;
        asset.stagedRelativePath    = stagedRelativePath;
        asset.protectedRelativePath = QStringLiteral("digikam-private/assets/1/0/photo.jpg");
        asset.containerRelativePath = containerRelativePath;
        asset.original              = fact(OriginalBytes);
        asset.proxy                 = fact(ProxyBytes);
        asset.container             = fact(ContainerBytes);

        if (identicalPublicFacts)
        {
            asset.original = fact(stagedBytes);
            asset.proxy    = fact(stagedBytes);
        }

        if (installedIsProxy)
        {
            asset.original.linkCount = currentLinkCount;
        }
        else
        {
            asset.proxy.linkCount = currentLinkCount;
        }

        record.transactionUuid          = TransactionUuid;
        record.categoryUuid             = CategoryUuid;
        record.rootUuid                 = RootUuid;
        record.rootDevice               = store->rootDevice();
        record.rootInode                = store->rootInode();
        record.rootIdentitySha256       = expectation.identitySha256;
        record.transactionType          = type;
        record.generation               = 8;
        record.credentialGeneration     = 3;
        record.fromCredentialGeneration = 3;
        record.toCredentialGeneration   = 3;
        record.stage                    = initialStage;
        record.assets                   = { asset };

        if (includeAbsentAssociated)
        {
            PrivacyJournalAsset associated;
            associated.itemUuid = ItemUuid;
            associated.containerUuid = ContainerUuid;
            associated.role = 3;
            associated.ordinal = 0;
            associated.publicRelativePath = associatedPublicRelativePath;
            associated.stagedRelativePath = associatedStagedRelativePath;
            associated.protectedRelativePath = QStringLiteral(
                "digikam-private/assets/3/0/photo.jpg.xmp");
            associated.containerRelativePath = containerRelativePath;
            associated.original = fact(SidecarBytes);
            associated.proxy = absentFact();
            associated.container = fact(ContainerBytes);
            record.assets << associated;
        }

        if (!store->create(record, &journalHash, &journalError, &detail))
        {
            return false;
        }

        if (advanceToProtected)
        {
            record.stage = PrivacyJournalStage::ProtectedCopyVerified;
            QByteArray protectedHash;

            if (!store->compareAndUpdate(record, journalHash, &protectedHash,
                                         &journalError, &detail))
            {
                return false;
            }

            journalHash = protectedHash;
        }

        return true;
    }

    bool refreshJournal()
    {
        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);

        if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
            !loaded.authoritative || !loaded.hasRecord)
        {
            return false;
        }

        record      = loaded.record;
        journalHash = loaded.sha256;
        return true;
    }

    PrivacyPublicReplacementStageRequest stageRequest() const
    {
        PrivacyPublicReplacementStageRequest request;
        request.absoluteRootPath           = root.path();
        request.rootExpectation            = expectation;
        request.journalRecord              = record;
        request.authoritativeJournalSha256 = journalHash;
        request.itemUuid                   = ItemUuid;
        request.role                       = 1;
        request.ordinal                    = 0;
        return request;
    }

    PrivacyPublicTransitionRequest request(
        PrivacyPublicTransitionMode mode =
            PrivacyPublicTransitionMode::ExchangePresent) const
    {
        PrivacyPublicTransitionRequest request;
        request.absoluteRootPath            = root.path();
        request.rootExpectation             = expectation;
        request.journalRecord               = record;
        request.authoritativeJournalSha256  = journalHash;
        request.itemUuid                    = ItemUuid;
        request.role                        = 1;
        request.ordinal                     = 0;
        request.mode                        = mode;

        const bool installedIsProxy =
            (record.transactionType == PrivacyTransactionType::ProtectItem);
        request.installedFact = installedIsProxy
                              ? PrivacyPublicTransitionFactKind::Proxy
                              : PrivacyPublicTransitionFactKind::Original;
        request.currentFact   = installedIsProxy
                              ? PrivacyPublicTransitionFactKind::Original
                              : PrivacyPublicTransitionFactKind::Proxy;
        return request;
    }

    PrivacyPublicTransitionRequest associatedRequest() const
    {
        PrivacyPublicTransitionRequest request;
        request.absoluteRootPath            = root.path();
        request.rootExpectation             = expectation;
        request.journalRecord               = record;
        request.authoritativeJournalSha256  = journalHash;
        request.itemUuid                    = ItemUuid;
        request.role                        = 3;
        request.ordinal                     = 0;
        request.mode                        = PrivacyPublicTransitionMode::RemovePresent;
        request.currentFact                 = PrivacyPublicTransitionFactKind::Original;
        request.installedFact               = PrivacyPublicTransitionFactKind::Proxy;
        return request;
    }

    PrivacyPublicTransitionRequest guardRequest() const
    {
        PrivacyPublicTransitionRequest request = this->request();
        request.direction =
            PrivacyPublicTransitionDirection::CompatibilityGuardRelock;
        request.currentFact = PrivacyPublicTransitionFactKind::Original;
        request.installedFact = PrivacyPublicTransitionFactKind::Proxy;
        return request;
    }

public:

    QTemporaryDir root;
    QString publicDirectory;
    QString publicRelativePath;
    QString stagedRelativePath;
    QString containerRelativePath;
    QString publicPath;
    QString stagedPath;
    QString containerPath;
    QString publicAliasPath;
    QString associatedPublicRelativePath;
    QString associatedStagedRelativePath;
    QString associatedPublicPath;
    QString associatedStagedPath;
    PrivacyJournalRootExpectation expectation;
    std::unique_ptr<PrivacyTransactionJournalStore> store;
    PrivacyJournalRecord record;
    QByteArray journalHash;
};

} // namespace

class PrivacyPublicTransitionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testExchangePresentAndRetainDisplaced();
    void testInstallAbsent();
    void testBatchExchangeAndRemoveReplaysPartialMutation();
    void testUnprotectDirection();
    void testCrossRootProtectedProof();
    void testAcknowledgedProtectHardlinks();
    void testDescriptorConfinedReplacementStaging();
    void testRejectJournalStageCasAndFactMismatch();
    void testHostileFilesystemEntries();
    void testParentReplacementAndNameRace();
    void testEveryNormalFaultBoundary();
    void testApplyingReplayBeforeAndAfterMutation();
    void testApplyingReplayRejectsMixedBytes();
    void testFinalJournalFailureRequiresReconciliation();
    void testRollbackSuccessAndFailure();
    void testCompatibilityGuardRelock();
};

void PrivacyPublicTransitionTest::testExchangePresentAndRetainDisplaced()
{
    TransitionFixture fixture;
    QVERIFY(fixture.initialize());
    PrivacyPublicTransitionEngine engine;
    const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
    QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
    QCOMPARE(readBytes(fixture.containerPath), ContainerBytes);
    QCOMPARE(result.displacedRelativePath, fixture.stagedRelativePath);
    QVERIFY(result.namespaceMutated);
    QVERIFY(result.installedVerified);
    QVERIFY(result.displacedVerified);
    QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
             PrivacyJournalStage::PublicStateVerified);
}

void PrivacyPublicTransitionTest::testInstallAbsent()
{
    TransitionFixture fixture;
    QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem, false));
    PrivacyPublicTransitionEngine engine;
    const PrivacyPublicTransitionResult result = engine.execute(
        fixture.request(PrivacyPublicTransitionMode::InstallAbsent));
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
    QVERIFY(!QFileInfo::exists(fixture.stagedPath));
    QCOMPARE(readBytes(fixture.containerPath), ContainerBytes);
    QVERIFY(result.namespaceMutated);
    QVERIFY(result.installedVerified);
    QVERIFY(!result.displacedVerified);
}

void PrivacyPublicTransitionTest::testBatchExchangeAndRemoveReplaysPartialMutation()
{
    TransitionFixture fixture;
    QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                               true, true, true, 1, true, false,
                               PrivacyJournalStage::Staged, true));
    PrivacyPublicTransitionEngine engine;
    int mutations = 0;
    engine.setFaultHook([&mutations](PrivacyPublicTransitionFaultPoint point)
    {
        if (point == PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation)
        {
            ++mutations;
            return (mutations == 1);
        }

        return false;
    });
    const QList<PrivacyPublicTransitionRequest> initialRequests =
    {
        fixture.request(), fixture.associatedRequest()
    };
    QList<PrivacyPublicTransitionRequest> malformedRequests = initialRequests;
    malformedRequests[1].installedFact =
        PrivacyPublicTransitionFactKind::Original;
    QCOMPARE(engine.executeBatch(malformedRequests).error,
             PrivacyPublicTransitionError::InvalidRequest);
    QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
             PrivacyJournalStage::ProtectedCopyVerified);
    QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
    QCOMPARE(readBytes(fixture.associatedPublicPath), SidecarBytes);

    const PrivacyPublicTransitionResult interrupted = engine.executeBatch(
        initialRequests);
    QCOMPARE(interrupted.error,
             PrivacyPublicTransitionError::DurabilityUncertain);
    QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
             PrivacyJournalStage::Applying);
    QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
    QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
    QCOMPARE(readBytes(fixture.associatedPublicPath), SidecarBytes);
    QVERIFY(!QFileInfo::exists(fixture.associatedStagedPath));

    engine.setFaultHook({});
    QVERIFY(fixture.refreshJournal());
    const QList<PrivacyPublicTransitionRequest> replayRequests =
    {
        fixture.request(), fixture.associatedRequest()
    };
    const PrivacyPublicTransitionResult recovered = engine.executeBatch(
        replayRequests);
    QVERIFY2(recovered.succeeded(), qPrintable(recovered.detail));
    QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
    QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
    QVERIFY(!QFileInfo::exists(fixture.associatedPublicPath));
    QCOMPARE(readBytes(fixture.associatedStagedPath), SidecarBytes);
    QCOMPARE(recovered.displacedRelativePaths.size(), 2);
    QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
             PrivacyJournalStage::PublicStateVerified);

    QVERIFY(fixture.refreshJournal());
    const PrivacyPublicTransitionResult replayed = engine.executeBatch(
        { fixture.request(), fixture.associatedRequest() });
    QVERIFY2(replayed.succeeded(), qPrintable(replayed.detail));
}

void PrivacyPublicTransitionTest::testUnprotectDirection()
{
    TransitionFixture fixture;
    QVERIFY(fixture.initialize(PrivacyTransactionType::UnprotectItem));
    PrivacyPublicTransitionEngine engine;
    const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
    QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    QCOMPARE(readBytes(fixture.containerPath), ContainerBytes);
}

void PrivacyPublicTransitionTest::testCompatibilityGuardRelock()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(
            PrivacyTransactionType::CompatibilityUnlock));
        PrivacyPublicTransitionEngine engine;
        QVERIFY(engine.execute(fixture.request()).succeeded());
        QVERIFY(fixture.refreshJournal());
        QCOMPARE(fixture.record.stage,
                 PrivacyJournalStage::PublicStateVerified);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);

        const PrivacyPublicTransitionResult guarded = engine.executeBatch(
            { fixture.guardRequest() });
        QVERIFY2(guarded.succeeded(), qPrintable(guarded.detail));
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::Complete);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(
            PrivacyTransactionType::CompatibilityUnlock));
        PrivacyPublicTransitionEngine engine;
        QVERIFY(engine.execute(fixture.request()).succeeded());
        QVERIFY(fixture.refreshJournal());
        int mutations = 0;
        engine.setFaultHook(
            [&mutations](PrivacyPublicTransitionFaultPoint point)
            {
                if (point ==
                    PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation)
                {
                    ++mutations;
                    return true;
                }

                return false;
            });
        const PrivacyPublicTransitionResult interrupted = engine.executeBatch(
            { fixture.guardRequest() });
        QCOMPARE(interrupted.error,
                 PrivacyPublicTransitionError::DurabilityUncertain);
        QCOMPARE(mutations, 1);
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::ReconciliationRequired);

        engine.setFaultHook({});
        QVERIFY(fixture.refreshJournal());
        const PrivacyPublicTransitionResult replayed = engine.executeBatch(
            { fixture.guardRequest() });
        QVERIFY2(replayed.succeeded(), qPrintable(replayed.detail));
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::Complete);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(
            PrivacyTransactionType::CompatibilityUnlock));
        PrivacyPublicTransitionEngine engine;
        QVERIFY(engine.execute(fixture.request()).succeeded());
        QVERIFY(fixture.refreshJournal());
        QVERIFY(replaceBytes(fixture.publicPath,
                             QByteArrayLiteral("outside-change")));
        const PrivacyPublicTransitionResult changed = engine.executeBatch(
            { fixture.guardRequest() });
        QCOMPARE(changed.error,
                 PrivacyPublicTransitionError::FileFactMismatch);
        QCOMPARE(readBytes(fixture.publicPath),
                 QByteArrayLiteral("outside-change"));
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::PublicStateVerified);
    }
}

void PrivacyPublicTransitionTest::testCrossRootProtectedProof()
{
    TransitionFixture fixture;
    QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                               true, true, false));
    QVERIFY(fixture.record.assets.first().containerRelativePath.isEmpty());
    QVERIFY(fixture.record.assets.first().container.presence ==
            PrivacyJournalExpectedPresence::Present);
    PrivacyPublicTransitionEngine engine;
    const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
    QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
}

void PrivacyPublicTransitionTest::testAcknowledgedProtectHardlinks()
{
    TransitionFixture fixture;
    QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                               true, true, true, 2));
    QCOMPARE(fixture.record.assets.first().original.linkCount, quint64(2));
    PrivacyPublicTransitionEngine engine;
    const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
    QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
    QCOMPARE(readBytes(fixture.publicAliasPath), OriginalBytes);
    struct stat displaced = {};
    QCOMPARE(::stat(QFile::encodeName(fixture.stagedPath).constData(), &displaced), 0);
    QCOMPARE(static_cast<quint64>(displaced.st_nlink), quint64(2));
}

void PrivacyPublicTransitionTest::testDescriptorConfinedReplacementStaging()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                                   true, false, true, 1, false, false,
                                   PrivacyJournalStage::Prepared));
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicReplacementStageResult result =
            engine.stageReplacement(fixture.stageRequest(), ProxyBytes);
        QVERIFY2(result.succeeded(), qPrintable(result.detail));
        QCOMPARE(result.stagedRelativePath, fixture.stagedRelativePath);
        QCOMPARE(result.fact.presence,
                 PrivacyJournalExpectedPresence::Present);
        QCOMPARE(result.fact.size, qlonglong(ProxyBytes.size()));
        QCOMPARE(result.fact.linkCount, quint64(1));
        QCOMPARE(result.fact.sha256, digest(ProxyBytes));
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);

        struct stat staged = {};
        QCOMPARE(::stat(QFile::encodeName(fixture.stagedPath).constData(),
                        &staged), 0);
        QCOMPARE(staged.st_mode & 0777, mode_t(0600));
        QCOMPARE(engine.stageReplacement(fixture.stageRequest(), ProxyBytes).error,
                 PrivacyPublicTransitionError::UnexpectedExistingFile);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                                   true, false, true, 1, false, false,
                                   PrivacyJournalStage::Prepared));
        bool producerSawSafeFd = false;
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicReplacementStageResult result =
            engine.stageReplacement(
                fixture.stageRequest(),
                [&producerSawSafeFd](int descriptor, QString*)
                {
                    struct stat facts = {};
                    producerSawSafeFd = ((::fstat(descriptor, &facts) == 0) &&
                                         S_ISREG(facts.st_mode) &&
                                         ((facts.st_mode & 0777) == 0600) &&
                                         (facts.st_nlink == 1));
                    return producerSawSafeFd &&
                           (::write(descriptor, ProxyBytes.constData(), 7) == 7) &&
                           (::write(descriptor, ProxyBytes.constData() + 7,
                                    static_cast<size_t>(ProxyBytes.size() - 7)) ==
                            ProxyBytes.size() - 7);
                });
        QVERIFY2(result.succeeded(), qPrintable(result.detail));
        QVERIFY(producerSawSafeFd);
        QCOMPARE(result.fact.sha256, digest(ProxyBytes));
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                                   true, false, true, 1, false, false,
                                   PrivacyJournalStage::Prepared));
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicReplacementStageResult result =
            engine.stageReplacement(
                fixture.stageRequest(),
                [](int descriptor, QString* detail)
                {
                    ::write(descriptor, ProxyBytes.constData(), 4);
                    *detail = QStringLiteral("synthetic producer failure");
                    return false;
                });
        QCOMPARE(result.error, PrivacyPublicTransitionError::IoFailure);
        QCOMPARE(result.detail, QStringLiteral("synthetic producer failure"));
        QVERIFY(!QFileInfo::exists(fixture.stagedPath));
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                                   true, false, true, 1, false, false,
                                   PrivacyJournalStage::Prepared));
        QCOMPARE(::symlink("photo.jpg",
                           QFile::encodeName(fixture.stagedPath).constData()), 0);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.stageReplacement(fixture.stageRequest(), ProxyBytes).error,
                 PrivacyPublicTransitionError::UnsafePath);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QVERIFY(QFileInfo(fixture.stagedPath).isSymLink());
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                                   true, false, true, 1, false, false,
                                   PrivacyJournalStage::Prepared));
        const QString moved = fixture.root.path() + QStringLiteral("/album/moved");
        const QString outside = fixture.root.path() + QStringLiteral("/outside");
        QVERIFY(QDir().mkdir(outside));
        bool replaced = false;
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicReplacementStageResult result =
            engine.stageReplacement(
                fixture.stageRequest(),
                [&fixture, moved, outside, &replaced](int descriptor, QString*)
                {
                    replaced = QDir().rename(fixture.publicDirectory, moved) &&
                               (::symlink(QFile::encodeName(outside).constData(),
                                          QFile::encodeName(fixture.publicDirectory)
                                              .constData()) == 0);
                    return replaced &&
                           (::write(descriptor, ProxyBytes.constData(),
                                    static_cast<size_t>(ProxyBytes.size())) ==
                            ProxyBytes.size());
                });
        QVERIFY(replaced);
        QCOMPARE(result.error, PrivacyPublicTransitionError::FileFactMismatch);
        QVERIFY(!QFileInfo::exists(outside + QLatin1Char('/') +
                                   QFileInfo(fixture.stagedPath).fileName()));
        QVERIFY(!QFileInfo::exists(moved + QLatin1Char('/') +
                                   QFileInfo(fixture.stagedPath).fileName()));
    }
}

void PrivacyPublicTransitionTest::testRejectJournalStageCasAndFactMismatch()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem, true, false));
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QCOMPARE(result.error, PrivacyPublicTransitionError::InvalidRequest);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyPublicTransitionRequest request = fixture.request();
        request.authoritativeJournalSha256 = digest(QByteArrayLiteral("stale"));
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicTransitionResult result = engine.execute(request);
        QCOMPARE(result.error, PrivacyPublicTransitionError::JournalRejected);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyJournalRecord applying = fixture.record;
        applying.stage = PrivacyJournalStage::Applying;
        fixture.store->setFaultHook([](PrivacyJournalFaultPoint point)
        {
            return (point == PrivacyJournalFaultPoint::AfterIntentFsynced);
        });
        PrivacyJournalError journalError = PrivacyJournalError::None;
        QString detail;
        QVERIFY(!fixture.store->compareAndUpdate(
            applying, fixture.journalHash, nullptr, &journalError, &detail));
        QCOMPARE(journalError, PrivacyJournalError::FaultInjected);
        fixture.store->setFaultHook({});
        QCOMPARE(fixture.store->load(TransactionUuid).disposition,
                 PrivacyJournalLoadDisposition::DurabilityUncertain);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::JournalRejected);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        QVERIFY(replaceBytes(fixture.stagedPath, QByteArrayLiteral("wrong-stage")));
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QCOMPARE(result.error, PrivacyPublicTransitionError::FileFactMismatch);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        QVERIFY(replaceBytes(fixture.publicPath, QByteArrayLiteral("wrong-current")));
        PrivacyPublicTransitionEngine engine;
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QCOMPARE(result.error, PrivacyPublicTransitionError::FileFactMismatch);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    }
}

void PrivacyPublicTransitionTest::testHostileFilesystemEntries()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        const QString alias = fixture.publicDirectory + QStringLiteral("/hardlink");
        QCOMPARE(::link(QFile::encodeName(fixture.stagedPath).constData(),
                       QFile::encodeName(alias).constData()), 0);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::UnsafePath);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        const QString alias = fixture.publicDirectory +
                              QStringLiteral("/known-public-alias.jpg");
        QCOMPARE(::link(QFile::encodeName(fixture.publicPath).constData(),
                       QFile::encodeName(alias).constData()), 0);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::HardlinkReconciliationRequired);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        QVERIFY(QFile::remove(fixture.stagedPath));
        QCOMPARE(::symlink("photo.jpg",
                           QFile::encodeName(fixture.stagedPath).constData()), 0);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::UnsafePath);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        QVERIFY(QFile::remove(fixture.stagedPath));
        QCOMPARE(::mkfifo(QFile::encodeName(fixture.stagedPath).constData(), 0600), 0);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::UnsafePath);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        QVERIFY(QFile::remove(fixture.stagedPath));
        const QByteArray socketPath = QFile::encodeName(fixture.stagedPath);
        QVERIFY(socketPath.size() < 108);
        const int socketFd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        QVERIFY(socketFd >= 0);
        struct sockaddr_un address = {};
        address.sun_family = AF_UNIX;
        std::memcpy(address.sun_path, socketPath.constData(),
                    static_cast<size_t>(socketPath.size() + 1));
        QCOMPARE(::bind(socketFd, reinterpret_cast<const sockaddr*>(&address),
                        sizeof(address)), 0);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::UnsafePath);
        ::close(socketFd);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        QVERIFY(QFile::remove(fixture.stagedPath));
        QVERIFY(QDir().mkdir(fixture.stagedPath));
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::UnsafePath);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        const QString saved = fixture.root.path() + QStringLiteral("/album/saved");
        QVERIFY(QDir().rename(fixture.publicDirectory, saved));
        QCOMPARE(::symlink("/dev/shm",
                           QFile::encodeName(fixture.publicDirectory).constData()), 0);
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::UnsafePath);
    }
}

void PrivacyPublicTransitionTest::testParentReplacementAndNameRace()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        const QString moved = fixture.root.path() + QStringLiteral("/album/moved");
        bool renamed = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, moved, &renamed](
            PrivacyPublicTransitionFaultPoint point)
        {
            if (point == PrivacyPublicTransitionFaultPoint::BeforeNamespaceMutation)
            {
                renamed = QDir().rename(fixture.publicDirectory, moved);
            }

            return false;
        });
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QVERIFY(renamed);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RootIdentityMismatch);
        QCOMPARE(readBytes(moved + QStringLiteral("/photo.jpg")), OriginalBytes);
        QCOMPARE(readBytes(moved + QLatin1Char('/') +
                           PrivacyPublicTransitionEngine::expectedStageFileName(
                               TransactionUuid, 1, 0)), ProxyBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        bool replaced = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, &replaced](
            PrivacyPublicTransitionFaultPoint point)
        {
            if (point == PrivacyPublicTransitionFaultPoint::BeforeNamespaceMutation)
            {
                replaced = QFile::remove(fixture.publicPath) &&
                           writeNew(fixture.publicPath, OriginalBytes);
            }

            return false;
        });
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QVERIFY(replaced);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RootIdentityMismatch);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyPublicTransitionRequest request = fixture.request();
        ++request.rootExpectation.inode;
        PrivacyPublicTransitionEngine engine;
        QCOMPARE(engine.execute(request).error,
                 PrivacyPublicTransitionError::RootIdentityMismatch);
    }
}

void PrivacyPublicTransitionTest::testEveryNormalFaultBoundary()
{
    const QList<PrivacyPublicTransitionFaultPoint> points =
    {
        PrivacyPublicTransitionFaultPoint::AfterRootOpened,
        PrivacyPublicTransitionFaultPoint::AfterJournalValidated,
        PrivacyPublicTransitionFaultPoint::AfterInitialVerification,
        PrivacyPublicTransitionFaultPoint::AfterStagedFsync,
        PrivacyPublicTransitionFaultPoint::AfterApplyingJournal,
        PrivacyPublicTransitionFaultPoint::BeforeNamespaceMutation,
        PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation,
        PrivacyPublicTransitionFaultPoint::AfterParentFsync,
        PrivacyPublicTransitionFaultPoint::AfterInstalledVerification,
        PrivacyPublicTransitionFaultPoint::AfterDisplacedVerification,
        PrivacyPublicTransitionFaultPoint::AfterPublicStateJournal
    };

    for (const PrivacyPublicTransitionFaultPoint faultPoint : points)
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([faultPoint](PrivacyPublicTransitionFaultPoint point)
        {
            return (point == faultPoint);
        });
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QVERIFY(!result.succeeded());
        QVERIFY(QFileInfo::exists(fixture.containerPath));

        const bool afterMutation =
            (static_cast<int>(faultPoint) >=
             static_cast<int>(PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation));

        if (afterMutation)
        {
            QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
            QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        }
        else
        {
            QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
            QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
        }

        const PrivacyJournalLoadResult journal = fixture.store->load(TransactionUuid);
        QVERIFY(journal.authoritative);

        if (faultPoint == PrivacyPublicTransitionFaultPoint::AfterPublicStateJournal)
        {
            QCOMPARE(journal.record.stage,
                     PrivacyJournalStage::PublicStateVerified);
        }
        else if (static_cast<int>(faultPoint) >=
                 static_cast<int>(PrivacyPublicTransitionFaultPoint::AfterApplyingJournal))
        {
            QCOMPARE(journal.record.stage, PrivacyJournalStage::Applying);
        }
        else
        {
            QCOMPARE(journal.record.stage,
                     PrivacyJournalStage::ProtectedCopyVerified);
        }
    }
}

void PrivacyPublicTransitionTest::testApplyingReplayBeforeAndAfterMutation()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyPublicTransitionEngine first;
        first.setFaultHook([](PrivacyPublicTransitionFaultPoint point)
        {
            return (point ==
                    PrivacyPublicTransitionFaultPoint::AfterApplyingJournal);
        });
        QCOMPARE(first.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::FaultInjected);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
        QVERIFY(fixture.refreshJournal());
        QCOMPARE(fixture.record.stage, PrivacyJournalStage::Applying);

        PrivacyPublicTransitionEngine replay;
        const PrivacyPublicTransitionResult resumed =
            replay.execute(fixture.request());
        QVERIFY2(resumed.succeeded(), qPrintable(resumed.detail));
        QVERIFY(resumed.namespaceMutated);
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        QVERIFY(fixture.refreshJournal());
        QCOMPARE(fixture.record.stage,
                 PrivacyJournalStage::PublicStateVerified);

        const PrivacyPublicTransitionResult idempotent =
            replay.execute(fixture.request());
        QVERIFY2(idempotent.succeeded(), qPrintable(idempotent.detail));
        QVERIFY(!idempotent.namespaceMutated);
        QVERIFY(idempotent.installedVerified);
        QVERIFY(idempotent.displacedVerified);
        QCOMPARE(idempotent.finalJournalSha256, fixture.journalHash);
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyPublicTransitionEngine first;
        first.setFaultHook([](PrivacyPublicTransitionFaultPoint point)
        {
            return (point ==
                    PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation);
        });
        QCOMPARE(first.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::DurabilityUncertain);
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        QVERIFY(fixture.refreshJournal());
        QCOMPARE(fixture.record.stage, PrivacyJournalStage::Applying);

        PrivacyPublicTransitionEngine replay;
        const PrivacyPublicTransitionResult resumed =
            replay.execute(fixture.request());
        QVERIFY2(resumed.succeeded(), qPrintable(resumed.detail));
        QVERIFY(!resumed.namespaceMutated);
        QVERIFY(resumed.installedVerified);
        QVERIFY(resumed.displacedVerified);
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::PublicStateVerified);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem,
                                   true, true, true, 1, true, true));
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
        PrivacyPublicTransitionEngine first;
        first.setFaultHook([](PrivacyPublicTransitionFaultPoint point)
        {
            return (point ==
                    PrivacyPublicTransitionFaultPoint::AfterApplyingJournal);
        });
        QCOMPARE(first.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::FaultInjected);
        QVERIFY(fixture.refreshJournal());
        QCOMPARE(fixture.record.stage, PrivacyJournalStage::Applying);

        PrivacyPublicTransitionEngine replay;
        const PrivacyPublicTransitionResult resumed =
            replay.execute(fixture.request());
        QVERIFY2(resumed.succeeded(), qPrintable(resumed.detail));
        QVERIFY(!resumed.namespaceMutated);
        QVERIFY(resumed.installedVerified);
        QVERIFY(resumed.displacedVerified);
        QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
        QCOMPARE(readBytes(fixture.stagedPath), ProxyBytes);
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::PublicStateVerified);
    }
}

void PrivacyPublicTransitionTest::testApplyingReplayRejectsMixedBytes()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyPublicTransitionEngine first;
        first.setFaultHook([](PrivacyPublicTransitionFaultPoint point)
        {
            return (point ==
                    PrivacyPublicTransitionFaultPoint::AfterApplyingJournal);
        });
        QCOMPARE(first.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::FaultInjected);
        QVERIFY(fixture.refreshJournal());
        QVERIFY(replaceBytes(fixture.stagedPath,
                             QByteArrayLiteral("unknown-staged-bytes")));

        PrivacyPublicTransitionEngine replay;
        QCOMPARE(replay.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::FileFactMismatch);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath),
                 QByteArrayLiteral("unknown-staged-bytes"));
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::Applying);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyPublicTransitionEngine first;
        first.setFaultHook([](PrivacyPublicTransitionFaultPoint point)
        {
            return (point ==
                    PrivacyPublicTransitionFaultPoint::AfterNamespaceMutation);
        });
        QCOMPARE(first.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::DurabilityUncertain);
        QVERIFY(fixture.refreshJournal());
        QVERIFY(replaceBytes(fixture.publicPath,
                             QByteArrayLiteral("unknown-public-bytes")));

        PrivacyPublicTransitionEngine replay;
        QCOMPARE(replay.execute(fixture.request()).error,
                 PrivacyPublicTransitionError::FileFactMismatch);
        QCOMPARE(readBytes(fixture.publicPath),
                 QByteArrayLiteral("unknown-public-bytes"));
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::Applying);
    }
}

void PrivacyPublicTransitionTest::testFinalJournalFailureRequiresReconciliation()
{
    TransitionFixture fixture;
    QVERIFY(fixture.initialize());
    bool journalAdvancedExternally = false;
    PrivacyPublicTransitionEngine engine;
    engine.setFaultHook([&fixture, &journalAdvancedExternally](
        PrivacyPublicTransitionFaultPoint point)
    {
        if (point != PrivacyPublicTransitionFaultPoint::AfterDisplacedVerification)
        {
            return false;
        }

        const PrivacyJournalLoadResult loaded = fixture.store->load(TransactionUuid);

        if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
            !loaded.authoritative)
        {
            return false;
        }

        PrivacyJournalRecord reconciliation = loaded.record;
        reconciliation.stage = PrivacyJournalStage::ReconciliationRequired;
        PrivacyJournalError error = PrivacyJournalError::None;
        QString detail;
        journalAdvancedExternally = fixture.store->compareAndUpdate(
            reconciliation, loaded.sha256, nullptr, &error, &detail);
        return false;
    });

    const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
    QVERIFY(journalAdvancedExternally);
    QCOMPARE(result.error, PrivacyPublicTransitionError::ReconciliationRequired);
    QVERIFY(result.namespaceMutated);
    QVERIFY(result.installedVerified);
    QVERIFY(result.displacedVerified);
    QCOMPARE(readBytes(fixture.publicPath), ProxyBytes);
    QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
    QCOMPARE(readBytes(fixture.containerPath), ContainerBytes);
    QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
             PrivacyJournalStage::ReconciliationRequired);
}

void PrivacyPublicTransitionTest::testRollbackSuccessAndFailure()
{
    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        bool tampered = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, &tampered](
            PrivacyPublicTransitionFaultPoint point)
        {
            if (point == PrivacyPublicTransitionFaultPoint::AfterParentFsync)
            {
                tampered = replaceBytes(fixture.publicPath,
                                        QByteArrayLiteral("tampered-installed"));
            }

            return false;
        });
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QVERIFY(tampered);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RollbackSucceeded);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), QByteArrayLiteral("tampered-installed"));
        QCOMPARE(readBytes(fixture.containerPath), ContainerBytes);
        QVERIFY(!result.namespaceMutated);
        QCOMPARE(fixture.store->load(TransactionUuid).record.stage,
                 PrivacyJournalStage::Applying);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        bool tampered = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, &tampered](
            PrivacyPublicTransitionFaultPoint point)
        {
            if (point == PrivacyPublicTransitionFaultPoint::AfterParentFsync)
            {
                tampered = replaceBytes(fixture.publicPath,
                                        QByteArrayLiteral("tampered-installed"));
                return false;
            }

            return (point == PrivacyPublicTransitionFaultPoint::BeforeRollback);
        });
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QVERIFY(tampered);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RollbackUncertain);
        QCOMPARE(readBytes(fixture.publicPath), QByteArrayLiteral("tampered-installed"));
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.containerPath), ContainerBytes);
        QVERIFY(result.namespaceMutated);
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        bool tampered = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, &tampered](PrivacyPublicTransitionFaultPoint point)
        {
            if (point == PrivacyPublicTransitionFaultPoint::AfterParentFsync)
            {
                tampered = replaceBytes(fixture.publicPath,
                                        QByteArrayLiteral("tampered-installed"));
                return false;
            }

            return (point == PrivacyPublicTransitionFaultPoint::AfterRollback);
        });
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QVERIFY(tampered);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RollbackUncertain);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), QByteArrayLiteral("tampered-installed"));
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::UnprotectItem));
        bool tampered = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, &tampered](
            PrivacyPublicTransitionFaultPoint point)
        {
            if (point ==
                PrivacyPublicTransitionFaultPoint::AfterInstalledVerification)
            {
                tampered = replaceBytes(
                    fixture.stagedPath, QByteArrayLiteral("tampered-displaced"));
                return false;
            }

            return (point == PrivacyPublicTransitionFaultPoint::AfterRollback);
        });
        PrivacyPublicTransitionRequest request = fixture.request();
        request.installedUnixMode = 0640;
        const PrivacyPublicTransitionResult result = engine.execute(request);
        QVERIFY(tampered);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RollbackUncertain);
        QCOMPARE(readBytes(fixture.stagedPath), OriginalBytes);
        struct stat stageFacts = {};
        QCOMPARE(::stat(QFile::encodeName(fixture.stagedPath).constData(),
                        &stageFacts), 0);
        QCOMPARE(stageFacts.st_mode & 0777, mode_t(0600));
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize());
        bool tampered = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, &tampered](PrivacyPublicTransitionFaultPoint point)
        {
            if (point == PrivacyPublicTransitionFaultPoint::AfterParentFsync)
            {
                tampered = replaceBytes(fixture.publicPath,
                                        QByteArrayLiteral("tampered-installed"));
                return false;
            }

            return (point == PrivacyPublicTransitionFaultPoint::AfterRollbackFsync);
        });
        const PrivacyPublicTransitionResult result = engine.execute(fixture.request());
        QVERIFY(tampered);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RollbackSucceeded);
        QCOMPARE(readBytes(fixture.publicPath), OriginalBytes);
        QCOMPARE(readBytes(fixture.stagedPath), QByteArrayLiteral("tampered-installed"));
    }

    {
        TransitionFixture fixture;
        QVERIFY(fixture.initialize(PrivacyTransactionType::ProtectItem, false));
        bool tampered = false;
        PrivacyPublicTransitionEngine engine;
        engine.setFaultHook([&fixture, &tampered](PrivacyPublicTransitionFaultPoint point)
        {
            if (point == PrivacyPublicTransitionFaultPoint::AfterParentFsync)
            {
                tampered = replaceBytes(fixture.publicPath,
                                        QByteArrayLiteral("tampered-install"));
            }

            return false;
        });
        const PrivacyPublicTransitionResult result = engine.execute(
            fixture.request(PrivacyPublicTransitionMode::InstallAbsent));
        QVERIFY(tampered);
        QCOMPARE(result.error, PrivacyPublicTransitionError::RollbackSucceeded);
        QVERIFY(!QFileInfo::exists(fixture.publicPath));
        QCOMPARE(readBytes(fixture.stagedPath), QByteArrayLiteral("tampered-install"));
        QCOMPARE(readBytes(fixture.containerPath), ContainerBytes);
    }
}

QTEST_GUILESS_MAIN(PrivacyPublicTransitionTest)

#include "privacypublictransition_utest.moc"
