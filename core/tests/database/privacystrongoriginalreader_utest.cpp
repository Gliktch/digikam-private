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
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "privacyassetinventory.h"
#include "privacystrongoriginalreader.h"

using namespace Digikam;

class PrivacyStrongOriginalReaderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPrepareAndRestorePrimary();
    void testPrepareAndRestoreSidecar();
    void testRejectsCasualCategory();
    void testRestoreFailsOnMissingObject();
    void testRestoreFailsOnTamperedObject();
    void testPrepareRejectsActiveTransaction();
};

namespace
{

const QString CategoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
const QString RootUuid = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString ItemUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QString ContainerUuid = QLatin1String("40000000-0000-0000-0000-000000000001");
const QString StoreUuid = QLatin1String("50000000-0000-0000-0000-000000000001");

struct Fixture
{
    PrivacyRepositorySnapshot snapshot;
    QString vaultRoot;
    QByteArray primaryBytes;
    QByteArray sidecarBytes;
};

bool writeVaultObject(const QString& path, const QByteArray& bytes)
{
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(bytes) == bytes.size());
}

bool buildFixture(const QString& publicRoot,
                  const QString& vaultRoot,
                  PrivacyBackend backend,
                  Fixture* const fixture)
{
    if (!fixture)
    {
        return false;
    }

    fixture->vaultRoot = vaultRoot;
    fixture->primaryBytes = QByteArray(70000, 'p');
    fixture->sidecarBytes = QByteArray(1100, 's');
    const QByteArray primaryHash = QCryptographicHash::hash(
        fixture->primaryBytes, QCryptographicHash::Sha256);
    const QByteArray sidecarHash = QCryptographicHash::hash(
        fixture->sidecarBytes, QCryptographicHash::Sha256);
    const QDateTime now = QDateTime::currentDateTimeUtc();

    PrivacyCategory category;
    category.uuid = CategoryUuid;
    category.name = QLatin1String("Synthetic Strong");
    category.backend = backend;
    category.presentationMode = PrivacyPresentationMode::Generic;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt = now;

    PrivacyCredential credential;
    credential.categoryUuid = CategoryUuid;
    credential.generation = 1;
    credential.encodingVersion = QLatin1String("utf8-nfc-v1");
    credential.envelopeFormat = QLatin1String("gocryptfs-config-v2");
    credential.envelopeBlob = QByteArray("opaque-config");
    credential.envelopeHashAlgorithm = QLatin1String("sha256");
    credential.envelopeHash = QString::fromLatin1(
        QCryptographicHash::hash(credential.envelopeBlob,
                                 QCryptographicHash::Sha256).toHex());
    credential.recoveryRecordVersion = 1;
    credential.createdAt = now;

    PrivacyStorageRoot publicStorageRoot;
    publicStorageRoot.uuid = RootUuid;
    publicStorageRoot.kind = PrivacyStorageRootKind::AlbumRoot;
    publicStorageRoot.albumRootId = 1;
    publicStorageRoot.configuredPath = publicRoot;
    publicStorageRoot.identityVersion = 1;
    publicStorageRoot.identityData = QByteArray("public-root");
    publicStorageRoot.createdAt = now;

    PrivacyStorageRoot managedRoot;
    managedRoot.uuid = QLatin1String("60000000-0000-0000-0000-000000000001");
    managedRoot.kind = PrivacyStorageRootKind::ManagedStoreRoot;
    managedRoot.configuredPath = vaultRoot;
    managedRoot.identityVersion = 1;
    managedRoot.identityData = QByteArray("managed-root");
    managedRoot.markerUuid =
        QLatin1String("80000000-0000-0000-0000-000000000001");
    managedRoot.createdAt = now;

    PrivacyStore store;
    store.uuid = StoreUuid;
    store.categoryUuid = CategoryUuid;
    store.rootUuid = managedRoot.uuid;
    store.format = QLatin1String("gocryptfs");
    store.formatVersion = 2;
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/") + StoreUuid;
    store.configRelativePath = store.cipherRelativePath +
                               QLatin1String("/gocryptfs.conf");
    store.configGeneration = 1;
    store.lifecycleState = PrivacyStoreLifecycleState::Active;
    store.createdAt = now;

    PrivacyItem item;
    item.imageId = 42;
    item.uuid = ItemUuid;
    item.categoryUuid = CategoryUuid;
    item.originalHash = QString::fromLatin1(primaryHash.toHex());
    item.originalSize = fixture->primaryBytes.size();
    item.originalWidth = 640;
    item.originalHeight = 480;
    item.originalCreationDate = now;
    item.expectedProxyHash = QString::fromLatin1(
        QCryptographicHash::hash(QByteArray("proxy"),
                                 QCryptographicHash::Sha256).toHex());
    item.expectedProxySize = 5;
    item.presentationVersion = 1;
    item.generation = 1;
    item.transactionState = static_cast<int>(PrivacyTransactionState::Complete);

    PrivacyContainer container;
    container.uuid = ContainerUuid;
    container.itemUuid = ItemUuid;
    container.kind = PrivacyContainerKind::StrongObject;
    container.storeUuid = StoreUuid;
    container.objectRelativePath = QLatin1String("originals/") + ContainerUuid;
    container.protectedSize = fixture->primaryBytes.size() +
                              fixture->sidecarBytes.size();
    container.protectedHashAlgorithm = QLatin1String("sha256");
    QByteArray combinedDigests = primaryHash;
    combinedDigests += sidecarHash;
    container.protectedHash = QString::fromLatin1(
        QCryptographicHash::hash(combinedDigests,
                                 QCryptographicHash::Sha256).toHex());
    container.formatVersion = 1;
    container.credentialGeneration = 1;
    container.state = PrivacyContainerState::Verified;
    container.createdAt = now;
    container.updatedAt = now;

    PrivacyAsset primary;
    primary.itemUuid = ItemUuid;
    primary.role = PrivacyAsset::PrimaryMediaRole;
    primary.ordinal = 0;
    primary.originalName = QLatin1String("photo.jpg");
    primary.publicRootUuid = RootUuid;
    primary.publicRelativePath = QLatin1String("album/photo.jpg");
    primary.containerUuid = ContainerUuid;
    primary.protectedRelativePath = QLatin1String("originals/") + ContainerUuid +
                                    QLatin1String("/0-photo.jpg");
    primary.hashAlgorithm = QLatin1String("sha256");
    primary.originalHash = QString::fromLatin1(primaryHash.toHex());
    primary.originalSize = fixture->primaryBytes.size();
    primary.originalCreationDate = now;
    primary.originalModificationDate = now;
    primary.portableAttributes = QByteArray("mode");
    primary.proxyHashAlgorithm = QLatin1String("sha256");
    primary.proxyHash = item.expectedProxyHash;
    primary.proxySize = item.expectedProxySize;
    primary.proxyPresentationVersion = 1;
    primary.proxyGeneration = 1;

    PrivacyAsset sidecar;
    sidecar.itemUuid = ItemUuid;
    sidecar.role = static_cast<int>(PrivacyInventoryAssetRole::XmpSidecar);
    sidecar.ordinal = 0;
    sidecar.originalName = QLatin1String("photo.xmp");
    sidecar.publicRootUuid = RootUuid;
    sidecar.publicRelativePath = QLatin1String("album/photo.xmp");
    sidecar.containerUuid = ContainerUuid;
    sidecar.protectedRelativePath = QLatin1String("originals/") + ContainerUuid +
                                    QLatin1String("/0-photo.xmp");
    sidecar.hashAlgorithm = QLatin1String("sha256");
    sidecar.originalHash = QString::fromLatin1(sidecarHash.toHex());
    sidecar.originalSize = fixture->sidecarBytes.size();
    sidecar.originalCreationDate = now;
    sidecar.originalModificationDate = now;
    sidecar.portableAttributes = QByteArray("mode");

    if (!category.isValid() || !credential.isValid() ||
        !publicStorageRoot.isValid() || !managedRoot.isValid() ||
        !store.isValid() || !item.isValid() || !container.isValid() ||
        !primary.isValid() || !sidecar.isValid())
    {
        return false;
    }

    fixture->snapshot.categories << category;
    fixture->snapshot.credentials << credential;
    fixture->snapshot.storageRoots << publicStorageRoot << managedRoot;
    fixture->snapshot.stores << store;
    fixture->snapshot.items << item;
    fixture->snapshot.containers << container;
    fixture->snapshot.assets << primary << sidecar;

    for (const PrivacyStoreRole role : { PrivacyStoreRole::CredentialAuthority,
                                         PrivacyStoreRole::Derivatives,
                                         PrivacyStoreRole::Originals })
    {
        PrivacyStoreBinding binding;
        binding.categoryUuid = CategoryUuid;
        binding.role = role;
        binding.storeUuid = StoreUuid;
        fixture->snapshot.storeBindings << binding;
    }

    const QString containerDir = QDir(vaultRoot).filePath(
        QLatin1String("originals/") + ContainerUuid);

    return QDir().mkpath(containerDir) &&
           writeVaultObject(QDir(containerDir).filePath(QLatin1String("0-photo.jpg")),
                            fixture->primaryBytes) &&
           writeVaultObject(QDir(containerDir).filePath(QLatin1String("0-photo.xmp")),
                            fixture->sidecarBytes);
}

} // namespace

