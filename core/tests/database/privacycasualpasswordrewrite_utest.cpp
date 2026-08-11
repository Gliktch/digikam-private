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

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "privacycasualpasswordrewrite.h"

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

class FakePersistence final : public PrivacyCasualPasswordRewritePersistence
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

    bool updateContainerCredentialGeneration(
        const QString& containerUuid, qlonglong expectedGeneration,
        qlonglong generation) override
    {
        for (PrivacyContainer& container : snapshot.containers)
        {
            if ((container.uuid == containerUuid) &&
                (container.credentialGeneration == expectedGeneration))
            {
                container.credentialGeneration = generation;
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
const QString ManagedRootUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QString PublicRootUuid = QLatin1String("40000000-0000-0000-0000-000000000001");
const QString ItemUuid = QLatin1String("50000000-0000-0000-0000-000000000001");
const QString ContainerUuid = QLatin1String("60000000-0000-0000-0000-000000000001");
const QByteArray ConfigBytes("opaque casual config");
const QByteArray Payload("casual rewrite payload");

QString sha256Hex(const QByteArray& bytes)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
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

bool archiveOpensWith(const QString& path, const PrivacyPassword& password,
                      const QByteArray& expectedPayload)
{
    PrivacyCasualArchiveEngine engine;
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
    QByteArray output;
    QBuffer buffer(&output);

    if (!buffer.open(QIODevice::WriteOnly))
    {
        return false;
    }

    PrivacyCasualArchiveRestoreRequest request;
    request.archivePath = path;
    request.categoryUuid = CategoryUuid;
    request.containerUuid = ContainerUuid;
    request.itemUuid = ItemUuid;
    request.protectedRelativePath = QLatin1String(
        "digikam-private/assets/1/0/photo.jpg");
    request.originalName = QLatin1String("photo.jpg");
    request.role = PrivacyAsset::PrimaryMediaRole;
    request.ordinal = 0;
    request.expectedArchiveSize = QFileInfo(path).size();
    request.expectedArchiveSha256 =
        QCryptographicHash::hash(readFile(path), QCryptographicHash::Sha256);
    request.expectedMemberSize = expectedPayload.size();
    request.expectedMemberSha256 =
        QCryptographicHash::hash(expectedPayload, QCryptographicHash::Sha256);

    return engine.restoreMember(request, password, &buffer, {}, &error) &&
           (output == expectedPayload);
}

void seedBundle(FakePersistence* persistence, const QString& publicRoot,
                const QString& managedRoot,
                const PrivacyContainer& container)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    PrivacyCategory category;
    category.uuid = CategoryUuid;
    category.name = QLatin1String("Synthetic casual");
    category.backend = PrivacyBackend::Casual;
    category.presentationMode = PrivacyPresentationMode::Generic;
    category.unlockedThumbnailMode = PrivacyUnlockedThumbnailMode::FocusedClear;
    category.tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt = now;
    persistence->snapshot.categories << category;

    PrivacyCredential credential;
    credential.categoryUuid = CategoryUuid;
    credential.generation = 1;
    credential.encodingVersion = QLatin1String("utf8-nfc-v1");
    credential.envelopeFormat = QLatin1String("gocryptfs-config-v2");
    credential.envelopeBlob = ConfigBytes;
    credential.envelopeHashAlgorithm = QLatin1String("sha256");
    credential.envelopeHash = sha256Hex(ConfigBytes);
    credential.createdAt = now;
    persistence->snapshot.credentials << credential;

    PrivacyStore store;
    store.uuid = StoreUuid;
    store.categoryUuid = CategoryUuid;
    store.rootUuid = ManagedRootUuid;
    store.format = QLatin1String("gocryptfs");
    store.formatVersion = 2;
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/") +
                               StoreUuid;
    store.configRelativePath = store.cipherRelativePath +
                               QLatin1String("/gocryptfs.conf");
    store.configGeneration = 1;
    store.lifecycleState = PrivacyStoreLifecycleState::Active;
    store.createdAt = now;
    persistence->snapshot.stores << store;

    PrivacyStorageRoot managed;
    managed.uuid = ManagedRootUuid;
    managed.kind = PrivacyStorageRootKind::ManagedStoreRoot;
    managed.configuredPath = managedRoot;
    managed.identityVersion = 1;
    managed.identityData = QByteArray("synthetic-managed-root");
    managed.markerUuid = QLatin1String("70000000-0000-0000-0000-000000000001");
    managed.createdAt = now;
    PrivacyStorageRoot publicRootRecord;
    publicRootRecord.uuid = PublicRootUuid;
    publicRootRecord.kind = PrivacyStorageRootKind::AlbumRoot;
    publicRootRecord.albumRootId = 1;
    publicRootRecord.configuredPath = publicRoot;
    publicRootRecord.identityVersion = 1;
    publicRootRecord.identityData = QByteArray("synthetic-public-root");
    publicRootRecord.createdAt = now;
    persistence->snapshot.storageRoots << managed << publicRootRecord;

    PrivacyItem item;
    item.imageId = 42;
    item.uuid = ItemUuid;
    item.categoryUuid = CategoryUuid;
    item.originalHash = QString(64, QLatin1Char('a'));
    item.originalSize = Payload.size();
    item.expectedProxyHash = QString(64, QLatin1Char('b'));
    item.expectedProxySize = 100;
    item.presentationVersion = 1;
    item.generation = 1;
    item.transactionState = 0;
    persistence->snapshot.items << item;

    PrivacyAsset asset;
    asset.itemUuid = ItemUuid;
    asset.role = PrivacyAsset::PrimaryMediaRole;
    asset.ordinal = 0;
    asset.originalName = QLatin1String("photo.jpg");
    asset.publicRootUuid = PublicRootUuid;
    asset.publicRelativePath = QLatin1String("album/photo.jpg");
    asset.containerUuid = ContainerUuid;
    asset.protectedRelativePath =
        QLatin1String("digikam-private/assets/1/0/photo.jpg");
    asset.hashAlgorithm = QLatin1String("sha256");
    asset.originalHash = QString::fromLatin1(
        QCryptographicHash::hash(Payload, QCryptographicHash::Sha256).toHex());
    asset.originalSize = Payload.size();
    asset.proxyHashAlgorithm = QLatin1String("sha256");
    asset.proxyHash = QString(64, QLatin1Char('c'));
    asset.proxySize = 100;
    asset.proxyPresentationVersion = 1;
    asset.proxyGeneration = 1;
    persistence->snapshot.assets << asset;

    persistence->snapshot.containers << container;

    writeFile(QDir(managedRoot).filePath(
        QLatin1String(".digikam-private/stores/") + StoreUuid +
        QLatin1String("/gocryptfs.conf")), ConfigBytes);
}

PrivacyContainer makeContainer()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    PrivacyContainer container;
    container.uuid = ContainerUuid;
    container.itemUuid = ItemUuid;
    container.kind = PrivacyContainerKind::CasualArchive;
    container.rootUuid = PublicRootUuid;
    container.objectRelativePath =
        QLatin1String("album/photo.jpg.digikam-private.zip");
    container.protectedSize = 123;
    container.protectedHashAlgorithm = QLatin1String("sha256");
    container.protectedHash = QString(64, QLatin1Char('d'));
    container.formatVersion = 1;
    container.credentialGeneration = 1;
    container.state = PrivacyContainerState::Verified;
    container.createdAt = now;
    container.updatedAt = now;
    return container;
}

void createOldArchive(const QString& publicRoot,
                      const PrivacyPassword& oldPassword)
{
    const QString sourcePath = QDir(publicRoot).filePath(
        QLatin1String("album/photo.jpg"));
    const QString archivePath = QDir(publicRoot).filePath(
        QLatin1String("album/photo.jpg.digikam-private.zip"));
    QVERIFY(writeFile(sourcePath, Payload));

    PrivacyCasualArchiveMember member;
    member.sourcePath = sourcePath;
    member.originalName = QLatin1String("photo.jpg");
    member.role = PrivacyAsset::PrimaryMediaRole;
    member.ordinal = 0;
    member.protectedRelativePath =
        QLatin1String("digikam-private/assets/1/0/photo.jpg");
    member.originalCreationDate = QDateTime::currentDateTimeUtc();
    member.originalModificationDate = QDateTime::currentDateTimeUtc();

    PrivacyCasualArchiveRequest request;
    request.finalArchivePath = archivePath;
    request.categoryUuid = CategoryUuid;
    request.containerUuid = ContainerUuid;
    request.itemUuid = ItemUuid;
    request.members << member;
    PrivacyCasualArchiveEngine engine;
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
    auto stage = engine.stageArchive(request, oldPassword, {}, &error);
    QVERIFY2(stage.isValid(),
             qPrintable(QString::number(static_cast<int>(error))));
    QVERIFY(engine.publishNew(&stage, &error));
}

} // namespace

class PrivacyCasualPasswordRewriteTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRewritesAllArchivesAndPublishesCredential();
    void testInterruptedApplyingResumesRemainingArchives();
    void testMissingArchiveLeavesRecoverableTransaction();
    void testSpacePreflight();
};

void PrivacyCasualPasswordRewriteTest::
    testRewritesAllArchivesAndPublishesCredential()
{
    QTemporaryDir publicRoot;
    QTemporaryDir managedRoot;
    QVERIFY(publicRoot.isValid());
    QVERIFY(managedRoot.isValid());
    const PrivacyPassword oldPassword =
        PrivacyPassword::fromUnicode(QLatin1String("old-pass"));
    const PrivacyPassword newPassword =
        PrivacyPassword::fromUnicode(QLatin1String("new-pass"));
    createOldArchive(publicRoot.path(), oldPassword);
    const QString archivePath = QDir(publicRoot.path()).filePath(
        QLatin1String("album/photo.jpg.digikam-private.zip"));
    QVERIFY(archiveOpensWith(archivePath, oldPassword, Payload));

    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, publicRoot.path(), managedRoot.path(),
               makeContainer());
    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyCasualPasswordRewriteEngine engine(
        persistence, backend, archiveEngine);
    const PrivacyCasualPasswordRewriteResult result =
        engine.rewrap(CategoryUuid, oldPassword, newPassword);
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(backend.rewrapCalls, 1);
    QCOMPARE(persistence.snapshot.containers.constFirst()
                 .credentialGeneration, 2);
    QCOMPARE(persistence.snapshot.categories.constFirst()
                 .currentCredentialGeneration, 2);
    QCOMPARE(persistence.snapshot.credentials.constLast().envelopeBlob,
             QByteArray("rewrapped-config"));
    QCOMPARE(persistence.snapshot.stores.constFirst().configGeneration, 2);
    QVERIFY(archiveOpensWith(archivePath, newPassword, Payload));
    QVERIFY(!archiveOpensWith(archivePath, oldPassword, Payload));
    QVERIFY(!QFileInfo::exists(QDir(managedRoot.path()).filePath(
        QLatin1String(".digikam-private/staging/") + StoreUuid +
        QLatin1String(".rewrap-") + result.transactionUuid)));
}

