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

#include <algorithm>
#include <cstring>

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

// Unix includes

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Local includes

#include "privacycontracts.h"
#include "privacytransactionjournal.h"

using namespace Digikam;

namespace
{

const QString TransactionUuid = QStringLiteral("11111111-1111-4111-8111-111111111111");
const QString CategoryUuid    = QStringLiteral("22222222-2222-4222-8222-222222222222");
const QString RootUuid        = QStringLiteral("33333333-3333-4333-8333-333333333333");
const QString ItemUuid        = QStringLiteral("44444444-4444-4444-8444-444444444444");
const QString ItemUuid2       = QStringLiteral("55555555-5555-4555-8555-555555555555");
const QString ContainerUuid   = QStringLiteral("66666666-6666-4666-8666-666666666666");
const QString MarkerUuid      = QStringLiteral("77777777-7777-4777-8777-777777777777");

QByteArray digest(const QByteArray& bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}

PrivacyJournalRootExpectation rootExpectation(const QString& path)
{
    struct stat status = {};
    const QByteArray encoded = QFile::encodeName(path);
    Q_ASSERT(::stat(encoded.constData(), &status) == 0);

    PrivacyJournalRootExpectation expectation;
    expectation.rootUuid       = RootUuid;
    expectation.identitySha256 = digest(QByteArrayLiteral("root-identity-v1"));
    expectation.device         = static_cast<quint64>(status.st_dev);
    expectation.inode          = static_cast<quint64>(status.st_ino);

    return expectation;
}

PrivacyJournalObjectFact presentFact(const QByteArray& bytes)
{
    PrivacyJournalObjectFact fact;
    fact.presence = PrivacyJournalExpectedPresence::Present;
    fact.size     = bytes.size();
    fact.linkCount = 1;
    fact.sha256   = digest(bytes);

    return fact;
}

PrivacyJournalRecord makeRecord(const PrivacyTransactionJournalStore& store,
                                const PrivacyJournalRootExpectation& expectation,
                                PrivacyJournalStage stage = PrivacyJournalStage::Created)
{
    PrivacyJournalAsset asset;
    asset.itemUuid              = ItemUuid;
    asset.containerUuid         = ContainerUuid;
    asset.role                  = 1;
    asset.ordinal               = 0;
    asset.publicRelativePath    = QStringLiteral("album/photo.jpg");
    asset.stagedRelativePath    = QStringLiteral("album/.photo.jpg.private-stage");
    asset.protectedRelativePath = QStringLiteral("assets/1/0/photo.jpg");
    asset.containerRelativePath = QStringLiteral("album/photo.jpg.digikam-private.zip");
    asset.original              = presentFact(QByteArrayLiteral("original"));
    asset.proxy                 = presentFact(QByteArrayLiteral("proxy"));
    asset.container             = presentFact(QByteArrayLiteral("container"));

    PrivacyJournalRecord record;
    record.transactionUuid          = TransactionUuid;
    record.categoryUuid             = CategoryUuid;
    record.rootUuid                 = RootUuid;
    record.rootDevice               = store.rootDevice();
    record.rootInode                = store.rootInode();
    record.rootIdentitySha256       = expectation.identitySha256;
    record.transactionType          = PrivacyTransactionType::ProtectItem;
    record.generation               = 12;
    record.credentialGeneration     = 4;
    record.fromCredentialGeneration = 4;
    record.toCredentialGeneration   = 4;
    record.stage                    = stage;
    record.assets.append(asset);

    return record;
}

std::unique_ptr<PrivacyTransactionJournalStore> openStore(
    const QString& path, PrivacyJournalRootExpectation* const expectation)
{
    *expectation = rootExpectation(path);
    PrivacyJournalError error = PrivacyJournalError::None;
    QString detail;
    auto store = PrivacyTransactionJournalStore::open(path, *expectation,
                                                       &error, &detail);
    Q_ASSERT_X(store, "openStore", qPrintable(detail));
    return store;
}

QString transactionDirectory(const QString& root)
{
    return root + QStringLiteral("/.digikam-private/transactions/") + TransactionUuid;
}

bool writeNewFile(const QString& path, const QByteArray& bytes, QFileDevice::Permissions mode)
{
    QFile file(path);

    return (file.open(QIODevice::WriteOnly | QIODevice::NewOnly) &&
            (file.write(bytes) == bytes.size()) && file.flush() &&
            file.setPermissions(mode));
}

} // namespace

class PrivacyTransactionJournalTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCanonicalCodecAndSecretExclusion();
    void testCreateCategoryHeaderOnlyCodec();
    void testCodecRejectsMalformedAndCollidingRecords();
    void testCreateAndEveryMonotonicStage();
    void testCompareAndUpdateGuards();
    void testCrashPointsPreserveVerifiedJournal();
    void testInterruptedCreateOffersOnlyRecoveryCandidate();
    void testIntentAndCandidateClassification();
    void testCorruptionAndFilesystemAttacks();
    void testRootExpectationInspectionIsReadOnly();
    void testManagedRootMarkerIdentity();
};

void PrivacyTransactionJournalTest::testCanonicalCodecAndSecretExclusion()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    PrivacyJournalRootExpectation expectation;
    auto store = openStore(root.path(), &expectation);
    PrivacyJournalRecord record = makeRecord(*store, expectation);

    PrivacyJournalAsset second = record.assets.first();
    second.itemUuid             = ItemUuid2;
    second.role                 = 2;
    second.publicRelativePath   = QStringLiteral("album/photo.xmp");
    second.stagedRelativePath   = QStringLiteral("album/.photo.xmp.private-stage");
    second.protectedRelativePath = QStringLiteral("assets/2/0/photo.xmp");
    record.assets.prepend(second);

    PrivacyJournalError error = PrivacyJournalError::None;
    QString detail;
    const QByteArray encoded = PrivacyTransactionJournalCodec::encode(
        record, &error, &detail);
    QVERIFY2(!encoded.isEmpty(), qPrintable(detail));
    QCOMPARE(error, PrivacyJournalError::None);
    QVERIFY(!encoded.contains("correct horse battery staple"));
    QVERIFY(!encoded.contains(root.path().toUtf8()));
    QVERIFY(!encoded.contains("password"));

    PrivacyJournalRecord reversed = record;
    std::reverse(reversed.assets.begin(), reversed.assets.end());
    QCOMPARE(PrivacyTransactionJournalCodec::encode(reversed), encoded);

    PrivacyJournalRecord decoded;
    QVERIFY2(PrivacyTransactionJournalCodec::decode(encoded, &decoded,
                                                     &error, &detail),
             qPrintable(detail));
    QCOMPARE(decoded.assets.size(), 2);
    QCOMPARE(decoded.assets.first().itemUuid, ItemUuid);
    QCOMPARE(PrivacyTransactionJournalCodec::relativeJournalPath(TransactionUuid),
             QStringLiteral(".digikam-private/transactions/%1/journal-v1.json")
                 .arg(TransactionUuid));

    QByteArray nonCanonical = encoded + '\n';
    QVERIFY(!PrivacyTransactionJournalCodec::decode(nonCanonical, &decoded,
                                                     &error, &detail));
    QCOMPARE(error, PrivacyJournalError::CorruptJournal);
}

void PrivacyTransactionJournalTest::testCreateCategoryHeaderOnlyCodec()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    PrivacyJournalRootExpectation expectation;
    auto store = openStore(root.path(), &expectation);
    PrivacyJournalRecord record = makeRecord(*store, expectation);
    record.transactionType = PrivacyTransactionType::CreateCategory;
    record.generation = 0;
    record.credentialGeneration = -1;
    record.fromCredentialGeneration = -1;
    record.toCredentialGeneration = -1;
    record.assets.clear();

    QVERIFY(PrivacyTransactionJournalCodec::validate(record));
    QVERIFY(!PrivacyTransactionJournalCodec::encode(record).isEmpty());

    record.generation = 1;
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record));
    record.generation = 0;
    record.assets.append(makeRecord(*store, expectation).assets.first());
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record));
    record.assets.clear();
    record.stage = PrivacyJournalStage::Complete;
    record.generation = 1;
    record.credentialGeneration = 1;
    record.toCredentialGeneration = 1;
    QVERIFY(PrivacyTransactionJournalCodec::validate(record));
    record.toCredentialGeneration = -1;
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record));
}

