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
#include <QUuid>

// Local includes

#include "privacycasualarchive.h"
#include "privacyassetinventory.h"
#include "privacyportableimport.h"
#include "privacystrongrecoverymanifest.h"

using namespace Digikam;

namespace
{

const QString RecoveryA = QLatin1String("aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa");
const QString CategoryA = QLatin1String("bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb");
const QString CategoryB = QLatin1String("cccccccc-cccc-4ccc-8ccc-cccccccccccc");

bool writeFile(const QString& path, const QByteArray& bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(bytes) == bytes.size()) && file.flush();
}

struct ArchiveSpec
{
    QString relativePath;
    QString categoryUuid;
    QString containerUuid;
    QString itemUuid;
    QByteArray payload;
};

bool createArchive(const QString& root, const ArchiveSpec& spec)
{
    const QString sourcePath = root + QLatin1String("/.source-") +
                               QUuid::createUuid().toString(
                                   QUuid::WithoutBraces);

    if (!writeFile(sourcePath, spec.payload))
    {
        return false;
    }

    const QString archivePath = QDir(root).filePath(spec.relativePath);

    if (!QDir().mkpath(QFileInfo(archivePath).absolutePath()))
    {
        return false;
    }

    const QString proxyRelativePath =
        spec.relativePath.left(
            spec.relativePath.size() -
            QStringLiteral(".digikam-private.zip").size());
    const QString originalName = QFileInfo(proxyRelativePath).fileName();
    PrivacyCasualArchiveMember member;
    member.sourcePath = sourcePath;
    member.publicRelativePath = proxyRelativePath;
    member.originalName = originalName;
    member.role = PrivacyAsset::PrimaryMediaRole;
    member.ordinal = 0;
    member.protectedRelativePath =
        PrivacyCasualArchiveEngine::expectedMemberPath(
            member.role, member.ordinal, originalName);
    member.originalCreationDate = QDateTime::currentDateTimeUtc();
    member.originalModificationDate = QDateTime::currentDateTimeUtc();
    member.expectedSize = spec.payload.size();
    member.expectedSha256 =
        QCryptographicHash::hash(spec.payload, QCryptographicHash::Sha256);

    PrivacyCasualArchiveRequest request;
    request.finalArchivePath = archivePath;
    request.categoryUuid = spec.categoryUuid;
    request.containerUuid = spec.containerUuid;
    request.itemUuid = spec.itemUuid;
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

PrivacyPortableDiscoveryGroup makeGroup(const QString& root,
                                        const QList<ArchiveSpec>& specs)
{
    PrivacyPortableDiscoveryGroup group;
    group.recoverySetUuid = RecoveryA;
    group.backend = PrivacyBackend::Casual;
    group.rootCount = 1;

    for (const ArchiveSpec& spec : specs)
    {
        PrivacyPortableCasualArchiveCandidate candidate;
        candidate.rootPath = root;
        candidate.absolutePath = QDir(root).filePath(spec.relativePath);
        candidate.relativePath = spec.relativePath;
        candidate.proxyRelativePath =
            spec.relativePath.left(
                spec.relativePath.size() -
                QStringLiteral(".digikam-private.zip").size());
        candidate.recoverySetUuid = RecoveryA;
        group.casualArchives << candidate;
    }

    return group;
}

class FakePortableStoreInspector final : public PrivacyPortableStoreInspector
{
public:

    struct MountedStore
    {
        QString plaintextRoot;
        QString sentinelCategoryUuid;
        QString sentinelStoreUuid;
    };

    bool inspect(
        const PrivacyPortableStrongStoreCandidate& store,
        const PrivacyPassword&,
        PrivacyPortableStoreInspection* const inspection,
        QString* const error) override
    {
        if (failInspect)
        {
            if (error)
            {
                *error = QStringLiteral("no mounted store available");
            }

            return false;
        }

        for (const MountedStore& mounted : stores)
        {
            if (mounted.sentinelStoreUuid == store.storeUuid)
            {
                inspection->valid = true;
                inspection->plaintextRoot = mounted.plaintextRoot;
                inspection->sentinelCategoryUuid =
                    mounted.sentinelCategoryUuid;
                inspection->sentinelStoreUuid = mounted.sentinelStoreUuid;
                return true;
            }
        }

        if (error)
        {
            *error = QStringLiteral("no matching mounted store");
        }

        return false;
    }

    bool release(const PrivacyPortableStoreInspection&, QString*) override
    {
        return true;
    }

    QList<MountedStore> stores;
    bool failInspect = false;
};

} // namespace

class PrivacyPortableImportTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testAuthenticatesCasualGroup();
    void testWrongPasswordRejected();
    void testInconsistentCategoryRejected();
    void testDuplicateItemRejected();
    void testStrongUnsupported();
    void testCasualLinksMatchingStore();
    void testCasualIgnoresNonMatchingStore();
    void testStrongAuthenticatesWithManifest();
    void testStrongSentinelMismatchRejected();
    void testCancelled();
};

void PrivacyPortableImportTest::testAuthenticatesCasualGroup()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const ArchiveSpec first = {
        QLatin1String("album/photo.jpg.digikam-private.zip"),
        CategoryA,
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("first payload")
    };
    const ArchiveSpec second = {
        QLatin1String("album/clip.mp4.digikam-private.zip"),
        CategoryA,
        QLatin1String("ffffffff-ffff-4fff-8fff-ffffffffffff"),
        QLatin1String("11111111-1111-4111-8111-111111111111"),
        QByteArray("second payload")
    };
    QVERIFY(createArchive(directory.path(), first));
    QVERIFY(createArchive(directory.path(), second));

    FakePortableStoreInspector emptyInspector;
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { first, second }),
            {}, PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
            emptyInspector);
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QVERIFY(result.candidate.isValid());
    QCOMPARE(result.candidate.categoryUuid, CategoryA);
    QCOMPARE(result.candidate.recoverySetUuid, RecoveryA);
    QCOMPARE(result.candidate.backend, PrivacyBackend::Casual);
    QVERIFY(!result.candidate.hasCredential);
    QCOMPARE(result.candidate.items.size(), 2);

    const PrivacyPortableImportItemFact* firstItem = nullptr;

    for (const PrivacyPortableImportItemFact& item : result.candidate.items)
    {
        if (item.itemUuid == first.itemUuid)
        {
            firstItem = &item;
            break;
        }
    }

    QVERIFY(firstItem);
    QCOMPARE(firstItem->containerUuid, first.containerUuid);
    QCOMPARE(firstItem->proxyRelativePath,
             QLatin1String("album/photo.jpg"));
    QCOMPARE(firstItem->assets.size(), 1);
    QCOMPARE(firstItem->assets.constFirst().publicRelativePath,
             QLatin1String("album/photo.jpg"));
    QCOMPARE(firstItem->assets.constFirst().originalName,
             QLatin1String("photo.jpg"));
    QCOMPARE(firstItem->assets.constFirst().originalSha256,
             QCryptographicHash::hash(first.payload,
                                      QCryptographicHash::Sha256));
    QCOMPARE(firstItem->assets.constFirst().originalSize,
             qlonglong(first.payload.size()));
}

void PrivacyPortableImportTest::testWrongPasswordRejected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const ArchiveSpec spec = {
        QLatin1String("photo.jpg.digikam-private.zip"),
        CategoryA,
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("payload")
    };
    QVERIFY(createArchive(directory.path(), spec));

    FakePortableStoreInspector emptyInspector;
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { spec }),
            {}, PrivacyPassword::fromUnicode(QLatin1String("wrong-secret")),
            emptyInspector);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.status,
             PrivacyPortableImportAuthenticationStatus::InvalidPassword);
}

void PrivacyPortableImportTest::testInconsistentCategoryRejected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const ArchiveSpec first = {
        QLatin1String("one.digikam-private.zip"),
        CategoryA,
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("one")
    };
    const ArchiveSpec second = {
        QLatin1String("two.digikam-private.zip"),
        CategoryB,
        QLatin1String("ffffffff-ffff-4fff-8fff-ffffffffffff"),
        QLatin1String("11111111-1111-4111-8111-111111111111"),
        QByteArray("two")
    };
    QVERIFY(createArchive(directory.path(), first));
    QVERIFY(createArchive(directory.path(), second));

    FakePortableStoreInspector emptyInspector;
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { first, second }),
            {}, PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
            emptyInspector);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.status,
             PrivacyPortableImportAuthenticationStatus::InconsistentManifests);
}

void PrivacyPortableImportTest::testDuplicateItemRejected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const ArchiveSpec first = {
        QLatin1String("one.digikam-private.zip"),
        CategoryA,
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("one")
    };
    const ArchiveSpec second = {
        QLatin1String("two.digikam-private.zip"),
        CategoryA,
        QLatin1String("ffffffff-ffff-4fff-8fff-ffffffffffff"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("two")
    };
    QVERIFY(createArchive(directory.path(), first));
    QVERIFY(createArchive(directory.path(), second));

    FakePortableStoreInspector emptyInspector;
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { first, second }),
            {}, PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
            emptyInspector);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.status,
             PrivacyPortableImportAuthenticationStatus::InconsistentManifests);
}

