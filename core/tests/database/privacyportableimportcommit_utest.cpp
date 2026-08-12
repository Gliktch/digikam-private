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
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

// Local includes

#include "collectionlocation.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "privacycontracts.h"

using namespace Digikam;

namespace
{

const QString CategoryUuid =
    QLatin1String("aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa");
const QString RecoverySetUuid =
    QLatin1String("bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb");
const QString RootUuid =
    QLatin1String("cccccccc-cccc-4ccc-8ccc-cccccccccccc");
const QString ItemUuid =
    QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd");
const QString ContainerUuid =
    QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee");

QString sha256Hex(const QByteArray& bytes)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

PrivacyPortableImportPublication makePublication(
    int albumRootId,
    const QString& configuredPath,
    const QString& itemUuid = ItemUuid)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString proxyHash = sha256Hex(QByteArray("imported proxy"));

    PrivacyPortableImportPublication publication;
    publication.category.uuid = CategoryUuid;
    publication.category.name = QLatin1String("Imported private media");
    publication.category.recoverySetUuid = RecoverySetUuid;
    publication.category.backend = PrivacyBackend::Casual;
    publication.category.presentationMode =
        PrivacyPresentationMode::Generic;
    publication.category.unlockedThumbnailMode =
        PrivacyUnlockedThumbnailMode::FocusedClear;
    publication.category.tagVisibilityMode =
        PrivacyTagVisibilityMode::UnlockedOnly;
    publication.category.lifecycleState =
        PrivacyCategoryLifecycleState::Active;
    publication.category.currentCredentialGeneration = 0;
    publication.category.schemaVersion = 1;
    publication.category.createdAt = now;

    PrivacyStorageRoot albumRoot;
    albumRoot.uuid = RootUuid;
    albumRoot.kind = PrivacyStorageRootKind::AlbumRoot;
    albumRoot.albumRootId = albumRootId;
    albumRoot.configuredPath = configuredPath;
    albumRoot.identityVersion = 1;
    albumRoot.identityData = PrivacyRootIdentityCodec::encodeAlbumRootV1(
        albumRootId, QLatin1String("imported-volume"));
    albumRoot.createdAt = now;
    publication.albumRoots << albumRoot;

    PrivacyPortableImportImageFact imageFact;
    imageFact.albumRootId = albumRootId;
    imageFact.publicRelativePath = QLatin1String("album/photo.jpg");
    imageFact.proxyHashHex = proxyHash;
    imageFact.proxySize = 12;
    imageFact.modificationDate = now;
    publication.imageFacts << imageFact;

    PrivacyItem item;
    item.imageId = -1;
    item.uuid = itemUuid;
    item.categoryUuid = CategoryUuid;
    item.originalHash = sha256Hex(QByteArray("original"));
    item.originalSize = 20;
    item.expectedProxyHash = proxyHash;
    item.expectedProxySize = 12;
    item.presentationVersion = 1;
    item.generation = 1;
    item.transactionState =
        static_cast<int>(PrivacyTransactionState::Complete);
    publication.items << item;

    PrivacyContainer container;
    container.uuid = ContainerUuid;
    container.itemUuid = itemUuid;
    container.kind = PrivacyContainerKind::CasualArchive;
    container.rootUuid = RootUuid;
    container.objectRelativePath =
        QLatin1String("album/photo.jpg.digikam-private.zip");
    container.protectedSize = 100;
    container.protectedHashAlgorithm = QLatin1String("sha256");
    container.protectedHash = sha256Hex(QByteArray("archive"));
    container.formatVersion = 1;
    container.credentialGeneration = 0;
    container.state = PrivacyContainerState::Verified;
    container.createdAt = now;
    container.updatedAt = now;
    publication.containers << container;

    PrivacyAsset primary;
    primary.itemUuid = itemUuid;
    primary.role = PrivacyAsset::PrimaryMediaRole;
    primary.ordinal = 0;
    primary.originalName = QLatin1String("photo.jpg");
    primary.publicRootUuid = RootUuid;
    primary.publicRelativePath = QLatin1String("album/photo.jpg");
    primary.containerUuid = ContainerUuid;
    primary.protectedRelativePath =
        QLatin1String("digikam-private/assets/1/0/photo.jpg");
    primary.hashAlgorithm = QLatin1String("sha256");
    primary.originalHash = item.originalHash;
    primary.originalSize = item.originalSize;
    primary.originalModificationDate = now;
    primary.proxyHashAlgorithm = QLatin1String("sha256");
    primary.proxyHash = proxyHash;
    primary.proxySize = 12;
    primary.proxyPresentationVersion = 1;
    primary.proxyGeneration = 1;
    publication.assets << primary;

    PrivacyAsset sidecar;
    sidecar.itemUuid = itemUuid;
    sidecar.role = 2;
    sidecar.ordinal = 0;
    sidecar.originalName = QLatin1String("photo.xmp");
    sidecar.publicRootUuid = RootUuid;
    sidecar.publicRelativePath = QLatin1String("album/photo.xmp");
    sidecar.containerUuid = ContainerUuid;
    sidecar.protectedRelativePath =
        QLatin1String("digikam-private/assets/2/0/photo.xmp");
    sidecar.hashAlgorithm = QLatin1String("sha256");
    sidecar.originalHash = sha256Hex(QByteArray("sidecar"));
    sidecar.originalSize = 10;
    sidecar.originalModificationDate = now;
    publication.assets << sidecar;

    return publication;
}

} // namespace

class PrivacyPortableImportCommitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPublishesStoreLessCategoryAtomically();
    void testRejectsExistingCategory();
    void testRejectsInvalidPublication();
};

