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
#include "privacycontracts.h"
#include "privacyportablediscovery.h"

using namespace Digikam;

namespace
{

const QString RecoveryA = QLatin1String("aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa");
const QString RecoveryB = QLatin1String("bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb");
const QString StoreUuid = QLatin1String("cccccccc-cccc-4ccc-8ccc-cccccccccccc");
const QString RootUuid  = QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd");
const QString MarkerUuid = QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee");
const QString CategoryUuid = QLatin1String("11111111-1111-4111-8111-111111111111");
const QString ContainerUuid = QLatin1String("22222222-2222-4222-8222-222222222222");
const QString ItemUuid = QLatin1String("33333333-3333-4333-8333-333333333333");

bool writeFile(const QString& path, const QByteArray& bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(bytes) == bytes.size()) && file.flush();
}

bool createCasualArchive(const QString& root, const QString& relativePath,
                         const QString& recoverySetUuid)
{
    const QString sourcePath = root + QLatin1String("/.source-") +
                               QUuid::createUuid().toString(
                                   QUuid::WithoutBraces);
    const QByteArray payload("portable discovery payload");

    if (!writeFile(sourcePath, payload))
    {
        return false;
    }

    const QString archivePath = QDir(root).filePath(relativePath);

    if (!QDir().mkpath(QFileInfo(archivePath).absolutePath()))
    {
        return false;
    }

    PrivacyCasualArchiveMember member;
    member.sourcePath = sourcePath;
    member.originalName = QFileInfo(relativePath).fileName();
    member.role = PrivacyAsset::PrimaryMediaRole;
    member.ordinal = 0;
    member.protectedRelativePath =
        PrivacyCasualArchiveEngine::expectedMemberPath(
            member.role, member.ordinal, member.originalName);
    member.originalCreationDate = QDateTime::currentDateTimeUtc();
    member.originalModificationDate = QDateTime::currentDateTimeUtc();
    member.expectedSize = payload.size();
    member.expectedSha256 =
        QCryptographicHash::hash(payload, QCryptographicHash::Sha256);

    PrivacyCasualArchiveRequest request;
    request.finalArchivePath = archivePath;
    request.categoryUuid = CategoryUuid;
    request.containerUuid = ContainerUuid;
    request.itemUuid = ItemUuid;
    request.recoverySetUuid = recoverySetUuid;
    request.members << member;

    PrivacyCasualArchiveEngine engine;
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
    auto stage = engine.stageArchive(
        request, PrivacyPassword::fromUnicode(QLatin1String("discovery")),
        {}, &error);

    return stage.isValid() && engine.publishNew(&stage, &error) &&
           QFile::remove(sourcePath);
}

bool createManagedRoot(const QString& root, const QString& storeUuid)
{
    const QString markerPath = QDir(root).filePath(
        QLatin1String(".digikam-private/root-marker-v1.json"));

    if (!writeFile(markerPath, PrivacyRootIdentityCodec::
                       encodeManagedRootMarkerV1(RootUuid, MarkerUuid)))
    {
        return false;
    }

    const QString configPath = QDir(root).filePath(
        QString::fromLatin1(".digikam-private/stores/%1/gocryptfs.conf")
            .arg(storeUuid));

    return writeFile(configPath, QByteArray("opaque gocryptfs config"));
}

const PrivacyPortableDiscoveryGroup* findGroup(
    const QList<PrivacyPortableDiscoveryGroup>& groups,
    const QString& recoverySetUuid)
{
    for (const PrivacyPortableDiscoveryGroup& group : groups)
    {
        if (group.recoverySetUuid == recoverySetUuid)
        {
            return &group;
        }
    }

    return nullptr;
}

} // namespace

class PrivacyPortableDiscoveryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testDiscoversCasualAndStrongGroups();
    void testRepeatedRootsCollapse();
    void testMalformedAndConflictingIssues();
    void testCancellation();
    void testMissingRootIssue();
};

