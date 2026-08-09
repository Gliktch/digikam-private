/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacystillitemtransaction.h"

// C++ includes

#include <algorithm>
#include <cerrno>
#include <utility>

// Qt includes

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QStringList>
#include <QTimeZone>
#include <QtEndian>
#include <QUuid>

#if defined(Q_OS_UNIX)
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

// Local includes

#include "privacycasualarchive.h"
#include "privacyproxygenerator.h"
#include "privacypublictransition.h"
#include "privacyrepository.h"
#include "privacyposixstorage_p.h"

namespace Digikam
{

namespace
{

constexpr qsizetype IoChunkBytes = 1024 * 1024;

class ScopedBooleanValue
{
public:

    ScopedBooleanValue(bool& target, bool value)
        : m_target(target),
          m_previous(target)
    {
        m_target = value;
    }

    ~ScopedBooleanValue()
    {
        m_target = m_previous;
    }

private:

    bool& m_target;
    bool  m_previous = false;

private:

    Q_DISABLE_COPY(ScopedBooleanValue)
};

QString normalizedUuid(const QString& value)
{
    const QUuid uuid(value);
    return uuid.isNull() ? QString() : uuid.toString(QUuid::WithoutBraces);
}

bool canonicalUuid(const QString& value)
{
    return (!value.isEmpty() && (normalizedUuid(value) == value));
}

QString parentPath(const QString& relativePath)
{
    const qsizetype slash = relativePath.lastIndexOf(QLatin1Char('/'));
    return (slash < 0) ? QString() : relativePath.first(slash);
}

QString absolutePath(const PrivacyStorageRoot& root, const QString& relativePath)
{
    if (!root.isValid() ||
        (root.kind != PrivacyStorageRootKind::AlbumRoot) ||
        relativePath.isEmpty() || QDir::isAbsolutePath(relativePath) ||
        (QDir::cleanPath(relativePath) != relativePath) ||
        relativePath.startsWith(QLatin1String("../")))
    {
        return {};
    }

    return QDir(root.configuredPath).absoluteFilePath(relativePath);
}

PrivacyStillItemTransactionResult failure(
    PrivacyStillItemTransactionStatus status, const QString& transactionUuid,
    const QString& itemUuid, const QString& detail)
{
    PrivacyStillItemTransactionResult result;
    result.status = status;
    result.transactionUuid = transactionUuid;
    result.itemUuid = itemUuid;
    result.detail = detail;
    return result;
}

bool sameRootExpectation(const PrivacyStorageRoot& root,
                         const PrivacyJournalRootExpectation& expectation)
{
    return (root.isValid() && (root.kind == PrivacyStorageRootKind::AlbumRoot) &&
            (expectation.rootUuid == root.uuid) && (expectation.device != 0) &&
            (expectation.inode != 0) &&
            (expectation.identitySha256 == QCryptographicHash::hash(
                 root.identityData, QCryptographicHash::Sha256)));
}

bool stableFileFact(const QString& path,
                    const PrivacyInventoryFileEvidence* const expectedEvidence,
                    PrivacyJournalObjectFact* const fact,
                    mode_t* const portableMode = nullptr,
                    QDateTime* const modificationDate = nullptr)
{
#if !defined(Q_OS_UNIX)
    Q_UNUSED(path);
    Q_UNUSED(expectedEvidence);
    Q_UNUSED(fact);
    Q_UNUSED(portableMode);
    Q_UNUSED(modificationDate);
    return false;
#else
    if (!fact || path.isEmpty() || !QDir::isAbsolutePath(path) ||
        (QDir::cleanPath(path) != path))
    {
        return false;
    }

    const QByteArray encoded = QFile::encodeName(path);
    const int fd = ::open(encoded.constData(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);

    if (fd < 0)
    {
        return false;
    }

    struct stat before = {};
    const bool safe = (::fstat(fd, &before) == 0) && S_ISREG(before.st_mode) &&
                      (before.st_nlink >= 1) &&
                      (!expectedEvidence ||
                       (expectedEvidence->isRegular() &&
                        expectedEvidence->identityComplete &&
                        (static_cast<quint64>(before.st_dev) ==
                         expectedEvidence->deviceId) &&
                        (static_cast<quint64>(before.st_ino) ==
                         expectedEvidence->inode) &&
                        (static_cast<quint64>(before.st_nlink) ==
                         expectedEvidence->linkCount) &&
                        (static_cast<qlonglong>(before.st_size) ==
                         expectedEvidence->byteSize)));

    if (!safe)
    {
        ::close(fd);
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
    qlonglong offset = 0;

    while (offset < static_cast<qlonglong>(before.st_size))
    {
        const qsizetype wanted = static_cast<qsizetype>(std::min<qlonglong>(
            buffer.size(), static_cast<qlonglong>(before.st_size) - offset));
        const ssize_t count = ::pread(fd, buffer.data(),
                                      static_cast<size_t>(wanted), offset);

        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            ::close(fd);
            return false;
        }

        if (count == 0)
        {
            ::close(fd);
            return false;
        }

        hash.addData(QByteArrayView(buffer.constData(), count));
        offset += count;
    }

    struct stat after = {};
    const bool stable = (::fstat(fd, &after) == 0) &&
                        (before.st_dev == after.st_dev) &&
                        (before.st_ino == after.st_ino) &&
                        (before.st_mode == after.st_mode) &&
                        (before.st_nlink == after.st_nlink) &&
                        (before.st_size == after.st_size) &&
                        (before.st_mtim.tv_sec == after.st_mtim.tv_sec) &&
                        (before.st_mtim.tv_nsec == after.st_mtim.tv_nsec) &&
                        (before.st_ctim.tv_sec == after.st_ctim.tv_sec) &&
                        (before.st_ctim.tv_nsec == after.st_ctim.tv_nsec);
    ::close(fd);

    if (!stable)
    {
        return false;
    }

    fact->presence = PrivacyJournalExpectedPresence::Present;
    fact->size = static_cast<qlonglong>(before.st_size);
    fact->linkCount = static_cast<quint64>(before.st_nlink);
    fact->sha256 = hash.result();

    if (portableMode)
    {
        *portableMode = before.st_mode & 07777;
    }

    if (modificationDate)
    {
        *modificationDate = QDateTime::fromMSecsSinceEpoch(
            (static_cast<qint64>(before.st_mtim.tv_sec) * 1000) +
            (before.st_mtim.tv_nsec / 1000000), QTimeZone::UTC);
    }

    return true;
#endif
}

bool sameFact(const PrivacyJournalObjectFact& left,
              const PrivacyJournalObjectFact& right)
{
    return ((left.presence == right.presence) && (left.size == right.size) &&
            (left.linkCount == right.linkCount) &&
            (left.sha256 == right.sha256));
}

const PrivacyCategory* categoryFor(const PrivacyRepositorySnapshot& snapshot,
                                   const QString& uuid)
{
    const auto it = std::find_if(snapshot.categories.cbegin(),
                                 snapshot.categories.cend(),
                                 [&uuid](const PrivacyCategory& category)
                                 {
                                     return (category.uuid == uuid);
                                 });
    return (it == snapshot.categories.cend()) ? nullptr : &*it;
}

const PrivacyItem* itemForImage(const PrivacyRepositorySnapshot& snapshot,
                                qlonglong imageId)
{
    const auto it = std::find_if(snapshot.items.cbegin(), snapshot.items.cend(),
                                 [imageId](const PrivacyItem& item)
                                 {
                                     return (item.imageId == imageId);
                                 });
    return (it == snapshot.items.cend()) ? nullptr : &*it;
}

const PrivacyItem* itemForUuid(const PrivacyRepositorySnapshot& snapshot,
                               const QString& itemUuid)
{
    const auto it = std::find_if(snapshot.items.cbegin(), snapshot.items.cend(),
                                 [&itemUuid](const PrivacyItem& item)
                                 {
                                     return (item.uuid == itemUuid);
                                 });
    return (it == snapshot.items.cend()) ? nullptr : &*it;
}

const PrivacyStorageRoot* storageRootFor(
    const PrivacyRepositorySnapshot& snapshot, const QString& rootUuid)
{
    const auto it = std::find_if(
        snapshot.storageRoots.cbegin(), snapshot.storageRoots.cend(),
        [&rootUuid](const PrivacyStorageRoot& root)
        {
            return (root.uuid == rootUuid);
        });
    return (it == snapshot.storageRoots.cend()) ? nullptr : &*it;
}

QList<PrivacyAsset> assetsForItem(const PrivacyRepositorySnapshot& snapshot,
                                  const QString& itemUuid)
{
    QList<PrivacyAsset> result;

    for (const PrivacyAsset& asset : snapshot.assets)
    {
        if (asset.itemUuid == itemUuid)
        {
            result << asset;
        }
    }

    return result;
}

const PrivacyContainer* containerForItem(
    const PrivacyRepositorySnapshot& snapshot, const QString& itemUuid)
{
    const auto it = std::find_if(snapshot.containers.cbegin(),
                                 snapshot.containers.cend(),
                                 [&itemUuid](const PrivacyContainer& container)
                                 {
                                     return (container.itemUuid == itemUuid);
                                 });
    return (it == snapshot.containers.cend()) ? nullptr : &*it;
}

bool removeExactFile(const PrivacyStorageRoot& root,
                     const PrivacyJournalRootExpectation& expectation,
                     const QString& relativePath,
                     const PrivacyJournalObjectFact& expected,
                     bool allowConfirmedAbsent)
{
#if !defined(Q_OS_UNIX)
    Q_UNUSED(root);
    Q_UNUSED(expectation);
    Q_UNUSED(relativePath);
    Q_UNUSED(expected);
    return false;
#else
    if (!sameRootExpectation(root, expectation) || relativePath.isEmpty() ||
        QDir::isAbsolutePath(relativePath) ||
        (QDir::cleanPath(relativePath) != relativePath) ||
        relativePath.startsWith(QLatin1String("../")) ||
        (expected.presence != PrivacyJournalExpectedPresence::Present) ||
        (expected.linkCount != 1) || (expected.size < 0) ||
        (expected.sha256.size() != 32))
    {
        return false;
    }

    const int rootFd = ::open(QFile::encodeName(root.configuredPath).constData(),
                              O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    struct stat rootStat = {};

    if ((rootFd < 0) || (::fstat(rootFd, &rootStat) != 0) ||
        (static_cast<quint64>(rootStat.st_dev) != expectation.device) ||
        (static_cast<quint64>(rootStat.st_ino) != expectation.inode))
    {
        if (rootFd >= 0)
        {
            ::close(rootFd);
        }

        return false;
    }

    const QStringList parts = relativePath.split(QLatin1Char('/'));
    int directoryFd = rootFd;

    for (int i = 0 ; i + 1 < parts.size() ; ++i)
    {
        const QByteArray component = parts.at(i).toUtf8();
        const int next = PrivacyPosixStorage::confinedOpenAt(
            directoryFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        struct stat directoryStat = {};

        if ((next < 0) || (::fstat(next, &directoryStat) != 0) ||
            !S_ISDIR(directoryStat.st_mode) ||
            (directoryStat.st_uid != rootStat.st_uid) ||
            (directoryStat.st_dev != rootStat.st_dev))
        {
            if (next >= 0)
            {
                ::close(next);
            }

            if (directoryFd != rootFd)
            {
                ::close(directoryFd);
            }

            ::close(rootFd);
            return false;
        }

        if (directoryFd != rootFd)
        {
            ::close(directoryFd);
        }

        directoryFd = next;
    }

    const QByteArray name = parts.constLast().toUtf8();
    const int fileFd = PrivacyPosixStorage::confinedOpenAt(
        directoryFd, name, O_RDONLY | O_CLOEXEC);

    if ((fileFd < 0) && (errno == ENOENT) && allowConfirmedAbsent)
    {
        struct stat absent = {};
        const bool confirmed = (::fstatat(directoryFd, name.constData(), &absent,
                                          AT_SYMLINK_NOFOLLOW) != 0) &&
                               (errno == ENOENT);

        if (directoryFd != rootFd)
        {
            ::close(directoryFd);
        }

        ::close(rootFd);
        return confirmed;
    }

    struct stat before = {};
    bool okay = (fileFd >= 0) && (::fstat(fileFd, &before) == 0) &&
                S_ISREG(before.st_mode) && (before.st_uid == rootStat.st_uid) &&
                (before.st_dev == rootStat.st_dev) &&
                (static_cast<quint64>(before.st_nlink) == expected.linkCount) &&
                (static_cast<qlonglong>(before.st_size) == expected.size);
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
    qlonglong offset = 0;

    while (okay && (offset < expected.size))
    {
        const qsizetype wanted = static_cast<qsizetype>(std::min<qlonglong>(
            buffer.size(), expected.size - offset));
        const ssize_t count = ::pread(fileFd, buffer.data(), wanted, offset);

        if (count <= 0)
        {
            okay = false;
            break;
        }

        hash.addData(QByteArrayView(buffer.constData(), count));
        offset += count;
    }

    struct stat after = {};
    struct stat named = {};
    okay = okay && (hash.result() == expected.sha256) &&
           (::fstat(fileFd, &after) == 0) &&
           (before.st_dev == after.st_dev) && (before.st_ino == after.st_ino) &&
           (before.st_size == after.st_size) && (before.st_nlink == after.st_nlink) &&
           (::fstatat(directoryFd, name.constData(), &named,
                      AT_SYMLINK_NOFOLLOW) == 0) &&
           (named.st_dev == before.st_dev) && (named.st_ino == before.st_ino) &&
           (::unlinkat(directoryFd, name.constData(), 0) == 0) &&
           (::fsync(directoryFd) == 0);

    if (fileFd >= 0)
    {
        ::close(fileFd);
    }

    if (directoryFd != rootFd)
    {
        ::close(directoryFd);
    }

    ::close(rootFd);
    return okay;
#endif
}

PrivacyJournalRecord recordAt(PrivacyJournalRecord record,
                              PrivacyJournalStage stage)
{
    record.stage = stage;
    return record;
}

QByteArray encodePreparedPayload(const PrivacyJournalRecord& stagedRecord,
                                 const QString& archiveStageRelativePath,
                                 const QDateTime& originalModificationDate,
                                 const QByteArray& teardownSnapshot = {})
{
    const QByteArray journal = PrivacyTransactionJournalCodec::encode(stagedRecord);

    if (journal.isEmpty() || !originalModificationDate.isValid() ||
        archiveStageRelativePath.isEmpty() ||
        QDir::isAbsolutePath(archiveStageRelativePath) ||
        (QDir::cleanPath(archiveStageRelativePath) != archiveStageRelativePath))
    {
        return {};
    }

    QJsonObject object;
    object.insert(QStringLiteral("archiveStageRelativePath"),
                  archiveStageRelativePath);
    object.insert(QStringLiteral("formatVersion"), 1);
    object.insert(QStringLiteral("journal"),
                  QString::fromLatin1(journal.toBase64()));
    object.insert(QStringLiteral("originalModificationDate"),
                  originalModificationDate.toUTC().toString(Qt::ISODateWithMs));
    object.insert(QStringLiteral("teardownSnapshot"),
                  QString::fromLatin1(teardownSnapshot.toBase64()));
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool decodePreparedPayload(const QByteArray& bytes,
                           PrivacyJournalRecord* const record,
                           QString* const archiveStageRelativePath,
                           QDateTime* const originalModificationDate,
                           QByteArray* const teardownSnapshot = nullptr)
{
    if (!record || !archiveStageRelativePath || !originalModificationDate ||
        bytes.isEmpty())
    {
        return false;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);

    if ((error.error != QJsonParseError::NoError) || !document.isObject())
    {
        return false;
    }

    const QJsonObject object = document.object();
    const QStringList objectKeys = object.keys();
    const QSet<QString> keys(objectKeys.cbegin(), objectKeys.cend());
    const QSet<QString> expected = {
        QStringLiteral("archiveStageRelativePath"),
        QStringLiteral("formatVersion"), QStringLiteral("journal"),
        QStringLiteral("originalModificationDate"),
        QStringLiteral("teardownSnapshot")
    };

    if ((keys != expected) ||
        (object.value(QStringLiteral("formatVersion")).toInt(-1) != 1))
    {
        return false;
    }

    const QString archiveStage =
        object.value(QStringLiteral("archiveStageRelativePath")).toString();
    const QByteArray journal = QByteArray::fromBase64(
        object.value(QStringLiteral("journal")).toString().toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    const QDateTime modificationDate = QDateTime::fromString(
        object.value(QStringLiteral("originalModificationDate")).toString(),
        Qt::ISODateWithMs);
    const QByteArray teardown = QByteArray::fromBase64(
        object.value(QStringLiteral("teardownSnapshot")).toString().toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    PrivacyJournalError journalError = PrivacyJournalError::None;

    if (archiveStage.isEmpty() || QDir::isAbsolutePath(archiveStage) ||
        (QDir::cleanPath(archiveStage) != archiveStage) ||
        !modificationDate.isValid() ||
        !PrivacyTransactionJournalCodec::decode(journal, record, &journalError))
    {
        return false;
    }

    *archiveStageRelativePath = archiveStage;
    *originalModificationDate = modificationDate.toUTC();

    if (teardownSnapshot)
    {
        *teardownSnapshot = teardown;
    }

    return true;
}

QByteArray encodeTeardownSnapshot(const PrivacyItem& item,
                                  const PrivacyContainer& container,
                                  const PrivacyAsset& asset,
                                  const QString& priorProtectTransactionUuid)
{
    if (!item.isValid() || !container.isValid() || !asset.isValid() ||
        !canonicalUuid(priorProtectTransactionUuid))
    {
        return {};
    }

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << quint32(1)
           << item.imageId << item.uuid << item.categoryUuid
           << item.originalHash << item.originalSize << item.originalWidth
           << item.originalHeight << item.originalCreationDate
           << item.expectedProxyHash << item.expectedProxySize
           << item.presentationVersion << item.generation
           << item.transactionState
           << container.uuid << container.itemUuid << qint32(container.kind)
           << container.rootUuid << container.storeUuid
           << container.objectRelativePath << container.protectedSize
           << container.protectedHashAlgorithm << container.protectedHash
           << container.formatVersion << container.credentialGeneration
           << qint32(container.state) << container.createdAt << container.updatedAt
           << asset.itemUuid << asset.role << asset.ordinal << asset.originalName
           << asset.publicRootUuid << asset.publicRelativePath
           << asset.containerUuid << asset.protectedRelativePath
           << asset.hashAlgorithm << asset.originalHash << asset.originalSize
           << asset.originalCreationDate << asset.originalModificationDate
           << asset.portableAttributes << asset.proxyHashAlgorithm
           << asset.proxyHash << asset.proxySize
           << asset.proxyPresentationVersion << asset.proxyGeneration
           << priorProtectTransactionUuid;
    return (stream.status() == QDataStream::Ok) ? bytes : QByteArray();
}

bool decodeTeardownSnapshot(const QByteArray& bytes, PrivacyItem* const item,
                            PrivacyContainer* const container,
                            PrivacyAsset* const asset,
                            QString* const priorProtectTransactionUuid)
{
    if (bytes.isEmpty() || !item || !container || !asset ||
        !priorProtectTransactionUuid)
    {
        return false;
    }

    QDataStream stream(bytes);
    stream.setVersion(QDataStream::Qt_6_0);
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 version = 0;
    qint32 containerKind = 0;
    qint32 containerState = 0;
    stream >> version
           >> item->imageId >> item->uuid >> item->categoryUuid
           >> item->originalHash >> item->originalSize >> item->originalWidth
           >> item->originalHeight >> item->originalCreationDate
           >> item->expectedProxyHash >> item->expectedProxySize
           >> item->presentationVersion >> item->generation
           >> item->transactionState
           >> container->uuid >> container->itemUuid >> containerKind
           >> container->rootUuid >> container->storeUuid
           >> container->objectRelativePath >> container->protectedSize
           >> container->protectedHashAlgorithm >> container->protectedHash
           >> container->formatVersion >> container->credentialGeneration
           >> containerState >> container->createdAt >> container->updatedAt
           >> asset->itemUuid >> asset->role >> asset->ordinal >> asset->originalName
           >> asset->publicRootUuid >> asset->publicRelativePath
           >> asset->containerUuid >> asset->protectedRelativePath
           >> asset->hashAlgorithm >> asset->originalHash >> asset->originalSize
           >> asset->originalCreationDate >> asset->originalModificationDate
           >> asset->portableAttributes >> asset->proxyHashAlgorithm
           >> asset->proxyHash >> asset->proxySize
           >> asset->proxyPresentationVersion >> asset->proxyGeneration
           >> *priorProtectTransactionUuid;
    container->kind = static_cast<PrivacyContainerKind>(containerKind);
    container->state = static_cast<PrivacyContainerState>(containerState);
    return ((version == 1) && stream.atEnd() &&
            (stream.status() == QDataStream::Ok) && item->isValid() &&
            container->isValid() && asset->isValid() &&
            canonicalUuid(*priorProtectTransactionUuid) &&
            (container->itemUuid == item->uuid) &&
            (asset->itemUuid == item->uuid) &&
            (asset->containerUuid == container->uuid));
}

QByteArray encodePortableMode(mode_t mode)
{
    const quint32 portable = static_cast<quint32>(mode & 07777);
    QByteArray bytes(4, Qt::Uninitialized);
    qToBigEndian(portable, bytes.data());
    return bytes;
}

bool decodePortableMode(const QByteArray& bytes, mode_t* const mode)
{
    if (!mode || (bytes.size() != 4))
    {
        return false;
    }

    const quint32 value = qFromBigEndian<quint32>(bytes.constData());

    if ((value & ~quint32(07777)) != 0)
    {
        return false;
    }

    *mode = static_cast<mode_t>(value);
    return true;
}

const PrivacyTransaction* transactionFor(
    const PrivacyRepositorySnapshot& snapshot, const QString& uuid)
{
    const auto it = std::find_if(snapshot.transactions.cbegin(),
                                 snapshot.transactions.cend(),
                                 [&uuid](const PrivacyTransaction& transaction)
                                 {
                                     return (transaction.uuid == uuid);
                                 });
    return (it == snapshot.transactions.cend()) ? nullptr : &*it;
}

const PrivacyTransactionJournal* databaseJournalFor(
    const PrivacyRepositorySnapshot& snapshot, const QString& transactionUuid,
    const QString& rootUuid)
{
    const auto it = std::find_if(
        snapshot.transactionJournals.cbegin(),
        snapshot.transactionJournals.cend(),
        [&transactionUuid, &rootUuid](const PrivacyTransactionJournal& journal)
        {
            return ((journal.transactionUuid == transactionUuid) &&
                    (journal.rootUuid == rootUuid));
        });
    return (it == snapshot.transactionJournals.cend()) ? nullptr : &*it;
}

bool transitionJournalBoundToPayload(
    const PrivacyRepositorySnapshot& snapshot,
    const PrivacyJournalRecord& prepared,
    const PrivacyJournalLoadResult& loaded)
{
    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || !loaded.hasRecord ||
        ((loaded.record.stage != PrivacyJournalStage::ProtectedCopyVerified) &&
         (loaded.record.stage != PrivacyJournalStage::Applying) &&
         (loaded.record.stage != PrivacyJournalStage::PublicStateVerified)))
    {
        return false;
    }

    const PrivacyJournalRecord exactLoaded = recordAt(prepared,
                                                       loaded.record.stage);
    const QByteArray exactLoadedBytes = PrivacyTransactionJournalCodec::encode(
        exactLoaded);

    if (exactLoadedBytes.isEmpty() ||
        (loaded.canonicalBytes != exactLoadedBytes) ||
        (loaded.sha256 != PrivacyTransactionJournalCodec::sha256(exactLoadedBytes)))
    {
        return false;
    }

    const PrivacyTransactionJournal* databaseJournal = databaseJournalFor(
        snapshot, prepared.transactionUuid, prepared.rootUuid);

    if (!databaseJournal ||
        (databaseJournal->expectedHashAlgorithm != QLatin1String("sha256")))
    {
        return false;
    }

    if (databaseJournal->stage == static_cast<int>(loaded.record.stage))
    {
        return (databaseJournal->expectedJournalHash ==
                QString::fromLatin1(loaded.sha256.toHex()));
    }

    if ((databaseJournal->stage !=
         static_cast<int>(PrivacyJournalStage::ProtectedCopyVerified)) ||
        ((loaded.record.stage != PrivacyJournalStage::Applying) &&
         (loaded.record.stage != PrivacyJournalStage::PublicStateVerified)))
    {
        return false;
    }

    const QByteArray predecessorBytes = PrivacyTransactionJournalCodec::encode(
        recordAt(prepared, PrivacyJournalStage::ProtectedCopyVerified));
    return (!predecessorBytes.isEmpty() &&
            (databaseJournal->expectedJournalHash == QString::fromLatin1(
                PrivacyTransactionJournalCodec::sha256(predecessorBytes).toHex())));
}

class NullWriteDevice final : public QIODevice
{
public:

    bool openWrite()
    {
        return open(QIODevice::WriteOnly);
    }

protected:

    qint64 readData(char*, qint64) override
    {
        return -1;
    }

    qint64 writeData(const char*, qint64 length) override
    {
        return length;
    }
};

bool verifyArchiveMember(const PrivacyCasualArchiveEngine& archive,
                         const PrivacyJournalRecord& record,
                         const QString& archivePath,
                         const PrivacyPassword& password)
{
    if (record.assets.size() != 1)
    {
        return false;
    }

    const PrivacyJournalAsset& asset = record.assets.constFirst();
    PrivacyCasualArchiveRestoreRequest restore;
    restore.archivePath = archivePath;
    restore.categoryUuid = record.categoryUuid;
    restore.containerUuid = asset.containerUuid;
    restore.itemUuid = asset.itemUuid;
    restore.protectedRelativePath = asset.protectedRelativePath;
    restore.originalName = QFileInfo(asset.publicRelativePath).fileName();
    restore.role = asset.role;
    restore.ordinal = asset.ordinal;
    restore.expectedArchiveSize = asset.container.size;
    restore.expectedArchiveSha256 = asset.container.sha256;
    restore.expectedMemberSize = asset.original.size;
    restore.expectedMemberSha256 = asset.original.sha256;
    NullWriteDevice sink;

    return (sink.openWrite() && archive.restoreMember(restore, password, &sink));
}

} // namespace

bool PrivacyStillItemTransactionResult::succeeded() const
{
    return ((status == PrivacyStillItemTransactionStatus::Protected) ||
            (status == PrivacyStillItemTransactionStatus::Unprotected));
}

bool PrivacyCoreDbStillItemPersistence::loadSnapshot(
    PrivacyRepositorySnapshot* const snapshot) const
{
    return PrivacyRepository().loadSnapshot(snapshot);
}

bool PrivacyCoreDbStillItemPersistence::beginProtection(
    const PrivacyItem& item, const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginItemProtection(item, transaction, journal);
}

bool PrivacyCoreDbStillItemPersistence::publishProtection(
    const PrivacyItem& item, const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets, const PrivacyTransaction& transaction)
{
    return PrivacyRepository().publishItemProtection(item, container, assets,
                                                      transaction);
}

bool PrivacyCoreDbStillItemPersistence::beginUnprotection(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginItemUnprotection(transaction, journal);
}

bool PrivacyCoreDbStillItemPersistence::publishUnprotection(
    qlonglong imageId, const QString& itemUuid, const QString& categoryUuid,
    qlonglong expectedItemGeneration,
    const QString& priorProtectTransactionUuid,
    const PrivacyTransaction& transaction)
{
    return PrivacyRepository().publishItemUnprotection(
        imageId, itemUuid, categoryUuid, expectedItemGeneration,
        priorProtectTransactionUuid, transaction);
}

bool PrivacyCoreDbStillItemPersistence::finalizeUnprotection(
    const QString& transactionUuid, const QString& categoryUuid)
{
    return PrivacyRepository().finalizeItemUnprotection(transactionUuid,
                                                         categoryUuid);
}

bool PrivacyCoreDbStillItemPersistence::compareAndUpdateTransaction(
    const PrivacyTransaction& transaction,
    PrivacyTransactionState expectedState, qlonglong expectedGeneration)
{
    return PrivacyRepository().compareAndUpdateTransaction(
        transaction, expectedState, expectedGeneration);
}

bool PrivacyCoreDbStillItemPersistence::compareAndUpdateJournal(
    const PrivacyTransactionJournal& journal, int expectedStage)
{
    return PrivacyRepository().compareAndUpdateTransactionJournal(journal,
                                                                   expectedStage);
}

class Q_DECL_HIDDEN PrivacyStillItemTransactionEngine::Private
{
public:

    Private(PrivacyStillItemPersistence& persistenceValue,
            PrivacyRuntimeCoordinator& runtimeValue,
            PrivacyStillItemCacheGate& cacheValue)
        : persistence(persistenceValue),
          runtime(runtimeValue),
          cache(cacheValue)
    {
    }

    bool fault(PrivacyStillItemFaultPoint point) const
    {
        return (faultHook && faultHook(point));
    }

    bool load(PrivacyRepositorySnapshot* snapshot) const
    {
        return persistence.loadSnapshot(snapshot);
    }

    bool advanceJournal(const PrivacyStorageRoot& root,
                        const PrivacyJournalRootExpectation& expectation,
                        const PrivacyJournalRecord& predecessorRecord,
                        const PrivacyJournalRecord& target,
                        QByteArray* const resultingHash,
                        QString* const detail)
    {
        if (!resultingHash || !detail ||
            (predecessorRecord.transactionUuid != target.transactionUuid) ||
            (predecessorRecord.categoryUuid != target.categoryUuid) ||
            (predecessorRecord.rootUuid != target.rootUuid) ||
            (predecessorRecord.rootDevice != target.rootDevice) ||
            (predecessorRecord.rootInode != target.rootInode) ||
            (predecessorRecord.rootIdentitySha256 != target.rootIdentitySha256) ||
            (predecessorRecord.transactionType != target.transactionType) ||
            (predecessorRecord.generation != target.generation) ||
            (predecessorRecord.credentialGeneration != target.credentialGeneration) ||
            (predecessorRecord.fromCredentialGeneration !=
             target.fromCredentialGeneration) ||
            (predecessorRecord.toCredentialGeneration !=
             target.toCredentialGeneration) ||
            (static_cast<int>(target.stage) <
             static_cast<int>(predecessorRecord.stage)))
        {
            if (detail)
            {
                *detail = QStringLiteral("journal predecessor/target identity mismatch");
            }

            return false;
        }

        PrivacyJournalError journalError = PrivacyJournalError::None;
        std::unique_ptr<PrivacyTransactionJournalStore> store =
            PrivacyTransactionJournalStore::open(root.configuredPath,
                                                  expectation, &journalError,
                                                  detail);

        if (!store)
        {
            return false;
        }

        PrivacyRepositorySnapshot snapshot;

        if (!load(&snapshot))
        {
            *detail = QStringLiteral("cannot load journal database state");
            return false;
        }

        const PrivacyTransactionJournal* dbJournal = databaseJournalFor(
            snapshot, target.transactionUuid, target.rootUuid);

        if (!dbJournal)
        {
            *detail = QStringLiteral("database journal row is missing");
            return false;
        }

        PrivacyJournalLoadResult loaded = store->load(target.transactionUuid);

        if (loaded.disposition == PrivacyJournalLoadDisposition::Missing)
        {
            if ((target.stage != PrivacyJournalStage::Created) ||
                (predecessorRecord.stage != PrivacyJournalStage::Created) ||
                (dbJournal->stage != static_cast<int>(PrivacyJournalStage::Created)) ||
                !dbJournal->expectedJournalHash.isEmpty() ||
                !store->create(target, &loaded.sha256, &journalError, detail))
            {
                *detail = QStringLiteral("missing journal is not an exact unbound Created replay");
                return false;
            }

            loaded = store->load(target.transactionUuid);
        }

        if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
            !loaded.authoritative || !loaded.hasRecord)
        {
            *detail = QStringLiteral("filesystem journal is not authoritative");
            return false;
        }

        const QByteArray predecessorBytes =
            PrivacyTransactionJournalCodec::encode(predecessorRecord);
        const QByteArray predecessorHash =
            PrivacyTransactionJournalCodec::sha256(predecessorBytes);
        const QByteArray targetBytes = PrivacyTransactionJournalCodec::encode(target);
        const QByteArray targetHash =
            PrivacyTransactionJournalCodec::sha256(targetBytes);

        if (targetBytes.isEmpty() || predecessorBytes.isEmpty())
        {
            *detail = QStringLiteral("journal stage record is invalid");
            return false;
        }

        if (loaded.record.stage == predecessorRecord.stage)
        {
            if ((loaded.canonicalBytes != predecessorBytes) ||
                (loaded.sha256 != predecessorHash))
            {
                *detail = QStringLiteral("filesystem predecessor journal is not exact");
                return false;
            }

            const QString dbHash = QString::fromLatin1(predecessorHash.toHex());

            if ((dbJournal->stage != static_cast<int>(predecessorRecord.stage)) ||
                (!dbJournal->expectedJournalHash.isEmpty() &&
                 ((dbJournal->expectedHashAlgorithm != QLatin1String("sha256")) ||
                  (dbJournal->expectedJournalHash != dbHash))))
            {
                *detail = QStringLiteral("database predecessor journal is not exact");
                return false;
            }

            if (dbJournal->expectedJournalHash.isEmpty())
            {
                if (predecessorRecord.stage != PrivacyJournalStage::Created)
                {
                    *detail = QStringLiteral("only Created may bind an empty database hash");
                    return false;
                }

                PrivacyTransactionJournal bound = *dbJournal;
                bound.expectedHashAlgorithm = QLatin1String("sha256");
                bound.expectedJournalHash = dbHash;

                if (!persistence.compareAndUpdateJournal(
                        bound, static_cast<int>(predecessorRecord.stage)))
                {
                    *detail = QStringLiteral("cannot bind Created database journal");
                    return false;
                }
            }

            QByteArray published;

            if ((target.stage != predecessorRecord.stage) &&
                !store->compareAndUpdate(target, predecessorHash, &published,
                                         &journalError, detail))
            {
                return false;
            }

            if (target.stage != predecessorRecord.stage)
            {
                loaded = store->load(target.transactionUuid);
            }
        }

        if ((loaded.record.stage != target.stage) ||
            (loaded.canonicalBytes != targetBytes) ||
            (loaded.sha256 != targetHash))
        {
            *detail = QStringLiteral("filesystem result journal is not exact");
            return false;
        }

        if (!load(&snapshot))
        {
            *detail = QStringLiteral("cannot reload database journal state");
            return false;
        }

        dbJournal = databaseJournalFor(snapshot, target.transactionUuid,
                                       target.rootUuid);

        if (!dbJournal)
        {
            return false;
        }

        const QString targetHex = QString::fromLatin1(targetHash.toHex());

        if ((dbJournal->stage == static_cast<int>(target.stage)) &&
            (dbJournal->expectedHashAlgorithm == QLatin1String("sha256")) &&
            (dbJournal->expectedJournalHash == targetHex))
        {
            *resultingHash = targetHash;
            return true;
        }

        if ((dbJournal->stage != static_cast<int>(predecessorRecord.stage)) ||
            (dbJournal->expectedHashAlgorithm != QLatin1String("sha256")) ||
            (dbJournal->expectedJournalHash !=
             QString::fromLatin1(predecessorHash.toHex())))
        {
            *detail = QStringLiteral("database journal did not retain exact predecessor");
            return false;
        }

        PrivacyTransactionJournal next = *dbJournal;
        next.stage = static_cast<int>(target.stage);
        next.expectedJournalHash = targetHex;

        if (!persistence.compareAndUpdateJournal(next,
                                                 static_cast<int>(predecessorRecord.stage)))
        {
            *detail = QStringLiteral("cannot bind database journal result");
            return false;
        }

        *resultingHash = targetHash;
        return true;
    }

public:

    PrivacyStillItemPersistence& persistence;
    PrivacyRuntimeCoordinator& runtime;
    PrivacyStillItemCacheGate& cache;
    PrivacyCasualArchiveEngine archive;
    PrivacyStillProxyGenerator proxy;
    PrivacyPublicTransitionEngine transition;
    FaultHook faultHook;
    bool durableReplay = false;
    bool authenticatedCreatedReplay = false;
};

PrivacyStillItemTransactionEngine::PrivacyStillItemTransactionEngine(
    PrivacyStillItemPersistence& persistence,
    PrivacyRuntimeCoordinator& runtime,
    PrivacyStillItemCacheGate& cacheGate)
    : d(new Private(persistence, runtime, cacheGate))
{
}

PrivacyStillItemTransactionEngine::~PrivacyStillItemTransactionEngine() = default;

void PrivacyStillItemTransactionEngine::setFaultHook(const FaultHook& hook)
{
    d->faultHook = hook;
}

PrivacyStillItemTransactionResult PrivacyStillItemTransactionEngine::protect(
    const PrivacyStillProtectRequest& request, const PrivacyPassword& password)
{
    const QString transactionUuid = normalizedUuid(request.transactionUuid);
    const QString itemUuid = normalizedUuid(request.itemUuid);
    const QString containerUuid = normalizedUuid(request.containerUuid);
    const auto fail = [&](PrivacyStillItemTransactionStatus status,
                          const QString& detail)
    {
        return failure(status, transactionUuid, itemUuid, detail);
    };

    if ((request.imageId <= 0) || !canonicalUuid(request.categoryUuid) ||
        !canonicalUuid(request.itemUuid) || !canonicalUuid(request.containerUuid) ||
        !canonicalUuid(request.transactionUuid) ||
        (!password.isValid() && !d->durableReplay) ||
        !request.publicRoot.isValid() ||
        !sameRootExpectation(request.publicRoot, request.rootExpectation) ||
        (request.rootExpectation.markerUuid != request.publicRoot.markerUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest,
                    QStringLiteral("protect request identity is invalid"));
    }

    if ((request.preflight.bridge.status != PrivacyInventoryStatus::Ready) ||
        !request.preflight.bridge.issues.isEmpty() ||
        (request.preflight.bridge.items.size() != 1) ||
        (request.preflight.bridge.items.constFirst().imageId != request.imageId) ||
        !request.preflight.bridge.items.constFirst().issues.isEmpty() ||
        !request.preflight.bridge.items.constFirst().inventory.isReady())
    {
        return fail(PrivacyStillItemTransactionStatus::PreflightRejected,
                    QStringLiteral("exact single-item preflight is not Ready"));
    }

    const PrivacyAssetInventoryResult& inventory =
        request.preflight.bridge.items.constFirst().inventory;

    if (inventory.requiredAssets.size() != 1)
    {
        return fail(PrivacyStillItemTransactionStatus::AssociatedAssetSetUnsupported,
                    QStringLiteral("version 1 still transaction requires exactly one asset"));
    }

    if (!request.associatedAssetsAcknowledged)
    {
        return fail(PrivacyStillItemTransactionStatus::AcknowledgementRequired,
                    QStringLiteral("associated-asset inventory requires acknowledgement"));
    }

    const PrivacyInventoryAsset& sourceAsset = inventory.requiredAssets.constFirst();

    if (!sourceAsset.isValid() ||
        (sourceAsset.role != PrivacyInventoryAssetRole::PrimaryMedia) ||
        (sourceAsset.ordinal != 0) ||
        (sourceAsset.location.root.uuid != request.publicRoot.uuid) ||
        (QDir::cleanPath(sourceAsset.location.root.absolutePath) !=
         QDir::cleanPath(request.publicRoot.configuredPath)))
    {
        return fail(PrivacyStillItemTransactionStatus::PreflightRejected,
                    QStringLiteral("preflight primary asset/root does not match request"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot load privacy snapshot"));
    }

    const PrivacyCategory* category = categoryFor(snapshot, request.categoryUuid);

    if (!category || (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (!d->durableReplay && !d->runtime.isCategoryUnlocked(category->uuid)))
    {
        return fail(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                    QStringLiteral("Casual category is not active and unlocked"));
    }

    const PrivacyCategory categoryValue = *category;

    const QString publicRelativePath = sourceAsset.location.relativePath;
    const QString sourcePath = absolutePath(request.publicRoot, publicRelativePath);
    const QString archiveRelativePath = publicRelativePath +
                                        QLatin1String(".digikam-private.zip");
    const QString archivePath = absolutePath(request.publicRoot,
                                             archiveRelativePath);
    const QString archiveStageRelativePath =
        parentPath(publicRelativePath) +
        (parentPath(publicRelativePath).isEmpty() ? QString() : QLatin1String("/")) +
        QLatin1String(".digikam-private-stage-") + transactionUuid +
        QLatin1String(".zip");
    const QString archiveStagePath = absolutePath(request.publicRoot,
                                                  archiveStageRelativePath);
    const QString replacementRelativePath =
        parentPath(publicRelativePath) +
        (parentPath(publicRelativePath).isEmpty() ? QString() : QLatin1String("/")) +
        PrivacyPublicTransitionEngine::expectedStageFileName(
            transactionUuid, PrivacyAsset::PrimaryMediaRole, 0);
    QDateTime originalModificationDate;
    mode_t originalMode = 0;

    if (sourcePath.isEmpty() || archivePath.isEmpty() || archiveStagePath.isEmpty() ||
        replacementRelativePath.isEmpty())
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest,
                    QStringLiteral("derived transaction path is unsafe"));
    }

    const PrivacyTransaction* existingTransaction = transactionFor(
        snapshot, transactionUuid);
    const PrivacyItem* existingItem = itemForImage(snapshot, request.imageId);

    PrivacyJournalRecord created;
    PrivacyJournalRecord prepared;
    QString storedArchiveStage;
    QByteArray protectMetadata;

    if (!existingTransaction)
    {
        if (existingItem)
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("image already has a different privacy item"));
        }

        PrivacyJournalObjectFact originalFact;

        if (!stableFileFact(sourcePath, &sourceAsset.evidence, &originalFact,
                            &originalMode, &originalModificationDate))
        {
            return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                        QStringLiteral("source no longer matches preflight evidence"));
        }

        created.transactionUuid = transactionUuid;
        created.categoryUuid = request.categoryUuid;
        created.rootUuid = request.publicRoot.uuid;
        created.rootDevice = request.rootExpectation.device;
        created.rootInode = request.rootExpectation.inode;
        created.rootIdentitySha256 = request.rootExpectation.identitySha256;
        created.transactionType = PrivacyTransactionType::ProtectItem;
        created.generation = 0;
        created.credentialGeneration = categoryValue.currentCredentialGeneration;
        created.fromCredentialGeneration = categoryValue.currentCredentialGeneration;
        created.toCredentialGeneration = categoryValue.currentCredentialGeneration;
        created.stage = PrivacyJournalStage::Created;

        PrivacyJournalAsset journalAsset;
        journalAsset.itemUuid = itemUuid;
        journalAsset.containerUuid = containerUuid;
        journalAsset.role = PrivacyAsset::PrimaryMediaRole;
        journalAsset.ordinal = 0;
        journalAsset.publicRelativePath = publicRelativePath;
        journalAsset.stagedRelativePath = replacementRelativePath;
        journalAsset.protectedRelativePath =
            PrivacyCasualArchiveEngine::expectedMemberPath(
                journalAsset.role, journalAsset.ordinal,
                QFileInfo(publicRelativePath).fileName());
        journalAsset.containerRelativePath = archiveRelativePath;
        journalAsset.original = originalFact;
        created.assets << journalAsset;

        PrivacyItem item;
        item.imageId = request.imageId;
        item.uuid = itemUuid;
        item.categoryUuid = request.categoryUuid;
        item.originalHash = QString::fromLatin1(originalFact.sha256.toHex());
        item.originalSize = originalFact.size;
        item.originalWidth = request.originalPixelSize.width();
        item.originalHeight = request.originalPixelSize.height();
        item.originalCreationDate = request.originalCreationDate;
        item.presentationVersion = 1;
        item.generation = 0;
        item.transactionState = static_cast<int>(PrivacyTransactionState::Created);

        PrivacyTransaction transaction;
        transaction.uuid = transactionUuid;
        transaction.categoryUuid = request.categoryUuid;
        transaction.itemUuid = itemUuid;
        transaction.type = PrivacyTransactionType::ProtectItem;
        transaction.state = PrivacyTransactionState::Created;
        transaction.generation = 0;
        transaction.fromCredentialGeneration = categoryValue.currentCredentialGeneration;
        transaction.toCredentialGeneration = categoryValue.currentCredentialGeneration;
        transaction.payloadFormatVersion = 1;
        protectMetadata = encodePortableMode(originalMode);
        transaction.payloadData = encodePreparedPayload(
            created, archiveStageRelativePath, originalModificationDate,
            protectMetadata);
        transaction.createdAt = QDateTime::currentDateTimeUtc();
        transaction.updatedAt = transaction.createdAt;

        PrivacyTransactionJournal journal;
        journal.transactionUuid = transactionUuid;
        journal.rootUuid = request.publicRoot.uuid;
        journal.journalRelativePath =
            PrivacyTransactionJournalCodec::relativeJournalPath(transactionUuid);
        journal.journalFormatVersion = PrivacyTransactionJournalCodec::FormatVersion;
        journal.stage = static_cast<int>(PrivacyJournalStage::Created);
        journal.updatedAt = transaction.createdAt;

        if (transaction.payloadData.isEmpty() ||
            !d->persistence.beginProtection(item, transaction, journal))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("cannot atomically begin protection"));
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterDatabaseBegin))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        QStringLiteral("fault after database begin"));
        }

        if (!d->load(&snapshot))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("cannot reload begun protection"));
        }

        existingTransaction = transactionFor(snapshot, transactionUuid);
        existingItem = itemForImage(snapshot, request.imageId);
    }

    if (!existingTransaction ||
        (existingTransaction->type != PrivacyTransactionType::ProtectItem) ||
        (existingTransaction->categoryUuid != request.categoryUuid) ||
        (existingTransaction->itemUuid != itemUuid) || !existingItem ||
        (existingItem->uuid != itemUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("existing Protect transaction is not exact"));
    }

    if (!decodePreparedPayload(existingTransaction->payloadData, &created,
                               &storedArchiveStage,
                               &originalModificationDate, &protectMetadata) ||
        !decodePortableMode(protectMetadata, &originalMode) ||
        (storedArchiveStage != archiveStageRelativePath))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Protect payload cannot be reconstructed"));
    }

    if (d->durableReplay && !d->authenticatedCreatedReplay &&
        (existingTransaction->state == PrivacyTransactionState::Created))
    {
        return fail(PrivacyStillItemTransactionStatus::AuthenticationRequired,
                    QStringLiteral("Created Protect requires category authentication"));
    }

    if (existingTransaction->state == PrivacyTransactionState::Created)
    {
        QByteArray journalHash;
        QString detail;

        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               created, created, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterFilesystemJournal))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        QStringLiteral("fault after filesystem Created journal"));
        }

        PrivacyStillProxyRequest proxyRequest;
        proxyRequest.sourcePath = sourcePath;
        proxyRequest.publicFileName = QFileInfo(publicRelativePath).fileName();
        proxyRequest.presentation =
            (categoryValue.presentationMode == PrivacyPresentationMode::Blur)
                ? PrivacyStillProxyPresentation::Blurred
                : PrivacyStillProxyPresentation::Generic;
        const PrivacyStillProxyResult proxyResult = d->proxy.generate(proxyRequest);

        if (!proxyResult.isValid())
        {
            return fail(PrivacyStillItemTransactionStatus::ProxyFailure,
                        QStringLiteral("cannot generate metadata-free proxy"));
        }

        PrivacyJournalObjectFact originalFact;

        mode_t verifiedMode = 0;
        QDateTime verifiedModificationDate;

        if (!stableFileFact(sourcePath, &sourceAsset.evidence, &originalFact,
                            &verifiedMode, &verifiedModificationDate) ||
            (verifiedMode != originalMode) ||
            (verifiedModificationDate != originalModificationDate) ||
            !sameFact(originalFact, created.assets.constFirst().original))
        {
            return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                        QStringLiteral("source changed while preparing protection"));
        }

        PrivacyJournalObjectFact existingArchiveStageFact;
        PrivacyCasualArchiveStage archiveStage = [&]()
        {
            if (stableFileFact(archiveStagePath, nullptr,
                               &existingArchiveStageFact))
            {
                return d->archive.resumeStagedArchive(
                    archiveStagePath, archivePath,
                    existingArchiveStageFact.size,
                    existingArchiveStageFact.sha256, password);
            }

            PrivacyCasualArchiveMember member;
            member.sourcePath = sourcePath;
            member.protectedRelativePath = created.assets.constFirst().protectedRelativePath;
            member.originalName = QFileInfo(publicRelativePath).fileName();
            member.role = PrivacyAsset::PrimaryMediaRole;
            member.ordinal = 0;
            member.originalCreationDate = request.originalCreationDate;
            member.originalModificationDate = originalModificationDate;
            member.portableAttributes = encodePortableMode(originalMode);
            member.expectedDevice = sourceAsset.evidence.deviceId;
            member.expectedInode = sourceAsset.evidence.inode;
            member.expectedLinkCount = sourceAsset.evidence.linkCount;
            member.expectedSize = originalFact.size;
            member.expectedSha256 = originalFact.sha256;
            PrivacyCasualArchiveRequest archiveRequest;
            archiveRequest.finalArchivePath = archivePath;
            archiveRequest.stagingArchivePath = archiveStagePath;
            archiveRequest.categoryUuid = request.categoryUuid;
            archiveRequest.containerUuid = containerUuid;
            archiveRequest.itemUuid = itemUuid;
            archiveRequest.members << member;
            return d->archive.stageArchive(archiveRequest, password);
        }();

        if (!archiveStage.isValid())
        {
            return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                        QStringLiteral("cannot create or resume verified archive stage"));
        }

        prepared = created;
        prepared.stage = PrivacyJournalStage::Prepared;
        prepared.assets[0].proxy.presence = PrivacyJournalExpectedPresence::Present;
        prepared.assets[0].proxy.size = proxyResult.encodedBytes.size();
        prepared.assets[0].proxy.linkCount = 1;
        prepared.assets[0].proxy.sha256 = proxyResult.sha256;
        prepared.assets[0].container.presence = PrivacyJournalExpectedPresence::Present;
        prepared.assets[0].container.size = archiveStage.archiveSize();
        prepared.assets[0].container.linkCount = 1;
        prepared.assets[0].container.sha256 = archiveStage.archiveSha256();

        PrivacyTransaction next = *existingTransaction;
        next.state = PrivacyTransactionState::Prepared;
        next.generation = 1;
        next.payloadData = encodePreparedPayload(
            prepared, archiveStageRelativePath, originalModificationDate,
            protectMetadata);
        next.updatedAt = QDateTime::currentDateTimeUtc();

        if (next.payloadData.isEmpty() ||
            !d->persistence.compareAndUpdateTransaction(
                next, PrivacyTransactionState::Created, 0))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("cannot publish exact Prepared transaction"));
        }

        existingTransaction = nullptr;
    }
    else if (!decodePreparedPayload(existingTransaction->payloadData, &prepared,
                                    &storedArchiveStage,
                                    &originalModificationDate,
                                    &protectMetadata) ||
             !decodePortableMode(protectMetadata, &originalMode) ||
             (prepared.stage != PrivacyJournalStage::Prepared))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Prepared Protect payload is invalid"));
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot reload Prepared protection"));
    }

    existingTransaction = transactionFor(snapshot, transactionUuid);
    existingItem = itemForImage(snapshot, request.imageId);

    if (!existingTransaction || !existingItem ||
        !decodePreparedPayload(existingTransaction->payloadData, &prepared,
                               &storedArchiveStage,
                               &originalModificationDate, &protectMetadata) ||
        !decodePortableMode(protectMetadata, &originalMode))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Prepared protection disappeared"));
    }

    created = prepared;
    created.stage = PrivacyJournalStage::Created;
    created.assets[0].proxy = PrivacyJournalObjectFact();
    created.assets[0].container = PrivacyJournalObjectFact();

    const PrivacyJournalRecord staged = recordAt(prepared,
                                                  PrivacyJournalStage::Staged);
    const PrivacyJournalRecord protectedCopy = recordAt(
        prepared, PrivacyJournalStage::ProtectedCopyVerified);
    const PrivacyJournalRecord publicVerified = recordAt(
        prepared, PrivacyJournalStage::PublicStateVerified);
    const PrivacyJournalRecord complete = recordAt(prepared,
                                                    PrivacyJournalStage::Complete);
    QByteArray journalHash;
    QString detail;
    const PrivacyTransactionJournal* dbJournal = databaseJournalFor(
        snapshot, transactionUuid, request.publicRoot.uuid);

    if (!dbJournal)
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                    QStringLiteral("Protect database journal is missing"));
    }

    if (dbJournal->stage < static_cast<int>(PrivacyJournalStage::Staged))
    {
        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               created, prepared, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
        }

        PrivacyPublicReplacementStageRequest stageRequest;
        stageRequest.absoluteRootPath = request.publicRoot.configuredPath;
        stageRequest.rootExpectation = request.rootExpectation;
        stageRequest.journalRecord = prepared;
        stageRequest.authoritativeJournalSha256 = journalHash;
        stageRequest.itemUuid = itemUuid;
        stageRequest.role = PrivacyAsset::PrimaryMediaRole;
        stageRequest.ordinal = 0;
        PrivacyStillProxyRequest proxyRequest;
        proxyRequest.sourcePath = sourcePath;
        proxyRequest.publicFileName = QFileInfo(publicRelativePath).fileName();
        proxyRequest.presentation =
            (categoryValue.presentationMode == PrivacyPresentationMode::Blur)
                ? PrivacyStillProxyPresentation::Blurred
                : PrivacyStillProxyPresentation::Generic;
        const PrivacyStillProxyResult proxyResult = d->proxy.generate(proxyRequest);

        if (!proxyResult.isValid() ||
            (proxyResult.sha256 != prepared.assets.constFirst().proxy.sha256))
        {
            return fail(PrivacyStillItemTransactionStatus::ProxyFailure,
                        QStringLiteral("proxy replay bytes differ from Prepared facts"));
        }

        const PrivacyPublicReplacementStageResult stageResult =
            d->transition.stageReplacement(stageRequest, proxyResult.encodedBytes);

        if (!stageResult.succeeded())
        {
            PrivacyJournalObjectFact replayFact;

            if ((stageResult.error !=
                 PrivacyPublicTransitionError::UnexpectedExistingFile) ||
                !stableFileFact(absolutePath(request.publicRoot,
                                             replacementRelativePath),
                                nullptr, &replayFact) ||
                !sameFact(replayFact, prepared.assets.constFirst().proxy))
            {
                return fail(PrivacyStillItemTransactionStatus::ProxyFailure,
                            stageResult.detail);
            }
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterReplacementStageCreated))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        QStringLiteral("fault after exact proxy stage creation"));
        }

        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               prepared, staged, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterStagesPrepared))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        QStringLiteral("fault after exact stages prepared"));
        }
    }

    PrivacyJournalObjectFact finalArchiveFact;

    if (!stableFileFact(archivePath, nullptr, &finalArchiveFact))
    {
        bool published = false;

        if (d->durableReplay)
        {
            if (!QFileInfo::exists(archiveStagePath))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::AuthenticationRequired,
                    QStringLiteral("Prepared Protect archive stage must be recreated"));
            }

            published = d->archive.publishExactPreparedStage(
                archiveStagePath, archivePath,
                prepared.assets.constFirst().container.size,
                prepared.assets.constFirst().container.sha256);
        }
        else
        {
            PrivacyCasualArchiveStage archiveStage = d->archive.resumeStagedArchive(
                archiveStagePath, archivePath,
                prepared.assets.constFirst().container.size,
                prepared.assets.constFirst().container.sha256, password);
            published = archiveStage.isValid() &&
                        d->archive.publishNew(&archiveStage);
        }

        if (!published)
        {
            return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                        QStringLiteral("cannot publish verified archive"));
        }
    }

    if (!stableFileFact(archivePath, nullptr, &finalArchiveFact) ||
        !sameFact(finalArchiveFact, prepared.assets.constFirst().container) ||
        (!d->durableReplay &&
         !verifyArchiveMember(d->archive, prepared, archivePath, password)))
    {
        return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                    QStringLiteral("published archive fails exact verification"));
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterArchivePublished))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after archive publication"));
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot reload archive publication state"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (dbJournal &&
        (dbJournal->stage < static_cast<int>(PrivacyJournalStage::ProtectedCopyVerified)))
    {
        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               staged, protectedCopy, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
        }
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterProtectedCopyJournal))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after protected-copy journal"));
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot reload before public transition"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (dbJournal &&
        (dbJournal->stage < static_cast<int>(PrivacyJournalStage::PublicStateVerified)))
    {
        if (!d->cache.begin(request.imageId, sourcePath, true,
                            request.associatedAssetsAcknowledged &&
                            inventory.isReady()))
        {
            return fail(PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                        QStringLiteral("cannot begin protected-source cache transition"));
        }

        PrivacyJournalError loadError = PrivacyJournalError::None;
        std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
            PrivacyTransactionJournalStore::open(
                request.publicRoot.configuredPath, request.rootExpectation,
                &loadError, &detail);

        if (!journalStore)
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
        }

        const PrivacyJournalLoadResult loaded = journalStore->load(transactionUuid);

        if (!transitionJournalBoundToPayload(snapshot, prepared, loaded))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        QStringLiteral("public transition journal is not authoritative"));
        }
        PrivacyPublicTransitionRequest transitionRequest;
        transitionRequest.absoluteRootPath = request.publicRoot.configuredPath;
        transitionRequest.rootExpectation = request.rootExpectation;
        transitionRequest.journalRecord = loaded.record;
        transitionRequest.authoritativeJournalSha256 = loaded.sha256;
        transitionRequest.itemUuid = itemUuid;
        transitionRequest.role = PrivacyAsset::PrimaryMediaRole;
        transitionRequest.ordinal = 0;
        transitionRequest.mode = PrivacyPublicTransitionMode::ExchangePresent;
        transitionRequest.currentFact = PrivacyPublicTransitionFactKind::Original;
        transitionRequest.installedFact = PrivacyPublicTransitionFactKind::Proxy;
        const PrivacyPublicTransitionResult transitioned =
            d->transition.execute(transitionRequest);

        if (!transitioned.succeeded())
        {
            return fail(PrivacyStillItemTransactionStatus::PublicTransitionFailure,
                        transitioned.detail);
        }

        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               protectedCopy, publicVerified, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterPublicTransition))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        QStringLiteral("fault after public transition"));
        }
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot reload before completion"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (dbJournal &&
        (dbJournal->stage < static_cast<int>(PrivacyJournalStage::Complete)))
    {
        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               publicVerified, complete, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
        }
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterCompleteJournal))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after Complete journal"));
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot reload before DB publication"));
    }

    existingTransaction = transactionFor(snapshot, transactionUuid);
    existingItem = itemForImage(snapshot, request.imageId);

    if (!existingTransaction || !existingItem)
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Protect DB evidence disappeared"));
    }

    PrivacyItem publishedItem = *existingItem;
    publishedItem.expectedProxyHash =
        QString::fromLatin1(prepared.assets.constFirst().proxy.sha256.toHex());
    publishedItem.expectedProxySize = prepared.assets.constFirst().proxy.size;
    publishedItem.transactionState =
        static_cast<int>(PrivacyTransactionState::Complete);
    PrivacyContainer container;
    container.uuid = containerUuid;
    container.itemUuid = itemUuid;
    container.kind = PrivacyContainerKind::CasualArchive;
    container.rootUuid = request.publicRoot.uuid;
    container.objectRelativePath = archiveRelativePath;
    container.protectedSize = prepared.assets.constFirst().container.size;
    container.protectedHashAlgorithm = QLatin1String("sha256");
    container.protectedHash = QString::fromLatin1(
        prepared.assets.constFirst().container.sha256.toHex());
    container.formatVersion = 1;
    container.credentialGeneration = prepared.credentialGeneration;
    container.state = PrivacyContainerState::Verified;
    container.createdAt = existingTransaction->createdAt;
    container.updatedAt = QDateTime::currentDateTimeUtc();
    PrivacyAsset asset;
    asset.itemUuid = itemUuid;
    asset.role = PrivacyAsset::PrimaryMediaRole;
    asset.ordinal = 0;
    asset.originalName = QFileInfo(publicRelativePath).fileName();
    asset.publicRootUuid = request.publicRoot.uuid;
    asset.publicRelativePath = publicRelativePath;
    asset.containerUuid = containerUuid;
    asset.protectedRelativePath = prepared.assets.constFirst().protectedRelativePath;
    asset.hashAlgorithm = QLatin1String("sha256");
    asset.originalHash = publishedItem.originalHash;
    asset.originalSize = publishedItem.originalSize;
    asset.originalCreationDate = request.originalCreationDate;
    asset.originalModificationDate = originalModificationDate;
    asset.portableAttributes = encodePortableMode(originalMode);
    asset.proxyHashAlgorithm = QLatin1String("sha256");
    asset.proxyHash = publishedItem.expectedProxyHash;
    asset.proxySize = publishedItem.expectedProxySize;
    asset.proxyPresentationVersion = publishedItem.presentationVersion;
    asset.proxyGeneration = publishedItem.generation;
    QList<PrivacyAsset> assets = { asset };

    if (existingTransaction->state == PrivacyTransactionState::Complete)
    {
        const PrivacyContainer* storedContainer = containerForItem(snapshot, itemUuid);
        const QList<PrivacyAsset> storedAssets = assetsForItem(snapshot, itemUuid);

        if (!storedContainer || (storedAssets.size() != 1))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral("completed mapping facts are incomplete"));
        }

        container = *storedContainer;
        assets = storedAssets;
        publishedItem = *existingItem;
    }

    PrivacyTransaction completedTransaction = *existingTransaction;

    if (existingTransaction->state != PrivacyTransactionState::Complete)
    {
        completedTransaction.state = PrivacyTransactionState::Complete;
        completedTransaction.generation = 2;
        completedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

        if (!d->persistence.publishProtection(publishedItem, container, assets,
                                              completedTransaction))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("cannot atomically publish protected mapping"));
        }
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterDatabasePublication))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after DB publication"));
    }

    if (!d->runtime.hasProtectedItem(publishedItem, container, assets) &&
        !d->runtime.publishProtectedItem(publishedItem, container, assets) &&
        !d->runtime.publishProtectedItemForProtectRecovery(
            publishedItem, container, assets, completedTransaction))
    {
        return fail(PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                    QStringLiteral("cannot hot-publish exact protected item"));
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterRuntimePublication))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after runtime publication"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (!d->cache.finish(
            request.imageId, sourcePath, true,
            dbJournal &&
            (dbJournal->stage >=
             static_cast<int>(PrivacyJournalStage::PublicStateVerified))))
    {
        return fail(PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                    QStringLiteral("cannot finish protected-source cache transition"));
    }

    PrivacyStillItemTransactionResult result;
    result.status = PrivacyStillItemTransactionStatus::Protected;
    result.transactionUuid = transactionUuid;
    result.itemUuid = itemUuid;
    return result;
}

