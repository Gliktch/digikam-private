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
#include <QTemporaryDir>
#include <QTest>
#include <QTimeZone>

// Local includes

#include "privacystrongobjectbackend.h"

using namespace Digikam;

class PrivacyStrongObjectBackendTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testStagePublishVerifyRestoreRoundTrip();
    void testRejectsHostileMemberPaths();
    void testRejectsSymlinkEscape();
    void testPublishRefusesExistingFinal();
    void testVerifyCatchesTamperedObject();
    void testRemoveConfinedDirectory();
};

namespace
{

bool writeFile(const QString& path, const QByteArray& contents)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(contents) == contents.size());
}

PrivacyStrongObjectMember memberFor(const QString& sourcePath,
                                    const QString& protectedRelativePath,
                                    const QByteArray& contents)
{
    PrivacyStrongObjectMember member;
    member.sourcePath = sourcePath;
    member.protectedRelativePath = protectedRelativePath;
    member.originalName = QFileInfo(protectedRelativePath).fileName();
    member.expectedSize = contents.size();
    member.expectedSha256 = QCryptographicHash::hash(
        contents, QCryptographicHash::Sha256);
    return member;
}

QByteArray concatenatedHash(const QList<QByteArray>& contents)
{
    QCryptographicHash hasher(QCryptographicHash::Sha256);

    for (const QByteArray& bytes : contents)
    {
        hasher.addData(bytes);
    }

    return hasher.result();
}

} // namespace

void PrivacyStrongObjectBackendTest::testStagePublishVerifyRestoreRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString vault = directory.filePath(QLatin1String("vault"));
    QVERIFY(QDir().mkpath(vault));

    const QByteArray primaryBytes(70000, 'p');
    const QByteArray sidecarBytes(1200, 's');
    const QString primarySource = directory.filePath(QLatin1String("source/photo.jpg"));
    const QString sidecarSource = directory.filePath(QLatin1String("source/photo.xmp"));
    QVERIFY(writeFile(primarySource, primaryBytes));
    QVERIFY(writeFile(sidecarSource, sidecarBytes));

    const QString stagedRelative = QLatin1String("originals/.staging-tx");
    const QString finalRelative = QLatin1String("originals/container-1");
    const QList<PrivacyStrongObjectMember> members = {
        memberFor(primarySource,
                  QLatin1String("originals/container-1/0-photo.jpg"),
                  primaryBytes),
        memberFor(sidecarSource,
                  QLatin1String("originals/container-1/1-photo.xmp"),
                  sidecarBytes)
    };
    const qlonglong totalSize = primaryBytes.size() + sidecarBytes.size();
    const QByteArray totalSha256 = concatenatedHash(
        { primaryBytes, sidecarBytes });

    QString error;
    const PrivacyStrongObjectStageResult staged =
        PrivacyStrongObjectBackend::stageObjects(vault, stagedRelative,
                                                 members, &error);
    QVERIFY2(staged.valid, qPrintable(error));
    QCOMPARE(staged.totalSize, totalSize);
    QCOMPARE(staged.totalSha256, totalSha256);
    QCOMPARE(staged.stagedRelativePaths.size(), 2);
    QVERIFY(QFileInfo::exists(QDir(vault).filePath(
        QLatin1String("originals/.staging-tx/0-photo.jpg"))));

    QVERIFY(PrivacyStrongObjectBackend::publishObjects(
        vault, stagedRelative, finalRelative, members,
        totalSize, totalSha256, &error));
    QVERIFY(!QFileInfo::exists(QDir(vault).filePath(stagedRelative)));
    QVERIFY(QFileInfo::exists(QDir(vault).filePath(
        QLatin1String("originals/container-1/0-photo.jpg"))));

    QVERIFY(PrivacyStrongObjectBackend::verifyObjects(
        vault, finalRelative, members, totalSize, totalSha256, &error));

    const QString restoredPath = directory.filePath(
        QLatin1String("restored/photo.jpg"));
    const QDateTime restoredTime(QDate(2026, 8, 11), QTime(10, 30),
                                 QTimeZone::UTC);
    const QFileDevice::Permissions permissions =
        QFileDevice::ReadOwner | QFileDevice::WriteOwner;
    QVERIFY2(PrivacyStrongObjectBackend::restoreObject(
        vault, QLatin1String("originals/container-1/0-photo.jpg"),
        restoredPath, permissions, restoredTime, &error),
             qPrintable(error));

    QFile restored(restoredPath);
    QVERIFY(restored.open(QIODevice::ReadOnly));
    QCOMPARE(restored.readAll(), primaryBytes);
    restored.close();
    QCOMPARE(QFileInfo(restoredPath).permissions() & permissions, permissions);
    QCOMPARE(QFileInfo(restoredPath).lastModified().toUTC(), restoredTime);

    QVERIFY(PrivacyStrongObjectBackend::removeObjects(
        vault, finalRelative, &error));
    QVERIFY(!QFileInfo::exists(QDir(vault).filePath(finalRelative)));
}

