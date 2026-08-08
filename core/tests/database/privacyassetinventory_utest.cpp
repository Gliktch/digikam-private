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

#include <QHash>
#include <QTest>

// Local includes

#include "privacyassetinventory.h"

using namespace Digikam;

namespace
{

QString locationKey(const PrivacyInventoryLocation& location)
{
    return location.root.uuid + QLatin1Char('/') + location.relativePath;
}

QString directoryKey(const PrivacyInventoryRoot& root, const QString& relativeDirectory)
{
    return root.uuid + QLatin1Char('/') + relativeDirectory;
}

QString identityKey(quint64 deviceId, quint64 inode)
{
    return QString::number(deviceId) + QLatin1Char(':') + QString::number(inode);
}

PrivacyInventoryRoot syntheticRoot(const QString& uuid = QLatin1String("10000000-0000-0000-0000-000000000001"),
                                   const QString& path = QLatin1String("/synthetic/root-a"))
{
    PrivacyInventoryRoot root;
    root.uuid         = uuid;
    root.absolutePath = path;

    return root;
}

PrivacyInventoryLocation syntheticLocation(const QString& relativePath,
                                            const PrivacyInventoryRoot& root = syntheticRoot())
{
    PrivacyInventoryLocation location;
    location.root         = root;
    location.relativePath = relativePath;

    return location;
}

PrivacyInventoryFileEvidence regularEvidence(quint64 deviceId,
                                             quint64 inode,
                                             quint64 linkCount = 1,
                                             qlonglong byteSize = 128)
{
    PrivacyInventoryFileEvidence evidence;
    evidence.type             = PrivacyInventoryFileType::Regular;
    evidence.identityComplete = true;
    evidence.deviceId         = deviceId;
    evidence.inode            = inode;
    evidence.linkCount        = linkCount;
    evidence.byteSize         = byteSize;

    return evidence;
}

PrivacyInventoryAliasCandidate aliasCandidate(PrivacyInventoryAliasKind kind,
                                              const PrivacyInventoryLocation& location,
                                              qlonglong imageId = -1,
                                              const QString& contentIdentity = QString())
{
    PrivacyInventoryAliasCandidate candidate;
    candidate.kind            = kind;
    candidate.location        = location;
    candidate.imageId         = imageId;
    candidate.contentIdentity = contentIdentity;

    return candidate;
}

class FakeFilesystemProvider final : public PrivacyAssetFilesystemProvider
{
public:

    PrivacyInventoryFileEvidence inspect(const PrivacyInventoryLocation& location) const override
    {
        return files.value(locationKey(location));
    }

    PrivacyInventoryDirectoryEvidence listDirectory(const PrivacyInventoryRoot& root,
                                                    const QString& relativeDirectory) const override
    {
        return directories.value(directoryKey(root, relativeDirectory));
    }

    PrivacyInventoryAliasEvidence hardlinkAliases(quint64 deviceId,
                                                  quint64 inode) const override
    {
        ++singleHardlinkQueryCount;
        return hardlinks.value(identityKey(deviceId, inode), completeEmptyAliases());
    }

    QList<PrivacyInventoryHardlinkEvidence> hardlinkAliasesFor(
        const QList<PrivacyInventoryFileIdentity>& identities) const override
    {
        ++batchHardlinkQueryCount;
        batchedIdentityCount += identities.size();

        return PrivacyAssetFilesystemProvider::hardlinkAliasesFor(identities);
    }

    void addRegular(const PrivacyInventoryLocation& location,
                    quint64 deviceId,
                    quint64 inode,
                    quint64 linkCount = 1)
    {
        files.insert(locationKey(location), regularEvidence(deviceId, inode, linkCount));
    }

    void setDirectory(const PrivacyInventoryRoot& root,
                      const QString& relativeDirectory,
                      const QStringList& entries,
                      bool complete = true)
    {
        PrivacyInventoryDirectoryEvidence evidence;
        evidence.complete   = complete;
        evidence.entryNames = entries;
        directories.insert(directoryKey(root, relativeDirectory), evidence);
    }

    static PrivacyInventoryAliasEvidence completeEmptyAliases()
    {
        PrivacyInventoryAliasEvidence evidence;
        evidence.complete = true;

        return evidence;
    }

public:

    QHash<QString, PrivacyInventoryFileEvidence>      files;
    QHash<QString, PrivacyInventoryDirectoryEvidence> directories;
    QHash<QString, PrivacyInventoryAliasEvidence>     hardlinks;
    mutable int                                        singleHardlinkQueryCount = 0;
    mutable int                                        batchHardlinkQueryCount = 0;
    mutable int                                        batchedIdentityCount = 0;
};

class FakeIdentityProvider final : public PrivacyAssetIdentityProvider
{
public:

