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
#include <QUuid>

// Local includes

#include "privacycasualarchive.h"
#include "privacyportableimportcoordinator.h"

using namespace Digikam;

namespace
{

const QString RecoveryA =
    QLatin1String("aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa");
const QString CategoryA =
    QLatin1String("bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb");

bool writeFile(const QString& path, const QByteArray& bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(bytes) == bytes.size()) && file.flush();
}

bool createArchiveWithProxy(const QString& root,
                            const QString& proxyRelative,
                            const QByteArray& proxyBytes)
{
    const QString proxyPath = QDir(root).filePath(proxyRelative);

    if (!writeFile(proxyPath, proxyBytes))
    {
        return false;
    }

    const QString sourcePath = root + QLatin1String("/.source-") +
                               QUuid::createUuid().toString(
                                   QUuid::WithoutBraces);
    const QByteArray original("coordinator original bytes");

    if (!writeFile(sourcePath, original))
    {
        return false;
    }

    const QString archiveRelative =
        proxyRelative + QLatin1String(".digikam-private.zip");
    PrivacyCasualArchiveMember member;
    member.sourcePath = sourcePath;
    member.publicRelativePath = proxyRelative;
    member.originalName = QFileInfo(proxyRelative).fileName();
    member.role = PrivacyAsset::PrimaryMediaRole;
    member.ordinal = 0;
    member.protectedRelativePath =
        PrivacyCasualArchiveEngine::expectedMemberPath(
            member.role, member.ordinal, member.originalName);
    member.originalCreationDate = QDateTime::currentDateTimeUtc();
    member.originalModificationDate = QDateTime::currentDateTimeUtc();
    member.expectedSize = original.size();
    member.expectedSha256 =
        QCryptographicHash::hash(original, QCryptographicHash::Sha256);

    PrivacyCasualArchiveRequest request;
    request.finalArchivePath = QDir(root).filePath(archiveRelative);
    request.categoryUuid = CategoryA;
    request.containerUuid =
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd");
    request.itemUuid =
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee");
    request.recoverySetUuid = RecoveryA;
    request.members << member;
    PrivacyCasualArchiveEngine engine;
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
    auto stage = engine.stageArchive(
        request, PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
        {}, &error);

    return stage.isValid() && engine.publishNew(&stage, &error) &&
           QFile::remove(sourcePath);
}

class FakeCommitTarget final : public PrivacyPortableImportCommitTarget
{
public:

    bool ensureAlbumRoot(
        int albumRootId, const QString& configuredPath,
        const QString&, PrivacyStorageRoot* const persisted) override
    {
        if (!persisted)
        {
            return false;
        }

        if (failEnsure)
        {
            return false;
        }

        PrivacyStorageRoot root;
        root.uuid = QLatin1String("10000000-0000-0000-0000-") +
                    QString::number(albumRootId).rightJustified(12,
                                                                QLatin1Char('0'));
        root.kind = PrivacyStorageRootKind::AlbumRoot;
        root.albumRootId = albumRootId;
        root.configuredPath = configuredPath;
        root.identityVersion = 1;
        root.identityData = QByteArray("fake-root-identity");
        root.createdAt = QDateTime::currentDateTimeUtc();
        rootsByPath.insert(configuredPath, root);
        *persisted = root;
        return true;
    }

    bool publish(
        const PrivacyPortableImportPublication& publication) override
    {
        ++publishCalls;

        if (failPublish)
        {
            return false;
        }

        lastPublication = publication;
        return true;
    }

    bool failEnsure = false;
    bool failPublish = false;
    int publishCalls = 0;
    QHash<QString, PrivacyStorageRoot> rootsByPath;
    PrivacyPortableImportPublication lastPublication;
};

class FakeInspector final : public PrivacyPortableStoreInspector
{
public:

    bool inspect(
        const PrivacyPortableStrongStoreCandidate&,
        const PrivacyPassword&,
        PrivacyPortableStoreInspection* const,
        QString* const error) override
    {
        if (error)
        {
            *error = QStringLiteral("no store available");
        }

        return false;
    }

    bool release(const PrivacyPortableStoreInspection&, QString*) override
    {
        return true;
    }
};

} // namespace

class PrivacyPortableImportCoordinatorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testImportsStoreLessCasualCategory();
    void testWrongPasswordNotCommitted();
    void testMissingPasswordReportedUnresolved();
    void testPublishFailureReported();
    void testMissingAlbumRootFails();
};

