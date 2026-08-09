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

#include "privacyderivativestore.h"

using namespace Digikam;

namespace
{

const QString ItemUuid =
    QLatin1String("20000000-0000-0000-0000-000000000001");
const QString StoreUuid =
    QLatin1String("40000000-0000-0000-0000-000000000001");

PrivacyDerivative derivativeFor(const QByteArray& bytes)
{
    PrivacyDerivative derivative;
    derivative.itemUuid = ItemUuid;
    derivative.kind = PrivacyDerivativeKind::ClearThumbnail;
    derivative.ordinal = 0;
    derivative.storeUuid = StoreUuid;
    derivative.sourceHashAlgorithm = QLatin1String("sha256");
    derivative.sourceOriginalHash = QString::fromLatin1(
        QCryptographicHash::hash(QByteArrayLiteral("synthetic original"),
                                 QCryptographicHash::Sha256).toHex());
    derivative.derivativeFormat = QLatin1String("jpeg");
    derivative.derivativeHashAlgorithm = QLatin1String("sha256");
    derivative.derivativeHash = QString::fromLatin1(
        QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
    derivative.derivativeSize = bytes.size();
    derivative.presentationVersion = 1;
    derivative.generation = 1;
    derivative.createdAt = QDateTime::currentDateTimeUtc();
    derivative.protectedRelativePath =
        PrivacyDerivativeStore::clearThumbnailRelativePath(
            derivative.itemUuid, derivative.sourceOriginalHash,
            derivative.presentationVersion);
    return derivative;
}

} // namespace

class PrivacyDerivativeStoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCanonicalPath();
    void testPutReadAndIdempotence();
    void testConflictAndUnsafeNamespaceFailClosed();
};

void PrivacyDerivativeStoreTest::testCanonicalPath()
{
    const QString hash(64, QLatin1Char('a'));
    QCOMPARE(PrivacyDerivativeStore::clearThumbnailRelativePath(
                 ItemUuid, hash, 1),
             QStringLiteral("derivatives/%1/clear-0-%2-v1.jpg")
                 .arg(ItemUuid, hash));
    QVERIFY(PrivacyDerivativeStore::clearThumbnailRelativePath(
                QLatin1String("not-a-uuid"), hash, 1).isEmpty());
    QVERIFY(PrivacyDerivativeStore::clearThumbnailRelativePath(
                ItemUuid, QLatin1String("ABC"), 1).isEmpty());
    QVERIFY(PrivacyDerivativeStore::clearThumbnailRelativePath(
                ItemUuid, hash, 0).isEmpty());
}

void PrivacyDerivativeStoreTest::testPutReadAndIdempotence()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(QFile::setPermissions(directory.path(), QFileDevice::ReadOwner |
                                                    QFileDevice::WriteOwner |
                                                    QFileDevice::ExeOwner));
    const QByteArray bytes = QByteArrayLiteral(
        "synthetic metadata-free clear derivative bytes");
    const PrivacyDerivative derivative = derivativeFor(bytes);
    QVERIFY(derivative.isValid());
    PrivacyDerivativeStore store;
    PrivacyDerivativeStoreError error = PrivacyDerivativeStoreError::None;
    QString detail;
    QVERIFY2(store.put(directory.path(), derivative, bytes, &error, &detail),
             qPrintable(detail));
    QVERIFY2(store.put(directory.path(), derivative, bytes, &error, &detail),
             qPrintable(detail));
    QCOMPARE(store.read(directory.path(), derivative, &error, &detail), bytes);
    QCOMPARE(error, PrivacyDerivativeStoreError::None);
    const QFileInfo info(QDir(directory.path()).filePath(
        derivative.protectedRelativePath));
    QVERIFY(info.isFile());
    QVERIFY(!info.isSymLink());
    QCOMPARE(info.permissions() & (QFileDevice::ReadGroup |
                                   QFileDevice::WriteGroup |
                                   QFileDevice::ExeGroup |
                                   QFileDevice::ReadOther |
                                   QFileDevice::WriteOther |
                                   QFileDevice::ExeOther),
             QFileDevice::Permissions());
}

void PrivacyDerivativeStoreTest::testConflictAndUnsafeNamespaceFailClosed()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(QFile::setPermissions(directory.path(), QFileDevice::ReadOwner |
                                                    QFileDevice::WriteOwner |
                                                    QFileDevice::ExeOwner));
    const QByteArray bytes = QByteArrayLiteral("first clear derivative");
    const PrivacyDerivative derivative = derivativeFor(bytes);
    PrivacyDerivativeStore store;
    PrivacyDerivativeStoreError error = PrivacyDerivativeStoreError::None;
    QString detail;
    QVERIFY(store.put(directory.path(), derivative, bytes, &error, &detail));

    const QByteArray replacement = QByteArrayLiteral("changed clear derivative");
    PrivacyDerivative conflicting = derivativeFor(replacement);
    conflicting.sourceOriginalHash = derivative.sourceOriginalHash;
    conflicting.protectedRelativePath = derivative.protectedRelativePath;
    QVERIFY(!store.put(directory.path(), conflicting, replacement,
                       &error, &detail));
    QCOMPARE(error, PrivacyDerivativeStoreError::Conflict);
    QCOMPARE(store.read(directory.path(), derivative, &error, &detail), bytes);

    QTemporaryDir unsafe;
    QVERIFY(unsafe.isValid());
    QVERIFY(QFile::setPermissions(unsafe.path(), QFileDevice::ReadOwner |
                                                QFileDevice::WriteOwner |
                                                QFileDevice::ExeOwner));
    const QString outside = unsafe.filePath(QLatin1String("outside"));
    QVERIFY(QDir().mkpath(outside));
    const QString itemPath = directory.filePath(
        QLatin1String("derivatives/") + ItemUuid);
    QVERIFY(QDir(itemPath).removeRecursively());
    QVERIFY(QFile::link(outside, itemPath));
    QVERIFY(store.read(directory.path(), derivative, &error, &detail).isEmpty());
    QVERIFY((error == PrivacyDerivativeStoreError::IoFailure) ||
            (error == PrivacyDerivativeStoreError::IntegrityFailure));
}

QTEST_GUILESS_MAIN(PrivacyDerivativeStoreTest)

#include "privacyderivativestore_utest.moc"