    PrivacyInventoryAliasEvidence aliasesFor(const PrivacyInventoryAsset& asset) const override
    {
        return aliases.value(locationKey(asset.location), completeEmptyAliases());
    }

    static PrivacyInventoryAliasEvidence completeEmptyAliases()
    {
        PrivacyInventoryAliasEvidence evidence;
        evidence.complete = true;

        return evidence;
    }

public:

    QHash<QString, PrivacyInventoryAliasEvidence> aliases;
};

PrivacyAssetInventoryRequest requestFor(const QString& primaryRelativePath)
{
    PrivacyAssetInventoryRequest request;
    request.primary = syntheticLocation(primaryRelativePath);

    return request;
}

bool hasIssue(const PrivacyAssetInventoryResult& result,
              PrivacyInventoryIssueCode code)
{
    for (const PrivacyInventoryIssue& issue : result.issues)
    {
        if (issue.code == code)
        {
            return true;
        }
    }

    return false;
}

QStringList requiredPaths(const PrivacyAssetInventoryResult& result)
{
    QStringList paths;

    for (const PrivacyInventoryAsset& asset : result.requiredAssets)
    {
        paths << asset.location.relativePath;
    }

    return paths;
}

} // namespace

class PrivacyAssetInventoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPublicValidityContracts();
    void testRootIdentityMustBeCanonicalAndScoped();
    void testDigiKamSidecarNamingForms();
    void testConservativeApplePairAndPairSidecars();
    void testPairRecognitionIsConservativeAndUnambiguous();
    void testAliasesAreWarningsAndDeterministic();
    void testIncompleteAliasEvidenceFailsClosed();
    void testUnsafeCandidatesAndPathEscapeAreRejected();
    void testHardlinkIdentityMustMatchExactly();
    void testSingleLinkFilesDoNotTriggerHardlinkScan();
    void testRequiredMembersCannotShareAnInode();
    void testProviderKindsCannotCrossBoundaries();
    void testReservedArchiveSuffixIsRejected();
    void testDirectoryCollisionIsRejected();
};

void PrivacyAssetInventoryTest::testPublicValidityContracts()
{
    QVERIFY(!PrivacyInventoryLocation().isValid());
    QVERIFY(!PrivacyInventoryAsset().isValid());
    QVERIFY(!PrivacyInventoryAliasCandidate().isValid());
    QVERIFY(!PrivacyInventoryExposureWarning().isValid());
    QVERIFY(!PrivacyInventoryIssue().isValid());
    QVERIFY(!PrivacyAssetInventoryRequest().isValid());
    QVERIFY(!PrivacyAssetInventoryResult().isValid());

    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyAssetInventoryResult rejected = PrivacyAssetInventory::build(
        PrivacyAssetInventoryRequest(), filesystem, identities);
    QVERIFY(rejected.isValid());
    QVERIFY(!rejected.isReady());
}

void PrivacyAssetInventoryTest::testRootIdentityMustBeCanonicalAndScoped()
{
    PrivacyInventoryRoot root = syntheticRoot();
    QVERIFY(root.isValid());

    root.uuid = QLatin1String("ROOT-A");
    QVERIFY(!root.isValid());

    root = syntheticRoot();
    root.absolutePath = QLatin1String("/");
    QVERIFY(!root.isValid());
}

void PrivacyAssetInventoryTest::testDigiKamSidecarNamingForms()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryRoot root = syntheticRoot();
    const QStringList entries {
        QLatin1String("IMAGE.JPG"),
        QLatin1String("IMAGE.JPG.pp3"),
        QLatin1String("IMAGE.JPG.xmp"),
        QLatin1String("IMAGE.pp3"),
        QLatin1String("IMAGE.xmp")
    };
    filesystem.setDirectory(root, QLatin1String("album"), entries);

    quint64 inode = 10;

    for (const QString& entry : entries)
    {
        filesystem.addRegular(syntheticLocation(QLatin1String("album/") + entry), 7, inode++);
    }

    PrivacyAssetInventoryRequest request = requestFor(QLatin1String("album/IMAGE.JPG"));
    request.configuredSidecarExtensions << QLatin1String("pp3");
    const PrivacyAssetInventoryResult result =
        PrivacyAssetInventory::build(request, filesystem, identities);

    QVERIFY(result.isReady());
    QCOMPARE(requiredPaths(result),
             QStringList({ QLatin1String("album/IMAGE.JPG"),
                           QLatin1String("album/IMAGE.JPG.xmp"),
                           QLatin1String("album/IMAGE.xmp"),
                           QLatin1String("album/IMAGE.JPG.pp3"),
                           QLatin1String("album/IMAGE.pp3") }));
    QVERIFY(result.exposureWarnings.isEmpty());
}

