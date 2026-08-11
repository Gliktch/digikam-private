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
#include "privacyportableimport.h"

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

    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { first, second }),
            PrivacyPassword::fromUnicode(QLatin1String("import-secret")));
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

    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { spec }),
            PrivacyPassword::fromUnicode(QLatin1String("wrong-secret")));
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

    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { first, second }),
            PrivacyPassword::fromUnicode(QLatin1String("import-secret")));
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

    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { first, second }),
            PrivacyPassword::fromUnicode(QLatin1String("import-secret")));
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

    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            strong,
            PrivacyPassword::fromUnicode(QLatin1String("secret")));
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

    const PrivacyPortableImportAuthenticationResult result =
        PrivacyPortableImportAuthenticator::authenticateCasual(
            makeGroup(directory.path(), { spec }),
            PrivacyPassword::fromUnicode(QLatin1String("import-secret")),
            []() { return true; });
    QVERIFY(!result.succeeded());
    QCOMPARE(result.status,
             PrivacyPortableImportAuthenticationStatus::Cancelled);
}

QTEST_GUILESS_MAIN(PrivacyPortableImportTest)

#include "privacyportableimport_utest.moc"