void PrivacyPortableDiscoveryTest::testDiscoversCasualAndStrongGroups()
{
    QTemporaryDir collection;
    QVERIFY(collection.isValid());
    QTemporaryDir managed;
    QVERIFY(managed.isValid());

    QVERIFY(createCasualArchive(
        collection.path(), QLatin1String("album/photo.jpg.digikam-private.zip"),
        RecoveryA));
    QVERIFY(createCasualArchive(
        collection.path(), QLatin1String("album/clip.mp4.digikam-private.zip"),
        RecoveryA));
    QVERIFY(createCasualArchive(
        collection.path(), QLatin1String("other/pic.png.digikam-private.zip"),
        RecoveryB));
    QVERIFY(writeFile(collection.path() + QLatin1String("/notes.txt"),
                      QByteArray("ordinary file")));
    QVERIFY(createManagedRoot(managed.path(), StoreUuid));

    const PrivacyPortableDiscoveryResult result =
        PrivacyPortableDiscovery::scan(
            { collection.path(), managed.path() });
    QVERIFY(!result.cancelled);
    QVERIFY(result.issues.isEmpty());
    QCOMPARE(result.scannedDirectoryCount, 2);
    QCOMPARE(result.groups.size(), 3);

    const PrivacyPortableDiscoveryGroup* groupA =
        findGroup(result.groups, RecoveryA);
    QVERIFY(groupA);
    QCOMPARE(groupA->backend, PrivacyBackend::Casual);
    QCOMPARE(groupA->rootCount, 1);
    QCOMPARE(groupA->casualArchives.size(), 2);

    QStringList proxies;

    for (const PrivacyPortableCasualArchiveCandidate& candidate :
         groupA->casualArchives)
    {
        proxies << candidate.proxyRelativePath;
        QVERIFY(candidate.isValid());
        QVERIFY(QFileInfo::exists(candidate.absolutePath));
    }

    QVERIFY(proxies.contains(QLatin1String("album/photo.jpg")));
    QVERIFY(proxies.contains(QLatin1String("album/clip.mp4")));

    const PrivacyPortableDiscoveryGroup* groupB =
        findGroup(result.groups, RecoveryB);
    QVERIFY(groupB);
    QCOMPARE(groupB->casualArchives.size(), 1);
    QCOMPARE(groupB->casualArchives.constFirst().proxyRelativePath,
             QLatin1String("other/pic.png"));

    const PrivacyPortableDiscoveryGroup* strong =
        findGroup(result.groups, StoreUuid);
    QVERIFY(strong);
    QCOMPARE(strong->backend, PrivacyBackend::Strong);
    QCOMPARE(strong->strongStores.size(), 1);
    QCOMPARE(strong->strongStores.constFirst().storeUuid, StoreUuid);
    QVERIFY(QFileInfo::exists(
                strong->strongStores.constFirst().configAbsolutePath));
}

void PrivacyPortableDiscoveryTest::testRepeatedRootsCollapse()
{
    QTemporaryDir first;
    QVERIFY(first.isValid());
    QTemporaryDir second;
    QVERIFY(second.isValid());

    QVERIFY(createCasualArchive(
        first.path(), QLatin1String("photo.jpg.digikam-private.zip"),
        RecoveryA));
    QVERIFY(createCasualArchive(
        second.path(), QLatin1String("clip.mp4.digikam-private.zip"),
        RecoveryA));

    const PrivacyPortableDiscoveryResult result =
        PrivacyPortableDiscovery::scan(
            { first.path(), second.path() });
    QVERIFY(result.issues.isEmpty());
    QCOMPARE(result.groups.size(), 1);

    const PrivacyPortableDiscoveryGroup* group =
        findGroup(result.groups, RecoveryA);
    QVERIFY(group);
    QCOMPARE(group->casualArchives.size(), 2);
    QCOMPARE(group->rootCount, 2);
}