void PrivacyCasualPasswordRewriteTest::
    testInterruptedApplyingResumesRemainingArchives()
{
    QTemporaryDir publicRoot;
    QTemporaryDir managedRoot;
    QVERIFY(publicRoot.isValid());
    QVERIFY(managedRoot.isValid());
    const PrivacyPassword oldPassword =
        PrivacyPassword::fromUnicode(QLatin1String("old-pass"));
    const PrivacyPassword newPassword =
        PrivacyPassword::fromUnicode(QLatin1String("new-pass"));
    createOldArchive(publicRoot.path(), oldPassword);

    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, publicRoot.path(), managedRoot.path(),
               makeContainer());
    const QString transactionUuid =
        QLatin1String("80000000-0000-0000-0000-000000000001");
    PrivacyTransaction transaction;
    transaction.uuid = transactionUuid;
    transaction.categoryUuid = CategoryUuid;
    transaction.type = PrivacyTransactionType::ChangePassword;
    transaction.state = PrivacyTransactionState::Applying;
    transaction.generation = 1;
    transaction.fromCredentialGeneration = 1;
    transaction.toCredentialGeneration = 2;
    transaction.payloadFormatVersion = 1;
    transaction.payloadData = QByteArray("{}");
    transaction.createdAt = QDateTime::currentDateTimeUtc();
    transaction.updatedAt = transaction.createdAt;
    persistence.snapshot.transactions << transaction;

    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyCasualPasswordRewriteEngine engine(
        persistence, backend, archiveEngine);
    const PrivacyCasualPasswordRewriteResult result =
        engine.recover(CategoryUuid, oldPassword, newPassword);
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(persistence.snapshot.containers.constFirst()
                 .credentialGeneration, 2);
    QCOMPARE(persistence.snapshot.categories.constFirst()
                 .currentCredentialGeneration, 2);
    QVERIFY(archiveOpensWith(
        QDir(publicRoot.path()).filePath(
            QLatin1String("album/photo.jpg.digikam-private.zip")),
        newPassword, Payload));
}

