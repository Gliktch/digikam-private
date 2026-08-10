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

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSet>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTimeZone>

// libzip includes

#include <zip.h>

// Local includes

#include "privacycasualarchive.h"
#include "privacycasualoriginalreader.h"

using namespace Digikam;

namespace
{

const QString CategoryUuid  = QLatin1String("11111111-1111-4111-8111-111111111111");
const QString ContainerUuid = QLatin1String("22222222-2222-4222-8222-222222222222");
const QString ItemUuid      = QLatin1String("33333333-3333-4333-8333-333333333333");
const QString RootUuid      = QLatin1String("44444444-4444-4444-8444-444444444444");

bool writeBytes(const QString& path, const QByteArray& bytes)
{
    QFile file(path);

    return (file.open(QIODevice::WriteOnly | QIODevice::NewOnly) &&
            (file.write(bytes) == bytes.size()) && file.flush());
}

QByteArray fileHash(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    if (!hash.addData(&file))
    {
        return {};
    }

    return hash.result();
}

PrivacyCasualArchiveMember makeMember(const QString& sourcePath,
                                      const QString& originalName,
                                      int role, int ordinal,
                                      const QByteArray& attributes = {})
{
    PrivacyCasualArchiveMember member;
    member.sourcePath            = sourcePath;
    member.originalName          = originalName;
    member.role                  = role;
    member.ordinal               = ordinal;
    member.protectedRelativePath =
        PrivacyCasualArchiveEngine::expectedMemberPath(role, ordinal, originalName);
    member.originalCreationDate =
        QDateTime::fromMSecsSinceEpoch(946684800123LL, QTimeZone::UTC);
    member.originalModificationDate =
        QDateTime::fromMSecsSinceEpoch(1700000000456LL, QTimeZone::UTC);
    member.portableAttributes = attributes;

    return member;
}

PrivacyCasualArchiveRequest makeRequest(
    const QString& finalPath, const QList<PrivacyCasualArchiveMember>& members)
{
    PrivacyCasualArchiveRequest request;
    request.finalArchivePath = finalPath;
    request.categoryUuid     = CategoryUuid;
    request.containerUuid    = ContainerUuid;
    request.itemUuid         = ItemUuid;
    request.members          = members;

    return request;
}

PrivacyRepositorySnapshot makeOriginalSnapshot(
    const QString& rootPath,
    const QString& publicRelativePath,
    const PrivacyCasualArchiveMember& member,
    qlonglong archiveSize,
    const QByteArray& archiveHash,
    const QByteArray& originalHash)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    PrivacyCategory category;
    category.uuid = CategoryUuid;
    category.name = QLatin1String("Synthetic");
    category.backend = PrivacyBackend::Casual;
    category.presentationMode = PrivacyPresentationMode::Generic;
    category.unlockedThumbnailMode = PrivacyUnlockedThumbnailMode::FocusedClear;
    category.tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt = now;

    PrivacyStorageRoot root;
    root.uuid = RootUuid;
    root.kind = PrivacyStorageRootKind::AlbumRoot;
    root.albumRootId = 1;
    root.configuredPath = rootPath;
    root.identityVersion = 1;
    root.identityData = QByteArray("synthetic-root");
    root.createdAt = now;

    PrivacyItem item;
    item.imageId = 42;
    item.uuid = ItemUuid;
    item.categoryUuid = CategoryUuid;
    item.originalHash = QString::fromLatin1(originalHash.toHex());
    item.originalSize = member.expectedSize;
    item.expectedProxyHash = QString(64, QLatin1Char('a'));
    item.expectedProxySize = 16;
    item.generation = 3;

    PrivacyContainer container;
    container.uuid = ContainerUuid;
    container.itemUuid = ItemUuid;
    container.kind = PrivacyContainerKind::CasualArchive;
    container.rootUuid = RootUuid;
    container.objectRelativePath = publicRelativePath +
                                   QLatin1String(".digikam-private.zip");
    container.protectedSize = archiveSize;
    container.protectedHashAlgorithm = QLatin1String("sha256");
    container.protectedHash = QString::fromLatin1(archiveHash.toHex());
    container.formatVersion = 1;
    container.credentialGeneration = 1;
    container.state = PrivacyContainerState::Verified;
    container.createdAt = now;
    container.updatedAt = now;