void PrivacyPortableImportTest::testStrongUnsupported()
{
    PrivacyPortableDiscoveryGroup strong;
    strong.recoverySetUuid = RecoveryA;
    strong.backend = PrivacyBackend::Strong;
    strong.rootCount = 1;
    PrivacyPortableStrongStoreCandidate store;
    store.rootPath = QLatin1String("/synthetic");
    store.storeUuid = RecoveryA;
    store.markerPath = QLatin1String("/synthetic/marker");
    store.configAbsolutePath = QLatin1String("/synthetic/config");
    store.cipherRelativePath = QLatin1String("stores/a");
    strong.strongStores << store;

    FakePortableStoreInspector emptyInspector;
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            strong,
            {}, PrivacyPassword::fromUnicode(QLatin1String("secret")),
            emptyInspector);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.status,
             PrivacyPortableImportAuthenticationStatus::UnsupportedBackend);
}

void PrivacyPortableImportTest::testCancelled()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const ArchiveSpec spec = {
        QLatin1String("photo.jpg.digikam-private.zip"),
        CategoryA,
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("payload")
    };
    QVERIFY(createArchive(directory.path(), spec));

    FakePortableStoreInspector emptyInspector;
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { spec }),
            {}, PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
            emptyInspector,
            []() { return true; });
    QVERIFY(!result.succeeded());
    QCOMPARE(result.status,
             PrivacyPortableImportAuthenticationStatus::Cancelled);
}

void PrivacyPortableImportTest::testCasualLinksMatchingStore()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir storeRoot;
    QVERIFY(storeRoot.isValid());
    const ArchiveSpec spec = {
        QLatin1String("photo.jpg.digikam-private.zip"),
        CategoryA,
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("payload")
    };
    QVERIFY(createArchive(directory.path(), spec));

    PrivacyPortableStrongStoreCandidate store;
    store.rootPath = storeRoot.path();
    store.storeUuid =
        QLatin1String("99999999-9999-4999-8999-999999999999");
    store.markerPath = QDir(storeRoot.path()).filePath(
        QLatin1String(".digikam-private/root-marker-v1.json"));
    store.configAbsolutePath = QDir(storeRoot.path()).filePath(
        QLatin1String(".digikam-private/stores/") + store.storeUuid +
        QLatin1String("/gocryptfs.conf"));
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/") +
                               store.storeUuid;
    QVERIFY(QDir().mkpath(QFileInfo(store.configAbsolutePath).absolutePath()));
    QFile config(store.configAbsolutePath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(config.write(QByteArray("opaque store config")),
             qint64(19));
    config.close();
    QVERIFY(QFile::setPermissions(
        store.configAbsolutePath,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner));

    FakePortableStoreInspector inspector;
    inspector.stores << FakePortableStoreInspector::MountedStore{
        storeRoot.path(), CategoryA, store.storeUuid
    };
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { spec }),
            { store },
            PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
            inspector);
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QVERIFY(result.candidate.hasCredential);
    QCOMPARE(result.candidate.storeUuid, store.storeUuid);
    QCOMPARE(result.candidate.managedStoreRootPath, storeRoot.path());
    QCOMPARE(result.candidate.cipherRelativePath, store.cipherRelativePath);
    QCOMPARE(result.candidate.credentialEnvelopeFormat,
             QLatin1String("gocryptfs-config-v2"));
    QCOMPARE(result.candidate.credentialEnvelopeBlob,
             QByteArray("opaque store config"));
    QVERIFY(result.candidate.isValid());
}

