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

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

// C includes

#include <sys/stat.h>

// Local includes

#include "privacyassetinventorybridge.h"

using namespace Digikam;

namespace
{

const QString rootUuid = QLatin1String("60000000-0000-0000-0000-000000000001");

bool writeFixture(const QString& path, const QByteArray& bytes = QByteArray("fixture"))
{
    QDir().mkpath(QFileInfo(path).path());
    QFile file(path);

    return file.open(QIODevice::WriteOnly) &&
           (file.write(bytes) == bytes.size()) && file.flush();
}

PrivacyInventoryCatalogueItem catalogueItem(
    qlonglong imageId,
    const QString& relativePath,
    const QString& databaseIdentity = QString(),
    const QString& contentIdentity = QLatin1String("sha256:fixture"),
    bool contentAuthoritative = true)
{
    PrivacyInventoryCatalogueItem item;
    item.imageId                      = imageId;
    item.publicRootUuid               = rootUuid;
    item.publicRelativePath           = relativePath;
    item.fileSize                     = 7;
    item.databaseIdentity             = databaseIdentity;
    item.storedContentIdentity        = contentIdentity;
    item.contentIdentityAuthoritative = contentAuthoritative;

    return item;
}

PrivacyInventoryRootRecord rootRecord(const QString& path,
                                      PrivacyRootRuntimeState state)
{
    PrivacyInventoryRootRecord record;
    record.uuid  = rootUuid;
    record.state = state;
    record.epoch = 4;

    if (state == PrivacyRootRuntimeState::VerifiedAvailable)
    {
        struct stat facts = {};

        if (::stat(QFile::encodeName(path).constData(), &facts) == 0)
        {
            record.scope.root.uuid         = rootUuid;
            record.scope.root.absolutePath = QDir::cleanPath(path);
            record.scope.expectedDeviceId  = static_cast<quint64>(facts.st_dev);
            record.scope.expectedInode     = static_cast<quint64>(facts.st_ino);
        }
    }

    return record;
}

class FakeCatalogueProvider final : public PrivacyInventoryCatalogueProvider
{
public:

    PrivacyInventoryCatalogueSnapshot snapshot(qsizetype, qsizetype) const override
    {
        return value;
    }

    bool generationMatches(const QByteArray& generation,
                           qsizetype, qsizetype) const override
    {
        return matches && (generation == value.generation);
    }

public:

    PrivacyInventoryCatalogueSnapshot value;
    bool matches = true;
};

class FakeRootProvider final : public PrivacyInventoryRootProvider
{
public:

    PrivacyInventoryRootSnapshot snapshot() const override
    {
        return value;
    }

    bool generationMatches(const QByteArray& generation) const override
    {
        return matches && (generation == value.generation);
    }

public:

    PrivacyInventoryRootSnapshot value;
    bool matches = true;
};

class CancelControl final : public PrivacyPosixInventoryControl
{
public:

    bool isCanceled() const override
    {
        return canceled;
    }

    void checkpoint(PrivacyPosixCheckpoint,
                    const PrivacyInventoryRoot&,
                    const QString&) const override
    {
    }

public:

    bool canceled = false;
};

FakeCatalogueProvider catalogue(const QList<PrivacyInventoryCatalogueItem>& items)
{
    FakeCatalogueProvider provider;
    provider.value.complete   = true;
    provider.value.generation = QByteArray("catalogue-generation-1");
    provider.value.items      = items;

    return provider;
}

FakeRootProvider roots(const QString& path,
                       PrivacyRootRuntimeState state =
                           PrivacyRootRuntimeState::VerifiedAvailable)
{
    FakeRootProvider provider;
    provider.value.complete   = true;
    provider.value.generation = QByteArray("root-generation-1");
    provider.value.roots << rootRecord(path, state);

    return provider;
}

PrivacyAssetInventoryBridgeRequest requestFor(
    const QList<qlonglong>& imageIds)
{
    PrivacyAssetInventoryBridgeRequest request;
    request.imageIds = imageIds;

    return request;
}

bool hasIssue(const QList<PrivacyAssetInventoryBridgeIssue>& issues,
              PrivacyAssetInventoryBridgeIssueCode code)
{
    for (const PrivacyAssetInventoryBridgeIssue& issue : issues)
    {
        if (issue.code == code)
        {
            return true;
        }
    }

    return false;
}

bool hasWarning(const PrivacyAssetInventoryResult& result,
                PrivacyInventoryAliasKind kind,
                qlonglong imageId)
{
    for (const PrivacyInventoryExposureWarning& warning : result.exposureWarnings)
    {
        if ((warning.kind == kind) && (warning.imageId == imageId))
        {
            return true;
        }
    }

    return false;
}

} // namespace

class PrivacyAssetInventoryBridgeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testAvailableRootCustomSidecarsAndCatalogueEvidence();
    void testOfflineAndMismatchedRootsRemainDistinct();
    void testDuplicateSelectionAndSelectedPathCollision();
    void testProviderFailureCancellationAndGenerationDrift();
    void testAlreadyProtectedAndReservedArchiveInput();
    void testSampledDigiKamHashIsWarningOnlyAndDoesNotBlockPreview();
    void testDeterministicSelectionOrdering();
};