void PrivacyAssetInventoryTest::testConservativeApplePairAndPairSidecars()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryRoot root = syntheticRoot();
    const QStringList entries {
        QLatin1String("IMG_0001.HEIC"),
        QLatin1String("IMG_0001.MOV"),
        QLatin1String("IMG_0001.HEIC.xmp"),
        QLatin1String("IMG_0001.MOV.xmp"),
        QLatin1String("IMG_0001.xmp")
    };
    filesystem.setDirectory(root, QLatin1String("album"), entries);

    quint64 inode = 20;

    for (const QString& entry : entries)
    {
        filesystem.addRegular(syntheticLocation(QLatin1String("album/") + entry), 7, inode++);
    }

    const PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(QLatin1String("album/IMG_0001.HEIC")), filesystem, identities);

    QVERIFY(result.isReady());
    QCOMPARE(requiredPaths(result),
             QStringList({ QLatin1String("album/IMG_0001.HEIC"),
                           QLatin1String("album/IMG_0001.MOV"),
                           QLatin1String("album/IMG_0001.HEIC.xmp"),
                           QLatin1String("album/IMG_0001.MOV.xmp"),
                           QLatin1String("album/IMG_0001.xmp") }));
}

void PrivacyAssetInventoryTest::testPairRecognitionIsConservativeAndUnambiguous()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryRoot root = syntheticRoot();
    filesystem.setDirectory(root, QLatin1String("album"),
                            { QLatin1String("STILL.PNG"), QLatin1String("STILL.MOV") });
    filesystem.addRegular(syntheticLocation(QLatin1String("album/STILL.PNG")), 7, 30);
    filesystem.addRegular(syntheticLocation(QLatin1String("album/STILL.MOV")), 7, 31);

    PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(QLatin1String("album/STILL.PNG")), filesystem, identities);
    QVERIFY(result.isReady());
    QCOMPARE(result.requiredAssets.size(), 1);

    filesystem.setDirectory(root, QLatin1String("album"),
                            { QLatin1String("STILL.HEIC"),
                              QLatin1String("STILL.MOV"),
                              QLatin1String("STILL.mov") });
    filesystem.addRegular(syntheticLocation(QLatin1String("album/STILL.HEIC")), 7, 32);
    filesystem.addRegular(syntheticLocation(QLatin1String("album/STILL.mov")), 7, 33);
    result = PrivacyAssetInventory::build(
        requestFor(QLatin1String("album/STILL.HEIC")), filesystem, identities);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::AmbiguousPairedMedia));
}