    PrivacyAsset asset;
    asset.itemUuid = ItemUuid;
    asset.role = PrivacyAsset::PrimaryMediaRole;
    asset.ordinal = 0;
    asset.originalName = member.originalName;
    asset.publicRootUuid = RootUuid;
    asset.publicRelativePath = publicRelativePath;
    asset.containerUuid = ContainerUuid;
    asset.protectedRelativePath = member.protectedRelativePath;
    asset.hashAlgorithm = QLatin1String("sha256");
    asset.originalHash = item.originalHash;
    asset.originalSize = member.expectedSize;
    asset.proxyHashAlgorithm = QLatin1String("sha256");
    asset.proxyHash = item.expectedProxyHash;
    asset.proxySize = item.expectedProxySize;
    asset.proxyPresentationVersion = item.presentationVersion;
    asset.proxyGeneration = item.generation;

    PrivacyRepositorySnapshot snapshot;
    snapshot.categories << category;
    snapshot.storageRoots << root;
    snapshot.items << item;
    snapshot.containers << container;
    snapshot.assets << asset;
    return snapshot;
}

bool readEncryptedMember(zip_t* archive, const QString& name,
                         const PrivacyPassword& password, QByteArray* output)
{
    if (!archive || !output)
    {
        return false;
    }

    const QByteArray encodedName = name.toUtf8();
    const zip_int64_t index = zip_name_locate(archive, encodedName.constData(),
                                               ZIP_FL_ENC_UTF_8);

    if (index < 0)
    {
        return false;
    }

    QByteArray bytes;
    bool readOkay = false;
    const bool invoked = password.withUtf8CString(
        [&](const char* secret)
        {
            zip_file_t* file = zip_fopen_index_encrypted(
                archive, static_cast<zip_uint64_t>(index), 0, secret);

            if (!file)
            {
                return false;
            }

            char buffer[4096];

            while (true)
            {
                const zip_int64_t count = zip_fread(file, buffer, sizeof(buffer));

                if (count < 0)
                {
                    break;
                }

                if (count == 0)
                {
                    readOkay = true;
                    break;
                }

                bytes.append(buffer, static_cast<qsizetype>(count));
            }

            if (zip_fclose(file) != 0)
            {
                readOkay = false;
            }

            return readOkay;
        });

    if (invoked && readOkay)
    {
        *output = bytes;
    }

    return (invoked && readOkay);
}

bool inspectArchive(const QString& archivePath,
                    const PrivacyPassword& password,
                    const QHash<QString, QByteArray>& expectedPayloads,
                    QJsonObject* manifestObject = nullptr)
{
    int error = 0;
    const QByteArray encodedPath = QFile::encodeName(archivePath);
    zip_t* archive = zip_open(encodedPath.constData(), ZIP_RDONLY | ZIP_CHECKCONS,
                              &error);

    if (!archive)
    {
        return false;
    }

    const zip_int64_t entryCount = zip_get_num_entries(archive, 0);
    bool okay = (entryCount == (expectedPayloads.size() + 1));
    QSet<QString> names;

    for (zip_uint64_t index = 0;
         okay && (index < static_cast<zip_uint64_t>(entryCount)); ++index)
    {
        zip_stat_t stat;
        zip_stat_init(&stat);
        const char* rawName = zip_get_name(archive, index, ZIP_FL_ENC_STRICT);

        okay = (rawName &&
                (zip_stat_index(archive, index, ZIP_FL_ENC_STRICT, &stat) == 0) &&
                (stat.valid & ZIP_STAT_COMP_METHOD) &&
                (stat.valid & ZIP_STAT_ENCRYPTION_METHOD) &&
                (stat.comp_method == ZIP_CM_STORE) &&
                (stat.encryption_method == ZIP_EM_TRAD_PKWARE));

        if (okay)
        {
            names.insert(QString::fromUtf8(rawName));
        }
    }

    QByteArray manifestBytes;
    okay = okay && names.contains(PrivacyCasualArchiveEngine::manifestMemberName()) &&
           readEncryptedMember(archive,
                               PrivacyCasualArchiveEngine::manifestMemberName(),
                               password, &manifestBytes);

    for (auto it = expectedPayloads.cbegin();
         okay && (it != expectedPayloads.cend()); ++it)
    {
        QByteArray actual;
        okay = names.contains(it.key()) &&
               readEncryptedMember(archive, it.key(), password, &actual) &&
               (actual == it.value());
    }

    const QJsonDocument manifest = QJsonDocument::fromJson(manifestBytes);
    okay = okay && manifest.isObject();

    if (okay && manifestObject)
    {
        *manifestObject = manifest.object();
    }

    zip_discard(archive);

    return okay;
}

} // namespace

class PrivacyCasualArchiveTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCapabilitiesAndArchivePolicy();
    void testStandardToolRecovery();
    void testResumeVerifiedStageFromJournalFacts();
    void testPublicationConflictAndReplacement();
    void testRejectsUnsafeInputsAndCancellation();
    void testTamperedStageCannotPublishOrDiscard();
    void testPreparedOriginalReader();
};

void PrivacyCasualArchiveTest::testCapabilitiesAndArchivePolicy()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QByteArray imageBytes("synthetic-image\0bytes", 21);
    const QByteArray videoBytes("synthetic-video-keyframe-bytes");
    const QString imagePath = directory.filePath(QLatin1String("image-source.bin"));
    const QString videoPath = directory.filePath(QLatin1String("video-source.bin"));
    QVERIFY(writeBytes(imagePath, imageBytes));
    QVERIFY(writeBytes(videoPath, videoBytes));
    QVERIFY(QFile::setPermissions(imagePath, QFileDevice::ReadOwner |
                                             QFileDevice::WriteOwner));
    QVERIFY(QFile::setPermissions(videoPath, QFileDevice::ReadOwner |
                                             QFileDevice::WriteOwner |
                                             QFileDevice::ReadGroup));

    const QString imageName = QString::fromUtf8("R\xC3\xA9sum\xC3\xA9.jpg");
    const QString videoName = QLatin1String("clip.mp4");
    const PrivacyCasualArchiveMember image =
        makeMember(imagePath, imageName, 1, 0, QByteArray("attrs\0image", 11));
    const PrivacyCasualArchiveMember video =
        makeMember(videoPath, videoName, 2, 1, QByteArray("attrs-video"));
    const QString finalPath = directory.filePath(
        QLatin1String("original.jpg.digikam-private.zip"));
    const PrivacyCasualArchiveRequest request = makeRequest(finalPath, { video, image });
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QString::fromUtf8("p\x61\xCC\x88ss"));
    const PrivacyPassword wrongPassword = PrivacyPassword::fromUnicode(
        QLatin1String("definitely-wrong"));
    PrivacyCasualArchiveEngine engine;
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;

    QVERIFY(engine.checkCapabilities(&error));
    QCOMPARE(error, PrivacyCasualArchiveError::None);

    auto stage = engine.stageArchive(request, password, {}, &error);
    QVERIFY(stage.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::None);
    QVERIFY(QFileInfo::exists(stage.stagingPath()));
    QVERIFY(!QFileInfo::exists(finalPath));
    QCOMPARE(QFileInfo(stage.stagingPath()).permissions() &
                 (QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                  QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                  QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                  QFileDevice::ReadOther | QFileDevice::WriteOther |
                  QFileDevice::ExeOther),
             QFileDevice::Permissions(QFileDevice::ReadOwner |
                                      QFileDevice::WriteOwner));
    QVERIFY(engine.verifyStagedArchive(stage, password, {}, &error));
    QCOMPARE(error, PrivacyCasualArchiveError::None);
    QVERIFY(!engine.verifyStagedArchive(stage, wrongPassword, {}, &error));
    QVERIFY(error != PrivacyCasualArchiveError::None);

    QHash<QString, QByteArray> expected;
    expected.insert(image.protectedRelativePath, imageBytes);
    expected.insert(video.protectedRelativePath, videoBytes);
    QJsonObject manifest;
    QVERIFY(inspectArchive(stage.stagingPath(), password, expected, &manifest));
    QCOMPARE(manifest.value(QLatin1String("format")).toString(),
             QLatin1String("digikam-private-casual"));
    QCOMPARE(manifest.value(QLatin1String("formatVersion")).toInt(), 1);
    QCOMPARE(manifest.value(QLatin1String("passwordEncoding")).toString(),
             QLatin1String("utf8-nfc-v1"));
    QCOMPARE(manifest.value(QLatin1String("categoryUuid")).toString(), CategoryUuid);
    QCOMPARE(manifest.value(QLatin1String("containerUuid")).toString(), ContainerUuid);
    QCOMPARE(manifest.value(QLatin1String("itemUuid")).toString(), ItemUuid);

    const QJsonArray members = manifest.value(QLatin1String("members")).toArray();
    QCOMPARE(members.size(), 2);
    QCOMPARE(members.at(0).toObject().value(QLatin1String("path")).toString(),
             image.protectedRelativePath);
    QCOMPARE(members.at(0).toObject().value(
                 QLatin1String("creationTimeUtcMs")).toString(),
             QLatin1String("946684800123"));
    QCOMPARE(members.at(0).toObject().value(
                 QLatin1String("modificationTimeUtcMs")).toString(),
             QLatin1String("1700000000456"));
    QCOMPARE(members.at(0).toObject().value(QLatin1String("portableAttributes"))
                 .toObject().value(QLatin1String("data")).toString().toLatin1(),
             image.portableAttributes.toBase64());

    const QByteArray stagedHash = stage.archiveSha256();
    QVERIFY(engine.publishNew(&stage, &error));
    QCOMPARE(error, PrivacyCasualArchiveError::None);
    QVERIFY(!stage.isValid());
    QVERIFY(QFileInfo::exists(finalPath));
    QCOMPARE(fileHash(finalPath), stagedHash);
    QVERIFY(inspectArchive(finalPath, password, expected));
}