void PrivacyTransactionJournalTest::testCodecRejectsMalformedAndCollidingRecords()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    PrivacyJournalRootExpectation expectation;
    auto store = openStore(root.path(), &expectation);
    PrivacyJournalRecord record = makeRecord(*store, expectation);
    QString detail;

    record.assets.first().publicRelativePath = QStringLiteral("../escape.jpg");
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record, &detail));

    record = makeRecord(*store, expectation);
    record.assets.first().publicRelativePath = root.path() + QStringLiteral("/secret.jpg");
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record, &detail));

    record = makeRecord(*store, expectation);
    PrivacyJournalAsset collision = record.assets.first();
    collision.itemUuid = ItemUuid2;
    collision.publicRelativePath = QStringLiteral("ALBUM/PHOTO.JPG");
    record.assets.append(collision);
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record, &detail));

    record = makeRecord(*store, expectation);
    collision = record.assets.first();
    collision.itemUuid = ItemUuid2;
    collision.publicRelativePath = QStringLiteral("album/second.jpg");
    collision.stagedRelativePath = QStringLiteral("ALBUM/PHOTO.JPG");
    record.assets.append(collision);
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record, &detail));

    record = makeRecord(*store, expectation);
    record.assets.first().original.sha256.clear();
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record, &detail));

    record = makeRecord(*store, expectation);
    record.transactionType = static_cast<PrivacyTransactionType>(6);
    QVERIFY(!PrivacyTransactionJournalCodec::validate(record, &detail));

    record = makeRecord(*store, expectation);
    const QByteArray canonical = PrivacyTransactionJournalCodec::encode(record);
    QJsonObject object = QJsonDocument::fromJson(canonical).object();
    object.insert(QStringLiteral("transactionType"), 6);
    PrivacyJournalRecord decoded;
    PrivacyJournalError error = PrivacyJournalError::None;
    QVERIFY(!PrivacyTransactionJournalCodec::decode(
        QJsonDocument(object).toJson(QJsonDocument::Compact), &decoded,
        &error, &detail));
    QCOMPARE(error, PrivacyJournalError::CorruptJournal);

    object = QJsonDocument::fromJson(canonical).object();
    object.insert(QStringLiteral("opaquePayload"), QStringLiteral("secret"));
    QVERIFY(!PrivacyTransactionJournalCodec::decode(
        QJsonDocument(object).toJson(QJsonDocument::Compact), &decoded,
        &error, &detail));
    QCOMPARE(error, PrivacyJournalError::CorruptJournal);
    QVERIFY(!PrivacyTransactionJournalCodec::decode(canonical.first(31),
                                                     &decoded, &error, &detail));
}

void PrivacyTransactionJournalTest::testCreateAndEveryMonotonicStage()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    PrivacyJournalRootExpectation expectation;
    auto store = openStore(root.path(), &expectation);
    PrivacyJournalRecord record = makeRecord(*store, expectation);
    PrivacyJournalError error = PrivacyJournalError::None;
    QString detail;
    QByteArray currentHash;

    QVERIFY2(store->create(record, &currentHash, &error, &detail), qPrintable(detail));
    QVERIFY(!currentHash.isEmpty());

    for (int stage = static_cast<int>(PrivacyJournalStage::Prepared) ;
         stage <= static_cast<int>(PrivacyJournalStage::Complete) ; ++stage)
    {
        record.stage = static_cast<PrivacyJournalStage>(stage);
        QByteArray nextHash;
        QVERIFY2(store->compareAndUpdate(record, currentHash, &nextHash,
                                         &error, &detail), qPrintable(detail));
        QVERIFY(nextHash != currentHash);
        currentHash = nextHash;

        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::Loaded);
        QVERIFY(loaded.authoritative);
        QCOMPARE(loaded.record.stage, record.stage);
        QCOMPARE(loaded.sha256, currentHash);
    }

    const QFileInfo journal(transactionDirectory(root.path()) +
                            QStringLiteral("/journal-v1.json"));
    QVERIFY(journal.isFile());
    QCOMPARE(journal.permissions() & (QFileDevice::ReadGroup |
                                      QFileDevice::WriteGroup |
                                      QFileDevice::ExeGroup |
                                      QFileDevice::ReadOther |
                                      QFileDevice::WriteOther |
                                      QFileDevice::ExeOther),
             QFileDevice::Permissions());
}

