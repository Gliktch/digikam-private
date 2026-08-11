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
#include <QJsonDocument>
#include <QJsonObject>
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

QTEST_GUILESS_MAIN(PrivacyPublicRecoveryLocatorTest)

#include "privacyrecoverylocator_utest.moc"