PrivacyStillItemTransactionResult PrivacyStillItemTransactionEngine::unprotect(
    const PrivacyStillUnprotectRequest& request, const PrivacyPassword& password)
{
    const QString transactionUuid = normalizedUuid(request.transactionUuid);
    const auto fail = [&](PrivacyStillItemTransactionStatus status,
                          const QString& itemUuid, const QString& detail)
    {
        return failure(status, transactionUuid, itemUuid, detail);
    };

    if ((request.imageId <= 0) || !canonicalUuid(request.categoryUuid) ||
        !canonicalUuid(request.transactionUuid) ||
        ((!password.isValid() || !request.freshAuthenticationConfirmed) &&
         !d->durableReplay) ||
        !request.publicRoot.isValid() ||
        !sameRootExpectation(request.publicRoot, request.rootExpectation) ||
        (request.rootExpectation.markerUuid != request.publicRoot.markerUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest, {},
                    QStringLiteral("unprotect request/authentication is invalid"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure, {},
                    QStringLiteral("cannot load privacy snapshot"));
    }

    const PrivacyCategory* category = categoryFor(snapshot, request.categoryUuid);

    if (!category || (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active))
    {
        return fail(PrivacyStillItemTransactionStatus::CategoryUnavailable, {},
                    QStringLiteral("Casual category is not active"));
    }

    const PrivacyTransaction* transaction = transactionFor(snapshot,
                                                            transactionUuid);

    // Final-cleanup replay after the mapping has been atomically detached.
    if (transaction && transaction->itemUuid.isEmpty() &&
        (transaction->type == PrivacyTransactionType::UnprotectItem) &&
        (transaction->state == PrivacyTransactionState::Applying) &&
        (transaction->generation == 2))
    {
        PrivacyJournalRecord record;
        QString ignoredStage;

        QDateTime ignoredModificationDate;
        QByteArray teardownBytes;
        PrivacyItem teardownItem;
        PrivacyContainer teardownContainer;
        PrivacyAsset teardownAsset;
        QString priorProtectUuid;

        if (!decodePreparedPayload(transaction->payloadData, &record,
                                   &ignoredStage,
                                   &ignoredModificationDate, &teardownBytes) ||
            !decodeTeardownSnapshot(teardownBytes, &teardownItem,
                                    &teardownContainer, &teardownAsset,
                                    &priorProtectUuid) ||
            (record.assets.size() != 1) ||
            (record.categoryUuid != request.categoryUuid) ||
            (teardownItem.imageId != request.imageId) ||
            (teardownItem.uuid != record.assets.constFirst().itemUuid) ||
            (teardownAsset.publicRootUuid != request.publicRoot.uuid))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, {},
                        QStringLiteral("detached Unprotect evidence is invalid"));
        }

        const PrivacyJournalAsset& journalAsset = record.assets.constFirst();
        const QString itemUuid = journalAsset.itemUuid;
        const QString publicPath = absolutePath(request.publicRoot,
                                                journalAsset.publicRelativePath);
        const PrivacyTransactionJournal* const replayJournal =
            databaseJournalFor(snapshot, transactionUuid,
                               request.publicRoot.uuid);
        PrivacyJournalObjectFact publicFact;
        mode_t publicMode = 0;
        mode_t expectedMode = 0;
        QDateTime publicModificationDate;

        if (!decodePortableMode(teardownAsset.portableAttributes,
                                &expectedMode) ||
            !stableFileFact(publicPath, nullptr, &publicFact, &publicMode,
                            &publicModificationDate) ||
            !sameFact(publicFact, journalAsset.original) ||
            (publicMode != expectedMode) ||
            (publicModificationDate !=
             teardownAsset.originalModificationDate.toUTC()))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        itemUuid,
                        QStringLiteral("restored public original is not exact"));
        }

        const QList<PrivacyAsset> teardownAssets = { teardownAsset };

        if (d->runtime.hasProtectedItem(teardownItem, teardownContainer,
                                        teardownAssets) &&
            !d->runtime.removeProtectedItem(teardownItem, teardownContainer,
                                            teardownAssets))
        {
            return fail(PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                        itemUuid,
                        QStringLiteral("detached mapping remains in live runtime"));
        }

        if (d->runtime.publicSourceDisposition(teardownItem.imageId) !=
            PrivacyPublicSourceDisposition::Unprotected)
        {
            return fail(PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                        itemUuid,
                        QStringLiteral("detached runtime state is not exact or absent"));
        }

        if (!d->cache.finish(
                teardownItem.imageId, publicPath, false,
                replayJournal &&
                (replayJournal->stage >=
                 static_cast<int>(PrivacyJournalStage::PublicStateVerified))))
        {
            return fail(PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                        itemUuid,
                        QStringLiteral("cannot finish replayed unprotect cache transition"));
        }

        if (!removeExactFile(request.publicRoot, request.rootExpectation,
                             journalAsset.containerRelativePath,
                             journalAsset.container, true) ||
            !removeExactFile(request.publicRoot, request.rootExpectation,
                             journalAsset.stagedRelativePath,
                             journalAsset.proxy, true))
        {
            return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                        itemUuid,
                        QStringLiteral("exact archive/proxy cleanup is pending"));
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterArchiveCleanup))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                        QStringLiteral("fault after archive cleanup"));
        }

        if (!d->persistence.finalizeUnprotection(transactionUuid,
                                                 request.categoryUuid))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        itemUuid,
                        QStringLiteral("cannot delete final detached DB evidence"));
        }

        PrivacyStillItemTransactionResult result;
        result.status = PrivacyStillItemTransactionStatus::Unprotected;
        result.transactionUuid = transactionUuid;
        result.itemUuid = itemUuid;
        return result;
    }

    if (d->durableReplay && !d->authenticatedCreatedReplay && transaction &&
        (transaction->state == PrivacyTransactionState::Created))
    {
        return fail(
            PrivacyStillItemTransactionStatus::AuthenticationRequired,
            transaction->itemUuid,
            QStringLiteral("Created Unprotect requires fresh authentication"));
    }

    const PrivacyItem* itemPointer = itemForImage(snapshot, request.imageId);

    if (!itemPointer || (itemPointer->categoryUuid != request.categoryUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest, {},
                    QStringLiteral("image is not protected by requested category"));
    }

    PrivacyItem item = *itemPointer;
    const QString itemUuid = item.uuid;
    const PrivacyContainer* containerPointer = containerForItem(snapshot, itemUuid);
    const QList<PrivacyAsset> assets = assetsForItem(snapshot, itemUuid);

    if (!containerPointer || (assets.size() != 1) ||
        (containerPointer->kind != PrivacyContainerKind::CasualArchive) ||
        (containerPointer->state != PrivacyContainerState::Verified) ||
        (assets.constFirst().role != PrivacyAsset::PrimaryMediaRole) ||
        (assets.constFirst().ordinal != 0) ||
        (assets.constFirst().publicRootUuid != request.publicRoot.uuid))
    {
        return fail(PrivacyStillItemTransactionStatus::AssociatedAssetSetUnsupported,
                    itemUuid,
                    QStringLiteral("mapping is not one exact Casual primary asset"));
    }

    const PrivacyContainer container = *containerPointer;
    const PrivacyAsset asset = assets.constFirst();
    mode_t restoredMode = 0;

    if (!decodePortableMode(asset.portableAttributes, &restoredMode) ||
        !asset.originalModificationDate.isValid())
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    itemUuid,
                    QStringLiteral("original mode/mtime evidence is invalid"));
    }

    const QString publicPath = absolutePath(request.publicRoot,
                                            asset.publicRelativePath);
    const QString archivePath = absolutePath(request.publicRoot,
                                             container.objectRelativePath);
    const QString replacementRelativePath =
        parentPath(asset.publicRelativePath) +
        (parentPath(asset.publicRelativePath).isEmpty()
             ? QString() : QLatin1String("/")) +
        PrivacyPublicTransitionEngine::expectedStageFileName(
            transactionUuid, PrivacyAsset::PrimaryMediaRole, 0);

    if (publicPath.isEmpty() || archivePath.isEmpty() ||
        replacementRelativePath.isEmpty())
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest, itemUuid,
                    QStringLiteral("stored public/archive path is unsafe"));
    }

    PrivacyJournalObjectFact proxyFact;
    proxyFact.presence = PrivacyJournalExpectedPresence::Present;
    proxyFact.size = asset.proxySize;
    proxyFact.linkCount = 1;
    proxyFact.sha256 = QByteArray::fromHex(asset.proxyHash.toLatin1());
    PrivacyJournalObjectFact originalFact;
    originalFact.presence = PrivacyJournalExpectedPresence::Present;
    originalFact.size = asset.originalSize;
    originalFact.linkCount = 1;
    originalFact.sha256 = QByteArray::fromHex(asset.originalHash.toLatin1());
    PrivacyJournalObjectFact publicFact;
    PrivacyJournalObjectFact archiveFact;

    if (!stableFileFact(publicPath, nullptr, &publicFact) ||
        !stableFileFact(archivePath, nullptr, &archiveFact) ||
        (QString::fromLatin1(archiveFact.sha256.toHex()) !=
         container.protectedHash) ||
        (archiveFact.size != container.protectedSize))
    {
        return fail(PrivacyStillItemTransactionStatus::SourceChanged, itemUuid,
                    QStringLiteral("proxy/archive no longer match stored facts"));
    }

    if (transaction)
    {
        QString journalDetail;
        PrivacyJournalError journalError = PrivacyJournalError::None;
        std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
            PrivacyTransactionJournalStore::open(
                request.publicRoot.configuredPath, request.rootExpectation,
                &journalError, &journalDetail);
        const PrivacyJournalLoadResult loaded = journalStore
                                              ? journalStore->load(transactionUuid)
                                              : PrivacyJournalLoadResult();
        const PrivacyTransactionJournal* databaseJournal = databaseJournalFor(
            snapshot, transactionUuid, request.publicRoot.uuid);
        const bool exactUnboundCreated = journalStore && databaseJournal &&
            (loaded.disposition == PrivacyJournalLoadDisposition::Missing) &&
            (transaction->state == PrivacyTransactionState::Created) &&
            (transaction->generation == 0) &&
            (databaseJournal->stage ==
             static_cast<int>(PrivacyJournalStage::Created)) &&
            databaseJournal->expectedJournalHash.isEmpty();

        if (!exactUnboundCreated && (!journalStore ||
            (loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
            !loaded.authoritative || !loaded.hasRecord ||
            (loaded.record.transactionUuid != transactionUuid) ||
            (loaded.record.transactionType !=
             PrivacyTransactionType::UnprotectItem)))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        itemUuid,
                        QStringLiteral("Unprotect filesystem journal is not authoritative"));
        }

        PrivacyJournalObjectFact stagedFact;
        const bool stagePresent = stableFileFact(
            absolutePath(request.publicRoot, replacementRelativePath), nullptr,
            &stagedFact);

        if (exactUnboundCreated)
        {
            if (!sameFact(publicFact, proxyFact) || stagePresent)
            {
                return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                            itemUuid,
                            QStringLiteral("unbound Created namespace is not exact"));
            }
        }
        else
        {

            switch (loaded.record.stage)
            {
            case PrivacyJournalStage::Created:
            case PrivacyJournalStage::Prepared:
            {
                if (!sameFact(publicFact, proxyFact))
                {
                    return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                                itemUuid,
                                QStringLiteral("pre-stage public proxy is not exact"));
                }

                break;
            }

            case PrivacyJournalStage::Staged:
            case PrivacyJournalStage::ProtectedCopyVerified:
            {
                if (!sameFact(publicFact, proxyFact) || !stagePresent ||
                    !sameFact(stagedFact, originalFact))
                {
                    return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                                itemUuid,
                                QStringLiteral("pre-exchange public/staged pair is not exact"));
                }

                break;
            }

            case PrivacyJournalStage::Applying:
            {
                const bool exactPre = sameFact(publicFact, proxyFact) &&
                                      stagePresent &&
                                      sameFact(stagedFact, originalFact);
                const bool exactPost = sameFact(publicFact, originalFact) &&
                                       stagePresent &&
                                       sameFact(stagedFact, proxyFact);

                if (!exactPre && !exactPost)
                {
                    return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                                itemUuid,
                                QStringLiteral("Applying public/staged pair is ambiguous"));
                }

                break;
            }

            case PrivacyJournalStage::PublicStateVerified:
            case PrivacyJournalStage::Complete:
            {
                if (!sameFact(publicFact, originalFact) || !stagePresent ||
                    !sameFact(stagedFact, proxyFact))
                {
                    return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                                itemUuid,
                                QStringLiteral("post-exchange public/staged pair is not exact"));
                }

                break;
            }

            case PrivacyJournalStage::ReconciliationRequired:
            {
                return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                            itemUuid,
                            QStringLiteral("Unprotect journal requires reconciliation"));
            }
            }
        }
    }
    else if (!sameFact(publicFact, proxyFact))
    {
        return fail(PrivacyStillItemTransactionStatus::SourceChanged, itemUuid,
                    QStringLiteral("public proxy no longer matches stored facts"));
    }

    QString priorProtectTransactionUuid;
    int priorProtectCount = 0;

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if ((candidate.itemUuid == itemUuid) &&
            (candidate.categoryUuid == request.categoryUuid) &&
            (candidate.type == PrivacyTransactionType::ProtectItem) &&
            (candidate.state == PrivacyTransactionState::Complete) &&
            (candidate.generation == 2))
        {
            const PrivacyTransactionJournal* priorJournal = databaseJournalFor(
                snapshot, candidate.uuid, request.publicRoot.uuid);

            if (priorJournal &&
                (priorJournal->stage ==
                 static_cast<int>(PrivacyJournalStage::Complete)))
            {
                priorProtectTransactionUuid = candidate.uuid;
                ++priorProtectCount;
            }
        }
    }

    if (priorProtectCount != 1)
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, itemUuid,
                    QStringLiteral("exact prior completed Protect evidence is missing"));
    }

    const QByteArray exactTeardown = encodeTeardownSnapshot(
        item, container, asset, priorProtectTransactionUuid);

    if (exactTeardown.isEmpty())
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, itemUuid,
                    QStringLiteral("cannot encode exact teardown snapshot"));
    }

    PrivacyJournalRecord created;
    PrivacyJournalRecord prepared;
    QString payloadPath;
    QDateTime payloadModificationDate = asset.originalModificationDate;
    QByteArray payloadTeardown;

    if (!transaction)
    {
        created.transactionUuid = transactionUuid;
        created.categoryUuid = request.categoryUuid;
        created.rootUuid = request.publicRoot.uuid;
        created.rootDevice = request.rootExpectation.device;
        created.rootInode = request.rootExpectation.inode;
        created.rootIdentitySha256 = request.rootExpectation.identitySha256;
        created.transactionType = PrivacyTransactionType::UnprotectItem;
        created.generation = item.generation;
        created.credentialGeneration = container.credentialGeneration;
        created.fromCredentialGeneration = container.credentialGeneration;
        created.toCredentialGeneration = container.credentialGeneration;
        created.stage = PrivacyJournalStage::Created;
        PrivacyJournalAsset journalAsset;
        journalAsset.itemUuid = itemUuid;
        journalAsset.containerUuid = container.uuid;
        journalAsset.role = asset.role;
        journalAsset.ordinal = asset.ordinal;
        journalAsset.publicRelativePath = asset.publicRelativePath;
        journalAsset.stagedRelativePath = replacementRelativePath;
        journalAsset.protectedRelativePath = asset.protectedRelativePath;
        journalAsset.containerRelativePath = container.objectRelativePath;
        journalAsset.original.presence = PrivacyJournalExpectedPresence::Present;
        journalAsset.original.size = asset.originalSize;
        journalAsset.original.linkCount = 1;
        journalAsset.original.sha256 = QByteArray::fromHex(
            asset.originalHash.toLatin1());
        journalAsset.proxy = proxyFact;
        journalAsset.container = archiveFact;
        created.assets << journalAsset;

        if (!verifyArchiveMember(d->archive, created, archivePath, password))
        {
            return fail(PrivacyStillItemTransactionStatus::ArchiveFailure, itemUuid,
                        QStringLiteral("archive/member/password verification failed"));
        }

        PrivacyTransaction begin;
        begin.uuid = transactionUuid;
        begin.categoryUuid = request.categoryUuid;
        begin.itemUuid = itemUuid;
        begin.type = PrivacyTransactionType::UnprotectItem;
        begin.state = PrivacyTransactionState::Created;
        begin.generation = 0;
        begin.fromCredentialGeneration = container.credentialGeneration;
        begin.toCredentialGeneration = container.credentialGeneration;
        begin.payloadFormatVersion = 1;
        begin.payloadData = encodePreparedPayload(
            created, container.objectRelativePath, payloadModificationDate,
            exactTeardown);
        begin.createdAt = QDateTime::currentDateTimeUtc();
        begin.updatedAt = begin.createdAt;
        PrivacyTransactionJournal journal;
        journal.transactionUuid = transactionUuid;
        journal.rootUuid = request.publicRoot.uuid;
        journal.journalRelativePath =
            PrivacyTransactionJournalCodec::relativeJournalPath(transactionUuid);
        journal.journalFormatVersion = PrivacyTransactionJournalCodec::FormatVersion;
        journal.stage = static_cast<int>(PrivacyJournalStage::Created);
        journal.updatedAt = begin.createdAt;

        if (begin.payloadData.isEmpty() ||
            !d->persistence.beginUnprotection(begin, journal))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        itemUuid,
                        QStringLiteral("cannot atomically begin Unprotect"));
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterDatabaseBegin))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                        QStringLiteral("fault after Unprotect DB begin"));
        }

        if (!d->load(&snapshot))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        itemUuid, QStringLiteral("cannot reload Unprotect begin"));
        }

        transaction = transactionFor(snapshot, transactionUuid);
    }

    if (!transaction || (transaction->type != PrivacyTransactionType::UnprotectItem) ||
        (transaction->categoryUuid != request.categoryUuid) ||
        (transaction->itemUuid != itemUuid) ||
        !decodePreparedPayload(transaction->payloadData, &created, &payloadPath,
                               &payloadModificationDate, &payloadTeardown) ||
        (payloadTeardown != exactTeardown))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, itemUuid,
                    QStringLiteral("Unprotect transaction/payload is not exact"));
    }

    QByteArray journalHash;
    QString detail;

    if (transaction->state == PrivacyTransactionState::Created)
    {
        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               created, created, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        itemUuid, detail);
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterFilesystemJournal))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        itemUuid,
                        QStringLiteral("fault after Unprotect Created journal"));
        }

        prepared = recordAt(created, PrivacyJournalStage::Prepared);
        PrivacyTransaction next = *transaction;
        next.state = PrivacyTransactionState::Prepared;
        next.generation = 1;
        next.payloadData = encodePreparedPayload(
            prepared, container.objectRelativePath, payloadModificationDate,
            exactTeardown);
        next.updatedAt = QDateTime::currentDateTimeUtc();

        if (!d->persistence.compareAndUpdateTransaction(
                next, PrivacyTransactionState::Created, 0))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        itemUuid,
                        QStringLiteral("cannot publish Prepared Unprotect"));
        }
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    itemUuid, QStringLiteral("cannot reload Prepared Unprotect"));
    }

    transaction = transactionFor(snapshot, transactionUuid);

    if (!transaction ||
        !decodePreparedPayload(transaction->payloadData, &prepared, &payloadPath,
                               &payloadModificationDate, &payloadTeardown) ||
        (payloadTeardown != exactTeardown) ||
        (prepared.stage != PrivacyJournalStage::Prepared))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, itemUuid,
                    QStringLiteral("Prepared Unprotect evidence is invalid"));
    }

    const PrivacyJournalRecord staged = recordAt(prepared,
                                                  PrivacyJournalStage::Staged);
    const PrivacyJournalRecord protectedCopy = recordAt(
        prepared, PrivacyJournalStage::ProtectedCopyVerified);
    const PrivacyJournalRecord publicVerified = recordAt(
        prepared, PrivacyJournalStage::PublicStateVerified);
    const PrivacyJournalRecord complete = recordAt(prepared,
                                                    PrivacyJournalStage::Complete);
    const PrivacyTransactionJournal* dbJournal = databaseJournalFor(
        snapshot, transactionUuid, request.publicRoot.uuid);

    if (!dbJournal)
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure, itemUuid,
                    QStringLiteral("Unprotect database journal is missing"));
    }

    if (dbJournal->stage < static_cast<int>(PrivacyJournalStage::Staged))
    {
        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               created, prepared, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        itemUuid, detail);
        }

        PrivacyPublicReplacementStageRequest stageRequest;
        stageRequest.absoluteRootPath = request.publicRoot.configuredPath;
        stageRequest.rootExpectation = request.rootExpectation;
        stageRequest.journalRecord = prepared;
        stageRequest.authoritativeJournalSha256 = journalHash;
        stageRequest.itemUuid = itemUuid;
        stageRequest.role = asset.role;
        stageRequest.ordinal = asset.ordinal;
        const QString stagedOriginalPath = absolutePath(
            request.publicRoot, replacementRelativePath);
        PrivacyJournalObjectFact replayFact;

        if (d->durableReplay && !d->authenticatedCreatedReplay)
        {
            if (!QFileInfo::exists(stagedOriginalPath))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::AuthenticationRequired,
                    itemUuid,
                    QStringLiteral("Prepared Unprotect original stage must be restored"));
            }

            if (!stableFileFact(stagedOriginalPath, nullptr, &replayFact) ||
                !sameFact(replayFact, prepared.assets.constFirst().original))
            {
                return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                            itemUuid,
                            QStringLiteral("Prepared Unprotect original stage is not exact"));
            }
        }
        else
        {
            PrivacyCasualArchiveRestoreRequest restore;
            restore.archivePath = archivePath;
            restore.categoryUuid = request.categoryUuid;
            restore.containerUuid = container.uuid;
            restore.itemUuid = itemUuid;
            restore.protectedRelativePath = asset.protectedRelativePath;
            restore.originalName = asset.originalName;
            restore.role = asset.role;
            restore.ordinal = asset.ordinal;
            restore.expectedArchiveSize = archiveFact.size;
            restore.expectedArchiveSha256 = archiveFact.sha256;
            restore.expectedMemberSize = asset.originalSize;
            restore.expectedMemberSha256 = QByteArray::fromHex(
                asset.originalHash.toLatin1());
            const PrivacyPublicReplacementStageResult stageResult =
                d->transition.stageReplacement(
                    stageRequest,
                    [&](int descriptor, QString* producerDetail)
                    {
                        QFile destination;

                        if (!destination.open(descriptor, QIODevice::WriteOnly,
                                              QFileDevice::DontCloseHandle))
                        {
                            if (producerDetail)
                            {
                                *producerDetail = QStringLiteral(
                                    "cannot attach archive restore destination");
                            }

                            return false;
                        }

                        const bool restored = d->archive.restoreMember(
                            restore, password, &destination);
                        destination.close();

                        if (!restored)
                        {
                            return false;
                        }

                        const qint64 milliseconds =
                            asset.originalModificationDate.toMSecsSinceEpoch();
                        struct timespec times[2] = {};
                        times[0].tv_nsec = UTIME_OMIT;
                        times[1].tv_sec = milliseconds / 1000;
                        times[1].tv_nsec = (milliseconds % 1000) * 1000000;
                        return (::futimens(descriptor, times) == 0);
                    });

            if (!stageResult.succeeded() &&
                ((stageResult.error !=
                  PrivacyPublicTransitionError::UnexpectedExistingFile) ||
                 !stableFileFact(stagedOriginalPath, nullptr, &replayFact) ||
                 !sameFact(replayFact, prepared.assets.constFirst().original)))
            {
                return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                            itemUuid, stageResult.detail);
            }
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterReplacementStageCreated))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        itemUuid,
                        QStringLiteral("fault after exact original stage creation"));
        }

        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               prepared, staged, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        itemUuid, detail);
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterStagesPrepared))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        itemUuid,
                        QStringLiteral("fault after Unprotect stage preparation"));
        }
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    itemUuid, QStringLiteral("cannot reload staged Unprotect"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (dbJournal &&
        (dbJournal->stage < static_cast<int>(PrivacyJournalStage::ProtectedCopyVerified)) &&
        !d->advanceJournal(request.publicRoot, request.rootExpectation,
                           staged, protectedCopy, &journalHash, &detail))
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                    itemUuid, detail);
    }


    if (d->fault(PrivacyStillItemFaultPoint::AfterProtectedCopyJournal))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                    QStringLiteral("fault after Unprotect protected-copy journal"));
    }

    if (!d->cache.begin(request.imageId, publicPath, false, false))
    {
        return fail(PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                    itemUuid,
                    QStringLiteral("cannot begin Unprotect cache transition"));
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    itemUuid, QStringLiteral("cannot reload public Unprotect"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (dbJournal &&
        (dbJournal->stage < static_cast<int>(PrivacyJournalStage::PublicStateVerified)))
    {
        PrivacyJournalError loadError = PrivacyJournalError::None;
        std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
            PrivacyTransactionJournalStore::open(
                request.publicRoot.configuredPath, request.rootExpectation,
                &loadError, &detail);

        if (!journalStore)
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        itemUuid, detail);
        }

        const PrivacyJournalLoadResult loaded = journalStore->load(transactionUuid);

        if (!transitionJournalBoundToPayload(snapshot, prepared, loaded))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        itemUuid,
                        QStringLiteral("Unprotect transition journal is not authoritative"));
        }

        PrivacyPublicTransitionRequest transitionRequest;
        transitionRequest.absoluteRootPath = request.publicRoot.configuredPath;
        transitionRequest.rootExpectation = request.rootExpectation;
        transitionRequest.journalRecord = loaded.record;
        transitionRequest.authoritativeJournalSha256 = loaded.sha256;
        transitionRequest.itemUuid = itemUuid;
        transitionRequest.role = asset.role;
        transitionRequest.ordinal = asset.ordinal;
        transitionRequest.mode = PrivacyPublicTransitionMode::ExchangePresent;
        transitionRequest.currentFact = PrivacyPublicTransitionFactKind::Proxy;
        transitionRequest.installedFact = PrivacyPublicTransitionFactKind::Original;
        transitionRequest.installedUnixMode = static_cast<int>(restoredMode);
        const PrivacyPublicTransitionResult transitioned =
            d->transition.execute(transitionRequest);

        if (!transitioned.succeeded())
        {
            return fail(PrivacyStillItemTransactionStatus::PublicTransitionFailure,
                        itemUuid, transitioned.detail);
        }

        if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                               protectedCopy, publicVerified, &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        itemUuid, detail);
        }
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterPublicTransition))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                    QStringLiteral("fault after Unprotect public transition"));
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    itemUuid, QStringLiteral("cannot reload complete Unprotect"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (dbJournal &&
        (dbJournal->stage < static_cast<int>(PrivacyJournalStage::Complete)) &&
        !d->advanceJournal(request.publicRoot, request.rootExpectation,
                           publicVerified, complete, &journalHash, &detail))
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                    itemUuid, detail);
    }


    if (d->fault(PrivacyStillItemFaultPoint::AfterCompleteJournal))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                    QStringLiteral("fault after Unprotect Complete journal"));
    }

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    itemUuid, QStringLiteral("cannot reload DB teardown state"));
    }

    transaction = transactionFor(snapshot, transactionUuid);

    if (!transaction)
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, itemUuid,
                    QStringLiteral("Unprotect transaction disappeared"));
    }

    PrivacyTransaction applying = *transaction;
    applying.state = PrivacyTransactionState::Applying;
    applying.generation = 2;
    applying.updatedAt = QDateTime::currentDateTimeUtc();

    if (!d->persistence.publishUnprotection(
            request.imageId, itemUuid, request.categoryUuid, item.generation,
            priorProtectTransactionUuid, applying))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    itemUuid,
                    QStringLiteral("cannot atomically detach protected mapping"));
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterUnprotectDatabaseTeardown))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                    QStringLiteral("fault after Unprotect DB teardown"));
    }

    if (d->runtime.hasProtectedItem(item, container, assets))
    {
        if (!d->runtime.removeProtectedItem(item, container, assets))
        {
            return fail(PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                        itemUuid,
                        QStringLiteral("cannot hot-remove exact protected item"));
        }
    }
    else if ((d->runtime.publicSourceDisposition(request.imageId) !=
              PrivacyPublicSourceDisposition::Unprotected) &&
             !d->runtime.removeProtectedItemForUnprotectRecovery(
                 item, container, assets, transactionUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                    itemUuid,
                    QStringLiteral("cannot remove exact recovering protected item"));
    }

    if (d->runtime.publicSourceDisposition(request.imageId) !=
        PrivacyPublicSourceDisposition::Unprotected)
    {
        return fail(PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                    itemUuid,
                    QStringLiteral("runtime item is mismatched rather than absent"));
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterUnprotectRuntimeRemoval))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                    QStringLiteral("fault after Unprotect runtime removal"));
    }

    dbJournal = databaseJournalFor(snapshot, transactionUuid,
                                   request.publicRoot.uuid);

    if (!d->cache.finish(
            request.imageId, publicPath, false,
            dbJournal &&
            (dbJournal->stage >=
             static_cast<int>(PrivacyJournalStage::PublicStateVerified))))
    {
        return fail(PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                    itemUuid,
                    QStringLiteral("cannot finish Unprotect cache transition"));
    }

    if (!removeExactFile(request.publicRoot, request.rootExpectation,
                         container.objectRelativePath, archiveFact, true) ||
        !removeExactFile(request.publicRoot, request.rootExpectation,
                         replacementRelativePath, proxyFact, true))
    {
        return fail(PrivacyStillItemTransactionStatus::CleanupPending, itemUuid,
                    QStringLiteral("exact archive/proxy cleanup is pending"));
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterArchiveCleanup))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected, itemUuid,
                    QStringLiteral("fault after archive cleanup"));
    }

    if (!d->persistence.finalizeUnprotection(transactionUuid,
                                             request.categoryUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    itemUuid,
                    QStringLiteral("cannot delete final detached DB evidence"));
    }

    PrivacyStillItemTransactionResult result;
    result.status = PrivacyStillItemTransactionStatus::Unprotected;
    result.transactionUuid = transactionUuid;
    result.itemUuid = itemUuid;
    return result;
}