void PrivacyCasualArchiveTest::testStandardToolRecovery()
{
    const QString unzip = QStandardPaths::findExecutable(QLatin1String("unzip"));

    if (unzip.isEmpty())
    {
        QSKIP("Info-ZIP unzip is unavailable");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QByteArray payload("independent recovery payload\0bytes", 34);
    const QString sourcePath = directory.filePath(QLatin1String("source.bin"));
    const QString archivePath = directory.filePath(
        QLatin1String("holiday.jpg.digikam-private.zip"));
    const QString restorePath = directory.filePath(QLatin1String("restored"));
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QLatin1String("external-recovery-passphrase"));
    const PrivacyCasualArchiveMember member = makeMember(
        sourcePath, QLatin1String("holiday.jpg"), PrivacyAsset::PrimaryMediaRole, 0);
    PrivacyCasualArchiveEngine engine;
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;

    QVERIFY(writeBytes(sourcePath, payload));
    auto stage = engine.stageArchive(makeRequest(archivePath, { member }),
                                     password, {}, &error);
    QVERIFY2(stage.isValid(), qPrintable(QString::number(static_cast<int>(error))));
    QVERIFY(engine.publishNew(&stage, &error));
    QVERIFY(QDir().mkpath(restorePath));

    QProcess process;
    process.setProgram(unzip);
    process.setArguments({ QLatin1String("-qq"), QLatin1String("-P"),
                           QLatin1String("external-recovery-passphrase"),
                           archivePath, QLatin1String("-d"), restorePath });
    process.start();
    QVERIFY(process.waitForFinished(30000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), 0);

    QFile restored(QDir(restorePath).filePath(member.protectedRelativePath));
    QVERIFY(restored.open(QIODevice::ReadOnly));
    QCOMPARE(restored.readAll(), payload);
    QVERIFY(QFileInfo::exists(QDir(restorePath).filePath(
        PrivacyCasualArchiveEngine::manifestMemberName())));
}