void PrivacyPortableImportCommitTest::testPublishesStoreLessCategoryAtomically()
{
    if (!QSqlDatabase::isDriverAvailable(
            DbEngineParameters::SQLiteDatabaseType()))
    {
        QSKIP("Qt SQLite driver is unavailable");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    DbEngineParameters parameters;
    parameters.databaseType = DbEngineParameters::SQLiteDatabaseType();
    parameters.setCoreDatabasePath(
        directory.filePath(QLatin1String("import-commit.db")));
    parameters.legacyAndDefaultChecks();
    CoreDbAccess::setParameters(parameters);
    CoreDbAccess access;
    QVERIFY2(CoreDbAccess::checkReadyForUse(),
             qPrintable(access.lastError()));
    const int albumRootId = access.db()->addAlbumRoot(
        CollectionLocation::VolumeHardWired,
        QLatin1String("imported-volume"),
        QLatin1String("/"),
        QLatin1String("Imported collection"));
    QVERIFY(albumRootId > 0);

    const PrivacyPortableImportPublication publication =
        makePublication(albumRootId, QLatin1String("/"));
    QVERIFY(publication.isValid());
    QVERIFY(access.db()->publishPrivacyPortableImport(publication));

    QList<PrivacyCategory> categories;
    QVERIFY(access.db()->getPrivacyCategories(&categories));
    QCOMPARE(categories.size(), 1);
    QCOMPARE(categories.constFirst().recoverySetUuid, RecoverySetUuid);

    QList<PrivacyStorageRoot> roots;
    QVERIFY(access.db()->getPrivacyStorageRoots(&roots));
    QCOMPARE(roots.size(), 1);
    QCOMPARE(roots.constFirst().albumRootId, albumRootId);

    QList<PrivacyItem> items;
    QVERIFY(access.db()->getPrivacyItems(&items));
    QCOMPARE(items.size(), 1);
    QVERIFY(items.constFirst().imageId > 0);

    QList<PrivacyContainer> containers;
    QVERIFY(access.db()->getPrivacyContainers(&containers));
    QCOMPARE(containers.size(), 1);

    QList<PrivacyAsset> assets;
    QVERIFY(access.db()->getPrivacyAssets(&assets));
    QCOMPARE(assets.size(), 2);

    const int albumId = access.db()->getAlbumForPath(
        albumRootId, QLatin1String("/album"), false);
    QVERIFY(albumId > 0);
    QCOMPARE(access.db()->getItemIDsInAlbum(albumId).size(), 1);

}

void PrivacyPortableImportCommitTest::testRejectsExistingCategory()
{
    if (!QSqlDatabase::isDriverAvailable(
            DbEngineParameters::SQLiteDatabaseType()))
    {
        QSKIP("Qt SQLite driver is unavailable");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    DbEngineParameters parameters;
    parameters.databaseType = DbEngineParameters::SQLiteDatabaseType();
    parameters.setCoreDatabasePath(
        directory.filePath(QLatin1String("import-conflict.db")));
    parameters.legacyAndDefaultChecks();
    CoreDbAccess::setParameters(parameters);
    CoreDbAccess access;
    QVERIFY2(CoreDbAccess::checkReadyForUse(),
             qPrintable(access.lastError()));
    const int albumRootId = access.db()->addAlbumRoot(
        CollectionLocation::VolumeHardWired,
        QLatin1String("imported-volume"),
        QLatin1String("/"),
        QLatin1String("Imported collection"));
    QVERIFY(albumRootId > 0);

    PrivacyPortableImportPublication publication =
        makePublication(albumRootId, QLatin1String("/"));
    QVERIFY(access.db()->publishPrivacyPortableImport(publication));
    QVERIFY(!access.db()->publishPrivacyPortableImport(publication));

}

void PrivacyPortableImportCommitTest::testRejectsInvalidPublication()
{
    if (!QSqlDatabase::isDriverAvailable(
            DbEngineParameters::SQLiteDatabaseType()))
    {
        QSKIP("Qt SQLite driver is unavailable");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    DbEngineParameters parameters;
    parameters.databaseType = DbEngineParameters::SQLiteDatabaseType();
    parameters.setCoreDatabasePath(
        directory.filePath(QLatin1String("import-invalid.db")));
    parameters.legacyAndDefaultChecks();
    CoreDbAccess::setParameters(parameters);
    CoreDbAccess access;
    QVERIFY2(CoreDbAccess::checkReadyForUse(),
             qPrintable(access.lastError()));
    const int albumRootId = access.db()->addAlbumRoot(
        CollectionLocation::VolumeHardWired,
        QLatin1String("imported-volume"),
        QLatin1String("/"),
        QLatin1String("Imported collection"));
    QVERIFY(albumRootId > 0);

    PrivacyPortableImportPublication duplicateItems =
        makePublication(albumRootId, QLatin1String("/"));
    PrivacyPortableImportPublication second =
        makePublication(albumRootId, QLatin1String("/"),
                        QLatin1String("ffffffff-ffff-4fff-8fff-ffffffffffff"));
    duplicateItems.items << second.items.constFirst();
    duplicateItems.containers << second.containers.constFirst();
    duplicateItems.imageFacts << second.imageFacts.constFirst();
    duplicateItems.assets << second.assets;
    QVERIFY(!duplicateItems.isValid());
    QVERIFY(!access.db()->publishPrivacyPortableImport(duplicateItems));

    QList<PrivacyCategory> categories;
    QVERIFY(access.db()->getPrivacyCategories(&categories));
    QVERIFY(categories.isEmpty());

}

QTEST_GUILESS_MAIN(PrivacyPortableImportCommitTest)

#include "privacyportableimportcommit_utest.moc"