void PrivacyCasualPasswordRewriteTest::
    testMissingArchiveLeavesRecoverableTransaction()
{
    QTemporaryDir publicRoot;
    QTemporaryDir managedRoot;
    QVERIFY(publicRoot.isValid());
    QVERIFY(managedRoot.isValid());
    const PrivacyPassword oldPassword =
        PrivacyPassword::fromUnicode(QLatin1String("old-pass"));
    const PrivacyPassword newPassword =
        PrivacyPassword::fromUnicode(QLatin1String("new-pass"));

    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, publicRoot.path(), managedRoot.path(),
               makeContainer());
    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyCasualPasswordRewriteEngine engine(
        persistence, backend, archiveEngine);
    const PrivacyCasualPasswordRewriteResult result =
        engine.rewrap(CategoryUuid, oldPassword, newPassword);
    QCOMPARE(result.status, PrivacyCasualPasswordRewriteStatus::ArchiveFailure);
    QCOMPARE(persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Applying);

    createOldArchive(publicRoot.path(), oldPassword);
    const PrivacyCasualPasswordRewriteResult recovered =
        engine.recover(CategoryUuid, oldPassword, newPassword);
    QVERIFY2(recovered.succeeded(), qPrintable(recovered.detail));
    QCOMPARE(persistence.snapshot.containers.constFirst()
                 .credentialGeneration, 2);
}

void PrivacyCasualPasswordRewriteTest::testSpacePreflight()
{
    QTemporaryDir publicRoot;
    QTemporaryDir managedRoot;
    QVERIFY(publicRoot.isValid());
    QVERIFY(managedRoot.isValid());
    const PrivacyPassword oldPassword =
        PrivacyPassword::fromUnicode(QLatin1String("old-pass"));
    createOldArchive(publicRoot.path(), oldPassword);
    const QString archivePath = QDir(publicRoot.path()).filePath(
        QLatin1String("album/photo.jpg.digikam-private.zip"));
    const qlonglong archiveSize = QFileInfo(archivePath).size();

    FakePersistence persistence;
    FakeStoreBackend backend;
    seedBundle(&persistence, publicRoot.path(), managedRoot.path(),
               makeContainer());
    PrivacyCasualArchiveEngine archiveEngine;
    PrivacyCasualPasswordRewriteEngine engine(
        persistence, backend, archiveEngine);

    const PrivacyCasualPasswordRewriteSpaceCheck check =
        engine.checkSpace(CategoryUuid);
    QVERIFY2(check.valid, qPrintable(check.detail));
    QCOMPARE(check.largestArchiveBytes, archiveSize);
    QCOMPARE(check.requiredBytes,
             PrivacyCasualPasswordRewriteEngine::
                 requiredSpaceForLargestArchive(archiveSize));
    QVERIFY(check.availableBytes > 0);
    QVERIFY(!check.insufficient);

    QCOMPARE(PrivacyCasualPasswordRewriteEngine::
                 requiredSpaceForLargestArchive(0), 0);
    QCOMPARE(PrivacyCasualPasswordRewriteEngine::
                 requiredSpaceForLargestArchive(1000),
             2000LL + (64LL * 1024 * 1024));
}

QTEST_GUILESS_MAIN(PrivacyCasualPasswordRewriteTest)

#include "privacycasualpasswordrewrite_utest.moc"