void PrivacyPortableDiscoveryTest::testMalformedAndConflictingIssues()
{
    QTemporaryDir collection;
    QVERIFY(collection.isValid());
    QTemporaryDir managed;
    QVERIFY(managed.isValid());

    QVERIFY(writeFile(
        collection.path() +
            QLatin1String("/broken.digikam-private.zip"),
        QByteArray("not a real archive")));
    QVERIFY(createCasualArchive(
        collection.path(), QLatin1String("photo.jpg.digikam-private.zip"),
        RecoveryA));
    QVERIFY(createManagedRoot(managed.path(), StoreUuid));
    QVERIFY(writeFile(
        managed.path() +
            QLatin1String("/.digikam-private/stores/not-a-uuid/gocryptfs.conf"),
        QByteArray("bad store")));

    // A Strong store whose UUID collides with a Casual recovery identity is a
    // conflict: same opaque identity, incompatible backends.
    QVERIFY(writeFile(
        collection.path() +
            QLatin1String("/.digikam-private/stores/") + RecoveryA +
            QLatin1String("/gocryptfs.conf"),
        QByteArray("conflicting store")));
    QVERIFY(writeFile(
        collection.path() +
            QLatin1String("/.digikam-private/root-marker-v1.json"),
        PrivacyRootIdentityCodec::encodeManagedRootMarkerV1(
            RootUuid, MarkerUuid)));

    const PrivacyPortableDiscoveryResult result =
        PrivacyPortableDiscovery::scan(
            { collection.path(), managed.path() });
    QVERIFY(!result.issues.isEmpty());

    bool sawMalformed = false;
    bool sawBadStore = false;
    bool sawConflict = false;

    for (const PrivacyPortableDiscoveryIssue& issue : result.issues)
    {
        if (issue.kind ==
            PrivacyPortableDiscoveryIssueKind::MalformedCasualArchive)
        {
            sawMalformed = true;
        }

        if (issue.kind ==
            PrivacyPortableDiscoveryIssueKind::InvalidStrongStore)
        {
            sawBadStore = true;
        }

        if (issue.kind ==
            PrivacyPortableDiscoveryIssueKind::ConflictingIdentity)
        {
            sawConflict = true;
        }
    }

    QVERIFY(sawMalformed);
    QVERIFY(sawBadStore);
    QVERIFY(sawConflict);
}

void PrivacyPortableDiscoveryTest::testCancellation()
{
    QTemporaryDir first;
    QVERIFY(first.isValid());
    QTemporaryDir second;
    QVERIFY(second.isValid());

    QVERIFY(createCasualArchive(
        first.path(), QLatin1String("photo.jpg.digikam-private.zip"),
        RecoveryA));
    QVERIFY(createCasualArchive(
        second.path(), QLatin1String("clip.mp4.digikam-private.zip"),
        RecoveryB));

    int calls = 0;
    const PrivacyPortableDiscoveryResult result =
        PrivacyPortableDiscovery::scan(
            { first.path(), second.path() },
            [&calls]()
            {
                return (++calls > 1);
            });

    QVERIFY(result.cancelled);
    QVERIFY(result.groups.size() <= 1);
}

void PrivacyPortableDiscoveryTest::testMissingRootIssue()
{
    const PrivacyPortableDiscoveryResult result =
        PrivacyPortableDiscovery::scan(
            { QLatin1String("/nonexistent/private-media-root") });
    QVERIFY(!result.cancelled);
    QCOMPARE(result.groups.size(), 0);
    QCOMPARE(result.issues.size(), 1);
    QCOMPARE(result.issues.constFirst().kind,
             PrivacyPortableDiscoveryIssueKind::InvalidScanRoot);
}

QTEST_GUILESS_MAIN(PrivacyPortableDiscoveryTest)

#include "privacyportablediscovery_utest.moc"
