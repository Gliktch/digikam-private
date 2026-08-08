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
                    quint64 currentLinkCount = 1)
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

        const bool installedIsProxy =
            ((type == PrivacyTransactionType::ProtectItem) ||
             (type == PrivacyTransactionType::CompatibilityRelock));
        const QByteArray stagedBytes = installedIsProxy ? ProxyBytes : OriginalBytes;
        const QByteArray currentBytes = installedIsProxy ? OriginalBytes : ProxyBytes;

        if (!writeNew(stagedPath, stagedBytes) ||
            (adjacentContainer && !writeNew(containerPath, ContainerBytes)) ||
            (publicPresent && !writeNew(publicPath, currentBytes, 0640)))
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
        record.stage                    = PrivacyJournalStage::Staged;
        record.assets                   = { asset };

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
            ((record.transactionType == PrivacyTransactionType::ProtectItem) ||
             (record.transactionType == PrivacyTransactionType::CompatibilityRelock));
        request.installedFact = installedIsProxy
                              ? PrivacyPublicTransitionFactKind::Proxy
                              : PrivacyPublicTransitionFactKind::Original;
        request.currentFact   = installedIsProxy
                              ? PrivacyPublicTransitionFactKind::Original
                              : PrivacyPublicTransitionFactKind::Proxy;
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
    void testUnprotectDirection();
    void testCrossRootProtectedProof();
    void testAcknowledgedProtectHardlinks();
    void testRejectJournalStageCasAndFactMismatch();
    void testHostileFilesystemEntries();
    void testParentReplacementAndNameRace();
    void testEveryNormalFaultBoundary();
    void testFinalJournalFailureRequiresReconciliation();
    void testRollbackSuccessAndFailure();
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
        QVERIFY(fixture.initialize(PrivacyTransactionType::CompatibilityRelock));
        const QString alias = fixture.publicDirectory +
                              QStringLiteral("/external-hardlink.jpg");
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
