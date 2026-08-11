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
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "privacypublicrecoverylocator.h"

using namespace Digikam;

namespace
{

QByteArray digest(const QByteArray& bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}

PrivacyPublicRecoveryLocatorEntry makeEntry(
    const QString& relativePath = QLatin1String("album/photo.jpg"),
    PrivacyBackend backend = PrivacyBackend::Casual)
{
    PrivacyPublicRecoveryLocatorEntry entry;
    entry.recoverySetUuid =
        QLatin1String("77777777-7777-4777-8777-777777777777");
    entry.backend = backend;
    entry.publicRelativePath = relativePath;
    entry.placeholderIdentity = QLatin1String("generic-v1");
    entry.expectedPlaceholderSize = 12345;
    entry.expectedPlaceholderSha256 = digest(QByteArray("placeholder"));
    return entry;
}

} // namespace

class PrivacyPublicRecoveryLocatorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRoundTrip();
    void testEmptyRoundTrip();
    void testRejectsInvalidEntries();
    void testRejectsTamperedDocument();
    void testStoreCommitLoadRoundTrip();
    void testMaintenanceHelpers();
};

void PrivacyPublicRecoveryLocatorTest::testRoundTrip()
{
    PrivacyPublicRecoveryLocatorError error =
        PrivacyPublicRecoveryLocatorError::None;
    const QList<PrivacyPublicRecoveryLocatorEntry> input = {
        makeEntry(),
        makeEntry(QLatin1String("R\xC3\xA9sum\xC3\xA9/clip.mp4"),
                  PrivacyBackend::Strong)
    };
    const QByteArray encoded =
        PrivacyPublicRecoveryLocatorCodec::encode(input, &error);
    QVERIFY(!encoded.isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);

    QList<PrivacyPublicRecoveryLocatorEntry> decoded;
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::decode(
        encoded, &decoded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);
    QCOMPARE(decoded.size(), input.size());

    for (int i = 0 ; i < input.size() ; ++i)
    {
        QCOMPARE(decoded.at(i).recoverySetUuid,
                 input.at(i).recoverySetUuid);
        QCOMPARE(decoded.at(i).backend, input.at(i).backend);
        QCOMPARE(decoded.at(i).publicRelativePath,
                 input.at(i).publicRelativePath);
        QCOMPARE(decoded.at(i).placeholderIdentity,
                 input.at(i).placeholderIdentity);
        QCOMPARE(decoded.at(i).expectedPlaceholderSize,
                 input.at(i).expectedPlaceholderSize);
        QCOMPARE(decoded.at(i).expectedPlaceholderSha256,
                 input.at(i).expectedPlaceholderSha256);
    }

    QCOMPARE(PrivacyPublicRecoveryLocatorCodec::relativePath(),
             QLatin1String(".digikam-private/recovery-locator-v1.json"));
}

void PrivacyPublicRecoveryLocatorTest::testEmptyRoundTrip()
{
    PrivacyPublicRecoveryLocatorError error =
        PrivacyPublicRecoveryLocatorError::None;
    const QByteArray encoded =
        PrivacyPublicRecoveryLocatorCodec::encode({}, &error);
    QVERIFY(!encoded.isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);

    QList<PrivacyPublicRecoveryLocatorEntry> decoded;
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::decode(
        encoded, &decoded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);
    QVERIFY(decoded.isEmpty());
}

