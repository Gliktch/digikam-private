/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// POSIX includes

#include <sys/stat.h>
#include <unistd.h>

// Qt includes

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "privacyposixfilesystemadapter.h"

using namespace Digikam;

namespace
{

const QString RootUuidA = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString RootUuidB = QLatin1String("20000000-0000-0000-0000-000000000002");

bool createFile(const QString& path, const QByteArray& bytes = QByteArray("synthetic"))
{
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(bytes) == bytes.size()) && file.flush();
}

PrivacyPosixRootScope verifiedScope(const QString& path,
                                    const QString& uuid,
                                    bool scanHardlinks = true)
{
    struct stat facts = {};
    PrivacyPosixRootScope scope;
    scope.root.uuid                      = uuid;
    scope.root.absolutePath              = path;
    scope.includeInHardlinkEnumeration   = scanHardlinks;

    if (::lstat(QFile::encodeName(path).constData(), &facts) == 0)
    {
        scope.expectedDeviceId = static_cast<quint64>(facts.st_dev);
        scope.expectedInode    = static_cast<quint64>(facts.st_ino);
    }

    return scope;
}

PrivacyInventoryLocation location(const PrivacyPosixRootScope& scope,
                                  const QString& relativePath)
{
    PrivacyInventoryLocation result;
    result.root         = scope.root;
    result.relativePath = relativePath;

    return result;
}

class CancelAfterControl final : public PrivacyPosixInventoryControl
{
public:

    explicit CancelAfterControl(int allowedPolls)
        : remainingPolls(allowedPolls)
    {
    }

    bool isCanceled() const override
    {
        return (--remainingPolls < 0);
    }

    void checkpoint(PrivacyPosixCheckpoint,
                    const PrivacyInventoryRoot&,
                    const QString&) const override
    {
    }

public:

    mutable int remainingPolls = 0;
};

class DirectoryMutationControl final : public PrivacyPosixInventoryControl
{
public:

    bool isCanceled() const override
    {
        return false;
    }

    void checkpoint(PrivacyPosixCheckpoint point,
                    const PrivacyInventoryRoot& root,
                    const QString& relativePath) const override
    {
        if (fired || (point != PrivacyPosixCheckpoint::AfterDirectoryRead))
        {
            return;
        }

        fired = true;
        const QString directory = relativePath.isEmpty()
                                ? root.absolutePath
                                : QDir(root.absolutePath).filePath(relativePath);
        created = createFile(QDir(directory).filePath(QLatin1String("late-entry.bin")));
    }

public:

    mutable bool fired = false;
    mutable bool created = false;
};

class CountingControl final : public PrivacyPosixInventoryControl
{
public:

    bool isCanceled() const override
    {
        return false;
    }

    void checkpoint(PrivacyPosixCheckpoint point,
                    const PrivacyInventoryRoot&,
                    const QString&) const override
    {
        if (point == PrivacyPosixCheckpoint::BeforeDirectoryRead)
        {
            ++directoryReads;
        }
    }

public:

    mutable int directoryReads = 0;
};

QStringList aliasPaths(const PrivacyInventoryAliasEvidence& evidence)
{
    QStringList paths;

    for (const PrivacyInventoryAliasCandidate& candidate : evidence.candidates)
    {
        paths << (candidate.location.root.uuid + QLatin1Char('/') +
                  candidate.location.relativePath);
    }

    return paths;
}

} // namespace

class PrivacyPosixFilesystemAdapterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testInspectAndDirectoryListingNeverFollowSymlinks();
    void testHardlinksAcrossConfiguredRootsAreCompleteAndSorted();
    void testBatchQueriesTraverseEachRootOnce();
    void testUnscannedAndMissingRootsFailClosed();
    void testUnreadableDirectoryFailsClosed();
    void testCancellationAndBoundsFailClosed();
    void testDirectoryRaceAndRootReplacementFailClosed();
    void testSymlinkCycleIsNotTraversed();
    void testOverlappingRootConfigurationIsRejected();
};