void PrivacyTransactionJournalTest::testCompareAndUpdateGuards()
{
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        PrivacyJournalError error = PrivacyJournalError::None;
        QString detail;
        QByteArray originalHash;
        QVERIFY(store->create(record, &originalHash, &error, &detail));

        record.stage = PrivacyJournalStage::Prepared;
        QVERIFY(!store->compareAndUpdate(record, digest(QByteArrayLiteral("stale")),
                                         nullptr, &error, &detail));
        QCOMPARE(error, PrivacyJournalError::StaleComparison);
        QCOMPARE(store->load(TransactionUuid).sha256, originalHash);

        record.assets.first().publicRelativePath = QStringLiteral("album/renamed.jpg");
        QVERIFY(!store->compareAndUpdate(record, originalHash, nullptr, &error, &detail));
        QCOMPARE(error, PrivacyJournalError::IdentityMismatch);

        record = makeRecord(*store, expectation, PrivacyJournalStage::Prepared);
        QByteArray preparedHash;
        QVERIFY(store->compareAndUpdate(record, originalHash, &preparedHash,
                                        &error, &detail));
        record.stage = PrivacyJournalStage::Created;
        QVERIFY(!store->compareAndUpdate(record, preparedHash, nullptr, &error, &detail));
        QCOMPARE(error, PrivacyJournalError::StageRegression);
        QCOMPARE(store->load(TransactionUuid).sha256, preparedHash);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord current = makeRecord(*store, expectation);
        PrivacyJournalError error = PrivacyJournalError::None;
        QString detail;
        QByteArray currentHash;
        QVERIFY(store->create(current, &currentHash, &error, &detail));

        PrivacyJournalRecord externallyChanged = current;
        externallyChanged.assets.first().proxy = presentFact(
            QByteArrayLiteral("external-race"));
        const QByteArray externallyChangedBytes =
            PrivacyTransactionJournalCodec::encode(externallyChanged);
        const QString currentPath = transactionDirectory(root.path()) +
                                    QStringLiteral("/journal-v1.json");
        bool changeSucceeded = false;
        store->setFaultHook([currentPath, externallyChangedBytes, &changeSucceeded](
            PrivacyJournalFaultPoint point)
        {
            if (point != PrivacyJournalFaultPoint::AfterIntentFsynced)
            {
                return false;
            }

            QFile file(currentPath);
            changeSucceeded = file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
                              (file.write(externallyChangedBytes) ==
                               externallyChangedBytes.size()) && file.flush();
            return false;
        });

        PrivacyJournalRecord next = current;
        next.stage = PrivacyJournalStage::Prepared;
        QVERIFY(!store->compareAndUpdate(next, currentHash, nullptr, &error, &detail));
        QVERIFY(changeSucceeded);
        QCOMPARE(error, PrivacyJournalError::StaleComparison);
        store->setFaultHook({});

        const PrivacyJournalLoadResult recovered = store->load(TransactionUuid);
        QCOMPARE(recovered.disposition,
                 PrivacyJournalLoadDisposition::DurabilityUncertain);
        QVERIFY(recovered.hasRecord);
        QVERIFY(!recovered.authoritative);
        QVERIFY(recovered.matchesCommitIntent);
        QCOMPARE(recovered.record.stage, PrivacyJournalStage::Prepared);
    }
}