PrivacyStillItemTransactionResult PrivacyStillItemTransactionEngine::recover(
    const PrivacyStorageRoot& publicRoot, const QString& transactionUuidText)
{
    return recoverInternal(publicRoot, transactionUuidText, nullptr, false);
}

PrivacyStillItemTransactionResult
PrivacyStillItemTransactionEngine::resumeAuthenticated(
    const PrivacyStorageRoot& publicRoot, const QString& transactionUuidText,
    const PrivacyPassword& verifiedPassword, bool freshAuthenticationConfirmed)
{
    return recoverInternal(publicRoot, transactionUuidText, &verifiedPassword,
                           freshAuthenticationConfirmed);
}

PrivacyStillItemTransactionResult
PrivacyStillItemTransactionEngine::recoverInternal(
    const PrivacyStorageRoot& publicRoot, const QString& transactionUuidText,
    const PrivacyPassword* const verifiedPassword,
    bool freshAuthenticationConfirmed)
{
    const QString transactionUuid = normalizedUuid(transactionUuidText);
    const auto fail = [&](PrivacyStillItemTransactionStatus status,
                          const QString& itemUuid, const QString& detail)
    {
        return failure(status, transactionUuid, itemUuid, detail);
    };

    if (!canonicalUuid(transactionUuidText) || !publicRoot.isValid() ||
        (publicRoot.kind != PrivacyStorageRootKind::AlbumRoot) ||
        d->durableReplay || (verifiedPassword && !verifiedPassword->isValid()))
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest, {},
                    QStringLiteral("durable recovery request is invalid"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure, {},
                    QStringLiteral("cannot load durable recovery snapshot"));
    }

    const PrivacyTransaction* const transaction = transactionFor(snapshot,
                                                                  transactionUuid);

    if (!transaction || !transaction->isValid() ||
        (transaction->payloadFormatVersion != 1) ||
        ((transaction->type != PrivacyTransactionType::ProtectItem) &&
         (transaction->type != PrivacyTransactionType::UnprotectItem)))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, {},
                    QStringLiteral("recoverable still transaction is missing"));
    }

    PrivacyJournalRecord record;
    QString payloadPath;
    QDateTime originalModificationDate;
    QByteArray metadata;

    if (!decodePreparedPayload(transaction->payloadData, &record, &payloadPath,
                               &originalModificationDate, &metadata) ||
        (record.transactionUuid != transactionUuid) ||
        (record.categoryUuid != transaction->categoryUuid) ||
        (record.rootUuid != publicRoot.uuid) ||
        (record.transactionType != transaction->type) ||
        (record.assets.size() != 1) ||
        (record.assets.constFirst().role != PrivacyAsset::PrimaryMediaRole) ||
        (record.assets.constFirst().ordinal != 0) ||
        (transaction->fromCredentialGeneration !=
         record.fromCredentialGeneration) ||
        (transaction->toCredentialGeneration != record.toCredentialGeneration))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    transaction->itemUuid,
                    QStringLiteral("durable still payload identity is not exact"));
    }

    const PrivacyStorageRoot* const storedRoot = storageRootFor(snapshot,
                                                                record.rootUuid);

    if (!storedRoot || (storedRoot->kind != publicRoot.kind) ||
        (storedRoot->albumRootId != publicRoot.albumRootId) ||
        (QDir::cleanPath(storedRoot->configuredPath) !=
         QDir::cleanPath(publicRoot.configuredPath)) ||
        (storedRoot->identityVersion != publicRoot.identityVersion) ||
        (storedRoot->identityData != publicRoot.identityData) ||
        (storedRoot->markerUuid != publicRoot.markerUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::RootUnavailable,
                    transaction->itemUuid,
                    QStringLiteral("registered durable root is not exact"));
    }

    const bool created = (transaction->state == PrivacyTransactionState::Created) &&
                         (transaction->generation == 0) &&
                         (record.stage == PrivacyJournalStage::Created);
    const bool resumable =
        (record.stage == PrivacyJournalStage::Prepared) &&
        (((transaction->state == PrivacyTransactionState::Prepared) &&
          (transaction->generation == 1)) ||
         ((transaction->type == PrivacyTransactionType::ProtectItem) &&
          (transaction->state == PrivacyTransactionState::Complete) &&
          (transaction->generation == 2)) ||
         ((transaction->type == PrivacyTransactionType::UnprotectItem) &&
          (transaction->state == PrivacyTransactionState::Applying) &&
          (transaction->generation == 2)));

    if (!created && !resumable)
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    transaction->itemUuid,
                    QStringLiteral("durable still state/generation is not resumable"));
    }

    PrivacyJournalRootExpectation rootExpectation;
    rootExpectation.rootUuid = record.rootUuid;
    rootExpectation.markerUuid = publicRoot.markerUuid;
    rootExpectation.identitySha256 = record.rootIdentitySha256;
    rootExpectation.device = record.rootDevice;
    rootExpectation.inode = record.rootInode;

    if (!sameRootExpectation(publicRoot, rootExpectation))
    {
        return fail(PrivacyStillItemTransactionStatus::RootUnavailable,
                    transaction->itemUuid,
                    QStringLiteral("durable still root identity is not exact"));
    }

    const PrivacyJournalAsset& journalAsset = record.assets.constFirst();
    const PrivacyItem* item = itemForUuid(snapshot, journalAsset.itemUuid);
    qlonglong imageId = item ? item->imageId : -1;
    PrivacyItem teardownItem;

    if (!item && (transaction->type == PrivacyTransactionType::UnprotectItem) &&
        (transaction->state == PrivacyTransactionState::Applying))
    {
        PrivacyContainer teardownContainer;
        PrivacyAsset teardownAsset;
        QString priorProtectTransactionUuid;

        if (!decodeTeardownSnapshot(metadata, &teardownItem, &teardownContainer,
                                    &teardownAsset,
                                    &priorProtectTransactionUuid) ||
            (teardownItem.uuid != journalAsset.itemUuid) ||
            (teardownItem.categoryUuid != transaction->categoryUuid) ||
            (teardownAsset.publicRootUuid != publicRoot.uuid))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        journalAsset.itemUuid,
                        QStringLiteral("detached durable teardown is not exact"));
        }

        imageId = teardownItem.imageId;
    }

    if ((imageId <= 0) ||
        (!transaction->itemUuid.isEmpty() &&
         (transaction->itemUuid != journalAsset.itemUuid)))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    journalAsset.itemUuid,
                    QStringLiteral("durable still item mapping is not exact"));
    }

    if (created)
    {
        if (!verifiedPassword ||
            ((transaction->type == PrivacyTransactionType::UnprotectItem) &&
             !freshAuthenticationConfirmed))
        {
            return fail(PrivacyStillItemTransactionStatus::AuthenticationRequired,
                        journalAsset.itemUuid,
                        (transaction->type == PrivacyTransactionType::ProtectItem)
                            ? QStringLiteral("Created Protect requires category authentication")
                            : QStringLiteral("Created Unprotect requires fresh authentication"));
        }
    }

    ScopedBooleanValue replayScope(d->durableReplay, true);
    ScopedBooleanValue authenticatedScope(d->authenticatedCreatedReplay,
                                           created &&
                                           (verifiedPassword != nullptr));
    const PrivacyPassword noPassword = PrivacyPassword::fromUnicode(QString());
    const PrivacyPassword& replayPassword = created ? *verifiedPassword
                                                    : noPassword;

    if (transaction->type == PrivacyTransactionType::ProtectItem)
    {
        const QString publicPath = absolutePath(publicRoot,
                                                journalAsset.publicRelativePath);
        quint64 publicDevice = 0;
        quint64 publicInode = 0;
        quint64 publicLinkCount = 0;
        qlonglong publicSize = -1;

#if defined(Q_OS_UNIX)
        struct stat status = {};
        const QByteArray encodedPath = QFile::encodeName(publicPath);

        if (publicPath.isEmpty() ||
            (::lstat(encodedPath.constData(), &status) != 0) ||
            !S_ISREG(status.st_mode) || S_ISLNK(status.st_mode) ||
            (status.st_nlink < 1))
        {
            return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                        journalAsset.itemUuid,
                        QStringLiteral("durable Protect public file is not regular"));
        }

        publicDevice = static_cast<quint64>(status.st_dev);
        publicInode = static_cast<quint64>(status.st_ino);
        publicLinkCount = static_cast<quint64>(status.st_nlink);
        publicSize = static_cast<qlonglong>(status.st_size);