void PrivacyPosixFilesystemAdapterTest::testInspectAndDirectoryListingNeverFollowSymlinks()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = QDir(temporary.path()).filePath(QLatin1String("root"));
    const QString albumPath = QDir(rootPath).filePath(QLatin1String("album"));
    QVERIFY(QDir().mkpath(albumPath));
    QVERIFY(createFile(QDir(albumPath).filePath(QLatin1String("image.jpg"))));
    QVERIFY(createFile(QDir(temporary.path()).filePath(QLatin1String("outside.jpg"))));
    QCOMPARE(::symlink("../../outside.jpg",
                       QFile::encodeName(QDir(albumPath).filePath(
                           QLatin1String("escape.jpg"))).constData()), 0);

    const PrivacyPosixRootScope scope = verifiedScope(rootPath, RootUuidA);
    PrivacyPosixFilesystemAdapter adapter({ scope });
    QVERIFY(adapter.isConfigurationValid());

    const PrivacyInventoryFileEvidence regular =
        adapter.inspect(location(scope, QLatin1String("album/image.jpg")));
    QCOMPARE(regular.type, PrivacyInventoryFileType::Regular);
    QVERIFY(regular.identityComplete);
    QVERIFY(regular.inode > 0);

    const PrivacyInventoryFileEvidence symlink =
        adapter.inspect(location(scope, QLatin1String("album/escape.jpg")));
    QCOMPARE(symlink.type, PrivacyInventoryFileType::Symlink);
    QVERIFY(!symlink.identityComplete);

    PrivacyInventoryLocation escaped = location(scope, QLatin1String("../outside.jpg"));
    const PrivacyInventoryFileEvidence outside = adapter.inspect(escaped);
    QCOMPARE(outside.type, PrivacyInventoryFileType::Missing);
    QVERIFY(!outside.identityComplete);

    const PrivacyInventoryDirectoryEvidence listing =
        adapter.listDirectory(scope.root, QLatin1String("album"));
    QVERIFY(listing.complete);
    QCOMPARE(listing.entryNames,
             QStringList({ QLatin1String("escape.jpg"), QLatin1String("image.jpg") }));
}

void PrivacyPosixFilesystemAdapterTest::testHardlinksAcrossConfiguredRootsAreCompleteAndSorted()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootAPath = QDir(temporary.path()).filePath(QLatin1String("root-a"));
    const QString rootBPath = QDir(temporary.path()).filePath(QLatin1String("root-b"));
    QVERIFY(QDir().mkpath(QDir(rootAPath).filePath(QLatin1String("album"))));
    QVERIFY(QDir().mkpath(QDir(rootBPath).filePath(QLatin1String("other"))));
    const QString original = QDir(rootAPath).filePath(QLatin1String("album/original.jpg"));
    const QString alias = QDir(rootBPath).filePath(QLatin1String("other/alias.jpg"));
    QVERIFY(createFile(original));
    QCOMPARE(::link(QFile::encodeName(original).constData(),
                    QFile::encodeName(alias).constData()), 0);

    const PrivacyPosixRootScope scopeA = verifiedScope(rootAPath, RootUuidA);
    const PrivacyPosixRootScope scopeB = verifiedScope(rootBPath, RootUuidB);
    PrivacyPosixFilesystemAdapter adapter({ scopeB, scopeA });
    const PrivacyInventoryFileEvidence facts =
        adapter.inspect(location(scopeA, QLatin1String("album/original.jpg")));
    QCOMPARE(facts.linkCount, quint64(2));

    const PrivacyInventoryAliasEvidence aliases =
        adapter.hardlinkAliases(facts.deviceId, facts.inode);
    QVERIFY(aliases.complete);
    QCOMPARE(aliasPaths(aliases),
             QStringList({ RootUuidA + QLatin1String("/album/original.jpg"),
                           RootUuidB + QLatin1String("/other/alias.jpg") }));
}

