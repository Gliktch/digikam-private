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

// Local includes

#include "privacystrongrecoverymanifest.h"
#include "privacytypes.h"

using namespace Digikam;

class PrivacyStrongRecoveryManifestTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCodecRoundTrip();
    void testCodecRejectsTamperedAndUnsafeManifests();
    void testStoreCommitAndLoad();
};

namespace
{

const QString CategoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
const QString StoreUuid = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString ItemUuid = QLatin1String("30000000-0000-0000-0000-000000000001");
const QString ContainerUuid = QLatin1String("40000000-0000-0000-0000-000000000001");

PrivacyStrongRecoveryManifest makeManifest()
{
    PrivacyStrongRecoveryManifest manifest;
    manifest.categoryUuid = CategoryUuid;
    manifest.categoryName = QLatin1String("Synthetic strong");
    manifest.presentationMode = 2;
    manifest.unlockedThumbnailMode = 3;
    manifest.tagVisibilityMode = 1;
    manifest.currentCredentialGeneration = 1;
    manifest.storeUuid = StoreUuid;

    PrivacyStrongRecoveryItem item;
    item.itemUuid = ItemUuid;
    item.containerUuid = ContainerUuid;
    item.generation = 3;

    PrivacyStrongRecoveryMember primary;
    primary.vaultRelativePath =
        QLatin1String("originals/") + ContainerUuid +
        QLatin1String("/0-photo.jpg");
    primary.publicRelativePath = QLatin1String("album/photo.jpg");
    primary.originalName = QLatin1String("photo.jpg");
    primary.role = PrivacyAsset::PrimaryMediaRole;
    primary.ordinal = 0;
    primary.hashAlgorithm = QLatin1String("sha256");
    primary.sha256Hex = QString(64, QLatin1Char('a'));
    primary.size = 70000;
    primary.creationTimeUtc =
        QDateTime::fromMSecsSinceEpoch(946684800123LL, Qt::UTC);
    primary.modificationTimeUtc =
        QDateTime::fromMSecsSinceEpoch(1700000000456LL, Qt::UTC);
    primary.portableAttributes = QByteArray("\x01\x02\x03\x04", 4);
    primary.unixMode = 0100640;
    item.members << primary;

    PrivacyStrongRecoveryMember sidecar = primary;
    sidecar.vaultRelativePath =
        QLatin1String("originals/") + ContainerUuid +
        QLatin1String("/1-photo.xmp");
    sidecar.publicRelativePath = QLatin1String("album/photo.xmp");
    sidecar.originalName = QLatin1String("photo.xmp");
    sidecar.role = 2;
    sidecar.ordinal = 0;
    sidecar.size = 1200;
    sidecar.sha256Hex = QString(64, QLatin1Char('b'));
    item.members << sidecar;

    manifest.items << item;
    return manifest;
}

} // namespace

void PrivacyStrongRecoveryManifestTest::testCodecRoundTrip()
{
    const PrivacyStrongRecoveryManifest manifest = makeManifest();
    QVERIFY(manifest.isValid());
    PrivacyStrongRecoveryManifestError error =
        PrivacyStrongRecoveryManifestError::None;
    const QByteArray bytes =
        PrivacyStrongRecoveryManifestCodec::encode(manifest, &error);
    QVERIFY2(!bytes.isEmpty(),
             qPrintable(QString::number(static_cast<int>(error))));

    PrivacyStrongRecoveryManifest decoded;
    QVERIFY(PrivacyStrongRecoveryManifestCodec::decode(bytes, &decoded, &error));
    QCOMPARE(decoded.format, QLatin1String("digikam-private-strong"));
    QCOMPARE(decoded.formatVersion, 1);
    QCOMPARE(decoded.passwordEncoding, QLatin1String("utf8-nfc-v1"));
    QCOMPARE(decoded.categoryUuid, CategoryUuid);
    QCOMPARE(decoded.categoryName, manifest.categoryName);
    QCOMPARE(decoded.presentationMode, 2);
    QCOMPARE(decoded.unlockedThumbnailMode, 3);
    QCOMPARE(decoded.tagVisibilityMode, 1);
    QCOMPARE(decoded.currentCredentialGeneration, 1);
    QCOMPARE(decoded.storeUuid, StoreUuid);
    QCOMPARE(decoded.items.size(), 1);
    QCOMPARE(decoded.items.constFirst().itemUuid, ItemUuid);
    QCOMPARE(decoded.items.constFirst().containerUuid, ContainerUuid);
    QCOMPARE(decoded.items.constFirst().generation, 3);
    QCOMPARE(decoded.items.constFirst().members.size(), 2);

    const PrivacyStrongRecoveryMember& primary =
        decoded.items.constFirst().members.constFirst();
    QCOMPARE(primary.vaultRelativePath,
             QLatin1String("originals/") + ContainerUuid +
             QLatin1String("/0-photo.jpg"));
    QCOMPARE(primary.publicRelativePath, QLatin1String("album/photo.jpg"));
    QCOMPARE(primary.originalName, QLatin1String("photo.jpg"));
    QCOMPARE(primary.role, PrivacyAsset::PrimaryMediaRole);
    QCOMPARE(primary.ordinal, 0);
    QCOMPARE(primary.hashAlgorithm, QLatin1String("sha256"));
    QCOMPARE(primary.sha256Hex, QString(64, QLatin1Char('a')));
    QCOMPARE(primary.size, 70000);
    QCOMPARE(primary.creationTimeUtc.toMSecsSinceEpoch(), 946684800123LL);
    QCOMPARE(primary.modificationTimeUtc.toMSecsSinceEpoch(),
             1700000000456LL);
    QCOMPARE(primary.portableAttributes, QByteArray("\x01\x02\x03\x04", 4));
    QCOMPARE(primary.unixMode, quint32(0100640));
}

