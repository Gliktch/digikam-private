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
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

// Local includes

#include "privacystrongpasswordrewrap.h"

using namespace Digikam;

class FakeStoreBackend final : public PrivacyCategoryStoreBackend
{
public:

    bool createOrResume(const PrivacyStorageRoot&, const PrivacyStore&,
                        const QString&, const PrivacyPassword&,
                        const QByteArray&, PrivacyGocryptfsEnvelope*,
                        PrivacyGocryptfsError*) override
    {
        return true;
    }

    bool validateEnvelope(const PrivacyGocryptfsEnvelope&,
                          const PrivacyPassword&,
                          PrivacyGocryptfsError*) override
    {
        return true;
    }

    std::unique_ptr<PrivacyCategoryStoreLease> unlock(
        const PrivacyStorageRoot&, const PrivacyStore&,
        const PrivacyGocryptfsEnvelope&, const PrivacyPassword&,
        const QByteArray&, PrivacyGocryptfsError*) override
    {
        return {};
    }

    bool lock(std::unique_ptr<PrivacyCategoryStoreLease>&,
              PrivacyGocryptfsError*) override
    {
        return true;
    }

    bool rewrapPassword(const PrivacyStorageRoot&, const PrivacyStore&,
                        const PrivacyGocryptfsEnvelope&,
                        const PrivacyPassword&, const PrivacyPassword&,
                        const QByteArray&, QByteArray* const newOpaqueConfig,
                        PrivacyGocryptfsError* error) override
    {
        ++rewrapCalls;

        if (failRewrap)
        {
            if (error)
            {
                *error = PrivacyGocryptfsError::ProcessFailed;
            }

            return false;
        }

        if (newOpaqueConfig)
        {
            *newOpaqueConfig = QByteArray("rewrapped-config");
        }

        return true;
    }

    int  rewrapCalls = 0;
    bool failRewrap = false;
};

class FakePersistence final : public PrivacyStrongPasswordRewrapPersistence
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* const output) const override
    {
        *output = snapshot;
        return true;
    }

    bool beginRewrap(const PrivacyTransaction& transaction,
                     const PrivacyTransactionJournal& journal) override
    {
        if (failBegin)
        {
            return false;
        }

        snapshot.transactions << transaction;
        snapshot.transactionJournals << journal;
        return true;
    }

    bool publishRewrap(const QString& categoryUuid,
                       qlonglong categoryGeneration,
                       const PrivacyCredential& credential,
                       const QString& storeUuid,
                       qlonglong storeGeneration,
                       const PrivacyTransaction& transaction,
                       PrivacyTransactionState,
                       qlonglong) override
    {
        if (failPublish)
        {
            return false;
        }

        for (PrivacyCategory& category : snapshot.categories)
        {
            if (category.uuid == categoryUuid)
            {
                category.currentCredentialGeneration = categoryGeneration;
            }
        }

        snapshot.credentials << credential;

        for (PrivacyStore& store : snapshot.stores)
        {
            if (store.uuid == storeUuid)
            {
                store.configGeneration = storeGeneration;
            }
        }

        for (PrivacyTransaction& existing : snapshot.transactions)
        {
            if (existing.uuid == transaction.uuid)
            {
                existing = transaction;
            }
        }

        return true;
    }

    bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) override
    {
        for (PrivacyTransaction& existing : snapshot.transactions)
        {
            if ((existing.uuid == transaction.uuid) &&
                (existing.state == expectedState) &&
                (existing.generation == expectedGeneration))
            {
                existing = transaction;
                return true;
            }
        }

        return false;
    }

    PrivacyRepositorySnapshot snapshot;
    bool failBegin = false;
    bool failPublish = false;
};