void PrivacyAssetInventoryTest::testAliasesAreWarningsAndDeterministic()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/ITEM.JPG"));
    const PrivacyInventoryRoot otherRoot = syntheticRoot(QLatin1String("10000000-0000-0000-0000-000000000002"),
                                                         QLatin1String("/synthetic/root-b"));
    const PrivacyInventoryLocation hardlink = syntheticLocation(QLatin1String("aliases/HARD.JPG"));
    const PrivacyInventoryLocation database = syntheticLocation(QLatin1String("copies/DB.JPG"), otherRoot);
    const PrivacyInventoryLocation content = syntheticLocation(QLatin1String("copies/CONTENT.JPG"), otherRoot);
    const PrivacyInventoryLocation group = syntheticLocation(QLatin1String("groups/GROUP.JPG"), otherRoot);
    filesystem.setDirectory(primary.root, QLatin1String("album"), { QLatin1String("ITEM.JPG") });
    filesystem.addRegular(primary, 8, 40, 2);
    filesystem.addRegular(hardlink, 8, 40, 2);
    filesystem.addRegular(database, 8, 41);
    filesystem.addRegular(content, 8, 42);
    filesystem.addRegular(group, 8, 43);

    PrivacyInventoryAliasEvidence hardlinkEvidence;
    hardlinkEvidence.complete = true;
    hardlinkEvidence.candidates = {
        aliasCandidate(PrivacyInventoryAliasKind::HardlinkAlias, hardlink),
        aliasCandidate(PrivacyInventoryAliasKind::HardlinkAlias, primary)
    };
    filesystem.hardlinks.insert(identityKey(8, 40), hardlinkEvidence);

    PrivacyInventoryAliasEvidence identityEvidence;
    identityEvidence.complete = true;
    identityEvidence.candidates = {
        aliasCandidate(PrivacyInventoryAliasKind::DigikamGroupMember, group, 103),
        aliasCandidate(PrivacyInventoryAliasKind::ContentIdentityAlias, content, 102,
                       QLatin1String("sha256:synthetic")),
        aliasCandidate(PrivacyInventoryAliasKind::DatabaseItemAlias, database, 101)
    };
    identities.aliases.insert(locationKey(primary), identityEvidence);

    const PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);

    QVERIFY(result.isReady());
    QCOMPARE(result.requiredAssets.size(), 1);
    QCOMPARE(result.exposureWarnings.size(), 4);

    for (const PrivacyInventoryExposureWarning& warning : result.exposureWarnings)
    {
        QVERIFY(warning.isValid());
    }

    QCOMPARE(result.exposureWarnings.at(0).kind, PrivacyInventoryAliasKind::HardlinkAlias);
    QCOMPARE(result.exposureWarnings.at(1).kind, PrivacyInventoryAliasKind::DatabaseItemAlias);
    QCOMPARE(result.exposureWarnings.at(2).kind, PrivacyInventoryAliasKind::ContentIdentityAlias);
    QCOMPARE(result.exposureWarnings.at(3).kind, PrivacyInventoryAliasKind::DigikamGroupMember);
    QVERIFY(!requiredPaths(result).contains(group.relativePath));
}

void PrivacyAssetInventoryTest::testIncompleteAliasEvidenceFailsClosed()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/ITEM.JPG"));
    filesystem.setDirectory(primary.root, QLatin1String("album"), { QLatin1String("ITEM.JPG") });
    filesystem.addRegular(primary, 9, 50, 2);

    PrivacyInventoryAliasEvidence incomplete;
    incomplete.complete = false;
    filesystem.hardlinks.insert(identityKey(9, 50), incomplete);
    PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);
    QVERIFY(result.isValid());
    QCOMPARE(result.status, PrivacyInventoryStatus::Incomplete);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::HardlinkEnumerationIncomplete));

    filesystem.hardlinks.insert(identityKey(9, 50),
                                FakeFilesystemProvider::completeEmptyAliases());
    identities.aliases.insert(locationKey(primary), incomplete);
    result = PrivacyAssetInventory::build(requestFor(primary.relativePath),
                                          filesystem, identities);
    QVERIFY(result.isValid());
    QCOMPARE(result.status, PrivacyInventoryStatus::Incomplete);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::IdentityAliasEnumerationIncomplete));
}

void PrivacyAssetInventoryTest::testSingleLinkFilesDoNotTriggerHardlinkScan()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/ITEM.JPG"));
    filesystem.setDirectory(primary.root, QLatin1String("album"), { QLatin1String("ITEM.JPG") });
    filesystem.addRegular(primary, 9, 51, 1);

    const PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);

    QVERIFY(result.isReady());
    QCOMPARE(filesystem.batchHardlinkQueryCount, 0);
    QCOMPARE(filesystem.batchedIdentityCount, 0);
    QCOMPARE(filesystem.singleHardlinkQueryCount, 0);
}

void PrivacyAssetInventoryTest::testUnsafeCandidatesAndPathEscapeAreRejected()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/ITEM.JPG"));
    const PrivacyInventoryLocation sidecar = syntheticLocation(QLatin1String("album/ITEM.JPG.xmp"));
    filesystem.setDirectory(primary.root, QLatin1String("album"),
                            { QLatin1String("ITEM.JPG"), QLatin1String("ITEM.JPG.xmp") });
    filesystem.addRegular(primary, 10, 60);
    PrivacyInventoryFileEvidence symlink;
    symlink.type = PrivacyInventoryFileType::Symlink;
    filesystem.files.insert(locationKey(sidecar), symlink);

    PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::UnsafeFileType));

    filesystem.setDirectory(primary.root, QLatin1String("album"), { QLatin1String("ITEM.JPG") });
    PrivacyInventoryAliasEvidence escaped;
    escaped.complete = true;
    escaped.candidates << aliasCandidate(PrivacyInventoryAliasKind::ContentIdentityAlias,
                                         syntheticLocation(QLatin1String("../escape.JPG")));
    identities.aliases.insert(locationKey(primary), escaped);
    result = PrivacyAssetInventory::build(requestFor(primary.relativePath), filesystem, identities);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::UnsafePath));
}