void PrivacyPosixFilesystemAdapterTest::testBatchQueriesTraverseEachRootOnce()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = QDir(temporary.path()).filePath(QLatin1String("root"));
    QVERIFY(QDir().mkpath(rootPath));
    QVERIFY(createFile(QDir(rootPath).filePath(QLatin1String("one.jpg"))));
    QVERIFY(createFile(QDir(rootPath).filePath(QLatin1String("two.jpg"))));
    const PrivacyPosixRootScope scope = verifiedScope(rootPath, RootUuidA);
    PrivacyPosixFilesystemAdapter baseline({ scope });
    const PrivacyInventoryFileEvidence one =
        baseline.inspect(location(scope, QLatin1String("one.jpg")));
    const PrivacyInventoryFileEvidence two =
        baseline.inspect(location(scope, QLatin1String("two.jpg")));
    PrivacyInventoryFileIdentity identityOne;
    identityOne.deviceId = one.deviceId;
    identityOne.inode    = one.inode;
    PrivacyInventoryFileIdentity identityTwo;
    identityTwo.deviceId = two.deviceId;
    identityTwo.inode    = two.inode;

    CountingControl counting;
    PrivacyPosixFilesystemAdapter batched({ scope }, PrivacyPosixScanLimits(), &counting);
    const QList<PrivacyInventoryHardlinkEvidence> results =
        batched.hardlinkAliasesFor({ identityTwo, identityOne });
    QCOMPARE(results.size(), 2);
    QVERIFY(results.at(0).aliases.complete);
    QVERIFY(results.at(1).aliases.complete);
    QCOMPARE(results.at(0).aliases.candidates.size(), 1);
    QCOMPARE(results.at(1).aliases.candidates.size(), 1);
    QCOMPARE(counting.directoryReads, 1);
}

void PrivacyPosixFilesystemAdapterTest::testUnscannedAndMissingRootsFailClosed()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootAPath = QDir(temporary.path()).filePath(QLatin1String("root-a"));
    const QString rootBPath = QDir(temporary.path()).filePath(QLatin1String("root-b"));
    QVERIFY(QDir().mkpath(rootAPath));
    QVERIFY(QDir().mkpath(rootBPath));
    const QString original = QDir(rootAPath).filePath(QLatin1String("original.jpg"));
    QVERIFY(createFile(original));

    const PrivacyPosixRootScope scopeA = verifiedScope(rootAPath, RootUuidA);
    PrivacyPosixRootScope scopeB = verifiedScope(rootBPath, RootUuidB, false);
    PrivacyPosixFilesystemAdapter unscanned({ scopeA, scopeB });
    const PrivacyInventoryFileEvidence facts =
        unscanned.inspect(location(scopeA, QLatin1String("original.jpg")));
    QVERIFY(!unscanned.hardlinkAliases(facts.deviceId, facts.inode).complete);

    scopeB.includeInHardlinkEnumeration = true;
    QVERIFY(QDir().rmdir(rootBPath));
    PrivacyPosixFilesystemAdapter missing({ scopeA, scopeB });
    QVERIFY(!missing.hardlinkAliases(facts.deviceId, facts.inode).complete);
}

void PrivacyPosixFilesystemAdapterTest::testUnreadableDirectoryFailsClosed()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = QDir(temporary.path()).filePath(QLatin1String("root"));
    const QString lockedPath = QDir(rootPath).filePath(QLatin1String("locked"));
    QVERIFY(QDir().mkpath(lockedPath));
    const QString original = QDir(rootPath).filePath(QLatin1String("original.jpg"));
    QVERIFY(createFile(original));
    QVERIFY(createFile(QDir(lockedPath).filePath(QLatin1String("hidden.jpg"))));
    const PrivacyPosixRootScope scope = verifiedScope(rootPath, RootUuidA);
    PrivacyPosixFilesystemAdapter adapter({ scope });
    const PrivacyInventoryFileEvidence facts =
        adapter.inspect(location(scope, QLatin1String("original.jpg")));
    QCOMPARE(::chmod(QFile::encodeName(lockedPath).constData(), 0), 0);
    const PrivacyInventoryAliasEvidence aliases =
        adapter.hardlinkAliases(facts.deviceId, facts.inode);
    QVERIFY(!aliases.complete);
    QCOMPARE(::chmod(QFile::encodeName(lockedPath).constData(), 0700), 0);
}