void PrivacyStrongObjectBackendTest::testRejectsHostileMemberPaths()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString vault = directory.filePath(QLatin1String("vault"));
    QVERIFY(QDir().mkpath(vault));
    const QByteArray bytes(100, 'x');
    const QString source = directory.filePath(QLatin1String("source.bin"));
    QVERIFY(writeFile(source, bytes));

    QString error;
    PrivacyStrongObjectMember traversal = memberFor(
        source, QLatin1String("../evil/0-x.bin"), bytes);
    QVERIFY(!PrivacyStrongObjectBackend::stageObjects(
        vault, QLatin1String("originals/.staging-bad"),
        { traversal }, &error).valid);

    PrivacyStrongObjectMember absolute = memberFor(
        source, QLatin1String("/tmp/escape/0-x.bin"), bytes);
    QVERIFY(!PrivacyStrongObjectBackend::stageObjects(
        vault, QLatin1String("originals/.staging-bad"),
        { absolute }, &error).valid);

    const QString linkSource = directory.filePath(QLatin1String("linked.bin"));
    QVERIFY(writeFile(linkSource, bytes));
    const QString linkPath = directory.filePath(QLatin1String("linked-symlink.bin"));
    QVERIFY(QFile::link(linkSource, linkPath));
    PrivacyStrongObjectMember symlinked = memberFor(
        linkPath, QLatin1String("originals/container-1/0-x.bin"), bytes);
    QVERIFY(!PrivacyStrongObjectBackend::stageObjects(
        vault, QLatin1String("originals/.staging-bad"),
        { symlinked }, &error).valid);
}

void PrivacyStrongObjectBackendTest::testRejectsSymlinkEscape()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString vault = directory.filePath(QLatin1String("vault"));
    QVERIFY(QDir().mkpath(QDir(vault).filePath(QLatin1String("originals"))));
    const QString outside = directory.filePath(QLatin1String("outside"));
    QVERIFY(QDir().mkpath(outside));
    QVERIFY(QFile::link(outside, QDir(vault).filePath(
        QLatin1String("originals/link"))));

    const QByteArray bytes(100, 'y');
    const QString source = directory.filePath(QLatin1String("source.bin"));
    QVERIFY(writeFile(source, bytes));
    const PrivacyStrongObjectMember member = memberFor(
        source, QLatin1String("originals/link/0-x.bin"), bytes);
    QString error;
    const PrivacyStrongObjectStageResult staged =
        PrivacyStrongObjectBackend::stageObjects(
        vault, QLatin1String("originals/.staging-escape"),
        { member }, &error);
    QVERIFY2(staged.valid, qPrintable(error));

    // The staged copy is confined, but publication through the escaping
    // symlink ancestor must fail closed.
    QVERIFY(!PrivacyStrongObjectBackend::publishObjects(
        vault, QLatin1String("originals/.staging-escape"),
        QLatin1String("originals/link/sub"), { member },
        staged.totalSize, staged.totalSha256, &error));
}

void PrivacyStrongObjectBackendTest::testPublishRefusesExistingFinal()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString vault = directory.filePath(QLatin1String("vault"));
    QVERIFY(QDir().mkpath(vault));
    const QByteArray bytes(50, 'z');
    const QString source = directory.filePath(QLatin1String("source.bin"));
    QVERIFY(writeFile(source, bytes));
    const PrivacyStrongObjectMember member = memberFor(
        source, QLatin1String("originals/container-2/0-x.bin"), bytes);
    QString error;
    const PrivacyStrongObjectStageResult staged =
        PrivacyStrongObjectBackend::stageObjects(
            vault, QLatin1String("originals/.staging-tx2"),
            { member }, &error);
    QVERIFY2(staged.valid, qPrintable(error));
    QVERIFY(QDir().mkpath(QDir(vault).filePath(
        QLatin1String("originals/container-2"))));

    QVERIFY(!PrivacyStrongObjectBackend::publishObjects(
        vault, QLatin1String("originals/.staging-tx2"),
        QLatin1String("originals/container-2"), { member },
        staged.totalSize, staged.totalSha256, &error));
}

void PrivacyStrongObjectBackendTest::testVerifyCatchesTamperedObject()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString vault = directory.filePath(QLatin1String("vault"));
    QVERIFY(QDir().mkpath(vault));
    const QByteArray bytes(500, 'q');
    const QString source = directory.filePath(QLatin1String("source.bin"));
    QVERIFY(writeFile(source, bytes));
    const PrivacyStrongObjectMember member = memberFor(
        source, QLatin1String("originals/container-3/0-x.bin"), bytes);
    QString error;
    const PrivacyStrongObjectStageResult staged =
        PrivacyStrongObjectBackend::stageObjects(
            vault, QLatin1String("originals/.staging-tx3"),
            { member }, &error);
    QVERIFY2(staged.valid, qPrintable(error));
    QVERIFY(PrivacyStrongObjectBackend::publishObjects(
        vault, QLatin1String("originals/.staging-tx3"),
        QLatin1String("originals/container-3"), { member },
        staged.totalSize, staged.totalSha256, &error));

    const QByteArray tampered(500, 'w');
    QVERIFY(writeFile(QDir(vault).filePath(
        QLatin1String("originals/container-3/0-x.bin")), tampered));
    QVERIFY(!PrivacyStrongObjectBackend::verifyObjects(
        vault, QLatin1String("originals/container-3"), { member },
        staged.totalSize, staged.totalSha256, &error));
}

void PrivacyStrongObjectBackendTest::testRemoveConfinedDirectory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString vault = directory.filePath(QLatin1String("vault"));
    QVERIFY(QDir().mkpath(vault));
    QString error;

    QVERIFY(PrivacyStrongObjectBackend::removeObjects(
        vault, QLatin1String("originals/absent"), &error));
    QVERIFY(!PrivacyStrongObjectBackend::removeObjects(
        vault, QLatin1String("../outside"), &error));
    QVERIFY(!PrivacyStrongObjectBackend::removeObjects(
        vault, QLatin1String("not-originals/x"), &error));
}

QTEST_GUILESS_MAIN(PrivacyStrongObjectBackendTest)

#include "privacystrongobjectbackend_utest.moc"