void PrivacyAssetInventoryBridgeTest::testAvailableRootCustomSidecarsAndCatalogueEvidence()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/A.JPG"))));
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/A.JPG.pp3"))));
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("aliases/B.JPG"))));
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("copies/C.JPG"))));
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("groups/G.JPG"))));

    FakeCatalogueProvider catalogueProvider = catalogue({
        catalogueItem(1, QLatin1String("album/A.JPG"), QLatin1String("logical-a")),
        catalogueItem(2, QLatin1String("aliases/B.JPG"), QLatin1String("logical-a"),
                      QLatin1String("sha256:other")),
        catalogueItem(3, QLatin1String("copies/C.JPG"), QString(),
                      QLatin1String("sha256:fixture")),
        catalogueItem(4, QLatin1String("groups/G.JPG"), QString(),
                      QLatin1String("sha256:group"))
    });
    PrivacyInventoryGroupRelation group;
    group.memberImageId = 1;
    group.leaderImageId = 4;
    catalogueProvider.value.groups << group;
    FakeRootProvider rootProvider = roots(temporary.path());
    PrivacyAssetInventoryBridgeRequest request = requestFor({ 1 });
    request.configuredSidecarExtensions << QLatin1String("pp3");

    const PrivacyAssetInventoryBridgeResult result = PrivacyAssetInventoryBridge::build(
        request, catalogueProvider, rootProvider);

    QCOMPARE(result.status, PrivacyInventoryStatus::Ready);
    QCOMPARE(result.items.size(), 1);
    QVERIFY(result.items.constFirst().inventory.isReady());
    QCOMPARE(result.items.constFirst().inventory.requiredAssets.size(), 2);
    QVERIFY(hasWarning(result.items.constFirst().inventory,
                       PrivacyInventoryAliasKind::DatabaseItemAlias, 2));
    QVERIFY(hasWarning(result.items.constFirst().inventory,
                       PrivacyInventoryAliasKind::ContentIdentityAlias, 3));
    QVERIFY(hasWarning(result.items.constFirst().inventory,
                       PrivacyInventoryAliasKind::DigikamGroupMember, 4));
    QCOMPARE(result.items.constFirst().inventory.requiredAssets.constLast().location.relativePath,
             QLatin1String("album/A.JPG.pp3"));
}

void PrivacyAssetInventoryBridgeTest::testOfflineAndMismatchedRootsRemainDistinct()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    FakeCatalogueProvider catalogueProvider = catalogue({
        catalogueItem(1, QLatin1String("album/A.JPG"))
    });

    FakeRootProvider rootProvider = roots(temporary.path(),
                                           PrivacyRootRuntimeState::Offline);
    PrivacyAssetInventoryBridgeResult result = PrivacyAssetInventoryBridge::build(
        requestFor({ 1 }), catalogueProvider, rootProvider);
    QCOMPARE(result.status, PrivacyInventoryStatus::Incomplete);
    QVERIFY(hasIssue(result.items.constFirst().issues,
                     PrivacyAssetInventoryBridgeIssueCode::RootOffline));
    QVERIFY(!hasIssue(result.items.constFirst().issues,
                      PrivacyAssetInventoryBridgeIssueCode::RootIdentityMismatch));

    rootProvider = roots(temporary.path(), PrivacyRootRuntimeState::IdentityMismatch);
    result = PrivacyAssetInventoryBridge::build(requestFor({ 1 }),
                                                catalogueProvider, rootProvider);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result.items.constFirst().issues,
                     PrivacyAssetInventoryBridgeIssueCode::RootIdentityMismatch));
}

void PrivacyAssetInventoryBridgeTest::testDuplicateSelectionAndSelectedPathCollision()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/A.JPG"))));
    FakeCatalogueProvider catalogueProvider = catalogue({
        catalogueItem(1, QLatin1String("album/A.JPG")),
        catalogueItem(2, QLatin1String("album/A.JPG"))
    });
    FakeRootProvider rootProvider = roots(temporary.path());

    PrivacyAssetInventoryBridgeResult result = PrivacyAssetInventoryBridge::build(
        requestFor({ 1, 1 }), catalogueProvider, rootProvider);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result.issues,
                     PrivacyAssetInventoryBridgeIssueCode::DuplicateSelectedImageId));

    result = PrivacyAssetInventoryBridge::build(requestFor({ 2, 1 }),
                                                catalogueProvider, rootProvider);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QCOMPARE(result.items.size(), 2);
    QVERIFY(hasIssue(result.items.at(0).issues,
                     PrivacyAssetInventoryBridgeIssueCode::DuplicateSelectedPath));
    QVERIFY(hasIssue(result.items.at(1).issues,
                     PrivacyAssetInventoryBridgeIssueCode::DuplicateSelectedPath));
}