void PrivacyCasualArchiveTest::testPreparedOriginalReader()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QByteArray payload("verified-private-original\0bytes", 31);
    const QByteArray originalHash = QCryptographicHash::hash(
        payload, QCryptographicHash::Sha256);
    const QString sourcePath = directory.filePath(QLatin1String("source.jpg"));
    const QString publicRelativePath = QLatin1String("album/source.jpg");
    QVERIFY(QDir().mkpath(directory.filePath(QLatin1String("album"))));
    QVERIFY(writeBytes(sourcePath, payload));

    PrivacyCasualArchiveMember member = makeMember(
        sourcePath, QLatin1String("source.jpg"), PrivacyAsset::PrimaryMediaRole, 0);
    member.expectedSize = payload.size();
    member.expectedSha256 = originalHash;
    const QString archivePath = directory.filePath(
        publicRelativePath + QLatin1String(".digikam-private.zip"));
    const PrivacyPassword password = PrivacyPassword::fromUnicode(
        QLatin1String("reader-passphrase"));
    PrivacyCasualArchiveEngine engine;
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
    auto stage = engine.stageArchive(makeRequest(archivePath, { member }),
                                     password, {}, &error);
    QVERIFY2(stage.isValid(), qPrintable(QString::number(static_cast<int>(error))));
    const qlonglong archiveSize = stage.archiveSize();
    const QByteArray archiveHash = stage.archiveSha256();
    QVERIFY(engine.publishNew(&stage, &error));

    const PrivacyRepositorySnapshot snapshot = makeOriginalSnapshot(
        directory.path(), publicRelativePath, member,
        archiveSize, archiveHash, originalHash);
    const QString logicalPath = directory.filePath(publicRelativePath);
    PrivacyCasualOriginalReader reader;
    PrivacyCasualOriginalSource prepared;
    QVERIFY(reader.prepare(snapshot, 42, logicalPath, &prepared));
    QVERIFY(prepared.isValid());
    QCOMPARE(prepared.itemUuid, ItemUuid);
    QCOMPARE(prepared.itemGeneration, 3);
    QCOMPARE(prepared.restore.archivePath, archivePath);

    QByteArray restored;
    QBuffer destination(&restored);
    QVERIFY(destination.open(QIODevice::WriteOnly));
    QVERIFY(reader.restore(prepared, password, &destination, &error));
    QCOMPARE(restored, payload);

    PrivacyCasualOriginalSource rejected;
    QVERIFY(!reader.prepare(snapshot, 42,
                            directory.filePath(QLatin1String("other.jpg")),
                            &rejected));
    PrivacyRepositorySnapshot busy = snapshot;
    PrivacyTransaction active;
    active.uuid = QLatin1String("55555555-5555-4555-8555-555555555555");
    active.categoryUuid = CategoryUuid;
    active.itemUuid = ItemUuid;
    active.type = PrivacyTransactionType::ExternalCheckout;
    active.state = PrivacyTransactionState::Created;
    active.generation = 0;
    active.payloadFormatVersion = 1;
    active.createdAt = QDateTime::currentDateTimeUtc();
    active.updatedAt = active.createdAt;
    busy.transactions << active;
    QVERIFY(!reader.prepare(busy, 42, logicalPath, &rejected));
}

void PrivacyCasualArchiveTest::testResumeVerifiedStageFromJournalFacts()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QByteArray payload("journal-resume-payload");
    const QString sourcePath = directory.filePath(QLatin1String("source.bin"));
    QVERIFY(writeBytes(sourcePath, payload));
    const QString finalPath = directory.filePath(
        QLatin1String("asset.ext.digikam-private.zip"));
    PrivacyCasualArchiveEngine engine;
    const PrivacyPassword password = PrivacyPassword::fromUnicode(QLatin1String("passphrase"));
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
    QString stagingPath;
    qlonglong archiveSize = -1;
    QByteArray archiveHash;

    {
        auto originalStage = engine.stageArchive(
            makeRequest(finalPath,
                        { makeMember(sourcePath, QLatin1String("asset.ext"), 1, 0) }),
            password, {}, &error);
        QVERIFY(originalStage.isValid());
        stagingPath = originalStage.stagingPath();
        archiveSize = originalStage.archiveSize();
        archiveHash = originalStage.archiveSha256();
    }

    QVERIFY(QFileInfo::exists(stagingPath));
    auto badHash = engine.resumeStagedArchive(
        stagingPath, finalPath, archiveSize, QByteArray(32, '\x11'),
        password, {}, &error);
    QVERIFY(!badHash.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::HashMismatch);

    const PrivacyPassword wrongPassword = PrivacyPassword::fromUnicode(
        QLatin1String("wrong-passphrase"));
    auto wrongSecret = engine.resumeStagedArchive(
        stagingPath, finalPath, archiveSize, archiveHash,
        wrongPassword, {}, &error);
    QVERIFY(!wrongSecret.isValid());
    QVERIFY(error != PrivacyCasualArchiveError::None);

    auto resumed = engine.resumeStagedArchive(
        stagingPath, finalPath, archiveSize, archiveHash,
        password, {}, &error);
    QVERIFY(resumed.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::None);
    QVERIFY(engine.publishNew(&resumed, &error));
    QCOMPARE(fileHash(finalPath), archiveHash);
}