void PrivacyStrongOriginalReaderTest::testPrepareAndRestorePrimary()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    Fixture fixture;
    QVERIFY(buildFixture(directory.path(), vault.path(),
                         PrivacyBackend::Strong, &fixture));

    PrivacyStrongOriginalSource source;
    PrivacyStrongOriginalReader reader;
    QVERIFY(reader.prepare(fixture.snapshot, 42,
                           QDir(directory.path()).filePath(
                               QLatin1String("album/photo.jpg")),
                           &source));
    source.vaultPlaintextRoot = vault.path();
    QVERIFY(source.isValid());
    QCOMPARE(source.originalHash,
             QString::fromLatin1(QCryptographicHash::hash(
                 fixture.primaryBytes, QCryptographicHash::Sha256).toHex()));

    QBuffer destination;
    QVERIFY(destination.open(QIODevice::WriteOnly));
    QString error;
    QVERIFY2(reader.restore(source, &destination, &error), qPrintable(error));
    destination.close();
    QVERIFY(destination.open(QIODevice::ReadOnly));
    QCOMPARE(destination.readAll(), fixture.primaryBytes);
}

void PrivacyStrongOriginalReaderTest::testPrepareAndRestoreSidecar()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    Fixture fixture;
    QVERIFY(buildFixture(directory.path(), vault.path(),
                         PrivacyBackend::Strong, &fixture));

    PrivacyStrongOriginalSource source;
    PrivacyStrongOriginalReader reader;
    QVERIFY(reader.prepareAsset(fixture.snapshot, 42,
                                QDir(directory.path()).filePath(
                                    QLatin1String("album/photo.xmp")),
                                static_cast<int>(
                                    PrivacyInventoryAssetRole::XmpSidecar),
                                0,
                                &source));
    source.vaultPlaintextRoot = vault.path();
    QBuffer destination;
    QVERIFY(destination.open(QIODevice::WriteOnly));
    QString error;
    QVERIFY2(reader.restore(source, &destination, &error), qPrintable(error));
    destination.close();
    QVERIFY(destination.open(QIODevice::ReadOnly));
    QCOMPARE(destination.readAll(), fixture.sidecarBytes);
}