void PrivacyPublicRecoveryLocatorTest::testRejectsInvalidEntries()
{
    PrivacyPublicRecoveryLocatorError error =
        PrivacyPublicRecoveryLocatorError::None;

    PrivacyPublicRecoveryLocatorEntry badUuid = makeEntry();
    badUuid.recoverySetUuid = QLatin1String("not-a-uuid");
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::encode(
        { badUuid }, &error).isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);

    PrivacyPublicRecoveryLocatorEntry absolute = makeEntry();
    absolute.publicRelativePath = QLatin1String("/etc/passwd");
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::encode(
        { absolute }, &error).isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);

    PrivacyPublicRecoveryLocatorEntry parent = makeEntry();
    parent.publicRelativePath = QLatin1String("album/../secret.jpg");
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::encode(
        { parent }, &error).isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);

    PrivacyPublicRecoveryLocatorEntry negativeSize = makeEntry();
    negativeSize.expectedPlaceholderSize = -1;
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::encode(
        { negativeSize }, &error).isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);

    PrivacyPublicRecoveryLocatorEntry shortHash = makeEntry();
    shortHash.expectedPlaceholderSha256 = QByteArray("short");
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::encode(
        { shortHash }, &error).isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);

    PrivacyPublicRecoveryLocatorEntry longIdentity = makeEntry();
    longIdentity.placeholderIdentity = QString(100, QLatin1Char('x'));
    QVERIFY(PrivacyPublicRecoveryLocatorCodec::encode(
        { longIdentity }, &error).isEmpty());
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);
}

void PrivacyPublicRecoveryLocatorTest::testRejectsTamperedDocument()
{
    PrivacyPublicRecoveryLocatorError error =
        PrivacyPublicRecoveryLocatorError::None;
    const QByteArray encoded =
        PrivacyPublicRecoveryLocatorCodec::encode({ makeEntry() }, &error);
    QVERIFY(!encoded.isEmpty());

    QJsonObject root =
        QJsonDocument::fromJson(encoded).object();
    root.insert(QLatin1String("format"),
                QLatin1String("digikam-private-other"));
    QList<PrivacyPublicRecoveryLocatorEntry> decoded;
    QVERIFY(!PrivacyPublicRecoveryLocatorCodec::decode(
        QJsonDocument(root).toJson(QJsonDocument::Compact),
        &decoded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);

    root = QJsonDocument::fromJson(encoded).object();
    root.insert(QLatin1String("formatVersion"), 2);
    QVERIFY(!PrivacyPublicRecoveryLocatorCodec::decode(
        QJsonDocument(root).toJson(QJsonDocument::Compact),
        &decoded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);

    QByteArray tampered = encoded;
    tampered.replace("generic-v1", "generic-v2");
    QVERIFY(!PrivacyPublicRecoveryLocatorCodec::decode(
        tampered, &decoded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::Invalid);
}

void PrivacyPublicRecoveryLocatorTest::testStoreCommitLoadRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QList<PrivacyPublicRecoveryLocatorEntry> input = {
        makeEntry(),
        makeEntry(QLatin1String("album/clip.mp4"), PrivacyBackend::Strong)
    };
    PrivacyPublicRecoveryLocatorError error =
        PrivacyPublicRecoveryLocatorError::None;
    QVERIFY(PrivacyPublicRecoveryLocatorStore::commit(
        directory.path(), input, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);

    QList<PrivacyPublicRecoveryLocatorEntry> loaded;
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        directory.path(), &loaded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);
    QCOMPARE(loaded.size(), input.size());
    QCOMPARE(loaded.constFirst().publicRelativePath,
             QLatin1String("album/photo.jpg"));
    QCOMPARE(loaded.constLast().backend, PrivacyBackend::Strong);

    const QString locatorPath = QDir(directory.path()).filePath(
        PrivacyPublicRecoveryLocatorCodec::relativePath());
    QVERIFY(QFileInfo::exists(locatorPath));

    QVERIFY(PrivacyPublicRecoveryLocatorStore::commit(
        directory.path(), {}, &error));
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        directory.path(), &loaded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);
    QVERIFY(loaded.isEmpty());

    QTemporaryDir empty;
    QVERIFY(empty.isValid());
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        empty.path(), &loaded, &error));
    QCOMPARE(error, PrivacyPublicRecoveryLocatorError::None);
    QVERIFY(loaded.isEmpty());
}