void PrivacyPortableImportCoordinatorTest::testImportsStoreLessCasualCategory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QByteArray proxyBytes("public placeholder proxy");
    QVERIFY(createArchiveWithProxy(
        directory.path(), QLatin1String("album/photo.jpg"), proxyBytes));

    FakeCommitTarget target;
    FakeInspector inspector;
    PrivacyPortableImportCoordinator coordinator(target);
    const PrivacyPortableImportCoordinatorResult result =
        coordinator.run(
            { directory.path() },
            { { RecoveryA, QLatin1String("import-secret") } },
            { { directory.path(), 1 } },
            QLatin1String("Imported media"),
            inspector);
    QVERIFY(!result.cancelled);
    QVERIFY(result.issues.isEmpty());
    QCOMPARE(result.groups.size(), 1);
    QVERIFY(result.groups.constFirst().committed);
    QCOMPARE(result.groups.constFirst().status,
             PrivacyPortableImportAuthenticationStatus::Authenticated);
    QCOMPARE(target.publishCalls, 1);
    QVERIFY(target.lastPublication.isValid());
    QCOMPARE(target.lastPublication.category.recoverySetUuid, RecoveryA);
    QVERIFY(!target.lastPublication.hasCredential);
    QCOMPARE(target.lastPublication.category.currentCredentialGeneration,
             qlonglong(0));
    QCOMPARE(target.lastPublication.items.size(), 1);
    QCOMPARE(target.lastPublication.assets.size(), 1);
    QCOMPARE(target.lastPublication.imageFacts.size(), 1);
    QCOMPARE(target.lastPublication.imageFacts.constFirst().albumRootId, 1);
    QCOMPARE(target.lastPublication.imageFacts.constFirst().proxyHashHex,
             QString::fromLatin1(
                 QCryptographicHash::hash(
                     proxyBytes, QCryptographicHash::Sha256).toHex()));
}

void PrivacyPortableImportCoordinatorTest::testWrongPasswordNotCommitted()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(createArchiveWithProxy(
        directory.path(), QLatin1String("photo.jpg"), QByteArray("proxy")));

    FakeCommitTarget target;
    FakeInspector inspector;
    PrivacyPortableImportCoordinator coordinator(target);
    const PrivacyPortableImportCoordinatorResult result =
        coordinator.run(
            { directory.path() },
            { { RecoveryA, QLatin1String("wrong-secret") } },
            { { directory.path(), 1 } },
            QString(),
            inspector);
    QCOMPARE(result.groups.size(), 1);
    QVERIFY(!result.groups.constFirst().committed);
    QCOMPARE(result.groups.constFirst().status,
             PrivacyPortableImportAuthenticationStatus::InvalidPassword);
    QCOMPARE(target.publishCalls, 0);
}

void PrivacyPortableImportCoordinatorTest::testMissingPasswordReportedUnresolved()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(createArchiveWithProxy(
        directory.path(), QLatin1String("photo.jpg"), QByteArray("proxy")));

    FakeCommitTarget target;
    FakeInspector inspector;
    PrivacyPortableImportCoordinator coordinator(target);
    const PrivacyPortableImportCoordinatorResult result =
        coordinator.run(
            { directory.path() },
            {},
            { { directory.path(), 1 } },
            QString(),
            inspector);
    QCOMPARE(result.groups.size(), 1);
    QVERIFY(!result.groups.constFirst().committed);
    QCOMPARE(result.groups.constFirst().status,
             PrivacyPortableImportAuthenticationStatus::InvalidRequest);
    QCOMPARE(target.publishCalls, 0);
}

void PrivacyPortableImportCoordinatorTest::testPublishFailureReported()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(createArchiveWithProxy(
        directory.path(), QLatin1String("photo.jpg"), QByteArray("proxy")));

    FakeCommitTarget target;
    target.failPublish = true;
    FakeInspector inspector;
    PrivacyPortableImportCoordinator coordinator(target);
    const PrivacyPortableImportCoordinatorResult result =
        coordinator.run(
            { directory.path() },
            { { RecoveryA, QLatin1String("import-secret") } },
            { { directory.path(), 1 } },
            QString(),
            inspector);
    QCOMPARE(result.groups.size(), 1);
    QVERIFY(!result.groups.constFirst().committed);
    QCOMPARE(result.groups.constFirst().status,
             PrivacyPortableImportAuthenticationStatus::InconsistentManifests);
}

void PrivacyPortableImportCoordinatorTest::testMissingAlbumRootFails()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(createArchiveWithProxy(
        directory.path(), QLatin1String("photo.jpg"), QByteArray("proxy")));

    FakeCommitTarget target;
    FakeInspector inspector;
    PrivacyPortableImportCoordinator coordinator(target);
    const PrivacyPortableImportCoordinatorResult result =
        coordinator.run(
            { directory.path() },
            { { RecoveryA, QLatin1String("import-secret") } },
            {},
            QString(),
            inspector);
    QCOMPARE(result.groups.size(), 1);
    QVERIFY(!result.groups.constFirst().committed);
    QVERIFY(result.groups.constFirst().detail.contains(
                QLatin1String("album root")));
    QCOMPARE(target.publishCalls, 0);
}

QTEST_GUILESS_MAIN(PrivacyPortableImportCoordinatorTest)

#include "privacyportableimportcoordinator_utest.moc"