void PrivacyPortableImportTest::testCasualIgnoresNonMatchingStore()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QTemporaryDir storeRoot;
    QVERIFY(storeRoot.isValid());
    const ArchiveSpec spec = {
        QLatin1String("photo.jpg.digikam-private.zip"),
        CategoryA,
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd"),
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee"),
        QByteArray("payload")
    };
    QVERIFY(createArchive(directory.path(), spec));

    PrivacyPortableStrongStoreCandidate store;
    store.rootPath = storeRoot.path();
    store.storeUuid =
        QLatin1String("99999999-9999-4999-8999-999999999999");
    store.markerPath = QDir(storeRoot.path()).filePath(
        QLatin1String(".digikam-private/root-marker-v1.json"));
    store.configAbsolutePath = QDir(storeRoot.path()).filePath(
        QLatin1String(".digikam-private/stores/") + store.storeUuid +
        QLatin1String("/gocryptfs.conf"));
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/") +
                               store.storeUuid;
    QVERIFY(QDir().mkpath(QFileInfo(store.configAbsolutePath).absolutePath()));
    QFile config(store.configAbsolutePath);
    QVERIFY(config.open(QIODevice::WriteOnly | QIODevice::Truncate));
    config.write(QByteArray("opaque store config"));
    config.close();
    QVERIFY(QFile::setPermissions(
        store.configAbsolutePath,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner));

    FakePortableStoreInspector inspector;
    inspector.stores << FakePortableStoreInspector::MountedStore{
        storeRoot.path(), CategoryB, store.storeUuid
    };
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { spec }),
            { store },
            PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
            inspector);
    QVERIFY(result.succeeded());
    QVERIFY(!result.candidate.hasCredential);
    QVERIFY(result.candidate.storeUuid.isEmpty());
}