void PrivacyStrongRecoveryManifestTest::
    testCodecRejectsTamperedAndUnsafeManifests()
{
    PrivacyStrongRecoveryManifest manifest = makeManifest();
    PrivacyStrongRecoveryManifestError error =
        PrivacyStrongRecoveryManifestError::None;

    // Tampered hash.
    PrivacyStrongRecoveryManifest tampered = manifest;
    tampered.items[0].members[0].sha256Hex[0] = QLatin1Char('g');
    QVERIFY(PrivacyStrongRecoveryManifestCodec::encode(tampered, &error).isEmpty());

    // Unsafe vault path.
    PrivacyStrongRecoveryManifest unsafe = manifest;
    unsafe.items[0].members[0].vaultRelativePath =
        QLatin1String("../escape.jpg");
    QVERIFY(PrivacyStrongRecoveryManifestCodec::encode(unsafe, &error).isEmpty());

    // Duplicate member path.
    PrivacyStrongRecoveryManifest duplicate = manifest;
    duplicate.items[0].members[1].vaultRelativePath =
        duplicate.items[0].members[0].vaultRelativePath;
    QVERIFY(PrivacyStrongRecoveryManifestCodec::encode(duplicate, &error).isEmpty());

    // Invalid category UUID.
    PrivacyStrongRecoveryManifest badUuid = manifest;
    badUuid.categoryUuid = QLatin1String("not-a-uuid");
    QVERIFY(PrivacyStrongRecoveryManifestCodec::encode(badUuid, &error).isEmpty());

    // Unsupported version.
    PrivacyStrongRecoveryManifest oldVersion = manifest;
    QByteArray bytes =
        PrivacyStrongRecoveryManifestCodec::encode(oldVersion, &error);
    QVERIFY(!bytes.isEmpty());
    bytes.replace("formatVersion\":1", "formatVersion\":99");
    PrivacyStrongRecoveryManifest ignored;
    QVERIFY(!PrivacyStrongRecoveryManifestCodec::decode(
                bytes, &ignored, &error));

    // Garbage.
    QVERIFY(!PrivacyStrongRecoveryManifestCodec::decode(
                QByteArray("not json"), &ignored, &error));
}

void PrivacyStrongRecoveryManifestTest::testStoreCommitAndLoad()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString vault = directory.filePath(QLatin1String("vault"));
    QVERIFY(QDir().mkpath(vault));

    const PrivacyStrongRecoveryManifest manifest = makeManifest();
    PrivacyStrongRecoveryManifestError error =
        PrivacyStrongRecoveryManifestError::None;
    PrivacyStrongRecoveryManifest loaded;
    QVERIFY(!PrivacyStrongRecoveryManifestStore::load(vault, &loaded, &error));
    QCOMPARE(error, PrivacyStrongRecoveryManifestError::Unavailable);

    QVERIFY(PrivacyStrongRecoveryManifestStore::commit(vault, manifest, &error));
    const QString manifestPath = QDir(vault).filePath(
        PrivacyStrongRecoveryManifestCodec::relativePath());
    QVERIFY(QFileInfo::exists(manifestPath));
    QVERIFY(!QFileInfo::exists(manifestPath + QLatin1String(".tmp")));

    QVERIFY(PrivacyStrongRecoveryManifestStore::load(vault, &loaded, &error));
    QCOMPARE(loaded.items.size(), 1);
    QCOMPARE(loaded.items.constFirst().members.size(), 2);

    PrivacyStrongRecoveryManifest updated = manifest;
    updated.items[0].generation = 4;
    QVERIFY(PrivacyStrongRecoveryManifestStore::commit(vault, updated, &error));
    QVERIFY(PrivacyStrongRecoveryManifestStore::load(vault, &loaded, &error));
    QCOMPARE(loaded.items.constFirst().generation, 4);
}

QTEST_GUILESS_MAIN(PrivacyStrongRecoveryManifestTest)

#include "privacystrongrecoverymanifest_utest.moc"