void PrivacyPublicRecoveryLocatorTest::testMaintenanceHelpers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QDateTime now = QDateTime::currentDateTimeUtc();

    PrivacyStorageRoot root;
    root.uuid = QLatin1String("aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa");
    root.kind = PrivacyStorageRootKind::AlbumRoot;
    root.albumRootId = 7;
    root.configuredPath = directory.path();
    root.identityVersion = 1;
    root.identityData = QByteArray("synthetic root identity");
    root.createdAt = now;

    PrivacyCategory category;
    category.uuid = QLatin1String("bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb");
    category.name = QLatin1String("Synthetic");
    category.recoverySetUuid =
        QLatin1String("cccccccc-cccc-4ccc-8ccc-cccccccccccc");
    category.backend = PrivacyBackend::Casual;
    category.presentationMode = PrivacyPresentationMode::Generic;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt = now;

    PrivacyItem item;
    item.imageId = 42;
    item.uuid = QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd");
    item.categoryUuid = category.uuid;
    item.expectedProxySize = 321;
    item.expectedProxyHash = QString::fromLatin1(
        digest(QByteArray("proxy")).toHex());
    item.presentationVersion = 1;
    item.generation = 1;
    item.transactionState = static_cast<int>(PrivacyTransactionState::Complete);

    PrivacyAsset primary;
    primary.itemUuid = item.uuid;
    primary.role = PrivacyAsset::PrimaryMediaRole;
    primary.ordinal = 0;
    primary.originalName = QLatin1String("photo.jpg");
    primary.publicRootUuid = root.uuid;
    primary.publicRelativePath = QLatin1String("album/photo.jpg");
    primary.containerUuid = QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee");
    primary.protectedRelativePath =
        QLatin1String("digikam-private/assets/1/0/photo.jpg");
    primary.hashAlgorithm = QLatin1String("sha256");
    primary.originalHash = QString::fromLatin1(
        digest(QByteArray("original")).toHex());
    primary.originalSize = 100;
    primary.proxyHashAlgorithm = QLatin1String("sha256");
    primary.proxyHash = item.expectedProxyHash;
    primary.proxySize = item.expectedProxySize;
    primary.proxyPresentationVersion = 1;
    primary.proxyGeneration = 1;

    QString detail;
    QVERIFY(PrivacyPublicRecoveryLocatorMaintenance::recordProtectedProxy(
        root, item, category, primary, &detail));

    QList<PrivacyPublicRecoveryLocatorEntry> entries;
    PrivacyPublicRecoveryLocatorError error =
        PrivacyPublicRecoveryLocatorError::None;
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        root.configuredPath, &entries, &error));
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.constFirst().recoverySetUuid,
             category.recoverySetUuid);
    QCOMPARE(entries.constFirst().backend, PrivacyBackend::Casual);
    QCOMPARE(entries.constFirst().placeholderIdentity,
             QLatin1String("generic-v1"));
    QCOMPARE(entries.constFirst().expectedPlaceholderSize,
             item.expectedProxySize);

    QVERIFY(PrivacyPublicRecoveryLocatorMaintenance::removePublicPaths(
        root, { primary.publicRelativePath }, &detail));
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        root.configuredPath, &entries, &error));
    QVERIFY(entries.isEmpty());

    QVERIFY(PrivacyPublicRecoveryLocatorMaintenance::recordProtectedProxy(
        root, item, category, primary, &detail));
    QVERIFY(PrivacyPublicRecoveryLocatorMaintenance::retargetProxy(
        root, primary.publicRelativePath,
        QLatin1String("ffffffff-ffff-4fff-8fff-ffffffffffff"),
        PrivacyBackend::Strong, &detail));
    QVERIFY(PrivacyPublicRecoveryLocatorStore::load(
        root.configuredPath, &entries, &error));
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.constFirst().recoverySetUuid,
             QLatin1String("ffffffff-ffff-4fff-8fff-ffffffffffff"));
    QCOMPARE(entries.constFirst().backend, PrivacyBackend::Strong);
}

QTEST_GUILESS_MAIN(PrivacyPublicRecoveryLocatorTest)

#include "privacyrecoverylocator_utest.moc"