void PrivacyPortableImportTest::testStrongAuthenticatesWithManifest()
{
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    const QString containerUuid =
        QLatin1String("dddddddd-dddd-4ddd-8ddd-dddddddddddd");
    const QString itemUuid =
        QLatin1String("eeeeeeee-eeee-4eee-8eee-eeeeeeeeeeee");
    const QString storeUuid =
        QLatin1String("99999999-9999-4999-8999-999999999999");
    const QByteArray primaryBytes("strong primary bytes");
    const QByteArray sidecarBytes("strong sidecar bytes");
    const QString objectDir = QDir(vault.path()).filePath(
        QLatin1String("originals/") + containerUuid);
    QVERIFY(QDir().mkpath(objectDir));
    QFile primaryFile(QDir(objectDir).filePath(QLatin1String("0-photo.jpg")));
    QVERIFY(primaryFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(primaryFile.write(primaryBytes), qint64(primaryBytes.size()));
    primaryFile.close();
    QFile sidecarFile(QDir(objectDir).filePath(QLatin1String("0-photo.xmp")));
    QVERIFY(sidecarFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(sidecarFile.write(sidecarBytes), qint64(sidecarBytes.size()));
    sidecarFile.close();

    PrivacyStrongRecoveryManifest manifest;
    manifest.categoryUuid = CategoryA;
    manifest.categoryName = QLatin1String("Imported Strong");
    manifest.presentationMode =
        static_cast<int>(PrivacyPresentationMode::Generic);
    manifest.unlockedThumbnailMode =
        static_cast<int>(PrivacyUnlockedThumbnailMode::FocusedClear);
    manifest.tagVisibilityMode =
        static_cast<int>(PrivacyTagVisibilityMode::UnlockedOnly);
    manifest.currentCredentialGeneration = 1;
    manifest.storeUuid = storeUuid;
    PrivacyStrongRecoveryItem recoveryItem;
    recoveryItem.itemUuid = itemUuid;
    recoveryItem.containerUuid = containerUuid;
    recoveryItem.generation = 1;
    PrivacyStrongRecoveryMember primary;
    primary.vaultRelativePath =
        QLatin1String("originals/") + containerUuid +
        QLatin1String("/0-photo.jpg");
    primary.publicRelativePath = QLatin1String("album/photo.jpg");
    primary.originalName = QLatin1String("photo.jpg");
    primary.role = PrivacyAsset::PrimaryMediaRole;
    primary.ordinal = 0;
    primary.hashAlgorithm = QLatin1String("sha256");
    primary.sha256Hex = QString::fromLatin1(
        QCryptographicHash::hash(primaryBytes,
                                 QCryptographicHash::Sha256).toHex());
    primary.size = primaryBytes.size();
    primary.creationTimeUtc = QDateTime::currentDateTimeUtc();
    primary.modificationTimeUtc = QDateTime::currentDateTimeUtc();
    primary.unixMode = 0100640;
    PrivacyStrongRecoveryMember sidecar;
    sidecar.vaultRelativePath =
        QLatin1String("originals/") + containerUuid +
        QLatin1String("/0-photo.xmp");
    sidecar.publicRelativePath = QLatin1String("album/photo.xmp");
    sidecar.originalName = QLatin1String("photo.xmp");
    sidecar.role = static_cast<int>(PrivacyInventoryAssetRole::XmpSidecar);
    sidecar.ordinal = 0;
    sidecar.hashAlgorithm = QLatin1String("sha256");
    sidecar.sha256Hex = QString::fromLatin1(
        QCryptographicHash::hash(sidecarBytes,
                                 QCryptographicHash::Sha256).toHex());
    sidecar.size = sidecarBytes.size();
    sidecar.creationTimeUtc = QDateTime::currentDateTimeUtc();
    sidecar.modificationTimeUtc = QDateTime::currentDateTimeUtc();
    sidecar.unixMode = 0100600;
    recoveryItem.members << primary << sidecar;
    manifest.items << recoveryItem;
    QVERIFY(manifest.isValid());

    const QString manifestPath = QDir(vault.path()).filePath(
        PrivacyStrongRecoveryManifestCodec::relativePath());
    QVERIFY(QDir().mkpath(QFileInfo(manifestPath).absolutePath()));
    const QByteArray manifestBytes =
        PrivacyStrongRecoveryManifestCodec::encode(manifest);
    QVERIFY(!manifestBytes.isEmpty());
    QFile manifestFile(manifestPath);
    QVERIFY(manifestFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(manifestFile.write(manifestBytes),
             qint64(manifestBytes.size()));
    manifestFile.close();

    PrivacyPortableDiscoveryGroup group;
    group.recoverySetUuid = storeUuid;
    group.backend = PrivacyBackend::Strong;
    group.rootCount = 1;
    PrivacyPortableStrongStoreCandidate store;
    store.rootPath = vault.path();
    store.storeUuid = storeUuid;
    store.markerPath = QDir(vault.path()).filePath(
        QLatin1String(".digikam-private/root-marker-v1.json"));
    store.configAbsolutePath = QDir(vault.path()).filePath(
        QLatin1String(".digikam-private/stores/") + storeUuid +
        QLatin1String("/gocryptfs.conf"));
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/") +
                               storeUuid;
    QVERIFY(QDir().mkpath(QFileInfo(store.configAbsolutePath).absolutePath()));
    QFile strongConfig(store.configAbsolutePath);
    QVERIFY(strongConfig.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(strongConfig.write(QByteArray("opaque strong config")),
             qint64(20));
    strongConfig.close();
    QVERIFY(QFile::setPermissions(
        store.configAbsolutePath,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner));
    group.strongStores << store;

    FakePortableStoreInspector inspector;
    inspector.stores << FakePortableStoreInspector::MountedStore{
        vault.path(), CategoryA, storeUuid
    };
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateStrong(
            group,
            PrivacyPassword::fromUnicode(QLatin1String("strong-secret")),
            inspector);
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QCOMPARE(result.candidate.backend, PrivacyBackend::Strong);
    QCOMPARE(result.candidate.categoryUuid, CategoryA);
    QCOMPARE(result.candidate.categoryName, QLatin1String("Imported Strong"));
    QVERIFY(result.candidate.hasCredential);
    QCOMPARE(result.candidate.items.size(), 1);
    QCOMPARE(result.candidate.items.constFirst().itemUuid, itemUuid);
    QCOMPARE(result.candidate.items.constFirst().containerUuid,
             containerUuid);
    QCOMPARE(result.candidate.items.constFirst().proxyRelativePath,
             QLatin1String("album/photo.jpg"));
    QCOMPARE(result.candidate.items.constFirst().assets.size(), 2);
    QVERIFY(result.candidate.isValid());
}

void PrivacyPortableImportTest::testStrongSentinelMismatchRejected()
{
    QTemporaryDir vault;
    QVERIFY(vault.isValid());
    PrivacyPortableDiscoveryGroup group;
    group.recoverySetUuid =
        QLatin1String("99999999-9999-4999-8999-999999999999");
    group.backend = PrivacyBackend::Strong;
    group.rootCount = 1;
    PrivacyPortableStrongStoreCandidate store;
    store.rootPath = vault.path();
    store.storeUuid = group.recoverySetUuid;
    store.markerPath = QDir(vault.path()).filePath(QLatin1String("marker"));
    store.configAbsolutePath = QDir(vault.path()).filePath(
        QLatin1String("gocryptfs.conf"));
    store.cipherRelativePath = QLatin1String(".digikam-private/stores/x");
    group.strongStores << store;

    FakePortableStoreInspector inspector;
    inspector.stores << FakePortableStoreInspector::MountedStore{
        vault.path(), CategoryA,
        QLatin1String("88888888-8888-4888-8888-888888888888")
    };
    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateStrong(
            group,
            PrivacyPassword::fromUnicode(QLatin1String("strong-secret")),
            inspector);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.status,
             PrivacyPortableImportAuthenticationStatus::InconsistentManifests);
}

QTEST_GUILESS_MAIN(PrivacyPortableImportTest)

#include "privacyportableimport_utest.moc"