namespace
{

const QString CategoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
const QString StoreUuid = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString RootUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QByteArray OldConfig("opaque old config");
const QByteArray NewConfig("rewrapped-config");

QString sha256Hex(const QByteArray& bytes)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

void writeConfig(const QString& managedRoot, const QByteArray& bytes)
{
    const QString path = managedRoot +
        QLatin1String("/.digikam-private/stores/") + StoreUuid +
        QLatin1String("/gocryptfs.conf");
    QVERIFY(QDir().mkpath(QFileInfo(path).absolutePath()));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(file.write(bytes), bytes.size());
}

void seedBundle(FakePersistence* persistence, const QString& managedRoot)
{
    PrivacyCategory category;
    category.uuid = CategoryUuid;
    category.name = QLatin1String("Synthetic strong");
    category.recoverySetUuid =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    category.backend = PrivacyBackend::Strong;
    category.presentationMode = PrivacyPresentationMode::Generic;
    category.unlockedThumbnailMode = PrivacyUnlockedThumbnailMode::FocusedClear;
    category.tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.schemaVersion = 1;
    category.createdAt = QDateTime::currentDateTimeUtc();
    persistence->snapshot.categories << category;

    PrivacyCredential credential;
    credential.categoryUuid = CategoryUuid;
    credential.generation = 1;
    credential.encodingVersion = QLatin1String("utf8-nfc-v1");
    credential.envelopeFormat = QLatin1String("gocryptfs-config-v2");
    credential.envelopeBlob = OldConfig;
    credential.envelopeHashAlgorithm = QLatin1String("sha256");
    credential.envelopeHash = sha256Hex(OldConfig);
    credential.createdAt = QDateTime::currentDateTimeUtc();
    persistence->snapshot.credentials << credential;

    PrivacyStore store;
    store.uuid = StoreUuid;
    store.categoryUuid = CategoryUuid;
    store.rootUuid = RootUuid;
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

    PrivacyStorageRoot root;
    root.uuid = RootUuid;
    root.kind = PrivacyStorageRootKind::ManagedStoreRoot;
    root.albumRootId = -1;
    root.configuredPath = managedRoot;
    root.identityVersion = 1;
    root.identityData = QByteArray("synthetic-managed-root");
    root.markerUuid = QLatin1String("40000000-0000-0000-0000-000000000001");
    root.createdAt = QDateTime::currentDateTimeUtc();
    persistence->snapshot.storageRoots << root;

    writeConfig(managedRoot, OldConfig);
}

PrivacyTransaction makeRewrapTransaction(PrivacyTransactionState state,
                                         qlonglong generation,
                                         const QString& transactionUuid)
{
    PrivacyTransaction transaction;
    transaction.uuid = transactionUuid;
    transaction.categoryUuid = CategoryUuid;
    transaction.type = PrivacyTransactionType::ChangePassword;
    transaction.state = state;
    transaction.generation = generation;
    transaction.fromCredentialGeneration = 1;
    transaction.toCredentialGeneration = 2;
    transaction.payloadFormatVersion = 1;
    transaction.payloadData = QByteArray("{}");
    transaction.createdAt = QDateTime::currentDateTimeUtc();
    transaction.updatedAt = transaction.createdAt;
    return transaction;
}

} // namespace

class PrivacyStrongPasswordRewrapTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRewrapPublishesCredentialAndRemovesBackup();
    void testInterruptedApplyingRecoveryFinalizesWithoutRewrap();
    void testInterruptedCreatedRecoveryRetries();
    void testActiveTransactionBlocksRewrap();
    void testStoreFailureLeavesRecoverableTransaction();
};

void PrivacyStrongPasswordRewrapTest::
    testRewrapPublishesCredentialAndRemovesBackup()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, directory.path());
    PrivacyStrongPasswordRewrapEngine engine(persistence, backend);
    const PrivacyPassword oldPassword =
        PrivacyPassword::fromUnicode(QLatin1String("old-secret"));
    const PrivacyPassword newPassword =
        PrivacyPassword::fromUnicode(QLatin1String("new-secret"));

    const PrivacyStrongPasswordRewrapResult result =
        engine.rewrap(CategoryUuid, oldPassword, newPassword);
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(backend.rewrapCalls, 1);
    QCOMPARE(persistence.snapshot.categories.constFirst()
                 .currentCredentialGeneration, 2);
    QCOMPARE(persistence.snapshot.credentials.size(), 2);
    QCOMPARE(persistence.snapshot.credentials.constLast().generation, 2);
    QCOMPARE(persistence.snapshot.credentials.constLast().envelopeBlob,
             NewConfig);
    QCOMPARE(persistence.snapshot.stores.constFirst().configGeneration, 2);
    QCOMPARE(persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Complete);
    QVERIFY(!QFileInfo::exists(QDir(directory.path()).filePath(
        QLatin1String(".digikam-private/staging/") + StoreUuid +
        QLatin1String(".rewrap-") + result.transactionUuid)));
}