void PrivacyTransactionJournalTest::testCrashPointsPreserveVerifiedJournal()
{
    const QList<PrivacyJournalFaultPoint> points =
    {
        PrivacyJournalFaultPoint::AfterDirectoriesFsynced,
        PrivacyJournalFaultPoint::AfterNextCreated,
        PrivacyJournalFaultPoint::AfterNextWritten,
        PrivacyJournalFaultPoint::AfterNextFsynced,
        PrivacyJournalFaultPoint::AfterNextVerified,
        PrivacyJournalFaultPoint::AfterIntentFsynced,
        PrivacyJournalFaultPoint::AfterPublishRename,
        PrivacyJournalFaultPoint::AfterPublishDirectoryFsync,
        PrivacyJournalFaultPoint::AfterPublishedReadback,
        PrivacyJournalFaultPoint::AfterPreviousRemoved,
        PrivacyJournalFaultPoint::AfterIntentRemoved,
        PrivacyJournalFaultPoint::AfterCleanupDirectoryFsync
    };

    for (const PrivacyJournalFaultPoint point : points)
    {
        QTemporaryDir root(QStringLiteral("/tmp/pj-XXXXXX"));
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        PrivacyJournalError error = PrivacyJournalError::None;
        QString detail;
        QByteArray oldHash;
        QVERIFY2(store->create(record, &oldHash, &error, &detail), qPrintable(detail));

        record.stage = PrivacyJournalStage::Prepared;
        store->setFaultHook([point](PrivacyJournalFaultPoint current)
        {
            return (current == point);
        });
        QVERIFY(!store->compareAndUpdate(record, oldHash, nullptr, &error, &detail));
        QCOMPARE(error, PrivacyJournalError::FaultInjected);
        store->setFaultHook({});

        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);
        QVERIFY2((loaded.disposition == PrivacyJournalLoadDisposition::Loaded) ||
                 (loaded.disposition == PrivacyJournalLoadDisposition::DurabilityUncertain),
                 qPrintable(QStringLiteral("point %1: %2")
                                .arg(static_cast<int>(point)).arg(loaded.detail)));
        QVERIFY(loaded.hasRecord);
        QCOMPARE(loaded.authoritative,
                 loaded.disposition == PrivacyJournalLoadDisposition::Loaded);

        if (static_cast<int>(point) <=
            static_cast<int>(PrivacyJournalFaultPoint::AfterNextVerified))
        {
            QCOMPARE(loaded.record.stage, PrivacyJournalStage::Created);
            QCOMPARE(loaded.sha256, oldHash);
        }
        else
        {
            QCOMPARE(loaded.record.stage, PrivacyJournalStage::Prepared);
        }
    }
}

void PrivacyTransactionJournalTest::testInterruptedCreateOffersOnlyRecoveryCandidate()
{
    const QList<PrivacyJournalFaultPoint> points =
    {
        PrivacyJournalFaultPoint::AfterIntentFsynced,
        PrivacyJournalFaultPoint::AfterPublishRename,
        PrivacyJournalFaultPoint::AfterPublishDirectoryFsync
    };

    for (const PrivacyJournalFaultPoint faultPoint : points)
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        store->setFaultHook([faultPoint](PrivacyJournalFaultPoint point)
        {
            return (point == faultPoint);
        });

        PrivacyJournalError error = PrivacyJournalError::None;
        QString detail;
        QVERIFY(!store->create(record, nullptr, &error, &detail));
        QCOMPARE(error, PrivacyJournalError::FaultInjected);
        store->setFaultHook({});

        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::DurabilityUncertain);
        QVERIFY(loaded.hasRecord);
        QVERIFY(!loaded.authoritative);
        QVERIFY(loaded.matchesCommitIntent);
        QCOMPARE(loaded.record.stage, PrivacyJournalStage::Created);
    }
}