void PrivacyStrongOriginalReaderTest::testRejectsCasualCategory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    Fixture fixture;
    QVERIFY(buildFixture(directory.path(), vault.path(),
                         PrivacyBackend::Casual, &fixture));

    PrivacyStrongOriginalSource source;
    PrivacyStrongOriginalReader reader;
    QVERIFY(!reader.prepare(fixture.snapshot, 42,
                            QDir(directory.path()).filePath(
                                QLatin1String("album/photo.jpg")),
                            &source));
}

void PrivacyStrongOriginalReaderTest::testRestoreFailsOnMissingObject()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    Fixture fixture;
    QVERIFY(buildFixture(directory.path(), vault.path(),
                         PrivacyBackend::Strong, &fixture));
    QVERIFY(QFile::remove(QDir(vault.path()).filePath(
        QLatin1String("originals/") + ContainerUuid +
        QLatin1String("/0-photo.jpg"))));

    PrivacyStrongOriginalSource source;
    PrivacyStrongOriginalReader reader;
    QVERIFY(reader.prepare(fixture.snapshot, 42,
                           QDir(directory.path()).filePath(
                               QLatin1String("album/photo.jpg")),
                           &source));
    source.vaultPlaintextRoot = vault.path();
    QBuffer destination;
    QVERIFY(destination.open(QIODevice::WriteOnly));
    QString error;
    QVERIFY(!reader.restore(source, &destination, &error));
}