void PrivacyStrongPasswordRewrapTest::
    testInterruptedApplyingRecoveryFinalizesWithoutRewrap()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, directory.path());
    const QString transactionUuid =
        QLatin1String("50000000-0000-0000-0000-000000000001");
    persistence.snapshot.transactions << makeRewrapTransaction(
        PrivacyTransactionState::Applying, 1, transactionUuid);
    const QString backupDirectory = QDir(directory.path()).filePath(
        QLatin1String(".digikam-private/staging/") + StoreUuid +
        QLatin1String(".rewrap-") + transactionUuid);
    QVERIFY(QDir().mkpath(backupDirectory));
    QFile backup(backupDirectory + QLatin1String("/gocryptfs.conf"));
    QVERIFY(backup.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(backup.write(OldConfig), OldConfig.size());
    backup.close();
    writeConfig(directory.path(), NewConfig);

    PrivacyStrongPasswordRewrapEngine engine(persistence, backend);
    const PrivacyStrongPasswordRewrapResult result = engine.recover(
        CategoryUuid,
        PrivacyPassword::fromUnicode(QLatin1String("old-secret")),
        PrivacyPassword::fromUnicode(QLatin1String("new-secret")));
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(backend.rewrapCalls, 0);
    QCOMPARE(persistence.snapshot.categories.constFirst()
                 .currentCredentialGeneration, 2);
    QCOMPARE(persistence.snapshot.credentials.constLast().envelopeBlob,
             NewConfig);
    QVERIFY(!QFileInfo::exists(backupDirectory));
}

void PrivacyStrongPasswordRewrapTest::testInterruptedCreatedRecoveryRetries()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, directory.path());
    const QString transactionUuid =
        QLatin1String("50000000-0000-0000-0000-000000000001");
    persistence.snapshot.transactions << makeRewrapTransaction(
        PrivacyTransactionState::Created, 0, transactionUuid);

    PrivacyStrongPasswordRewrapEngine engine(persistence, backend);
    const PrivacyStrongPasswordRewrapResult result = engine.recover(
        CategoryUuid,
        PrivacyPassword::fromUnicode(QLatin1String("old-secret")),
        PrivacyPassword::fromUnicode(QLatin1String("new-secret")));
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(backend.rewrapCalls, 1);
    QCOMPARE(persistence.snapshot.categories.constFirst()
                 .currentCredentialGeneration, 2);
}

void PrivacyStrongPasswordRewrapTest::testActiveTransactionBlocksRewrap()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, directory.path());
    PrivacyTransaction other = makeRewrapTransaction(
        PrivacyTransactionState::Applying, 1,
        QLatin1String("50000000-0000-0000-0000-000000000099"));
    other.type = PrivacyTransactionType::ProtectItem;
    other.itemUuid = QLatin1String("60000000-0000-0000-0000-000000000001");
    persistence.snapshot.transactions << other;

    PrivacyStrongPasswordRewrapEngine engine(persistence, backend);
    const PrivacyStrongPasswordRewrapResult result = engine.rewrap(
        CategoryUuid,
        PrivacyPassword::fromUnicode(QLatin1String("old-secret")),
        PrivacyPassword::fromUnicode(QLatin1String("new-secret")));
    QCOMPARE(result.status, PrivacyStrongPasswordRewrapStatus::AlreadyActive);
    QCOMPARE(backend.rewrapCalls, 0);
}

void PrivacyStrongPasswordRewrapTest::testStoreFailureLeavesRecoverableTransaction()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, directory.path());
    backend.failRewrap = true;

    PrivacyStrongPasswordRewrapEngine engine(persistence, backend);
    const PrivacyStrongPasswordRewrapResult result = engine.rewrap(
        CategoryUuid,
        PrivacyPassword::fromUnicode(QLatin1String("old-secret")),
        PrivacyPassword::fromUnicode(QLatin1String("new-secret")));
    QCOMPARE(result.status, PrivacyStrongPasswordRewrapStatus::StoreFailure);
    QCOMPARE(persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Applying);

    backend.failRewrap = false;
    const PrivacyStrongPasswordRewrapResult recovered = engine.recover(
        CategoryUuid,
        PrivacyPassword::fromUnicode(QLatin1String("old-secret")),
        PrivacyPassword::fromUnicode(QLatin1String("new-secret")));
    QVERIFY2(recovered.succeeded(), qPrintable(recovered.detail));
    QCOMPARE(persistence.snapshot.categories.constFirst()
                 .currentCredentialGeneration, 2);
}

QTEST_GUILESS_MAIN(PrivacyStrongPasswordRewrapTest)

#include "privacystrongpasswordrewrap_utest.moc"