void PrivacyAssetInventoryTest::testHardlinkIdentityMustMatchExactly()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/ITEM.JPG"));
    const PrivacyInventoryLocation alleged = syntheticLocation(QLatin1String("album/ALLEGED.JPG"));
    filesystem.setDirectory(primary.root, QLatin1String("album"), { QLatin1String("ITEM.JPG") });
    filesystem.addRegular(primary, 11, 70, 2);
    filesystem.addRegular(alleged, 11, 71);
    PrivacyInventoryAliasEvidence evidence;
    evidence.complete = true;
    evidence.candidates << aliasCandidate(PrivacyInventoryAliasKind::HardlinkAlias, alleged);
    filesystem.hardlinks.insert(identityKey(11, 70), evidence);

    const PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::AliasEvidenceMismatch));
}

void PrivacyAssetInventoryTest::testRequiredMembersCannotShareAnInode()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/PAIR.HEIC"));
    const PrivacyInventoryLocation pair = syntheticLocation(QLatin1String("album/PAIR.MOV"));
    filesystem.setDirectory(primary.root, QLatin1String("album"),
                            { QLatin1String("PAIR.HEIC"), QLatin1String("PAIR.MOV") });
    filesystem.addRegular(primary, 14, 100, 2);
    filesystem.addRegular(pair, 14, 100, 2);

    PrivacyInventoryAliasEvidence hardlinks;
    hardlinks.complete = true;
    hardlinks.candidates = {
        aliasCandidate(PrivacyInventoryAliasKind::HardlinkAlias, pair),
        aliasCandidate(PrivacyInventoryAliasKind::HardlinkAlias, primary)
    };
    filesystem.hardlinks.insert(identityKey(14, 100), hardlinks);

    const PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);
    QVERIFY(result.isValid());
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::RequiredAssetIdentityCollision));
    QCOMPARE(result.requiredAssets.size(), 2);
    QCOMPARE(result.issues.constFirst().location.relativePath, pair.relativePath);
}

void PrivacyAssetInventoryTest::testProviderKindsCannotCrossBoundaries()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/ITEM.JPG"));
    const PrivacyInventoryLocation alias = syntheticLocation(QLatin1String("album/ALIAS.JPG"));
    filesystem.setDirectory(primary.root, QLatin1String("album"), { QLatin1String("ITEM.JPG") });
    filesystem.addRegular(primary, 13, 90, 2);
    filesystem.addRegular(alias, 13, 90, 2);

    PrivacyInventoryAliasEvidence evidence;
    evidence.complete = true;
    evidence.candidates << aliasCandidate(PrivacyInventoryAliasKind::DatabaseItemAlias, alias);
    filesystem.hardlinks.insert(identityKey(13, 90), evidence);
    PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::AliasEvidenceMismatch));

    filesystem.files.insert(locationKey(primary), regularEvidence(13, 91));
    filesystem.hardlinks.remove(identityKey(13, 90));
    evidence.candidates = { aliasCandidate(PrivacyInventoryAliasKind::HardlinkAlias, alias) };
    identities.aliases.insert(locationKey(primary), evidence);
    result = PrivacyAssetInventory::build(requestFor(primary.relativePath), filesystem, identities);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::AliasEvidenceMismatch));
}

void PrivacyAssetInventoryTest::testReservedArchiveSuffixIsRejected()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(QLatin1String("album/ITEM.JPG.DIGIKAM-PRIVATE.ZIP")),
        filesystem, identities);

    QVERIFY(result.isValid());
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::ReservedPrivateArchivePath));
    QVERIFY(result.requiredAssets.isEmpty());
}

void PrivacyAssetInventoryTest::testDirectoryCollisionIsRejected()
{
    FakeFilesystemProvider filesystem;
    FakeIdentityProvider identities;
    const PrivacyInventoryLocation primary = syntheticLocation(QLatin1String("album/ITEM.JPG"));
    filesystem.setDirectory(primary.root, QLatin1String("album"),
                            { QLatin1String("ITEM.JPG"), QLatin1String("ITEM.JPG") });
    filesystem.addRegular(primary, 12, 80);

    const PrivacyAssetInventoryResult result = PrivacyAssetInventory::build(
        requestFor(primary.relativePath), filesystem, identities);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result, PrivacyInventoryIssueCode::PathCollision));
}

QTEST_GUILESS_MAIN(PrivacyAssetInventoryTest)

#include "privacyassetinventory_utest.moc"