void PrivacyAssetInventoryBridgeTest::testProviderFailureCancellationAndGenerationDrift()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/A.JPG"))));
    FakeCatalogueProvider catalogueProvider = catalogue({
        catalogueItem(1, QLatin1String("album/A.JPG"))
    });
    FakeRootProvider rootProvider = roots(temporary.path());

    catalogueProvider.value.complete = false;
    PrivacyAssetInventoryBridgeResult result = PrivacyAssetInventoryBridge::build(
        requestFor({ 1 }), catalogueProvider, rootProvider);
    QCOMPARE(result.status, PrivacyInventoryStatus::Incomplete);
    QVERIFY(hasIssue(result.issues,
                     PrivacyAssetInventoryBridgeIssueCode::CatalogueEvidenceIncomplete));

    catalogueProvider.value.complete = true;
    CancelControl control;
    control.canceled = true;
    result = PrivacyAssetInventoryBridge::build(requestFor({ 1 }), catalogueProvider,
                                                rootProvider, &control);
    QCOMPARE(result.status, PrivacyInventoryStatus::Incomplete);
    QVERIFY(hasIssue(result.issues, PrivacyAssetInventoryBridgeIssueCode::Canceled));

    control.canceled = false;
    catalogueProvider.matches = false;
    result = PrivacyAssetInventoryBridge::build(requestFor({ 1 }), catalogueProvider,
                                                rootProvider, &control);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result.issues,
                     PrivacyAssetInventoryBridgeIssueCode::GenerationChanged));
}

void PrivacyAssetInventoryBridgeTest::testAlreadyProtectedAndReservedArchiveInput()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/A.JPG"))));
    QVERIFY(writeFixture(temporary.filePath(
        QLatin1String("album/A.JPG.digikam-private.zip"))));
    FakeCatalogueProvider catalogueProvider = catalogue({
        catalogueItem(1, QLatin1String("album/A.JPG")),
        catalogueItem(2, QLatin1String("album/A.JPG.digikam-private.zip"))
    });
    catalogueProvider.value.protectedImageIds.insert(1);
    FakeRootProvider rootProvider = roots(temporary.path());

    PrivacyAssetInventoryBridgeResult result = PrivacyAssetInventoryBridge::build(
        requestFor({ 1 }), catalogueProvider, rootProvider);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result.items.constFirst().issues,
                     PrivacyAssetInventoryBridgeIssueCode::AlreadyProtected));

    result = PrivacyAssetInventoryBridge::build(requestFor({ 2 }),
                                                catalogueProvider, rootProvider);
    QCOMPARE(result.status, PrivacyInventoryStatus::Rejected);
    QVERIFY(hasIssue(result.items.constFirst().issues,
                     PrivacyAssetInventoryBridgeIssueCode::ReservedPrivateArchivePath));
}

void PrivacyAssetInventoryBridgeTest::testSampledDigiKamHashIsWarningOnlyAndDoesNotBlockPreview()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/A.JPG"))));
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("copies/B.JPG"))));
    FakeCatalogueProvider catalogueProvider = catalogue({
        catalogueItem(1, QLatin1String("album/A.JPG"), QString(),
                      QLatin1String("digikam-unique-hash-v3:sample:7"), false),
        catalogueItem(2, QLatin1String("copies/B.JPG"), QString(),
                      QLatin1String("digikam-unique-hash-v3:sample:7"), false)
    });
    FakeRootProvider rootProvider = roots(temporary.path());

    const PrivacyAssetInventoryBridgeResult result = PrivacyAssetInventoryBridge::build(
        requestFor({ 1 }), catalogueProvider, rootProvider);

    QCOMPARE(result.status, PrivacyInventoryStatus::Ready);
    QVERIFY(hasWarning(result.items.constFirst().inventory,
                       PrivacyInventoryAliasKind::DatabaseItemAlias, 2));
    QVERIFY(!hasWarning(result.items.constFirst().inventory,
                        PrivacyInventoryAliasKind::ContentIdentityAlias, 2));
}

void PrivacyAssetInventoryBridgeTest::testDeterministicSelectionOrdering()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/A.JPG"))));
    QVERIFY(writeFixture(temporary.filePath(QLatin1String("album/B.JPG"))));
    FakeCatalogueProvider catalogueProvider = catalogue({
        catalogueItem(2, QLatin1String("album/B.JPG"), QString(),
                      QLatin1String("sha256:b")),
        catalogueItem(1, QLatin1String("album/A.JPG"), QString(),
                      QLatin1String("sha256:a"))
    });
    FakeRootProvider rootProvider = roots(temporary.path());

    const PrivacyAssetInventoryBridgeResult result = PrivacyAssetInventoryBridge::build(
        requestFor({ 2, 1 }), catalogueProvider, rootProvider);

    QCOMPARE(result.status, PrivacyInventoryStatus::Ready);
    QCOMPARE(result.items.size(), 2);
    QCOMPARE(result.items.at(0).imageId, 1);
    QCOMPARE(result.items.at(1).imageId, 2);
}

QTEST_GUILESS_MAIN(PrivacyAssetInventoryBridgeTest)

#include "privacyassetinventorybridge_utest.moc"