void PrivacyPosixFilesystemAdapterTest::testCancellationAndBoundsFailClosed()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = QDir(temporary.path()).filePath(QLatin1String("root"));
    QVERIFY(QDir().mkpath(rootPath));
    QVERIFY(createFile(QDir(rootPath).filePath(QLatin1String("one.jpg"))));
    QVERIFY(createFile(QDir(rootPath).filePath(QLatin1String("two.jpg"))));
    const PrivacyPosixRootScope scope = verifiedScope(rootPath, RootUuidA);
    PrivacyPosixFilesystemAdapter baseline({ scope });
    const PrivacyInventoryFileEvidence facts =
        baseline.inspect(location(scope, QLatin1String("one.jpg")));

    CancelAfterControl cancellation(3);
    PrivacyPosixFilesystemAdapter canceled({ scope }, PrivacyPosixScanLimits(),
                                           &cancellation);
    QVERIFY(!canceled.hardlinkAliases(facts.deviceId, facts.inode).complete);

    PrivacyPosixScanLimits limits;
    limits.maximumEntriesTotal = 1;
    PrivacyPosixFilesystemAdapter bounded({ scope }, limits);
    QVERIFY(!bounded.hardlinkAliases(facts.deviceId, facts.inode).complete);
}

void PrivacyPosixFilesystemAdapterTest::testDirectoryRaceAndRootReplacementFailClosed()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = QDir(temporary.path()).filePath(QLatin1String("root"));
    QVERIFY(QDir().mkpath(rootPath));
    QVERIFY(createFile(QDir(rootPath).filePath(QLatin1String("original.jpg"))));
    const PrivacyPosixRootScope scope = verifiedScope(rootPath, RootUuidA);

    DirectoryMutationControl mutation;
    PrivacyPosixFilesystemAdapter racing({ scope }, PrivacyPosixScanLimits(), &mutation);
    const PrivacyInventoryDirectoryEvidence listing =
        racing.listDirectory(scope.root, QString());
    QVERIFY(mutation.fired);
    QVERIFY(mutation.created);
    QVERIFY(!listing.complete);

    const QString movedPath = rootPath + QLatin1String("-moved");
    QVERIFY(QDir().rename(rootPath, movedPath));
    QVERIFY(QDir().mkpath(rootPath));
    PrivacyPosixFilesystemAdapter replaced({ scope });
    const PrivacyInventoryFileEvidence missing =
        replaced.inspect(location(scope, QLatin1String("original.jpg")));
    QCOMPARE(missing.type, PrivacyInventoryFileType::Missing);
    QVERIFY(!replaced.hardlinkAliases(scope.expectedDeviceId, 1).complete);
    QVERIFY(QDir().rmdir(rootPath));
    QVERIFY(QDir().rename(movedPath, rootPath));
}

void PrivacyPosixFilesystemAdapterTest::testSymlinkCycleIsNotTraversed()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = QDir(temporary.path()).filePath(QLatin1String("root"));
    QVERIFY(QDir().mkpath(rootPath));
    const QString original = QDir(rootPath).filePath(QLatin1String("original.jpg"));
    QVERIFY(createFile(original));
    QCOMPARE(::symlink(".", QFile::encodeName(
        QDir(rootPath).filePath(QLatin1String("cycle"))).constData()), 0);

    const PrivacyPosixRootScope scope = verifiedScope(rootPath, RootUuidA);
    PrivacyPosixFilesystemAdapter adapter({ scope });
    const PrivacyInventoryFileEvidence facts =
        adapter.inspect(location(scope, QLatin1String("original.jpg")));
    const PrivacyInventoryAliasEvidence aliases =
        adapter.hardlinkAliases(facts.deviceId, facts.inode);
    QVERIFY(aliases.complete);
    QCOMPARE(aliases.candidates.size(), 1);
    QCOMPARE(aliases.candidates.constFirst().location.relativePath,
             QLatin1String("original.jpg"));
}

void PrivacyPosixFilesystemAdapterTest::testOverlappingRootConfigurationIsRejected()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = QDir(temporary.path()).filePath(QLatin1String("root"));
    const QString nestedPath = QDir(rootPath).filePath(QLatin1String("nested"));
    QVERIFY(QDir().mkpath(nestedPath));
    const PrivacyPosixRootScope outer = verifiedScope(rootPath, RootUuidA);
    const PrivacyPosixRootScope nested = verifiedScope(nestedPath, RootUuidB);
    PrivacyPosixFilesystemAdapter adapter({ outer, nested });
    QVERIFY(!adapter.isConfigurationValid());
    QVERIFY(!adapter.listDirectory(outer.root, QString()).complete);
}

QTEST_GUILESS_MAIN(PrivacyPosixFilesystemAdapterTest)

#include "privacyposixfilesystemadapter_utest.moc"