#else
        return fail(PrivacyStillItemTransactionStatus::RootUnavailable,
                    journalAsset.itemUuid,
                    QStringLiteral("durable still recovery is unsupported"));
#endif

        PrivacyInventoryAsset inventoryAsset;
        inventoryAsset.role = PrivacyInventoryAssetRole::PrimaryMedia;
        inventoryAsset.ordinal = 0;
        inventoryAsset.location.root.uuid = publicRoot.uuid;
        inventoryAsset.location.root.absolutePath = publicRoot.configuredPath;
        inventoryAsset.location.relativePath = journalAsset.publicRelativePath;
        inventoryAsset.evidence.type = PrivacyInventoryFileType::Regular;
        inventoryAsset.evidence.identityComplete = true;
        inventoryAsset.evidence.deviceId = publicDevice;
        inventoryAsset.evidence.inode = publicInode;
        inventoryAsset.evidence.linkCount = publicLinkCount;
        inventoryAsset.evidence.byteSize = publicSize;
        PrivacyAssetInventoryBridgeItemResult bridgeItem;
        bridgeItem.imageId = imageId;
        bridgeItem.inventory.status = PrivacyInventoryStatus::Ready;
        bridgeItem.inventory.requiredAssets << inventoryAsset;
        PrivacyStillProtectRequest request;
        request.imageId = imageId;
        request.categoryUuid = transaction->categoryUuid;
        request.itemUuid = journalAsset.itemUuid;
        request.containerUuid = journalAsset.containerUuid;
        request.transactionUuid = transactionUuid;
        request.preflight.bridge.status = PrivacyInventoryStatus::Ready;
        request.preflight.bridge.items << bridgeItem;
        request.associatedAssetsAcknowledged = true;
        request.publicRoot = publicRoot;
        request.rootExpectation = rootExpectation;
        request.originalPixelSize = QSize(item->originalWidth,
                                          item->originalHeight);
        request.originalCreationDate = item->originalCreationDate;

        return protect(request, replayPassword);
    }

    PrivacyStillUnprotectRequest request;
    request.imageId = imageId;
    request.categoryUuid = transaction->categoryUuid;
    request.transactionUuid = transactionUuid;
    request.publicRoot = publicRoot;
    request.rootExpectation = rootExpectation;
    request.freshAuthenticationConfirmed = created &&
                                           freshAuthenticationConfirmed;

    return unprotect(request, replayPassword);
}

} // namespace Digikam