void PrivacyStrongOriginalReaderTest::testRestoreFailsOnTamperedObject()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    Fixture fixture;
    QVERIFY(buildFixture(directory.path(), vault.path(),
                         PrivacyBackend::Strong, &fixture));
    const QByteArray tampered(fixture.primaryBytes.size(), 'x');
    QFile tamperedFile(QDir(vault.path()).filePath(
        QLatin1String("originals/") + ContainerUuid +
        QLatin1String("/0-photo.jpg")));
    QVERIFY(tamperedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(tamperedFile.write(tampered), tampered.size());
    tamperedFile.close();

    PrivacyStrongOriginalSource source;
    PrivacyStrongOriginalReader reader;
    QVERIFY(reader.prepare(fixture.snapshot, 42,
                           QDir(directory.path()).filePath(
                               QLatin1String("album/photo.jpg")),
                           &source));
    source.vaultPlaintextRoot = vault.path();
    QBuffer destination;
    QVERIFY(destination.open(QIODevice::WriteOnly));
    QString error;
    QVERIFY(!reader.restore(source, &destination, &error));
}

void PrivacyStrongOriginalReaderTest::testPrepareRejectsActiveTransaction()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    Fixture fixture;
    QVERIFY(buildFixture(directory.path(), vault.path(),
                         PrivacyBackend::Strong, &fixture));

    PrivacyTransaction transaction;
    transaction.uuid = QLatin1String("70000000-0000-0000-0000-000000000001");
    transaction.categoryUuid = CategoryUuid;
    transaction.itemUuid = ItemUuid;
    transaction.type = PrivacyTransactionType::ProtectItem;
    transaction.state = PrivacyTransactionState::Created;
    transaction.generation = 0;
    transaction.fromCredentialGeneration = 1;
    transaction.toCredentialGeneration = 1;
    transaction.payloadFormatVersion = 1;
    transaction.payloadData = QByteArray("payload");
    transaction.createdAt = QDateTime::currentDateTimeUtc();
    transaction.updatedAt = transaction.createdAt;
    fixture.snapshot.transactions << transaction;

    PrivacyStrongOriginalSource source;
    PrivacyStrongOriginalReader reader;
    QVERIFY(!reader.prepare(fixture.snapshot, 42,
                            QDir(directory.path()).filePath(
                                QLatin1String("album/photo.jpg")),
                            &source));
}

QTEST_GUILESS_MAIN(PrivacyStrongOriginalReaderTest)

#include "privacystrongoriginalreader_utest.moc"