void PrivacyCasualArchiveTest::testPublicationConflictAndReplacement()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString sourcePath = directory.filePath(QLatin1String("source.bin"));
    const QString replacementPath = directory.filePath(QLatin1String("replacement.bin"));
    QVERIFY(writeBytes(sourcePath, QByteArray("first archive bytes")));
    QVERIFY(writeBytes(replacementPath, QByteArray("replacement archive bytes")));
    const QString finalPath = directory.filePath(
        QLatin1String("asset.ext.digikam-private.zip"));
    PrivacyCasualArchiveEngine engine;
    const PrivacyPassword password = PrivacyPassword::fromUnicode(QLatin1String("passphrase"));
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;

    const auto firstRequest = makeRequest(
        finalPath, { makeMember(sourcePath, QLatin1String("asset.ext"), 1, 0) });
    auto first = engine.stageArchive(firstRequest, password, {}, &error);
    QVERIFY(first.isValid());
    const QByteArray firstHash = first.archiveSha256();
    QVERIFY(engine.publishNew(&first, &error));
    QCOMPARE(fileHash(finalPath), firstHash);

    auto conflict = engine.stageArchive(firstRequest, password, {}, &error);
    QVERIFY(conflict.isValid());
    QVERIFY(!engine.publishNew(&conflict, &error));
    QCOMPARE(error, PrivacyCasualArchiveError::PublicationConflict);
    QCOMPARE(fileHash(finalPath), firstHash);
    QVERIFY(engine.discardStaged(&conflict, &error));

    const auto replacementRequest = makeRequest(
        finalPath, { makeMember(replacementPath, QLatin1String("asset.ext"), 1, 0) });
    auto replacement = engine.stageArchive(replacementRequest, password, {}, &error);
    QVERIFY(replacement.isValid());
    const QByteArray replacementHash = replacement.archiveSha256();
    QVERIFY(!engine.publishReplacement(&replacement, QByteArray(32, '\x7f'), &error));
    QCOMPARE(error, PrivacyCasualArchiveError::ExistingArchiveMismatch);
    QCOMPARE(fileHash(finalPath), firstHash);
    QVERIFY(replacement.isValid());
    QVERIFY(engine.publishReplacement(&replacement, firstHash, &error));
    QCOMPARE(error, PrivacyCasualArchiveError::None);
    QVERIFY(!replacement.isValid());
    QCOMPARE(fileHash(finalPath), replacementHash);
}