void PrivacyTransactionJournalTest::testIntentAndCandidateClassification()
{
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        QVERIFY(store->create(record));
        const QString directory = transactionDirectory(root.path());
        QVERIFY(writeNewFile(directory + QStringLiteral("/journal-v1.intent"),
                             QByteArray(64, '0') + '\n',
                             QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::DurabilityUncertain);
        QVERIFY(!loaded.hasRecord);
        QVERIFY(!loaded.authoritative);
        QVERIFY(!loaded.matchesCommitIntent);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        QVERIFY(store->create(record));
        QVERIFY(writeNewFile(transactionDirectory(root.path()) +
                                 QStringLiteral("/journal-v1.intent"),
                             QByteArrayLiteral("malformed\n"),
                             QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::Corrupt);
        QVERIFY(!loaded.hasRecord);
        QVERIFY(!loaded.authoritative);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        QVERIFY(store->create(record));
        PrivacyJournalRecord foreign = record;
        foreign.transactionUuid = ItemUuid2;
        QVERIFY(writeNewFile(transactionDirectory(root.path()) +
                                 QStringLiteral("/journal-v1.next"),
                             PrivacyTransactionJournalCodec::encode(foreign),
                             QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        QCOMPARE(store->load(TransactionUuid).disposition,
                 PrivacyJournalLoadDisposition::IdentityMismatch);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        QVERIFY(store->create(record));
        QVERIFY(writeNewFile(transactionDirectory(root.path()) +
                                 QStringLiteral("/journal-v1.next"),
                             QByteArrayLiteral("{corrupt"),
                             QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::DurabilityUncertain);
        QVERIFY(loaded.hasRecord);
        QVERIFY(!loaded.authoritative);
        QVERIFY(!loaded.matchesCommitIntent);
        QCOMPARE(loaded.record.stage, PrivacyJournalStage::Created);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord current = makeRecord(*store, expectation);
        QVERIFY(store->create(current));
        PrivacyJournalRecord next = current;
        next.stage = PrivacyJournalStage::Prepared;
        const QByteArray nextBytes = PrivacyTransactionJournalCodec::encode(next);
        const QByteArray nextHash = PrivacyTransactionJournalCodec::sha256(nextBytes);
        const QString directory = transactionDirectory(root.path());
        QVERIFY(writeNewFile(directory + QStringLiteral("/journal-v1.next"),
                             nextBytes,
                             QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        QVERIFY(writeNewFile(directory + QStringLiteral("/journal-v1.intent"),
                             nextHash.toHex() + '\n',
                             QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        QFile currentFile(directory + QStringLiteral("/journal-v1.json"));
        QVERIFY(currentFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(currentFile.write("{"), 1);
        QVERIFY(currentFile.flush());
        currentFile.close();
        const PrivacyJournalLoadResult loaded = store->load(TransactionUuid);
        QCOMPARE(loaded.disposition, PrivacyJournalLoadDisposition::DurabilityUncertain);
        QVERIFY(loaded.hasRecord);
        QVERIFY(!loaded.authoritative);
        QVERIFY(loaded.matchesCommitIntent);
        QCOMPARE(loaded.record.stage, PrivacyJournalStage::Prepared);
    }
}

void PrivacyTransactionJournalTest::testCorruptionAndFilesystemAttacks()
{
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        QVERIFY(store->create(record));
        QFile file(transactionDirectory(root.path()) + QStringLiteral("/journal-v1.json"));
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(file.write("{\"truncated\":"), 13);
        QVERIFY(file.flush());
        file.close();
        QCOMPARE(store->load(TransactionUuid).disposition,
                 PrivacyJournalLoadDisposition::Corrupt);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        QVERIFY(store->create(record));
        const QByteArray journal = QFile::encodeName(
            transactionDirectory(root.path()) + QStringLiteral("/journal-v1.json"));
        const QByteArray second = QFile::encodeName(
            transactionDirectory(root.path()) + QStringLiteral("/hardlink"));
        QCOMPARE(::link(journal.constData(), second.constData()), 0);
        QCOMPARE(store->load(TransactionUuid).disposition,
                 PrivacyJournalLoadDisposition::UnsafeStorage);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation = rootExpectation(root.path());
        auto store = PrivacyTransactionJournalStore::open(root.path(), expectation);
        QVERIFY(store);
        const QString base = root.path() + QStringLiteral("/.digikam-private/transactions");
        QVERIFY(QDir().mkpath(base));
        QCOMPARE(::chmod(QFile::encodeName(root.path() + QStringLiteral("/.digikam-private")).constData(),
                         0700), 0);
        QCOMPARE(::chmod(QFile::encodeName(base).constData(), 0700), 0);
        const QByteArray linkName = QFile::encodeName(base + QLatin1Char('/') + TransactionUuid);
        QCOMPARE(::symlink("/dev/shm", linkName.constData()), 0);
        QCOMPARE(store->load(TransactionUuid).disposition,
                 PrivacyJournalLoadDisposition::UnsafeStorage);
    }

    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        store->setFaultHook([](PrivacyJournalFaultPoint point)
        {
            return (point == PrivacyJournalFaultPoint::AfterDirectoriesFsynced);
        });
        QVERIFY(!store->create(record));
        store->setFaultHook({});
        const QByteArray fifo = QFile::encodeName(
            transactionDirectory(root.path()) + QStringLiteral("/journal-v1.json"));
        QCOMPARE(::mkfifo(fifo.constData(), 0600), 0);
        QCOMPARE(store->load(TransactionUuid).disposition,
                 PrivacyJournalLoadDisposition::UnsafeStorage);
    }

    {
        QTemporaryDir root(QStringLiteral("/tmp/pj-socket-XXXXXX"));
        QVERIFY(root.isValid());
        PrivacyJournalRootExpectation expectation;
        auto store = openStore(root.path(), &expectation);
        PrivacyJournalRecord record = makeRecord(*store, expectation);
        store->setFaultHook([](PrivacyJournalFaultPoint point)
        {
            return (point == PrivacyJournalFaultPoint::AfterDirectoriesFsynced);
        });
        QVERIFY(!store->create(record));
        store->setFaultHook({});

        const QByteArray socketPath = QFile::encodeName(
            transactionDirectory(root.path()) + QStringLiteral("/journal-v1.next"));
        struct sockaddr_un address = {};
        QVERIFY(socketPath.size() < static_cast<qsizetype>(sizeof(address.sun_path)));
        const int socketFd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        QVERIFY(socketFd >= 0);
        address.sun_family = AF_UNIX;
        std::memcpy(address.sun_path, socketPath.constData(),
                    static_cast<size_t>(socketPath.size() + 1));
        QCOMPARE(::bind(socketFd, reinterpret_cast<const sockaddr*>(&address),
                        sizeof(address)), 0);
        QCOMPARE(store->load(TransactionUuid).disposition,
                 PrivacyJournalLoadDisposition::UnsafeStorage);
        PrivacyJournalError error = PrivacyJournalError::None;
        QString detail;
        QVERIFY(!store->create(record, nullptr, &error, &detail));
        ::close(socketFd);
    }
}

void PrivacyTransactionJournalTest::testRootExpectationInspectionIsReadOnly()
{
    QTemporaryDir albumDirectory;
    QVERIFY(albumDirectory.isValid());
    PrivacyStorageRoot albumRoot;
    albumRoot.uuid = RootUuid;
    albumRoot.kind = PrivacyStorageRootKind::AlbumRoot;
    albumRoot.albumRootId = 42;
    albumRoot.configuredPath = albumDirectory.path();
    albumRoot.identityVersion = 1;
    albumRoot.identityData = PrivacyRootIdentityCodec::encodeAlbumRootV1(
        42, QStringLiteral("synthetic-album-root"));
    albumRoot.schemaVersion = 1;
    albumRoot.createdAt = QDateTime::currentDateTimeUtc();
    QVERIFY(albumRoot.isValid());
    PrivacyJournalRootExpectation albumExpectation;
    PrivacyJournalError error = PrivacyJournalError::None;
    QString detail;
    QVERIFY(PrivacyTransactionJournalStore::inspectRootExpectation(
        albumRoot, &albumExpectation, &error, &detail));
    QVERIFY(albumExpectation.device != 0);
    QVERIFY(albumExpectation.inode != 0);
    QVERIFY(!QFileInfo::exists(albumDirectory.path() +
                               QStringLiteral("/.digikam-private")));

    QTemporaryDir root;
    QVERIFY(root.isValid());

    PrivacyStorageRoot managedRoot;
    managedRoot.uuid = RootUuid;
    managedRoot.kind = PrivacyStorageRootKind::ManagedStoreRoot;
    managedRoot.configuredPath = root.path();
    managedRoot.identityVersion = 1;
    managedRoot.identityData = PrivacyRootIdentityCodec::encodeManagedRootV1(
        MarkerUuid);
    managedRoot.markerUuid = MarkerUuid;
    managedRoot.schemaVersion = 1;
    managedRoot.createdAt = QDateTime::currentDateTimeUtc();
    QVERIFY(managedRoot.isValid());

    PrivacyJournalRootExpectation inspected;
    QVERIFY(!PrivacyTransactionJournalStore::inspectRootExpectation(
        managedRoot, &inspected, &error, &detail));
    QCOMPARE(error, PrivacyJournalError::RootIdentityMismatch);
    QVERIFY(!QFileInfo::exists(root.path() + QStringLiteral("/.digikam-private")));

    const QString metadata = root.path() + QStringLiteral("/.digikam-private");
    QVERIFY(QDir().mkpath(metadata));
    QCOMPARE(::chmod(QFile::encodeName(metadata).constData(), 0700), 0);
    const QByteArray markerBytes = PrivacyRootIdentityCodec::encodeManagedRootMarkerV1(
        RootUuid, MarkerUuid);
    QVERIFY(writeNewFile(metadata + QStringLiteral("/root-marker-v1.json"),
                         markerBytes,
                         QFileDevice::ReadOwner | QFileDevice::WriteOwner));

    const QStringList before = QDir(metadata).entryList(
        QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);
    QVERIFY(PrivacyTransactionJournalStore::inspectRootExpectation(
        managedRoot, &inspected, &error, &detail));
    QCOMPARE(inspected.rootUuid, RootUuid);
    QCOMPARE(inspected.markerUuid, MarkerUuid);
    QCOMPARE(inspected.identitySha256, digest(managedRoot.identityData));
    QVERIFY(inspected.device != 0);
    QVERIFY(inspected.inode != 0);
    QCOMPARE(QDir(metadata).entryList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                      QDir::Name),
             before);
    QVERIFY(!QFileInfo::exists(metadata + QStringLiteral("/transactions")));

    PrivacyJournalRootExpectation repeated;
    QVERIFY(PrivacyTransactionJournalStore::inspectRootExpectation(
        managedRoot, &repeated, &error, &detail));
    QCOMPARE(repeated.device, inspected.device);
    QCOMPARE(repeated.inode, inspected.inode);
    QCOMPARE(repeated.identitySha256, inspected.identitySha256);

    managedRoot.markerUuid = ItemUuid;
    managedRoot.identityData = PrivacyRootIdentityCodec::encodeManagedRootV1(ItemUuid);
    QVERIFY(!PrivacyTransactionJournalStore::inspectRootExpectation(
        managedRoot, &repeated, &error, &detail));
    QCOMPARE(error, PrivacyJournalError::RootIdentityMismatch);
    QCOMPARE(QDir(metadata).entryList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                      QDir::Name),
             before);
}

void PrivacyTransactionJournalTest::testManagedRootMarkerIdentity()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const QString metadata = root.path() + QStringLiteral("/.digikam-private");
    QVERIFY(QDir().mkpath(metadata));
    QCOMPARE(::chmod(QFile::encodeName(metadata).constData(), 0700), 0);

    QJsonObject marker;
    marker.insert(QStringLiteral("kind"),
                  QStringLiteral("digikam-private-root-marker-v1"));
    marker.insert(QStringLiteral("markerUuid"), MarkerUuid);
    marker.insert(QStringLiteral("rootUuid"), RootUuid);
    const QByteArray markerBytes = QJsonDocument(marker).toJson(QJsonDocument::Compact);
    const QFileDevice::Permissions ownerReadWrite = QFileDevice::ReadOwner |
                                                    QFileDevice::WriteOwner;
    QVERIFY(writeNewFile(metadata + QStringLiteral("/root-marker-v1.json"),
                         markerBytes, ownerReadWrite));

    PrivacyJournalRootExpectation expectation = rootExpectation(root.path());
    expectation.markerUuid = MarkerUuid;
    PrivacyJournalError error = PrivacyJournalError::None;
    QString detail;
    auto store = PrivacyTransactionJournalStore::open(root.path(), expectation,
                                                       &error, &detail);
    QVERIFY2(store, qPrintable(detail));

    marker.insert(QStringLiteral("markerUuid"), TransactionUuid);
    QFile markerFile(metadata + QStringLiteral("/root-marker-v1.json"));
    QVERIFY(markerFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    const QByteArray changedMarker = QJsonDocument(marker).toJson(QJsonDocument::Compact);
    QCOMPARE(markerFile.write(changedMarker), changedMarker.size());
    QVERIFY(markerFile.flush());
    markerFile.close();
    QCOMPARE(store->load(TransactionUuid).disposition,
             PrivacyJournalLoadDisposition::IdentityMismatch);

    expectation.markerUuid = ItemUuid;
    store = PrivacyTransactionJournalStore::open(root.path(), expectation,
                                                  &error, &detail);
    QVERIFY(!store);
    QCOMPARE(error, PrivacyJournalError::RootIdentityMismatch);
}

QTEST_GUILESS_MAIN(PrivacyTransactionJournalTest)

#include "privacytransactionjournal_utest.moc"