void PrivacyCasualArchiveTest::testRejectsUnsafeInputsAndCancellation()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString sourcePath = directory.filePath(QLatin1String("source.bin"));
    QVERIFY(writeBytes(sourcePath, QByteArray("source")));
    const QString finalPath = directory.filePath(
        QLatin1String("asset.ext.digikam-private.zip"));
    PrivacyCasualArchiveEngine engine;
    const PrivacyPassword password = PrivacyPassword::fromUnicode(QLatin1String("passphrase"));
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;

    auto invalid = makeMember(sourcePath, QLatin1String("asset.ext"), 1, 0);
    invalid.protectedRelativePath = QLatin1String("../asset.ext");
    auto stage = engine.stageArchive(makeRequest(finalPath, { invalid }),
                                     password, {}, &error);
    QVERIFY(!stage.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::InvalidMemberName);

    QString invalidUnicodeName = QLatin1String("asset-");
    invalidUnicodeName.append(QChar(0xd800));
    invalidUnicodeName.append(QLatin1String(".ext"));
    auto invalidUnicode = makeMember(sourcePath, invalidUnicodeName, 1, 0);
    auto invalidUnicodeStage = engine.stageArchive(
        makeRequest(finalPath, { invalidUnicode }), password, {}, &error);
    QVERIFY(!invalidUnicodeStage.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::InvalidMemberName);

    const auto upper = makeMember(sourcePath, QLatin1String("Asset.ext"), 1, 0);
    const auto lower = makeMember(sourcePath, QLatin1String("asset.ext"), 1, 0);
    auto duplicate = engine.stageArchive(makeRequest(finalPath, { upper, lower }),
                                         password, {}, &error);
    QVERIFY(!duplicate.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::DuplicateMember);

    const QString symlinkPath = directory.filePath(QLatin1String("source-link.bin"));
    QVERIFY(QFile::link(sourcePath, symlinkPath));
    auto linkedMember = makeMember(symlinkPath, QLatin1String("asset.ext"), 1, 0);
    auto linked = engine.stageArchive(makeRequest(finalPath, { linkedMember }),
                                      password, {}, &error);
    QVERIFY(!linked.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::UnsafeSource);

    const QString linkDestination = directory.filePath(
        QLatin1String("linked.ext.digikam-private.zip"));
    QVERIFY(QFile::link(sourcePath, linkDestination));
    auto unsafeDestination = engine.stageArchive(
        makeRequest(linkDestination, { lower }), password, {}, &error);
    QVERIFY(!unsafeDestination.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::UnsafeDestination);

    int cancellationCalls = 0;
    auto cancelledStage = engine.stageArchive(
        makeRequest(finalPath, { lower }), password,
        [&cancellationCalls]()
        {
            ++cancellationCalls;

            return true;
        },
        &error);
    QVERIFY(!cancelledStage.isValid());
    QCOMPARE(error, PrivacyCasualArchiveError::Cancelled);
    QVERIFY(cancellationCalls > 0);
    QCOMPARE(QDir(directory.path()).entryList(
                 { QLatin1String(".digikam-private-stage-*.zip") }, QDir::Files).size(), 0);
}

void PrivacyCasualArchiveTest::testTamperedStageCannotPublishOrDiscard()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString sourcePath = directory.filePath(QLatin1String("source.bin"));
    QVERIFY(writeBytes(sourcePath, QByteArray("source bytes")));
    const QString finalPath = directory.filePath(
        QLatin1String("asset.ext.digikam-private.zip"));
    PrivacyCasualArchiveEngine engine;
    const PrivacyPassword password = PrivacyPassword::fromUnicode(QLatin1String("passphrase"));
    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
    auto stage = engine.stageArchive(
        makeRequest(finalPath,
                    { makeMember(sourcePath, QLatin1String("asset.ext"), 1, 0) }),
        password, {}, &error);
    QVERIFY(stage.isValid());

    QFile archive(stage.stagingPath());
    QVERIFY(archive.open(QIODevice::ReadWrite));
    QVERIFY(archive.seek(0));
    char firstByte = 0;
    QCOMPARE(archive.read(&firstByte, 1), qint64(1));
    QVERIFY(archive.seek(0));
    firstByte ^= 0x01;
    QCOMPARE(archive.write(&firstByte, 1), qint64(1));
    QVERIFY(archive.flush());
    archive.close();

    QVERIFY(!engine.publishNew(&stage, &error));
    QCOMPARE(error, PrivacyCasualArchiveError::HashMismatch);
    QVERIFY(!QFileInfo::exists(finalPath));
    QVERIFY(!engine.discardStaged(&stage, &error));
    QCOMPARE(error, PrivacyCasualArchiveError::ExistingArchiveMismatch);
    QVERIFY(QFileInfo::exists(stage.stagingPath()));
}

QTEST_GUILESS_MAIN(PrivacyCasualArchiveTest)

#include "privacycasualarchive_utest.moc"
