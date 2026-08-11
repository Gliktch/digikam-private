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
#include "privacystrongobjectbackend.h"
#include "privacyprocessrunner.h"
#include "privacyproxygenerator.h"
#include "privacypublictransition.h"
#include "privacyrepository.h"
#include "privacyvideoproxygenerator.h"
#include "privacyposixstorage_p.h"

namespace Digikam
{

namespace
{

constexpr qsizetype IoChunkBytes = 1024 * 1024;
constexpr qsizetype MaximumPreparedProxyBytes = 8 * 1024 * 1024;

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

QString protectArchiveStageRelativePath(const QString& publicRelativePath,
                                        const QString& transactionUuid)
{
    const QString parent = parentPath(publicRelativePath);
    return parent + (parent.isEmpty() ? QString() : QLatin1String("/")) +
           QLatin1String(".digikam-private-stage-") + transactionUuid +
           QLatin1String(".zip");
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

bool singleItemContainerMatches(const PrivacyJournalRecord& record)
{
    if (record.assets.isEmpty())
    {
        return false;
    }

    const PrivacyJournalAsset& first = record.assets.constFirst();

    if (first.containerRelativePath.isEmpty())
    {
        return false;
    }

    int primaryCount = 0;

    for (const PrivacyJournalAsset& asset : record.assets)
    {
        if ((asset.itemUuid != first.itemUuid) ||
            (asset.containerUuid != first.containerUuid) ||
            (asset.containerRelativePath != first.containerRelativePath) ||
            !sameFact(asset.container, first.container))
        {
            return false;
        }

        primaryCount += ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                         (asset.ordinal == 0)) ? 1 : 0;
    }

    return (primaryCount == 1);
}

bool preparedProxyPayloadMatches(const PrivacyTransaction& transaction,
                                 const PrivacyJournalRecord& prepared,
                                 const QByteArray& proxyBytes)
{
    if ((prepared.stage != PrivacyJournalStage::Prepared) ||
        prepared.assets.isEmpty())
    {
        return false;
    }

    const PrivacyJournalAsset* primary = nullptr;

    for (const PrivacyJournalAsset& asset : prepared.assets)
    {
        if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
            (asset.ordinal == 0))
        {
            if (primary)
            {
                return false;
            }

            primary = &asset;
        }
        else if ((asset.proxy.presence !=
                  PrivacyJournalExpectedPresence::Absent) ||
                 (asset.proxy.size != -1) || (asset.proxy.linkCount != 0) ||
                 !asset.proxy.sha256.isEmpty())
        {
            return false;
        }
    }

    if (!primary ||
        (primary->proxy.presence != PrivacyJournalExpectedPresence::Present))
    {
        return false;
    }

    if (transaction.state == PrivacyTransactionState::Complete)
    {
        return proxyBytes.isEmpty();
    }

    return ((transaction.state == PrivacyTransactionState::Prepared) &&
            (proxyBytes.size() == primary->proxy.size) &&
            (QCryptographicHash::hash(proxyBytes, QCryptographicHash::Sha256) ==
             primary->proxy.sha256));
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

const PrivacyCategory* categoryForUuid(
    const PrivacyRepositorySnapshot& snapshot, const QString& categoryUuid)
{
    const auto it = std::find_if(
        snapshot.categories.cbegin(), snapshot.categories.cend(),
        [&categoryUuid](const PrivacyCategory& category)
        {
            return (category.uuid == categoryUuid);
        });
    return (it == snapshot.categories.cend()) ? nullptr : &*it;
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
        (expected.linkCount < 1) || (expected.size < 0) ||
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
                                 const QByteArray& teardownSnapshot = {},
                                 const QByteArray& preparedProxyBytes = {})
{
    const QByteArray journal = PrivacyTransactionJournalCodec::encode(stagedRecord);

    if (journal.isEmpty() || !originalModificationDate.isValid() ||
        (preparedProxyBytes.size() > MaximumPreparedProxyBytes) ||
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
    object.insert(QStringLiteral("preparedProxyBytes"),
                  QString::fromLatin1(preparedProxyBytes.toBase64()));
    object.insert(QStringLiteral("teardownSnapshot"),
                  QString::fromLatin1(teardownSnapshot.toBase64()));
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool decodePreparedPayload(const QByteArray& bytes,
                           PrivacyJournalRecord* const record,
                           QString* const archiveStageRelativePath,
                           QDateTime* const originalModificationDate,
                           QByteArray* const teardownSnapshot = nullptr,
                           QByteArray* const preparedProxyBytes = nullptr)
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
        QStringLiteral("preparedProxyBytes"),
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
    const QByteArray encodedProxy =
        object.value(QStringLiteral("preparedProxyBytes")).toString().toLatin1();

    if (encodedProxy.size() > (((MaximumPreparedProxyBytes + 2) / 3) * 4))
    {
        return false;
    }

    const QByteArray proxyBytes = QByteArray::fromBase64(
        encodedProxy,
        QByteArray::AbortOnBase64DecodingErrors);
    const QByteArray teardown = QByteArray::fromBase64(
        object.value(QStringLiteral("teardownSnapshot")).toString().toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    PrivacyJournalError journalError = PrivacyJournalError::None;

    if (archiveStage.isEmpty() || QDir::isAbsolutePath(archiveStage) ||
        (QDir::cleanPath(archiveStage) != archiveStage) ||
        !modificationDate.isValid() ||
        (proxyBytes.size() > MaximumPreparedProxyBytes) ||
        !PrivacyTransactionJournalCodec::decode(journal, record, &journalError) ||
        !singleItemContainerMatches(*record))
    {
        return false;
    }

    *archiveStageRelativePath = archiveStage;
    *originalModificationDate = modificationDate.toUTC();

    if (teardownSnapshot)
    {
        *teardownSnapshot = teardown;
    }

    if (preparedProxyBytes)
    {
        *preparedProxyBytes = proxyBytes;
    }

    return true;
}

QByteArray encodeCompatibilityPayload(
    const PrivacyJournalRecord& record, const QString& groupUuid)
{
    const QByteArray journal = PrivacyTransactionJournalCodec::encode(record);

    if (journal.isEmpty() || !canonicalUuid(groupUuid) ||
        (record.transactionType !=
         PrivacyTransactionType::CompatibilityUnlock))
    {
        return {};
    }

    QJsonObject object;
    object.insert(QStringLiteral("formatVersion"), 1);
    object.insert(QStringLiteral("groupUuid"), groupUuid);
    object.insert(QStringLiteral("journal"),
                  QString::fromLatin1(journal.toBase64()));
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool decodeCompatibilityPayload(
    const QByteArray& bytes, PrivacyJournalRecord* const record,
    QString* const groupUuid)
{
    if (!record || !groupUuid || bytes.isEmpty())
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
        QStringLiteral("formatVersion"), QStringLiteral("groupUuid"),
        QStringLiteral("journal")
    };

    if ((keys != expected) ||
        (object.value(QStringLiteral("formatVersion")).toInt(-1) != 1))
    {
        return false;
    }

    const QString decodedGroup =
        object.value(QStringLiteral("groupUuid")).toString();
    const QByteArray journal = QByteArray::fromBase64(
        object.value(QStringLiteral("journal")).toString().toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    PrivacyJournalError journalError = PrivacyJournalError::None;

    if (!canonicalUuid(decodedGroup) ||
        !PrivacyTransactionJournalCodec::decode(journal, record, &journalError) ||
        (record->transactionType !=
         PrivacyTransactionType::CompatibilityUnlock))
    {
        return false;
    }

    *groupUuid = decodedGroup;
    return true;
}

QString assetIdentity(int role, int ordinal);

QByteArray encodeTeardownSnapshot(const PrivacyItem& item,
                                  const PrivacyContainer& container,
                                  const QList<PrivacyAsset>& assets,
                                  const QString& priorProtectTransactionUuid)
{
    if (!item.isValid() || !container.isValid() || assets.isEmpty() ||
        (assets.size() > PrivacyTransactionJournalCodec::MaximumAssetCount) ||
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
           << quint32(assets.size());

    for (const PrivacyAsset& asset : assets)
    {
        if (!asset.isValid() || (asset.itemUuid != item.uuid) ||
            (asset.containerUuid != container.uuid))
        {
            return {};
        }

        stream << asset.itemUuid << asset.role << asset.ordinal
               << asset.originalName << asset.publicRootUuid
               << asset.publicRelativePath << asset.containerUuid
               << asset.protectedRelativePath << asset.hashAlgorithm
               << asset.originalHash << asset.originalSize
               << asset.originalCreationDate << asset.originalModificationDate
               << asset.portableAttributes << asset.proxyHashAlgorithm
               << asset.proxyHash << asset.proxySize
               << asset.proxyPresentationVersion << asset.proxyGeneration;
    }

    stream << priorProtectTransactionUuid;
    return (stream.status() == QDataStream::Ok) ? bytes : QByteArray();
}

bool decodeTeardownSnapshot(const QByteArray& bytes, PrivacyItem* const item,
                            PrivacyContainer* const container,
                            QList<PrivacyAsset>* const assets,
                            QString* const priorProtectTransactionUuid)
{
    if (bytes.isEmpty() || !item || !container || !assets ||
        !priorProtectTransactionUuid)
    {
        return false;
    }

    QDataStream stream(bytes);
    stream.setVersion(QDataStream::Qt_6_0);
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 version = 0;
    quint32 assetCount = 0;
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
           >> assetCount;
    container->kind = static_cast<PrivacyContainerKind>(containerKind);
    container->state = static_cast<PrivacyContainerState>(containerState);

    if ((version != 1) || (assetCount == 0) ||
        (assetCount > static_cast<quint32>(
            PrivacyTransactionJournalCodec::MaximumAssetCount)))
    {
        return false;
    }

    QList<PrivacyAsset> decodedAssets;
    QSet<QString> identities;
    int primaryCount = 0;

    for (quint32 index = 0 ; index < assetCount ; ++index)
    {
        PrivacyAsset asset;
        stream >> asset.itemUuid >> asset.role >> asset.ordinal
               >> asset.originalName >> asset.publicRootUuid
               >> asset.publicRelativePath >> asset.containerUuid
               >> asset.protectedRelativePath >> asset.hashAlgorithm
               >> asset.originalHash >> asset.originalSize
               >> asset.originalCreationDate >> asset.originalModificationDate
               >> asset.portableAttributes >> asset.proxyHashAlgorithm
               >> asset.proxyHash >> asset.proxySize
               >> asset.proxyPresentationVersion >> asset.proxyGeneration;
        const QString identity = assetIdentity(asset.role, asset.ordinal);

        if ((stream.status() != QDataStream::Ok) || !asset.isValid() ||
            identities.contains(identity))
        {
            return false;
        }

        identities.insert(identity);
        primaryCount += ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                         (asset.ordinal == 0)) ? 1 : 0;
        decodedAssets << asset;
    }

    stream >> *priorProtectTransactionUuid;

    for (const PrivacyAsset& asset : std::as_const(decodedAssets))
    {
        if ((asset.itemUuid != item->uuid) ||
            (asset.containerUuid != container->uuid))
        {
            return false;
        }
    }

    if (!((primaryCount == 1) && stream.atEnd() &&
          (stream.status() == QDataStream::Ok) && item->isValid() &&
          container->isValid() && canonicalUuid(*priorProtectTransactionUuid) &&
          (container->itemUuid == item->uuid)))
    {
        return false;
    }

    *assets = decodedAssets;
    return true;
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

struct ProtectAssetMetadata
{
    int        role = 0;
    int        ordinal = -1;
    QDateTime  creationDate;
    QDateTime  modificationDate;
    QByteArray portableAttributes;
};

QString assetIdentity(int role, int ordinal)
{
    return QStringLiteral("%1:%2").arg(role).arg(ordinal);
}

QByteArray encodeProtectAssetMetadata(
    const QList<ProtectAssetMetadata>& metadata)
{
    if (metadata.isEmpty() ||
        (metadata.size() > PrivacyTransactionJournalCodec::MaximumAssetCount))
    {
        return {};
    }

    QSet<QString> identities;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << quint32(1) << quint32(metadata.size());

    for (const ProtectAssetMetadata& entry : metadata)
    {
        mode_t ignoredMode = 0;
        const QString identity = assetIdentity(entry.role, entry.ordinal);

        if ((entry.role <= 0) || (entry.ordinal < 0) ||
            !entry.modificationDate.isValid() ||
            !decodePortableMode(entry.portableAttributes, &ignoredMode) ||
            identities.contains(identity))
        {
            return {};
        }

        identities.insert(identity);
        stream << entry.role << entry.ordinal << entry.creationDate
               << entry.modificationDate << entry.portableAttributes;
    }

    return ((stream.status() == QDataStream::Ok) && (bytes.size() <= 64 * 1024))
         ? bytes
         : QByteArray();
}

bool decodeProtectAssetMetadata(
    const QByteArray& bytes, const PrivacyJournalRecord& record,
    QList<ProtectAssetMetadata>* const metadata)
{
    if (!metadata || bytes.isEmpty() || (bytes.size() > 64 * 1024) ||
        record.assets.isEmpty())
    {
        return false;
    }

    QDataStream stream(bytes);
    stream.setVersion(QDataStream::Qt_6_0);
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 version = 0;
    quint32 count = 0;
    stream >> version >> count;

    if ((version != 1) || (count != static_cast<quint32>(record.assets.size())) ||
        (count > static_cast<quint32>(
            PrivacyTransactionJournalCodec::MaximumAssetCount)))
    {
        return false;
    }

    QList<ProtectAssetMetadata> decoded;
    QSet<QString> identities;

    for (quint32 index = 0 ; index < count ; ++index)
    {
        ProtectAssetMetadata entry;
        stream >> entry.role >> entry.ordinal >> entry.creationDate
               >> entry.modificationDate >> entry.portableAttributes;
        mode_t ignoredMode = 0;
        const QString identity = assetIdentity(entry.role, entry.ordinal);

        if ((stream.status() != QDataStream::Ok) || (entry.role <= 0) ||
            (entry.ordinal < 0) || !entry.modificationDate.isValid() ||
            !decodePortableMode(entry.portableAttributes, &ignoredMode) ||
            identities.contains(identity))
        {
            return false;
        }

        identities.insert(identity);
        decoded << entry;
    }

    if (!stream.atEnd())
    {
        return false;
    }

    for (const PrivacyJournalAsset& asset : record.assets)
    {
        if (!identities.contains(assetIdentity(asset.role, asset.ordinal)))
        {
            return false;
        }
    }

    *metadata = decoded;
    return true;
}

const ProtectAssetMetadata* metadataFor(
    const QList<ProtectAssetMetadata>& metadata, int role, int ordinal)
{
    const auto it = std::find_if(metadata.cbegin(), metadata.cend(),
                                 [role, ordinal](const ProtectAssetMetadata& entry)
                                 {
                                     return ((entry.role == role) &&
                                             (entry.ordinal == ordinal));
                                 });
    return (it == metadata.cend()) ? nullptr : &*it;
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
    if (record.assets.isEmpty())
    {
        return false;
    }

    for (const PrivacyJournalAsset& asset : record.assets)
    {
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

        if (!sink.openWrite() ||
            !archive.restoreMember(restore, password, &sink))
        {
            return false;
        }
    }

    return true;
}

} // namespace

bool PrivacyStillItemTransactionResult::succeeded() const
{
    return ((status == PrivacyStillItemTransactionStatus::Protected) ||
            (status == PrivacyStillItemTransactionStatus::Unprotected) ||
            (status == PrivacyStillItemTransactionStatus::CompatibilityUnlocked) ||
            (status == PrivacyStillItemTransactionStatus::CompatibilityRelocked));
}

bool PrivacyCompatibilityBatchResult::succeeded() const
{
    return ((status ==
             PrivacyStillItemTransactionStatus::CompatibilityUnlocked) ||
            (status ==
             PrivacyStillItemTransactionStatus::CompatibilityRelocked));
}

PrivacyStillItemTransactionResult PrivacyCompatibilityExposureGuardEngine::relock(
    const PrivacyStorageRoot& publicRoot,
    const PrivacyJournalRootExpectation& rootExpectation,
    const QString& unlockTransactionUuid)
{
    QString itemUuid;
    const auto fail = [&](PrivacyStillItemTransactionStatus status,
                          const QString& detail)
    {
        return failure(status, unlockTransactionUuid, itemUuid, detail);
    };

    if (!canonicalUuid(unlockTransactionUuid) ||
        !sameRootExpectation(publicRoot, rootExpectation))
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest,
                    QStringLiteral("Compatibility guard request is invalid"));
    }

    PrivacyJournalError journalError = PrivacyJournalError::None;
    QString detail;
    std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
        PrivacyTransactionJournalStore::open(
            publicRoot.configuredPath, rootExpectation, &journalError, &detail);

    if (!journalStore)
    {
        return fail(PrivacyStillItemTransactionStatus::RootUnavailable,
                    detail.isEmpty()
                        ? QStringLiteral("Compatibility guard root is unavailable")
                        : detail);
    }

    PrivacyJournalLoadResult loaded = journalStore->load(
        unlockTransactionUuid);

    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || !loaded.hasRecord)
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                    loaded.detail.isEmpty()
                        ? QStringLiteral(
                              "Compatibility guard journal is not authoritative")
                        : loaded.detail);
    }

    const PrivacyJournalRecord record = loaded.record;

    if (!record.assets.isEmpty())
    {
        itemUuid = record.assets.constFirst().itemUuid;
    }

    if ((record.transactionUuid != unlockTransactionUuid) ||
        (record.transactionType !=
         PrivacyTransactionType::CompatibilityUnlock) ||
        (record.rootUuid != publicRoot.uuid) ||
        (record.rootDevice != rootExpectation.device) ||
        (record.rootInode != rootExpectation.inode) ||
        (record.rootIdentitySha256 != rootExpectation.identitySha256) ||
        !singleItemContainerMatches(record) ||
        ((record.stage != PrivacyJournalStage::Created) &&
         (record.stage != PrivacyJournalStage::Prepared) &&
         (record.stage != PrivacyJournalStage::Staged) &&
         (record.stage != PrivacyJournalStage::ProtectedCopyVerified) &&
         (record.stage != PrivacyJournalStage::Applying) &&
         (record.stage != PrivacyJournalStage::PublicStateVerified) &&
         (record.stage != PrivacyJournalStage::ReconciliationRequired) &&
         (record.stage != PrivacyJournalStage::Complete)))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral(
                        "Compatibility guard journal facts are not exact"));
    }

    const PrivacyJournalObjectFact& expectedArchive =
        record.assets.constFirst().container;
    const QString archivePath = absolutePath(
        publicRoot, record.assets.constFirst().containerRelativePath);
    const auto protectedCopyExact = [&]()
    {
        PrivacyJournalObjectFact archiveFact;
        return (!archivePath.isEmpty() &&
                stableFileFact(archivePath, nullptr, &archiveFact) &&
                sameFact(archiveFact, expectedArchive));
    };
    const auto publicStateLocked = [&]()
    {
        for (const PrivacyJournalAsset& asset : record.assets)
        {
            const QString publicPath = absolutePath(
                publicRoot, asset.publicRelativePath);

            if (publicPath.isEmpty())
            {
                return false;
            }

            if (asset.proxy.presence ==
                PrivacyJournalExpectedPresence::Present)
            {
                PrivacyJournalObjectFact publicFact;

                if (!stableFileFact(publicPath, nullptr, &publicFact) ||
                    !sameFact(publicFact, asset.proxy))
                {
                    return false;
                }
            }
            else
            {
                const QFileInfo publicInfo(publicPath);

                if (publicInfo.exists() || publicInfo.isSymLink())
                {
                    return false;
                }
            }
        }

        return true;
    };
    const auto cleanupRetainedOriginals = [&]()
    {
        if (!protectedCopyExact())
        {
            return false;
        }

        for (const PrivacyJournalAsset& asset : record.assets)
        {
            if (!removeExactFile(publicRoot, rootExpectation,
                                 asset.stagedRelativePath,
                                 asset.original, true))
            {
                return false;
            }
        }

        return true;
    };

    if (record.stage == PrivacyJournalStage::Complete)
    {
        if (!publicStateLocked())
        {
            return fail(
                PrivacyStillItemTransactionStatus::ReconciliationRequired,
                QStringLiteral(
                    "guard-complete public content is mixed or changed"));
        }

        if (!cleanupRetainedOriginals())
        {
            return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                        QStringLiteral(
                            "guard-complete original cleanup is pending"));
        }

        PrivacyStillItemTransactionResult result;
        result.status = PrivacyStillItemTransactionStatus::CompatibilityRelocked;
        result.transactionUuid = unlockTransactionUuid;
        result.itemUuid = itemUuid;
        return result;
    }

    if ((record.stage == PrivacyJournalStage::Created) ||
        (record.stage == PrivacyJournalStage::Prepared) ||
        (record.stage == PrivacyJournalStage::Staged) ||
        (record.stage == PrivacyJournalStage::ProtectedCopyVerified))
    {
        if (!publicStateLocked() || !cleanupRetainedOriginals())
        {
            return fail(
                PrivacyStillItemTransactionStatus::ReconciliationRequired,
                QStringLiteral(
                    "pre-exposure guard cancellation requires reconciliation"));
        }

        PrivacyJournalRecord complete = record;
        complete.stage = PrivacyJournalStage::Complete;
        QByteArray completeHash;

        if (!journalStore->compareAndUpdate(
                complete, loaded.sha256, &completeHash,
                &journalError, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        detail);
        }

        PrivacyStillItemTransactionResult result;
        result.status = PrivacyStillItemTransactionStatus::CompatibilityRelocked;
        result.transactionUuid = unlockTransactionUuid;
        result.itemUuid = itemUuid;
        return result;
    }

    if (!protectedCopyExact())
    {
        return fail(PrivacyStillItemTransactionStatus::ReconciliationRequired,
                    QStringLiteral(
                        "protected copy changed before guarded relock"));
    }

    if (record.stage == PrivacyJournalStage::Applying)
    {
        QList<PrivacyPublicTransitionRequest> forwardTransitions;

        for (const PrivacyJournalAsset& asset : record.assets)
        {
            PrivacyPublicTransitionRequest transition;
            transition.absoluteRootPath = publicRoot.configuredPath;
            transition.rootExpectation = rootExpectation;
            transition.journalRecord = record;
            transition.authoritativeJournalSha256 = loaded.sha256;
            transition.itemUuid = asset.itemUuid;
            transition.role = asset.role;
            transition.ordinal = asset.ordinal;
            transition.mode =
                (asset.proxy.presence ==
                 PrivacyJournalExpectedPresence::Present)
                    ? PrivacyPublicTransitionMode::ExchangePresent
                    : PrivacyPublicTransitionMode::InstallAbsent;
            transition.currentFact = PrivacyPublicTransitionFactKind::Proxy;
            transition.installedFact =
                PrivacyPublicTransitionFactKind::Original;
            forwardTransitions << transition;
        }

        const PrivacyPublicTransitionResult exposed =
            PrivacyPublicTransitionEngine().executeBatch(forwardTransitions);

        if (!exposed.succeeded())
        {
            return fail(
                PrivacyStillItemTransactionStatus::ReconciliationRequired,
                exposed.detail.isEmpty()
                    ? QStringLiteral(
                          "interrupted exposure requires reconciliation")
                    : exposed.detail);
        }

        return relock(publicRoot, rootExpectation, unlockTransactionUuid);
    }

    QList<PrivacyPublicTransitionRequest> transitions;

    for (const PrivacyJournalAsset& asset : record.assets)
    {
        PrivacyPublicTransitionRequest transition;
        transition.absoluteRootPath = publicRoot.configuredPath;
        transition.rootExpectation = rootExpectation;
        transition.journalRecord = record;
        transition.authoritativeJournalSha256 = loaded.sha256;
        transition.itemUuid = asset.itemUuid;
        transition.role = asset.role;
        transition.ordinal = asset.ordinal;
        transition.mode =
            (asset.proxy.presence == PrivacyJournalExpectedPresence::Present)
                ? PrivacyPublicTransitionMode::ExchangePresent
                : PrivacyPublicTransitionMode::RemovePresent;
        transition.currentFact = PrivacyPublicTransitionFactKind::Original;
        transition.installedFact = PrivacyPublicTransitionFactKind::Proxy;
        transition.direction =
            PrivacyPublicTransitionDirection::CompatibilityGuardRelock;
        transitions << transition;
    }

    const PrivacyPublicTransitionResult transitioned =
        PrivacyPublicTransitionEngine().executeBatch(transitions);

    if (!transitioned.succeeded())
    {
        loaded = journalStore->load(unlockTransactionUuid);

        if ((loaded.disposition == PrivacyJournalLoadDisposition::Loaded) &&
            loaded.authoritative && loaded.hasRecord &&
            (loaded.record.stage ==
             PrivacyJournalStage::PublicStateVerified))
        {
            PrivacyJournalRecord reconciliation = loaded.record;
            reconciliation.stage =
                PrivacyJournalStage::ReconciliationRequired;
            QByteArray ignoredHash;
            journalStore->compareAndUpdate(
                reconciliation, loaded.sha256, &ignoredHash,
                &journalError, &detail);
        }

        return fail(PrivacyStillItemTransactionStatus::ReconciliationRequired,
                    transitioned.detail.isEmpty()
                        ? QStringLiteral("guarded relock requires reconciliation")
                        : transitioned.detail);
    }

    loaded = journalStore->load(unlockTransactionUuid);

    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || !loaded.hasRecord ||
        (loaded.record.stage != PrivacyJournalStage::Complete))
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                    QStringLiteral(
                        "guarded relock completion journal is not authoritative"));
    }

    if (!publicStateLocked())
    {
        return fail(PrivacyStillItemTransactionStatus::ReconciliationRequired,
                    QStringLiteral(
                        "guarded relock public content did not verify"));
    }

    if (!cleanupRetainedOriginals())
    {
        return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                    QStringLiteral("guarded original cleanup is pending"));
    }

    PrivacyStillItemTransactionResult result;
    result.status = PrivacyStillItemTransactionStatus::CompatibilityRelocked;
    result.transactionUuid = unlockTransactionUuid;
    result.itemUuid = itemUuid;
    return result;
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

bool PrivacyCoreDbStillItemPersistence::beginCompatibilityUnlock(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginCompatibilityUnlock(transaction, journal);
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

    struct GeneratedProxy
    {
        QByteArray encodedBytes;
        QByteArray sha256;

        bool isValid() const
        {
            return (!encodedBytes.isEmpty() && (sha256.size() == 32));
        }
    };

    Private(PrivacyStillItemPersistence& persistenceValue,
            PrivacyRuntimeCoordinator& runtimeValue,
            PrivacyStillItemCacheGate& cacheValue)
        : persistence(persistenceValue),
          runtime(runtimeValue),
          cache(cacheValue),
          videoProxy(processRunner, PrivacyVideoToolPaths::discover())
    {
    }

    GeneratedProxy generateProxy(const QString& sourcePath,
                                 const QString& publicFileName,
                                 PrivacyPresentationMode presentation) const
    {
        GeneratedProxy generated;

        if (PrivacyVideoProxyGenerator::isSameContainerCandidate(publicFileName))
        {
            PrivacyVideoProxyRequest request;
            request.sourcePath = sourcePath;
            request.publicFileName = publicFileName;
            request.presentation =
                (presentation == PrivacyPresentationMode::Blur)
                    ? PrivacyVideoProxyPresentation::Blurred
                    : PrivacyVideoProxyPresentation::Generic;
            const PrivacyVideoProxyResult result = videoProxy.generate(request);

            if (result.isValid())
            {
                generated.encodedBytes = result.encodedBytes;
                generated.sha256 = result.sha256;
            }

            return generated;
        }

        PrivacyStillProxyRequest request;
        request.sourcePath = sourcePath;
        request.publicFileName = publicFileName;
        request.presentation =
            (presentation == PrivacyPresentationMode::Blur)
                ? PrivacyStillProxyPresentation::Blurred
                : PrivacyStillProxyPresentation::Generic;
        const PrivacyStillProxyResult result = stillProxy.generate(request);

        if (result.isValid())
        {
            generated.encodedBytes = result.encodedBytes;
            generated.sha256 = result.sha256;
        }

        return generated;
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
    PrivacyStillProxyGenerator stillProxy;
    QProcessPrivacyProcessRunner processRunner;
    PrivacyVideoProxyGenerator videoProxy;
    PrivacyPublicTransitionEngine transition;
    FaultHook faultHook;
    CompatibilityGuardArmHook compatibilityGuardArmHook;
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

void PrivacyStillItemTransactionEngine::setCompatibilityGuardArmHook(
    const CompatibilityGuardArmHook& hook)
{
    d->compatibilityGuardArmHook = hook;
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

    if (inventory.requiredAssets.isEmpty() ||
        (inventory.requiredAssets.size() >
         PrivacyTransactionJournalCodec::MaximumAssetCount))
    {
        return fail(PrivacyStillItemTransactionStatus::AssociatedAssetSetUnsupported,
                    QStringLiteral("associated-asset count exceeds transaction limits"));
    }

    if (!request.associatedAssetsAcknowledged)
    {
        return fail(PrivacyStillItemTransactionStatus::AcknowledgementRequired,
                    QStringLiteral("associated-asset inventory requires acknowledgement"));
    }

    const PrivacyInventoryAsset* sourceAsset = nullptr;

    for (const PrivacyInventoryAsset& candidate : inventory.requiredAssets)
    {
        if (!candidate.isValid() ||
            (candidate.location.root.uuid != request.publicRoot.uuid) ||
            (QDir::cleanPath(candidate.location.root.absolutePath) !=
             QDir::cleanPath(request.publicRoot.configuredPath)))
        {
            return fail(PrivacyStillItemTransactionStatus::PreflightRejected,
                        QStringLiteral(
                            "associated asset/root does not match request"));
        }

        if ((candidate.role == PrivacyInventoryAssetRole::PrimaryMedia) &&
            (candidate.ordinal == 0))
        {
            if (sourceAsset)
            {
                return fail(PrivacyStillItemTransactionStatus::PreflightRejected,
                            QStringLiteral("preflight has multiple primary assets"));
            }

            sourceAsset = &candidate;
        }
    }

    if (!sourceAsset)
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

    if (!category ||
        ((category->backend != PrivacyBackend::Casual) &&
         (category->backend != PrivacyBackend::Strong)) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (!d->durableReplay && !d->runtime.isCategoryUnlocked(category->uuid)))
    {
        return fail(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                    QStringLiteral("privacy category is not active and unlocked"));
    }

    const PrivacyCategory categoryValue = *category;
    const bool strongBackend = (categoryValue.backend == PrivacyBackend::Strong);
    PrivacyStore strongStore;

    if (strongBackend)
    {
        if (!canonicalUuid(request.strongStoreUuid) ||
            request.vaultPlaintextRoot.isEmpty() ||
            !QFileInfo(request.vaultPlaintextRoot).isDir())
        {
            return fail(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                        QStringLiteral(
                            "Strong protect requires the mounted Originals vault"));
        }

        const PrivacyStoreBinding* originalsBinding = nullptr;

        for (const PrivacyStoreBinding& binding : snapshot.storeBindings)
        {
            if ((binding.categoryUuid == categoryValue.uuid) &&
                (binding.role == PrivacyStoreRole::Originals))
            {
                originalsBinding = &binding;
                break;
            }
        }

        for (const PrivacyStore& candidate : snapshot.stores)
        {
            if (candidate.uuid == request.strongStoreUuid)
            {
                strongStore = candidate;
            }
        }

        if (!originalsBinding ||
            (originalsBinding->storeUuid != request.strongStoreUuid) ||
            (strongStore.categoryUuid != categoryValue.uuid) ||
            (strongStore.lifecycleState != PrivacyStoreLifecycleState::Active))
        {
            return fail(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                        QStringLiteral("Strong Originals store binding is invalid"));
        }
    }

    const QString publicRelativePath = sourceAsset->location.relativePath;
    const QString sourcePath = absolutePath(request.publicRoot, publicRelativePath);
    const QString archiveRelativePath = publicRelativePath +
                                        QLatin1String(".digikam-private.zip");
    const QString archivePath = absolutePath(request.publicRoot,
                                             archiveRelativePath);
    const QString archiveStageRelativePath = protectArchiveStageRelativePath(
        publicRelativePath, transactionUuid);
    const QString archiveStagePath = absolutePath(request.publicRoot,
                                                  archiveStageRelativePath);
    const QString replacementRelativePath =
        parentPath(publicRelativePath) +
        (parentPath(publicRelativePath).isEmpty() ? QString() : QLatin1String("/")) +
        PrivacyPublicTransitionEngine::expectedStageFileName(
            transactionUuid, PrivacyAsset::PrimaryMediaRole, 0);
    const QString strongStagedRelative =
        QLatin1String("originals/.staging-") + transactionUuid;
    const QString strongFinalRelative =
        QLatin1String("originals/") + containerUuid;
    const QString payloadStageRelative =
        strongBackend ? strongStagedRelative : archiveStageRelativePath;
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
    QByteArray preparedProxyBytes;
    QList<ProtectAssetMetadata> sourceMetadata;
    QList<PrivacyStrongObjectMember> strongMembers;

    if (!existingTransaction)
    {
        if (existingItem)
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("image already has a different privacy item"));
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
        PrivacyJournalObjectFact primaryOriginalFact;

        for (const PrivacyInventoryAsset& candidate : inventory.requiredAssets)
        {
            const QString candidatePath = absolutePath(
                request.publicRoot, candidate.location.relativePath);
            PrivacyJournalObjectFact originalFact;
            mode_t candidateMode = 0;
            QDateTime candidateModificationDate;

            if (candidatePath.isEmpty() ||
                !stableFileFact(candidatePath, &candidate.evidence,
                                &originalFact, &candidateMode,
                                &candidateModificationDate))
            {
                return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                            QStringLiteral(
                                "associated asset no longer matches preflight evidence"));
            }

            PrivacyJournalAsset journalAsset;
            journalAsset.itemUuid = itemUuid;
            journalAsset.containerUuid = containerUuid;
            journalAsset.role = static_cast<int>(candidate.role);
            journalAsset.ordinal = candidate.ordinal;
            journalAsset.publicRelativePath = candidate.location.relativePath;
            journalAsset.stagedRelativePath =
                parentPath(candidate.location.relativePath) +
                (parentPath(candidate.location.relativePath).isEmpty()
                    ? QString() : QLatin1String("/")) +
                PrivacyPublicTransitionEngine::expectedStageFileName(
                    transactionUuid, journalAsset.role, journalAsset.ordinal);
            journalAsset.protectedRelativePath =
                strongBackend
                    ? QString::fromLatin1("originals/%1/%2-%3")
                          .arg(containerUuid)
                          .arg(journalAsset.ordinal)
                          .arg(QFileInfo(
                              candidate.location.relativePath).fileName())
                    : PrivacyCasualArchiveEngine::expectedMemberPath(
                        journalAsset.role, journalAsset.ordinal,
                        QFileInfo(candidate.location.relativePath).fileName());
            journalAsset.containerRelativePath =
                strongBackend ? strongFinalRelative : archiveRelativePath;
            journalAsset.original = originalFact;
            created.assets << journalAsset;

            ProtectAssetMetadata metadata;
            metadata.role = journalAsset.role;
            metadata.ordinal = journalAsset.ordinal;
            metadata.creationDate =
                ((candidate.role == PrivacyInventoryAssetRole::PrimaryMedia) &&
                 (candidate.ordinal == 0))
                    ? request.originalCreationDate
                    : QFileInfo(candidatePath).birthTime();
            metadata.modificationDate = candidateModificationDate;
            metadata.portableAttributes = encodePortableMode(candidateMode);
            sourceMetadata << metadata;

            if ((journalAsset.role == PrivacyAsset::PrimaryMediaRole) &&
                (journalAsset.ordinal == 0))
            {
                primaryOriginalFact = originalFact;
                originalMode = candidateMode;
                originalModificationDate = candidateModificationDate;
            }
        }

        protectMetadata = encodeProtectAssetMetadata(sourceMetadata);

        if (protectMetadata.isEmpty() ||
            (primaryOriginalFact.presence !=
             PrivacyJournalExpectedPresence::Present))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("associated-asset metadata is invalid"));
        }

        PrivacyItem item;
        item.imageId = request.imageId;
        item.uuid = itemUuid;
        item.categoryUuid = request.categoryUuid;
        item.originalHash = QString::fromLatin1(
            primaryOriginalFact.sha256.toHex());
        item.originalSize = primaryOriginalFact.size;
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
        transaction.payloadData = encodePreparedPayload(
            created, payloadStageRelative, originalModificationDate,
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
                               &originalModificationDate, &protectMetadata,
                               &preparedProxyBytes) ||
        !decodeProtectAssetMetadata(protectMetadata, created,
                                    &sourceMetadata) ||
        (storedArchiveStage != payloadStageRelative) ||
        ((existingTransaction->state == PrivacyTransactionState::Created) &&
         !preparedProxyBytes.isEmpty()))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Protect payload cannot be reconstructed"));
    }

    const ProtectAssetMetadata* primaryMetadata = metadataFor(
        sourceMetadata, PrivacyAsset::PrimaryMediaRole, 0);

    if (!primaryMetadata ||
        !decodePortableMode(primaryMetadata->portableAttributes,
                            &originalMode) ||
        (primaryMetadata->modificationDate.toUTC() !=
         originalModificationDate.toUTC()))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("primary Protect metadata is not exact"));
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

        const Private::GeneratedProxy proxyResult = d->generateProxy(
            sourcePath, QFileInfo(publicRelativePath).fileName(),
            categoryValue.presentationMode);

        if (!proxyResult.isValid())
        {
            return fail(PrivacyStillItemTransactionStatus::ProxyFailure,
                        QStringLiteral("cannot generate metadata-free proxy"));
        }

        PrivacyCasualArchiveRequest archiveRequest;
        archiveRequest.finalArchivePath = archivePath;
        archiveRequest.stagingArchivePath = archiveStagePath;
        archiveRequest.categoryUuid = request.categoryUuid;
        archiveRequest.containerUuid = containerUuid;
        archiveRequest.itemUuid = itemUuid;

        for (const PrivacyInventoryAsset& candidate : inventory.requiredAssets)
        {
            const int role = static_cast<int>(candidate.role);
            const auto journalIt = std::find_if(
                created.assets.cbegin(), created.assets.cend(),
                [role, &candidate](const PrivacyJournalAsset& asset)
                {
                    return ((asset.role == role) &&
                            (asset.ordinal == candidate.ordinal));
                });
            const ProtectAssetMetadata* metadata = metadataFor(
                sourceMetadata, role, candidate.ordinal);
            const QString candidatePath = absolutePath(
                request.publicRoot, candidate.location.relativePath);
            PrivacyJournalObjectFact originalFact;
            mode_t verifiedMode = 0;
            mode_t expectedMode = 0;
            QDateTime verifiedModificationDate;

            if ((journalIt == created.assets.cend()) || !metadata ||
                !decodePortableMode(metadata->portableAttributes,
                                    &expectedMode) ||
                !stableFileFact(candidatePath, &candidate.evidence,
                                &originalFact, &verifiedMode,
                                &verifiedModificationDate) ||
                (verifiedMode != expectedMode) ||
                (verifiedModificationDate.toUTC() !=
                 metadata->modificationDate.toUTC()) ||
                !sameFact(originalFact, journalIt->original))
            {
                return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                            QStringLiteral(
                                "associated asset changed while preparing protection"));
            }

            PrivacyCasualArchiveMember member;
            member.sourcePath = candidatePath;
            member.protectedRelativePath = journalIt->protectedRelativePath;
            member.originalName = QFileInfo(
                candidate.location.relativePath).fileName();
            member.role = role;
            member.ordinal = candidate.ordinal;
            member.originalCreationDate = metadata->creationDate;
            member.originalModificationDate = metadata->modificationDate;
            member.portableAttributes = metadata->portableAttributes;
            member.expectedDevice = candidate.evidence.deviceId;
            member.expectedInode = candidate.evidence.inode;
            member.expectedLinkCount = candidate.evidence.linkCount;
            member.expectedSize = originalFact.size;
            member.expectedSha256 = originalFact.sha256;
            archiveRequest.members << member;

            if (strongBackend)
            {
                PrivacyStrongObjectMember strongMember;
                strongMember.sourcePath = candidatePath;
                strongMember.protectedRelativePath =
                    journalIt->protectedRelativePath;
                strongMember.originalName = QFileInfo(
                    candidate.location.relativePath).fileName();
                strongMember.expectedSize = originalFact.size;
                strongMember.expectedSha256 = originalFact.sha256;
                strongMembers << strongMember;
            }
        }

        PrivacyStrongObjectStageResult strongStage;
        std::unique_ptr<PrivacyCasualArchiveStage> archiveStage;

        if (strongBackend)
        {
            QString stageDetail;
            strongStage = PrivacyStrongObjectBackend::stageObjects(
                request.vaultPlaintextRoot, strongStagedRelative,
                strongMembers, &stageDetail);

            if (!strongStage.valid)
            {
                return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                            stageDetail.isEmpty()
                                ? QStringLiteral(
                                    "cannot stage Strong vault objects")
                                : stageDetail);
            }
        }
        else
        {
            PrivacyJournalObjectFact existingArchiveStageFact;

            if (stableFileFact(archiveStagePath, nullptr,
                               &existingArchiveStageFact))
            {
                archiveStage = std::make_unique<PrivacyCasualArchiveStage>(
                    d->archive.resumeStagedArchive(
                        archiveStagePath, archivePath,
                        existingArchiveStageFact.size,
                        existingArchiveStageFact.sha256, password));
            }
            else
            {
                archiveStage = std::make_unique<PrivacyCasualArchiveStage>(
                    d->archive.stageArchive(archiveRequest, password));
            }

            if (!archiveStage || !archiveStage->isValid())
            {
                return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                            QStringLiteral(
                                "cannot create or resume verified archive stage"));
            }
        }

        prepared = created;
        prepared.stage = PrivacyJournalStage::Prepared;

        for (PrivacyJournalAsset& asset : prepared.assets)
        {
            if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                asset.proxy.presence = PrivacyJournalExpectedPresence::Present;
                asset.proxy.size = proxyResult.encodedBytes.size();
                asset.proxy.linkCount = 1;
                asset.proxy.sha256 = proxyResult.sha256;
            }
            else
            {
                asset.proxy.presence = PrivacyJournalExpectedPresence::Absent;
            }

            asset.container.presence = PrivacyJournalExpectedPresence::Present;
            asset.container.size = strongBackend
                                 ? strongStage.totalSize
                                 : archiveStage->archiveSize();
            asset.container.linkCount = 1;
            asset.container.sha256 = strongBackend
                                   ? strongStage.totalSha256
                                   : archiveStage->archiveSha256();
        }

        PrivacyTransaction next = *existingTransaction;
        next.state = PrivacyTransactionState::Prepared;
        next.generation = 1;
        next.payloadData = encodePreparedPayload(
            prepared, payloadStageRelative, originalModificationDate,
            protectMetadata, proxyResult.encodedBytes);
        next.updatedAt = QDateTime::currentDateTimeUtc();

        if (next.payloadData.isEmpty() ||
            !d->persistence.compareAndUpdateTransaction(
                next, PrivacyTransactionState::Created, 0))
        {
            return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("cannot publish exact Prepared transaction"));
        }

        if (d->fault(PrivacyStillItemFaultPoint::AfterPreparedPayload))
        {
            return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                        QStringLiteral("fault after exact Prepared payload"));
        }

        existingTransaction = nullptr;
    }
    else if (!decodePreparedPayload(existingTransaction->payloadData, &prepared,
                                    &storedArchiveStage,
                                    &originalModificationDate, &protectMetadata,
                                    &preparedProxyBytes) ||
             !decodeProtectAssetMetadata(protectMetadata, prepared,
                                         &sourceMetadata) ||
             !preparedProxyPayloadMatches(*existingTransaction, prepared,
                                          preparedProxyBytes))
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
                               &originalModificationDate, &protectMetadata,
                               &preparedProxyBytes) ||
        !decodeProtectAssetMetadata(protectMetadata, prepared,
                                    &sourceMetadata) ||
        !preparedProxyPayloadMatches(*existingTransaction, prepared,
                                     preparedProxyBytes))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Prepared protection disappeared"));
    }

    created = prepared;
    created.stage = PrivacyJournalStage::Created;

    for (PrivacyJournalAsset& asset : created.assets)
    {
        asset.proxy = PrivacyJournalObjectFact();
        asset.container = PrivacyJournalObjectFact();
    }

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
    const auto primaryPreparedIt = std::find_if(
        prepared.assets.cbegin(), prepared.assets.cend(),
        [](const PrivacyJournalAsset& asset)
        {
            return ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                    (asset.ordinal == 0));
        });

    if (primaryPreparedIt == prepared.assets.cend())
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Prepared primary asset is missing"));
    }

    if (strongBackend && strongMembers.isEmpty() &&
        !prepared.assets.isEmpty())
    {
        for (const PrivacyJournalAsset& journalAsset : prepared.assets)
        {
            PrivacyStrongObjectMember strongMember;
            strongMember.sourcePath.clear();
            strongMember.protectedRelativePath =
                journalAsset.protectedRelativePath;
            strongMember.originalName = QFileInfo(
                journalAsset.publicRelativePath).fileName();
            strongMember.expectedSize = journalAsset.original.size;
            strongMember.expectedSha256 = journalAsset.original.sha256;
            strongMembers << strongMember;
        }
    }

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
        const PrivacyPublicReplacementStageResult stageResult =
            d->transition.stageReplacement(stageRequest, preparedProxyBytes);

        if (!stageResult.succeeded())
        {
            PrivacyJournalObjectFact replayFact;

            if ((stageResult.error !=
                 PrivacyPublicTransitionError::UnexpectedExistingFile) ||
                !stableFileFact(absolutePath(request.publicRoot,
                                             replacementRelativePath),
                                nullptr, &replayFact) ||
                !sameFact(replayFact, primaryPreparedIt->proxy))
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

    if (strongBackend)
    {
        QString strongDetail;

        if (!PrivacyStrongObjectBackend::verifyObjects(
                request.vaultPlaintextRoot, strongFinalRelative,
                strongMembers,
                prepared.assets.constFirst().container.size,
                prepared.assets.constFirst().container.sha256,
                &strongDetail))
        {
            if (d->durableReplay)
            {
                return fail(
                    PrivacyStillItemTransactionStatus::AuthenticationRequired,
                    QStringLiteral(
                        "Strong vault objects must be re-staged through an "
                        "unlocked category"));
            }

            if (!PrivacyStrongObjectBackend::publishObjects(
                    request.vaultPlaintextRoot, strongStagedRelative,
                    strongFinalRelative, strongMembers,
                    prepared.assets.constFirst().container.size,
                    prepared.assets.constFirst().container.sha256,
                    &strongDetail) ||
                !PrivacyStrongObjectBackend::verifyObjects(
                    request.vaultPlaintextRoot, strongFinalRelative,
                    strongMembers,
                    prepared.assets.constFirst().container.size,
                    prepared.assets.constFirst().container.sha256,
                    &strongDetail))
            {
                return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                            strongDetail);
            }
        }
    }
    else
    {
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
                        QStringLiteral(
                            "Prepared Protect archive stage must be recreated"));
                }

                published = d->archive.publishExactPreparedStage(
                    archiveStagePath, archivePath,
                    prepared.assets.constFirst().container.size,
                    prepared.assets.constFirst().container.sha256);
            }
            else
            {
                PrivacyCasualArchiveStage archiveStage =
                    d->archive.resumeStagedArchive(
                        archiveStagePath, archivePath,
                        prepared.assets.constFirst().container.size,
                        prepared.assets.constFirst().container.sha256,
                        password);
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
            !sameFact(finalArchiveFact,
                      prepared.assets.constFirst().container) ||
            (!d->durableReplay &&
             !verifyArchiveMember(d->archive, prepared, archivePath,
                                  password)))
        {
            return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                        QStringLiteral(
                            "published archive fails exact verification"));
        }
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
        QList<PrivacyPublicTransitionRequest> transitionRequests;

        for (const PrivacyJournalAsset& asset : loaded.record.assets)
        {
            PrivacyPublicTransitionRequest transitionRequest;
            transitionRequest.absoluteRootPath = request.publicRoot.configuredPath;
            transitionRequest.rootExpectation = request.rootExpectation;
            transitionRequest.journalRecord = loaded.record;
            transitionRequest.authoritativeJournalSha256 = loaded.sha256;
            transitionRequest.itemUuid = itemUuid;
            transitionRequest.role = asset.role;
            transitionRequest.ordinal = asset.ordinal;
            transitionRequest.mode =
                ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                 (asset.ordinal == 0))
                    ? PrivacyPublicTransitionMode::ExchangePresent
                    : PrivacyPublicTransitionMode::RemovePresent;
            transitionRequest.currentFact =
                PrivacyPublicTransitionFactKind::Original;
            transitionRequest.installedFact =
                PrivacyPublicTransitionFactKind::Proxy;
            transitionRequests << transitionRequest;
        }

        const PrivacyPublicTransitionResult transitioned =
            d->transition.executeBatch(transitionRequests);

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

    for (const PrivacyJournalAsset& journalAsset : std::as_const(complete.assets))
    {
        if (!removeExactFile(request.publicRoot, request.rootExpectation,
                             journalAsset.stagedRelativePath,
                             journalAsset.original, true))
        {
            return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                        QStringLiteral(
                            "exact displaced-original cleanup is pending"));
        }
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterProtectedStageCleanup))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after displaced-original cleanup"));
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
        QString::fromLatin1(primaryPreparedIt->proxy.sha256.toHex());
    publishedItem.expectedProxySize = primaryPreparedIt->proxy.size;
    publishedItem.transactionState =
        static_cast<int>(PrivacyTransactionState::Complete);
    PrivacyContainer container;
    container.uuid = containerUuid;
    container.itemUuid = itemUuid;
    container.kind = strongBackend ? PrivacyContainerKind::StrongObject
                                   : PrivacyContainerKind::CasualArchive;
    container.rootUuid = strongBackend ? QString()
                                       : request.publicRoot.uuid;
    container.storeUuid = strongBackend ? request.strongStoreUuid
                                        : QString();
    container.objectRelativePath = strongBackend ? strongFinalRelative
                                                 : archiveRelativePath;
    container.protectedSize = prepared.assets.constFirst().container.size;
    container.protectedHashAlgorithm = QLatin1String("sha256");
    container.protectedHash = QString::fromLatin1(
        prepared.assets.constFirst().container.sha256.toHex());
    container.formatVersion = 1;
    container.credentialGeneration = prepared.credentialGeneration;
    container.state = PrivacyContainerState::Verified;
    container.createdAt = existingTransaction->createdAt;
    container.updatedAt = QDateTime::currentDateTimeUtc();
    QList<PrivacyAsset> assets;

    for (const PrivacyJournalAsset& journalAsset : prepared.assets)
    {
        const ProtectAssetMetadata* metadata = metadataFor(
            sourceMetadata, journalAsset.role, journalAsset.ordinal);

        if (!metadata)
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral("asset publication metadata is missing"));
        }

        PrivacyAsset asset;
        asset.itemUuid = itemUuid;
        asset.role = journalAsset.role;
        asset.ordinal = journalAsset.ordinal;
        asset.originalName = QFileInfo(
            journalAsset.publicRelativePath).fileName();
        asset.publicRootUuid = request.publicRoot.uuid;
        asset.publicRelativePath = journalAsset.publicRelativePath;
        asset.containerUuid = containerUuid;
        asset.protectedRelativePath = journalAsset.protectedRelativePath;
        asset.hashAlgorithm = QLatin1String("sha256");
        asset.originalHash = QString::fromLatin1(
            journalAsset.original.sha256.toHex());
        asset.originalSize = journalAsset.original.size;
        asset.originalCreationDate = metadata->creationDate;
        asset.originalModificationDate = metadata->modificationDate;
        asset.portableAttributes = metadata->portableAttributes;

        if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
            (asset.ordinal == 0))
        {
            asset.proxyHashAlgorithm = QLatin1String("sha256");
            asset.proxyHash = publishedItem.expectedProxyHash;
            asset.proxySize = publishedItem.expectedProxySize;
            asset.proxyPresentationVersion = publishedItem.presentationVersion;
            asset.proxyGeneration = publishedItem.generation;
        }

        assets << asset;
    }

    if (existingTransaction->state == PrivacyTransactionState::Complete)
    {
        const PrivacyContainer* storedContainer = containerForItem(snapshot, itemUuid);
        const QList<PrivacyAsset> storedAssets = assetsForItem(snapshot, itemUuid);

        if (!storedContainer ||
            (storedAssets.size() != prepared.assets.size()))
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
        completedTransaction.payloadData = encodePreparedPayload(
            prepared, payloadStageRelative, originalModificationDate,
            protectMetadata);
        completedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

        if (completedTransaction.payloadData.isEmpty() ||
            !d->persistence.publishProtection(publishedItem, container, assets,
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

    if (!category ||
        ((category->backend != PrivacyBackend::Casual) &&
         (category->backend != PrivacyBackend::Strong)) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active))
    {
        return fail(PrivacyStillItemTransactionStatus::CategoryUnavailable, {},
                    QStringLiteral("privacy category is not active"));
    }

    const bool strongBackend = (category->backend == PrivacyBackend::Strong);

    if (strongBackend &&
        (request.vaultPlaintextRoot.isEmpty() ||
         !QFileInfo(request.vaultPlaintextRoot).isDir() ||
         !canonicalUuid(request.strongStoreUuid)))
    {
        return fail(PrivacyStillItemTransactionStatus::CategoryUnavailable, {},
                    QStringLiteral(
                        "Strong unprotect requires the mounted Originals vault"));
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
        QList<PrivacyAsset> teardownAssets;
        QString priorProtectUuid;

        if (!decodePreparedPayload(transaction->payloadData, &record,
                                   &ignoredStage,
                                   &ignoredModificationDate, &teardownBytes) ||
            !decodeTeardownSnapshot(teardownBytes, &teardownItem,
                                    &teardownContainer, &teardownAssets,
                                    &priorProtectUuid) ||
            (record.assets.size() != teardownAssets.size()) ||
            (record.categoryUuid != request.categoryUuid) ||
            (teardownItem.imageId != request.imageId))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, {},
                        QStringLiteral("detached Unprotect evidence is invalid"));
        }

        const auto primaryJournalIt = std::find_if(
            record.assets.cbegin(), record.assets.cend(),
            [](const PrivacyJournalAsset& candidate)
            {
                return ((candidate.role == PrivacyAsset::PrimaryMediaRole) &&
                        (candidate.ordinal == 0));
            });

        if ((primaryJournalIt == record.assets.cend()) ||
            (teardownItem.uuid != primaryJournalIt->itemUuid))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, {},
                        QStringLiteral("detached primary evidence is invalid"));
        }

        const PrivacyJournalAsset& primaryJournalAsset = *primaryJournalIt;
        const QString itemUuid = primaryJournalAsset.itemUuid;
        const QString publicPath = absolutePath(request.publicRoot,
                                                primaryJournalAsset.publicRelativePath);
        const PrivacyTransactionJournal* const replayJournal =
            databaseJournalFor(snapshot, transactionUuid,
                               request.publicRoot.uuid);
        for (const PrivacyAsset& teardownAsset : std::as_const(teardownAssets))
        {
            const auto journalIt = std::find_if(
                record.assets.cbegin(), record.assets.cend(),
                [&teardownAsset](const PrivacyJournalAsset& candidate)
                {
                    return ((candidate.role == teardownAsset.role) &&
                            (candidate.ordinal == teardownAsset.ordinal));
                });
            const QString restoredPath = absolutePath(
                request.publicRoot, teardownAsset.publicRelativePath);
            PrivacyJournalObjectFact publicFact;
            mode_t publicMode = 0;
            mode_t expectedMode = 0;
            QDateTime publicModificationDate;

            if ((journalIt == record.assets.cend()) ||
                (teardownAsset.publicRootUuid != request.publicRoot.uuid) ||
                !decodePortableMode(teardownAsset.portableAttributes,
                                    &expectedMode) ||
                !stableFileFact(restoredPath, nullptr, &publicFact, &publicMode,
                                &publicModificationDate) ||
                !sameFact(publicFact, journalIt->original) ||
                (publicMode != expectedMode) ||
                (publicModificationDate !=
                 teardownAsset.originalModificationDate.toUTC()))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::RecoveryRequired,
                    itemUuid,
                    QStringLiteral("restored public asset set is not exact"));
            }
        }

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

        if (strongBackend)
        {
            QString strongDetail;

            if (!PrivacyStrongObjectBackend::removeObjects(
                    request.vaultPlaintextRoot,
                    primaryJournalAsset.containerRelativePath,
                    &strongDetail))
            {
                return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                            itemUuid,
                            strongDetail.isEmpty()
                                ? QStringLiteral("Strong vault cleanup is pending")
                                : strongDetail);
            }
        }
        else if (!removeExactFile(request.publicRoot, request.rootExpectation,
                                  primaryJournalAsset.containerRelativePath,
                                  primaryJournalAsset.container, true))
        {
            return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                        itemUuid,
                        QStringLiteral("exact archive cleanup is pending"));
        }

        for (const PrivacyJournalAsset& journalAsset : record.assets)
        {
            if ((journalAsset.proxy.presence ==
                 PrivacyJournalExpectedPresence::Present) &&
                !removeExactFile(request.publicRoot, request.rootExpectation,
                                 journalAsset.stagedRelativePath,
                                 journalAsset.proxy, true))
            {
                return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                            itemUuid,
                            QStringLiteral("exact proxy cleanup is pending"));
            }
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
    const bool containerMatches =
        (strongBackend
             ? ((containerPointer &&
                 (containerPointer->kind ==
                  PrivacyContainerKind::StrongObject)) &&
                (containerPointer->storeUuid == request.strongStoreUuid) &&
                containerPointer->rootUuid.isEmpty())
             : ((containerPointer &&
                 (containerPointer->kind ==
                  PrivacyContainerKind::CasualArchive)) &&
                !containerPointer->rootUuid.isEmpty()));

    if (!containerPointer || assets.isEmpty() ||
        (assets.size() > PrivacyTransactionJournalCodec::MaximumAssetCount) ||
        !containerMatches ||
        (containerPointer->state != PrivacyContainerState::Verified))
    {
        return fail(PrivacyStillItemTransactionStatus::AssociatedAssetSetUnsupported,
                    itemUuid,
                    QStringLiteral("mapping is not a bounded protected asset set"));
    }

    const PrivacyContainer container = *containerPointer;
    const PrivacyAsset* asset = nullptr;

    for (const PrivacyAsset& candidate : assets)
    {
        mode_t candidateMode = 0;

        if ((candidate.itemUuid != itemUuid) ||
            (candidate.containerUuid != container.uuid) ||
            (candidate.publicRootUuid != request.publicRoot.uuid) ||
            !decodePortableMode(candidate.portableAttributes,
                                &candidateMode) ||
            !candidate.originalModificationDate.isValid())
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        itemUuid,
                        QStringLiteral("asset mode/mtime evidence is invalid"));
        }

        if ((candidate.role == PrivacyAsset::PrimaryMediaRole) &&
            (candidate.ordinal == 0))
        {
            if (asset || (candidate.proxySize < 0))
            {
                return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                            itemUuid,
                            QStringLiteral("primary asset mapping is ambiguous"));
            }

            asset = &candidate;
        }
        else if (candidate.proxySize >= 0)
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        itemUuid,
                        QStringLiteral(
                            "associated asset unexpectedly has a public proxy"));
        }
    }

    if (!asset)
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    itemUuid,
                    QStringLiteral("primary asset mapping is missing"));
    }

    const QString publicPath = absolutePath(request.publicRoot,
                                            asset->publicRelativePath);
    const QString archivePath = absolutePath(request.publicRoot,
                                             container.objectRelativePath);

    if (publicPath.isEmpty() || (!strongBackend && archivePath.isEmpty()))
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest, itemUuid,
                    QStringLiteral("stored public/archive path is unsafe"));
    }

    PrivacyJournalObjectFact archiveFact;

    if (strongBackend)
    {
        archiveFact.presence = PrivacyJournalExpectedPresence::Present;
        archiveFact.size = container.protectedSize;
        archiveFact.linkCount = 1;
        archiveFact.sha256 = QByteArray::fromHex(
            container.protectedHash.toLatin1());
    }
    else if (!stableFileFact(archivePath, nullptr, &archiveFact) ||
             (QString::fromLatin1(archiveFact.sha256.toHex()) !=
              container.protectedHash) ||
             (archiveFact.size != container.protectedSize))
    {
        return fail(PrivacyStillItemTransactionStatus::SourceChanged, itemUuid,
                    QStringLiteral("proxy/archive no longer match stored facts"));
    }

    if (!transaction)
    {
        for (const PrivacyAsset& candidate : assets)
        {
            const QString candidatePath = absolutePath(
                request.publicRoot, candidate.publicRelativePath);
            PrivacyJournalObjectFact candidateFact;

            if (candidate.proxySize >= 0)
            {
                PrivacyJournalObjectFact expected;
                expected.presence = PrivacyJournalExpectedPresence::Present;
                expected.size = candidate.proxySize;
                expected.linkCount = 1;
                expected.sha256 = QByteArray::fromHex(
                    candidate.proxyHash.toLatin1());

                if (!stableFileFact(candidatePath, nullptr, &candidateFact) ||
                    !sameFact(candidateFact, expected))
                {
                    return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                                itemUuid,
                                QStringLiteral("public proxy is not exact"));
                }
            }
            else
            {
                const QFileInfo candidateInfo(candidatePath);

                if (candidateInfo.exists() || candidateInfo.isSymLink())
                {
                    return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                                itemUuid,
                                QStringLiteral(
                                    "associated public asset is unexpectedly present"));
                }
            }
        }
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
        item, container, assets, priorProtectTransactionUuid);

    if (exactTeardown.isEmpty())
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, itemUuid,
                    QStringLiteral("cannot encode exact teardown snapshot"));
    }

    PrivacyJournalRecord created;
    PrivacyJournalRecord prepared;
    QString payloadPath;
    QDateTime payloadModificationDate = asset->originalModificationDate;
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
        for (const PrivacyAsset& candidate : assets)
        {
            PrivacyJournalAsset journalAsset;
            journalAsset.itemUuid = itemUuid;
            journalAsset.containerUuid = container.uuid;
            journalAsset.role = candidate.role;
            journalAsset.ordinal = candidate.ordinal;
            journalAsset.publicRelativePath = candidate.publicRelativePath;
            journalAsset.stagedRelativePath =
                parentPath(candidate.publicRelativePath) +
                (parentPath(candidate.publicRelativePath).isEmpty()
                    ? QString() : QLatin1String("/")) +
                PrivacyPublicTransitionEngine::expectedStageFileName(
                    transactionUuid, candidate.role, candidate.ordinal);
            journalAsset.protectedRelativePath =
                candidate.protectedRelativePath;
            journalAsset.containerRelativePath = container.objectRelativePath;
            journalAsset.original.presence =
                PrivacyJournalExpectedPresence::Present;
            journalAsset.original.size = candidate.originalSize;
            journalAsset.original.linkCount = 1;
            journalAsset.original.sha256 = QByteArray::fromHex(
                candidate.originalHash.toLatin1());

            if (candidate.proxySize >= 0)
            {
                journalAsset.proxy.presence =
                    PrivacyJournalExpectedPresence::Present;
                journalAsset.proxy.size = candidate.proxySize;
                journalAsset.proxy.linkCount = 1;
                journalAsset.proxy.sha256 = QByteArray::fromHex(
                    candidate.proxyHash.toLatin1());
            }
            else
            {
                journalAsset.proxy.presence =
                    PrivacyJournalExpectedPresence::Absent;
            }

            journalAsset.container = archiveFact;
            created.assets << journalAsset;
        }

        if (strongBackend)
        {
            QList<PrivacyStrongObjectMember> strongMembers;

            for (const PrivacyJournalAsset& journalAsset : created.assets)
            {
                PrivacyStrongObjectMember strongMember;
                strongMember.sourcePath.clear();
                strongMember.protectedRelativePath =
                    journalAsset.protectedRelativePath;
                strongMember.originalName = QFileInfo(
                    journalAsset.publicRelativePath).fileName();
                strongMember.expectedSize = journalAsset.original.size;
                strongMember.expectedSha256 = journalAsset.original.sha256;
                strongMembers << strongMember;
            }

            QString strongDetail;

            if (!PrivacyStrongObjectBackend::verifyObjects(
                    request.vaultPlaintextRoot,
                    container.objectRelativePath, strongMembers,
                    container.protectedSize,
                    QByteArray::fromHex(container.protectedHash.toLatin1()),
                    &strongDetail))
            {
                return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                            itemUuid, strongDetail);
            }
        }
        else if (!verifyArchiveMember(d->archive, created, archivePath,
                                      password))
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
        (payloadPath != container.objectRelativePath) ||
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
        (payloadPath != container.objectRelativePath) ||
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

        for (const PrivacyJournalAsset& journalAsset : prepared.assets)
        {
            const auto mappedIt = std::find_if(
                assets.cbegin(), assets.cend(),
                [&journalAsset](const PrivacyAsset& candidate)
                {
                    return ((candidate.role == journalAsset.role) &&
                            (candidate.ordinal == journalAsset.ordinal));
                });

            if (mappedIt == assets.cend())
            {
                return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                            itemUuid,
                            QStringLiteral("Unprotect asset mapping disappeared"));
            }

            PrivacyPublicReplacementStageRequest stageRequest;
            stageRequest.absoluteRootPath = request.publicRoot.configuredPath;
            stageRequest.rootExpectation = request.rootExpectation;
            stageRequest.journalRecord = prepared;
            stageRequest.authoritativeJournalSha256 = journalHash;
            stageRequest.itemUuid = itemUuid;
            stageRequest.role = journalAsset.role;
            stageRequest.ordinal = journalAsset.ordinal;
            const QString stagedOriginalPath = absolutePath(
                request.publicRoot, journalAsset.stagedRelativePath);
            PrivacyJournalObjectFact replayFact;

            if (d->durableReplay && !d->authenticatedCreatedReplay)
            {
                if (!QFileInfo::exists(stagedOriginalPath))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::AuthenticationRequired,
                        itemUuid,
                        QStringLiteral(
                            "Prepared Unprotect asset stages must be restored"));
                }

                if (!stableFileFact(stagedOriginalPath, nullptr, &replayFact) ||
                    !sameFact(replayFact, journalAsset.original))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::RecoveryRequired,
                        itemUuid,
                        QStringLiteral(
                            "Prepared Unprotect asset stage is not exact"));
                }

                continue;
            }

            PrivacyCasualArchiveRestoreRequest restore;

            if (!strongBackend)
            {
                restore.archivePath = archivePath;
                restore.categoryUuid = request.categoryUuid;
                restore.containerUuid = container.uuid;
                restore.itemUuid = itemUuid;
                restore.protectedRelativePath = mappedIt->protectedRelativePath;
                restore.originalName = mappedIt->originalName;
                restore.role = mappedIt->role;
                restore.ordinal = mappedIt->ordinal;
                restore.expectedArchiveSize = archiveFact.size;
                restore.expectedArchiveSha256 = archiveFact.sha256;
                restore.expectedMemberSize = mappedIt->originalSize;
                restore.expectedMemberSha256 = QByteArray::fromHex(
                    mappedIt->originalHash.toLatin1());
            }

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

                        if (strongBackend)
                        {
                            const QString vaultObjectPath = QDir(
                                request.vaultPlaintextRoot).filePath(
                                mappedIt->protectedRelativePath);
                            QFile vaultObject(vaultObjectPath);

                            if (!vaultObject.open(QIODevice::ReadOnly) ||
                                (vaultObject.size() != mappedIt->originalSize))
                            {
                                destination.close();

                                if (producerDetail)
                                {
                                    *producerDetail = QStringLiteral(
                                        "cannot open exact Strong vault object");
                                }

                                return false;
                            }

                            const QByteArray bytes = vaultObject.readAll();

                            if ((bytes.size() != mappedIt->originalSize) ||
                                (destination.write(bytes) != bytes.size()))
                            {
                                destination.close();

                                if (producerDetail)
                                {
                                    *producerDetail = QStringLiteral(
                                        "cannot restore Strong vault object bytes");
                                }

                                return false;
                            }
                        }
                        else
                        {
                            const bool restored = d->archive.restoreMember(
                                restore, password, &destination);

                            if (!restored)
                            {
                                destination.close();
                                return false;
                            }
                        }

                        destination.close();

                        const qint64 milliseconds =
                            mappedIt->originalModificationDate.toMSecsSinceEpoch();
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
                 !sameFact(replayFact, journalAsset.original)))
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

        QList<PrivacyPublicTransitionRequest> transitionRequests;

        for (const PrivacyJournalAsset& journalAsset : loaded.record.assets)
        {
            const auto mappedIt = std::find_if(
                assets.cbegin(), assets.cend(),
                [&journalAsset](const PrivacyAsset& candidate)
                {
                    return ((candidate.role == journalAsset.role) &&
                            (candidate.ordinal == journalAsset.ordinal));
                });
            mode_t installedMode = 0;

            if ((mappedIt == assets.cend()) ||
                !decodePortableMode(mappedIt->portableAttributes,
                                    &installedMode))
            {
                return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                            itemUuid,
                            QStringLiteral("Unprotect transition mode is missing"));
            }

            PrivacyPublicTransitionRequest transitionRequest;
            transitionRequest.absoluteRootPath = request.publicRoot.configuredPath;
            transitionRequest.rootExpectation = request.rootExpectation;
            transitionRequest.journalRecord = loaded.record;
            transitionRequest.authoritativeJournalSha256 = loaded.sha256;
            transitionRequest.itemUuid = itemUuid;
            transitionRequest.role = journalAsset.role;
            transitionRequest.ordinal = journalAsset.ordinal;
            transitionRequest.mode =
                (journalAsset.proxy.presence ==
                 PrivacyJournalExpectedPresence::Present)
                    ? PrivacyPublicTransitionMode::ExchangePresent
                    : PrivacyPublicTransitionMode::InstallAbsent;
            transitionRequest.currentFact =
                PrivacyPublicTransitionFactKind::Proxy;
            transitionRequest.installedFact =
                PrivacyPublicTransitionFactKind::Original;
            transitionRequest.installedUnixMode =
                static_cast<int>(installedMode);
            transitionRequests << transitionRequest;
        }

        const PrivacyPublicTransitionResult transitioned =
            d->transition.executeBatch(transitionRequests);

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

    if (strongBackend)
    {
        QString strongDetail;

        if (!PrivacyStrongObjectBackend::removeObjects(
                request.vaultPlaintextRoot,
                container.objectRelativePath, &strongDetail))
        {
            return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                        itemUuid,
                        strongDetail.isEmpty()
                            ? QStringLiteral("Strong vault cleanup is pending")
                            : strongDetail);
        }
    }
    else if (!removeExactFile(request.publicRoot, request.rootExpectation,
                              container.objectRelativePath, archiveFact, true))
    {
        return fail(PrivacyStillItemTransactionStatus::CleanupPending, itemUuid,
                    QStringLiteral("exact archive cleanup is pending"));
    }

    for (const PrivacyJournalAsset& journalAsset : prepared.assets)
    {
        if ((journalAsset.proxy.presence ==
             PrivacyJournalExpectedPresence::Present) &&
            !removeExactFile(request.publicRoot, request.rootExpectation,
                             journalAsset.stagedRelativePath,
                             journalAsset.proxy, true))
        {
            return fail(PrivacyStillItemTransactionStatus::CleanupPending,
                        itemUuid,
                        QStringLiteral("exact proxy cleanup is pending"));
        }
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

PrivacyStillItemTransactionResult
PrivacyStillItemTransactionEngine::compatibilityUnlock(
    const PrivacyCompatibilityUnlockRequest& request,
    const PrivacyPassword& password)
{
    const QString transactionUuid = normalizedUuid(request.transactionUuid);
    const QString itemUuid = normalizedUuid(request.itemUuid);
    const auto fail = [&](PrivacyStillItemTransactionStatus status,
                          const QString& detail)
    {
        return failure(status, transactionUuid, itemUuid, detail);
    };

    if ((request.imageId <= 0) || !canonicalUuid(request.categoryUuid) ||
        !canonicalUuid(request.itemUuid) ||
        !canonicalUuid(request.transactionUuid) ||
        !canonicalUuid(request.groupUuid) || !request.publicRoot.isValid() ||
        (request.publicRoot.kind != PrivacyStorageRootKind::AlbumRoot) ||
        !sameRootExpectation(request.publicRoot, request.rootExpectation) ||
        !password.isValid() || d->durableReplay)
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest,
                    QStringLiteral("Compatibility Unlock request is invalid"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot load Compatibility Unlock snapshot"));
    }

    const PrivacyItem* const item = itemForUuid(snapshot, itemUuid);
    const PrivacyCategory* const category = categoryForUuid(
        snapshot, request.categoryUuid);
    const PrivacyContainer* const container = containerForItem(snapshot, itemUuid);
    const QList<PrivacyAsset> assets = assetsForItem(snapshot, itemUuid);
    const bool strongBackend =
        (category && (category->backend == PrivacyBackend::Strong));
    const bool containerMatches =
        (category && !strongBackend)
            ? ((container &&
                (container->kind ==
                 PrivacyContainerKind::CasualArchive)) &&
               (container->rootUuid == request.publicRoot.uuid))
            : (strongBackend &&
               (container &&
                (container->kind ==
                 PrivacyContainerKind::StrongObject)) &&
               container->rootUuid.isEmpty() &&
               (container->storeUuid == request.strongStoreUuid));

    if (strongBackend &&
        (request.vaultPlaintextRoot.isEmpty() ||
         !QFileInfo(request.vaultPlaintextRoot).isDir() ||
         !canonicalUuid(request.strongStoreUuid)))
    {
        return fail(PrivacyStillItemTransactionStatus::InvalidRequest,
                    QStringLiteral(
                        "Strong Compatibility Unlock requires the mounted Originals vault"));
    }

    if (!item || (item->imageId != request.imageId) ||
        (item->categoryUuid != request.categoryUuid) || !category ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        !container || !containerMatches ||
        (container->state != PrivacyContainerState::Verified) ||
        assets.isEmpty() ||
        (assets.size() > PrivacyTransactionJournalCodec::MaximumAssetCount))
    {
        return fail(
            PrivacyStillItemTransactionStatus::AssociatedAssetSetUnsupported,
            QStringLiteral("item is not one exact active protected asset set"));
    }

    const QString archivePath = absolutePath(request.publicRoot,
                                             container->objectRelativePath);
    PrivacyJournalObjectFact archiveFact;

    if (strongBackend)
    {
        QList<PrivacyStrongObjectMember> strongMembers;

        for (const PrivacyAsset& asset : assets)
        {
            PrivacyStrongObjectMember strongMember;
            strongMember.sourcePath.clear();
            strongMember.protectedRelativePath =
                asset.protectedRelativePath;
            strongMember.originalName = asset.originalName;
            strongMember.expectedSize = asset.originalSize;
            strongMember.expectedSha256 = QByteArray::fromHex(
                asset.originalHash.toLatin1());
            strongMembers << strongMember;
        }

        QString strongDetail;

        if (!PrivacyStrongObjectBackend::verifyObjects(
                request.vaultPlaintextRoot,
                container->objectRelativePath, strongMembers,
                container->protectedSize,
                QByteArray::fromHex(container->protectedHash.toLatin1()),
                &strongDetail))
        {
            return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                        strongDetail);
        }

        archiveFact.presence = PrivacyJournalExpectedPresence::Present;
        archiveFact.size = container->protectedSize;
        archiveFact.linkCount = 1;
        archiveFact.sha256 = QByteArray::fromHex(
            container->protectedHash.toLatin1());
    }
    else if (archivePath.isEmpty() ||
             !stableFileFact(archivePath, nullptr, &archiveFact) ||
             (archiveFact.linkCount != 1) ||
             (archiveFact.size != container->protectedSize) ||
             (container->protectedHashAlgorithm != QLatin1String("sha256")) ||
             (archiveFact.sha256 !=
              QByteArray::fromHex(container->protectedHash.toLatin1())))
    {
        return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                    QStringLiteral("verified Casual archive facts do not match"));
    }

    PrivacyJournalRecord created;
    created.transactionUuid = transactionUuid;
    created.categoryUuid = request.categoryUuid;
    created.rootUuid = request.publicRoot.uuid;
    created.rootDevice = request.rootExpectation.device;
    created.rootInode = request.rootExpectation.inode;
    created.rootIdentitySha256 = request.rootExpectation.identitySha256;
    created.transactionType = PrivacyTransactionType::CompatibilityUnlock;
    created.generation = item->generation;
    created.credentialGeneration = container->credentialGeneration;
    created.fromCredentialGeneration = container->credentialGeneration;
    created.toCredentialGeneration = container->credentialGeneration;
    created.stage = PrivacyJournalStage::Created;
    bool foundPrimary = false;

    for (const PrivacyAsset& asset : assets)
    {
        mode_t portableMode = 0;

        if ((asset.itemUuid != itemUuid) ||
            (asset.containerUuid != container->uuid) ||
            (asset.publicRootUuid != request.publicRoot.uuid) ||
            (asset.hashAlgorithm != QLatin1String("sha256")) ||
            (asset.originalSize < 0) ||
            (QByteArray::fromHex(asset.originalHash.toLatin1()).size() != 32) ||
            !decodePortableMode(asset.portableAttributes, &portableMode) ||
            !asset.originalModificationDate.isValid())
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral("asset identity evidence is incomplete"));
        }

        const bool primary =
            ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
             (asset.ordinal == 0));
        PrivacyJournalAsset journalAsset;
        journalAsset.itemUuid = itemUuid;
        journalAsset.containerUuid = container->uuid;
        journalAsset.role = asset.role;
        journalAsset.ordinal = asset.ordinal;
        journalAsset.publicRelativePath = asset.publicRelativePath;
        journalAsset.stagedRelativePath =
            parentPath(asset.publicRelativePath) +
            (parentPath(asset.publicRelativePath).isEmpty()
                ? QString() : QLatin1String("/")) +
            PrivacyPublicTransitionEngine::expectedStageFileName(
                transactionUuid, asset.role, asset.ordinal);
        journalAsset.protectedRelativePath = asset.protectedRelativePath;
        journalAsset.containerRelativePath = container->objectRelativePath;
        journalAsset.original.presence = PrivacyJournalExpectedPresence::Present;
        journalAsset.original.size = asset.originalSize;
        journalAsset.original.linkCount = 1;
        journalAsset.original.sha256 = QByteArray::fromHex(
            asset.originalHash.toLatin1());
        journalAsset.container = archiveFact;
        const QString publicPath = absolutePath(request.publicRoot,
                                                asset.publicRelativePath);

        if (primary)
        {
            PrivacyJournalObjectFact proxyFact;

            if (foundPrimary || (asset.proxySize < 0) ||
                (asset.proxyHashAlgorithm != QLatin1String("sha256")) ||
                !stableFileFact(publicPath, nullptr, &proxyFact) ||
                (proxyFact.linkCount != 1) ||
                (proxyFact.size != asset.proxySize) ||
                (proxyFact.sha256 !=
                 QByteArray::fromHex(asset.proxyHash.toLatin1())))
            {
                return fail(PrivacyStillItemTransactionStatus::SourceChanged,
                            QStringLiteral("primary public proxy is not exact"));
            }

            foundPrimary = true;
            journalAsset.proxy = proxyFact;
        }
        else
        {
            const QFileInfo publicInfo(publicPath);

            if ((asset.proxySize >= 0) || publicInfo.exists() ||
                publicInfo.isSymLink())
            {
                return fail(
                    PrivacyStillItemTransactionStatus::SourceChanged,
                    QStringLiteral("associated public path is not absent"));
            }

            journalAsset.proxy.presence =
                PrivacyJournalExpectedPresence::Absent;
        }

        created.assets << journalAsset;
    }

    if (!foundPrimary ||
        (!strongBackend &&
         !verifyArchiveMember(d->archive, created, archivePath, password)))
    {
        return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                    QStringLiteral("archive/member/password verification failed"));
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    PrivacyTransaction transaction;
    transaction.uuid = transactionUuid;
    transaction.categoryUuid = request.categoryUuid;
    transaction.itemUuid = itemUuid;
    transaction.type = PrivacyTransactionType::CompatibilityUnlock;
    transaction.state = PrivacyTransactionState::Created;
    transaction.generation = 0;
    transaction.fromCredentialGeneration = container->credentialGeneration;
    transaction.toCredentialGeneration = container->credentialGeneration;
    transaction.payloadFormatVersion = 1;
    transaction.payloadData = encodeCompatibilityPayload(
        created, request.groupUuid);
    transaction.createdAt = now;
    transaction.updatedAt = now;
    PrivacyTransactionJournal databaseJournal;
    databaseJournal.transactionUuid = transactionUuid;
    databaseJournal.rootUuid = request.publicRoot.uuid;
    databaseJournal.journalRelativePath =
        PrivacyTransactionJournalCodec::relativeJournalPath(transactionUuid);
    databaseJournal.journalFormatVersion =
        PrivacyTransactionJournalCodec::FormatVersion;
    databaseJournal.stage = static_cast<int>(PrivacyJournalStage::Created);
    databaseJournal.updatedAt = now;

    if (transaction.payloadData.isEmpty() ||
        !d->persistence.beginCompatibilityUnlock(transaction, databaseJournal))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot atomically begin Compatibility Unlock"));
    }

    if (d->fault(
            PrivacyStillItemFaultPoint::AfterCompatibilityUnlockDatabaseBegin))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after Compatibility Unlock DB begin"));
    }

    QByteArray journalHash;
    QString detail;

    if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                           created, created, &journalHash, &detail))
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
    }

    const PrivacyJournalRecord prepared = recordAt(
        created, PrivacyJournalStage::Prepared);
    PrivacyTransaction preparedTransaction = transaction;
    preparedTransaction.state = PrivacyTransactionState::Prepared;
    preparedTransaction.generation = 1;
    preparedTransaction.payloadData = encodeCompatibilityPayload(
        prepared, request.groupUuid);
    preparedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

    if (preparedTransaction.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            preparedTransaction, PrivacyTransactionState::Created, 0) ||
        !d->advanceJournal(request.publicRoot, request.rootExpectation,
                           created, prepared, &journalHash, &detail))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    detail.isEmpty()
                        ? QStringLiteral("cannot prepare Compatibility Unlock")
                        : detail);
    }

    for (const PrivacyJournalAsset& journalAsset : prepared.assets)
    {
        const auto assetIt = std::find_if(
            assets.cbegin(), assets.cend(),
            [&journalAsset](const PrivacyAsset& asset)
            {
                return ((asset.role == journalAsset.role) &&
                        (asset.ordinal == journalAsset.ordinal));
            });

        if (assetIt == assets.cend())
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral("asset mapping disappeared"));
        }

        PrivacyPublicReplacementStageRequest stageRequest;
        stageRequest.absoluteRootPath = request.publicRoot.configuredPath;
        stageRequest.rootExpectation = request.rootExpectation;
        stageRequest.journalRecord = prepared;
        stageRequest.authoritativeJournalSha256 = journalHash;
        stageRequest.itemUuid = itemUuid;
        stageRequest.role = journalAsset.role;
        stageRequest.ordinal = journalAsset.ordinal;
        const PrivacyPublicReplacementStageResult staged =
            d->transition.stageReplacement(
                stageRequest,
                [&](int descriptor, QString* producerDetail)
                {
                    QFile destination;

                    if (!destination.open(descriptor, QIODevice::WriteOnly,
                                          QFileDevice::DontCloseHandle))
                    {
                        return false;
                    }

                    if (strongBackend)
                    {
                        const QString objectPath = QDir(
                            request.vaultPlaintextRoot).filePath(
                            assetIt->protectedRelativePath);
                        QFile object(objectPath);

                        if (!object.open(QIODevice::ReadOnly) ||
                            (object.size() != assetIt->originalSize))
                        {
                            destination.close();
                            return false;
                        }

                        QCryptographicHash hasher(QCryptographicHash::Sha256);
                        QByteArray buffer;
                        buffer.resize(1024 * 1024);

                        while (!object.atEnd())
                        {
                            const qint64 read =
                                object.read(buffer.data(), buffer.size());

                            if ((read <= 0) ||
                                (destination.write(buffer.constData(),
                                                   read) != read))
                            {
                                destination.close();
                                return false;
                            }

                            hasher.addData(buffer.constData(), read);
                        }

                        destination.close();

                        if (hasher.result() !=
                            QByteArray::fromHex(
                                assetIt->originalHash.toLatin1()))
                        {
                            if (producerDetail)
                            {
                                *producerDetail = QStringLiteral(
                                    "Strong vault object hash mismatch");
                            }

                            return false;
                        }

                        return true;
                    }
                    else
                    {
                        PrivacyCasualArchiveRestoreRequest restore;
                        restore.archivePath = archivePath;
                        restore.categoryUuid = request.categoryUuid;
                        restore.containerUuid = container->uuid;
                        restore.itemUuid = itemUuid;
                        restore.protectedRelativePath =
                            assetIt->protectedRelativePath;
                        restore.originalName = assetIt->originalName;
                        restore.role = assetIt->role;
                        restore.ordinal = assetIt->ordinal;
                        restore.expectedArchiveSize = archiveFact.size;
                        restore.expectedArchiveSha256 = archiveFact.sha256;
                        restore.expectedMemberSize = assetIt->originalSize;
                        restore.expectedMemberSha256 =
                            journalAsset.original.sha256;
                        const bool restored = d->archive.restoreMember(
                            restore, password, &destination);
                        destination.close();
                        return restored;
                    }
                });

        if (!staged.succeeded())
        {
            return fail(PrivacyStillItemTransactionStatus::ArchiveFailure,
                        staged.detail);
        }
    }

    const PrivacyJournalRecord staged = recordAt(
        prepared, PrivacyJournalStage::Staged);
    const PrivacyJournalRecord protectedCopy = recordAt(
        prepared, PrivacyJournalStage::ProtectedCopyVerified);

    if (!d->advanceJournal(request.publicRoot, request.rootExpectation,
                           prepared, staged, &journalHash, &detail) ||
        !d->advanceJournal(request.publicRoot, request.rootExpectation,
                           staged, protectedCopy, &journalHash, &detail))
    {
        return fail(PrivacyStillItemTransactionStatus::JournalFailure, detail);
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterCompatibilityUnlockStages))
    {
        return fail(PrivacyStillItemTransactionStatus::FaultInjected,
                    QStringLiteral("fault after Compatibility Unlock stages"));
    }

    PrivacyTransaction applying = preparedTransaction;
    applying.state = PrivacyTransactionState::Applying;
    applying.generation = 2;
    applying.payloadData = encodeCompatibilityPayload(
        protectedCopy, request.groupUuid);
    applying.updatedAt = QDateTime::currentDateTimeUtc();

    if (applying.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            applying, PrivacyTransactionState::Prepared, 1))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot publish applying Compatibility Unlock"));
    }

    if (!d->runtime.publishCompatibilityExposure(request.imageId, itemUuid,
                                                  true))
    {
        return fail(
            PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                    QStringLiteral("cannot conservatively gate Compatibility exposure"));
    }

    if (d->compatibilityGuardArmHook)
    {
        QString guardDetail;

        if (!d->compatibilityGuardArmHook(
                request.publicRoot, request.rootExpectation,
                transactionUuid, &guardDetail))
        {
            const PrivacyStillItemTransactionResult cancelled =
                PrivacyCompatibilityExposureGuardEngine::relock(
                    request.publicRoot, request.rootExpectation,
                    transactionUuid);
            const PrivacyJournalRecord complete = recordAt(
                protectedCopy, PrivacyJournalStage::Complete);

            if (!cancelled.succeeded() ||
                !d->advanceJournal(request.publicRoot,
                                   request.rootExpectation,
                                   protectedCopy, complete,
                                   &journalHash, &detail))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::RecoveryRequired,
                    guardDetail + QStringLiteral(
                        "; pre-exposure cancellation requires recovery: ") +
                    (cancelled.succeeded() ? detail : cancelled.detail));
            }

            PrivacyTransaction completed = applying;
            completed.state = PrivacyTransactionState::Complete;
            completed.generation = 3;
            completed.payloadData = encodeCompatibilityPayload(
                complete, request.groupUuid);
            completed.updatedAt = QDateTime::currentDateTimeUtc();

            if (completed.payloadData.isEmpty() ||
                !d->persistence.compareAndUpdateTransaction(
                    completed, PrivacyTransactionState::Applying, 2) ||
                !d->runtime.publishCompatibilityExposure(
                    request.imageId, itemUuid, false))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::RecoveryRequired,
                    guardDetail + QStringLiteral(
                        "; cancelled exposure state requires recovery"));
            }

            return fail(
                PrivacyStillItemTransactionStatus::RecoveryRequired,
                guardDetail + QStringLiteral(
                    "; Compatibility Unlock was cancelled before exposure"));
        }
    }

    if (d->fault(PrivacyStillItemFaultPoint::AfterCompatibilityUnlockApplying))
    {
        return fail(
            PrivacyStillItemTransactionStatus::FaultInjected,
            QStringLiteral("fault before Compatibility Unlock public transition"));
    }

    QList<PrivacyPublicTransitionRequest> transitions;

    for (const PrivacyAsset& asset : assets)
    {
        mode_t installedMode = 0;

        if (!decodePortableMode(asset.portableAttributes, &installedMode))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral("asset mode evidence disappeared"));
        }

        const auto journalIt = std::find_if(
            protectedCopy.assets.cbegin(), protectedCopy.assets.cend(),
            [&asset](const PrivacyJournalAsset& candidate)
            {
                return ((candidate.role == asset.role) &&
                        (candidate.ordinal == asset.ordinal));
            });

        if (journalIt == protectedCopy.assets.cend())
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral("journal asset mapping disappeared"));
        }

        PrivacyPublicTransitionRequest transition;
        transition.absoluteRootPath = request.publicRoot.configuredPath;
        transition.rootExpectation = request.rootExpectation;
        transition.journalRecord = protectedCopy;
        transition.authoritativeJournalSha256 = journalHash;
        transition.itemUuid = itemUuid;
        transition.role = asset.role;
        transition.ordinal = asset.ordinal;
        transition.mode =
            (journalIt->proxy.presence ==
             PrivacyJournalExpectedPresence::Present)
                ? PrivacyPublicTransitionMode::ExchangePresent
                : PrivacyPublicTransitionMode::InstallAbsent;
        transition.currentFact = PrivacyPublicTransitionFactKind::Proxy;
        transition.installedFact = PrivacyPublicTransitionFactKind::Original;
        transition.installedUnixMode = static_cast<int>(installedMode);
        transitions << transition;
    }

    const auto primaryTransitionAsset = std::find_if(
        protectedCopy.assets.cbegin(), protectedCopy.assets.cend(),
        [](const PrivacyJournalAsset& asset)
        {
            return ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                    (asset.ordinal == 0));
        });
    const QString primaryPublicPath =
        (primaryTransitionAsset == protectedCopy.assets.cend())
            ? QString()
            : absolutePath(request.publicRoot,
                           primaryTransitionAsset->publicRelativePath);
    PrivacyJournalObjectFact transitionArchiveFact;

    if (!strongBackend &&
        (!stableFileFact(archivePath, nullptr, &transitionArchiveFact) ||
         !sameFact(transitionArchiveFact, archiveFact)))
    {
        return fail(
            PrivacyStillItemTransactionStatus::ArchiveFailure,
            QStringLiteral(
                "protected archive changed before Compatibility exposure"));
    }

    if (primaryPublicPath.isEmpty() ||
        !d->cache.begin(request.imageId, primaryPublicPath, false, false))
    {
        return fail(PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                    QStringLiteral("cannot begin Compatibility Unlock cache gate"));
    }

    const PrivacyPublicTransitionResult transitioned =
        d->transition.executeBatch(transitions);
    const PrivacyJournalRecord publicVerified = recordAt(
        prepared, PrivacyJournalStage::PublicStateVerified);

    if (!transitioned.succeeded() ||
        !d->advanceJournal(request.publicRoot, request.rootExpectation,
                           protectedCopy, publicVerified, &journalHash, &detail))
    {
        return fail(PrivacyStillItemTransactionStatus::PublicTransitionFailure,
                    transitioned.succeeded() ? detail : transitioned.detail);
    }

    if (d->fault(
            PrivacyStillItemFaultPoint::AfterCompatibilityUnlockPublicTransition))
    {
        return fail(
            PrivacyStillItemTransactionStatus::FaultInjected,
            QStringLiteral("fault after Compatibility Unlock public transition"));
    }

    if (!d->cache.finish(request.imageId, primaryPublicPath, false, true))
    {
        return fail(PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                    QStringLiteral("cannot finish Compatibility Unlock cache gate"));
    }

    PrivacyTransaction exposed = applying;
    exposed.state = PrivacyTransactionState::Exposed;
    exposed.generation = 3;
    exposed.payloadData = encodeCompatibilityPayload(
        publicVerified, request.groupUuid);
    exposed.updatedAt = QDateTime::currentDateTimeUtc();

    if (exposed.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            exposed, PrivacyTransactionState::Applying, 2))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot publish Compatibility Unlock exposure"));
    }

    PrivacyStillItemTransactionResult result;
    result.status = PrivacyStillItemTransactionStatus::CompatibilityUnlocked;
    result.transactionUuid = transactionUuid;
    result.itemUuid = itemUuid;
    return result;
}

PrivacyCompatibilityBatchResult
PrivacyStillItemTransactionEngine::compatibilityUnlockBatch(
    const QList<PrivacyCompatibilityUnlockRequest>& requests,
    const PrivacyPassword& password,
    const CompatibilityBatchProgress& progress)
{
    PrivacyCompatibilityBatchResult batch;
    batch.requestedCount = requests.size();

    if (requests.isEmpty() || !password.isValid())
    {
        batch.detail = QStringLiteral(
            "the grouped Compatibility Unlock request is invalid");
        return batch;
    }

    const QString categoryUuid = normalizedUuid(
        requests.constFirst().categoryUuid);
    const QString groupUuid = normalizedUuid(requests.constFirst().groupUuid);
    QSet<QString> itemUuids;
    QSet<QString> transactionUuids;
    QSet<qlonglong> imageIds;

    for (const PrivacyCompatibilityUnlockRequest& request : requests)
    {
        const QString itemUuid = normalizedUuid(request.itemUuid);
        const QString transactionUuid = normalizedUuid(request.transactionUuid);

        if (categoryUuid.isEmpty() || groupUuid.isEmpty() || itemUuid.isEmpty() ||
            transactionUuid.isEmpty() || (request.imageId <= 0) ||
            (normalizedUuid(request.categoryUuid) != categoryUuid) ||
            (normalizedUuid(request.groupUuid) != groupUuid) ||
            itemUuids.contains(itemUuid) ||
            transactionUuids.contains(transactionUuid) ||
            imageIds.contains(request.imageId))
        {
            batch.detail = QStringLiteral(
                "grouped Compatibility Unlock members conflict");
            return batch;
        }

        itemUuids.insert(itemUuid);
        transactionUuids.insert(transactionUuid);
        imageIds.insert(request.imageId);
    }

    for (int index = 0 ; index < requests.size() ; ++index)
    {
        const PrivacyStillItemTransactionResult item =
            compatibilityUnlock(requests.at(index), password);
        batch.itemResults.append(item);
        batch.processedCount = index + 1;

        if (progress)
        {
            progress(batch.processedCount, batch.requestedCount);
        }

        if (item.succeeded())
        {
            continue;
        }

        PrivacyRepositorySnapshot snapshot;
        const bool loaded = d->persistence.loadSnapshot(&snapshot);
        QSet<QString> activeTransactionUuids;

        if (loaded)
        {
            for (const PrivacyTransaction& transaction :
                 std::as_const(snapshot.transactions))
            {
                if (transaction.isActive() &&
                    transactionUuids.contains(transaction.uuid))
                {
                    activeTransactionUuids.insert(transaction.uuid);
                }
            }
        }

        QList<PrivacyCompatibilityRelockRequest> rollbackRequests;

        for (int rollbackIndex = index ; rollbackIndex >= 0 ; --rollbackIndex)
        {
            const PrivacyCompatibilityUnlockRequest& request =
                requests.at(rollbackIndex);

            if (loaded &&
                !activeTransactionUuids.contains(request.transactionUuid))
            {
                continue;
            }

            PrivacyCompatibilityRelockRequest rollback;
            rollback.transactionUuid = request.transactionUuid;
            rollback.publicRoot = request.publicRoot;
            rollback.rootExpectation = request.rootExpectation;
            rollbackRequests.append(rollback);
        }

        const PrivacyCompatibilityBatchResult rolledBack =
            compatibilityRelockBatch(rollbackRequests);
        batch.rollbackResults = rolledBack.itemResults;
        batch.remainingExposureCount = rolledBack.remainingExposureCount;

        if (!loaded || (batch.remainingExposureCount > 0))
        {
            batch.status = std::any_of(
                batch.rollbackResults.cbegin(), batch.rollbackResults.cend(),
                [](const PrivacyStillItemTransactionResult& result)
                {
                    return (result.status ==
                            PrivacyStillItemTransactionStatus::
                                ReconciliationRequired);
                })
                ? PrivacyStillItemTransactionStatus::ReconciliationRequired
                : PrivacyStillItemTransactionStatus::RecoveryRequired;
            batch.detail = QStringLiteral(
                "Compatibility Unlock stopped after %1 of %2 items; "
                "%3 exposure(s) still require recovery")
                               .arg(batch.processedCount)
                               .arg(batch.requestedCount)
                               .arg(batch.remainingExposureCount);
        }
        else
        {
            batch.status = item.status;
            batch.detail = QStringLiteral(
                "Compatibility Unlock stopped after %1 of %2 items; "
                "every started exposure was safely relocked: %3")
                               .arg(batch.processedCount)
                               .arg(batch.requestedCount)
                               .arg(item.detail);
        }

        return batch;
    }

    batch.status = PrivacyStillItemTransactionStatus::CompatibilityUnlocked;
    batch.remainingExposureCount = batch.requestedCount;
    return batch;
}

PrivacyCompatibilityBatchResult
PrivacyStillItemTransactionEngine::compatibilityRelockBatch(
    const QList<PrivacyCompatibilityRelockRequest>& requests,
    const CompatibilityBatchProgress& progress)
{
    PrivacyCompatibilityBatchResult batch;
    batch.requestedCount = requests.size();

    if (requests.isEmpty())
    {
        batch.detail = QStringLiteral(
            "the grouped Compatibility Relock request is empty");
        return batch;
    }

    QSet<QString> transactionUuids;

    for (const PrivacyCompatibilityRelockRequest& request : requests)
    {
        const QString transactionUuid = normalizedUuid(request.transactionUuid);

        if (transactionUuid.isEmpty() ||
            transactionUuids.contains(transactionUuid) ||
            !request.publicRoot.isValid())
        {
            batch.detail = QStringLiteral(
                "grouped Compatibility Relock members conflict");
            return batch;
        }

        transactionUuids.insert(transactionUuid);
    }

    bool reconciliationRequired = false;

    for (const PrivacyCompatibilityRelockRequest& request : requests)
    {
        const PrivacyStillItemTransactionResult guarded =
            PrivacyCompatibilityExposureGuardEngine::relock(
                request.publicRoot, request.rootExpectation,
                request.transactionUuid);
        PrivacyStillItemTransactionResult settled = recover(
            request.publicRoot, request.transactionUuid);

        if (!settled.succeeded() && settled.detail.isEmpty())
        {
            settled.detail = guarded.detail;
        }

        reconciliationRequired = reconciliationRequired ||
            (guarded.status ==
             PrivacyStillItemTransactionStatus::ReconciliationRequired) ||
            (settled.status ==
             PrivacyStillItemTransactionStatus::ReconciliationRequired);
        batch.itemResults.append(settled);
        ++batch.processedCount;

        if (!settled.succeeded())
        {
            ++batch.remainingExposureCount;
        }

        if (progress)
        {
            progress(batch.processedCount, batch.requestedCount);
        }
    }

    if (batch.remainingExposureCount == 0)
    {
        batch.status = PrivacyStillItemTransactionStatus::CompatibilityRelocked;
    }
    else
    {
        batch.status = reconciliationRequired
                     ? PrivacyStillItemTransactionStatus::ReconciliationRequired
                     : PrivacyStillItemTransactionStatus::RecoveryRequired;
        batch.detail = QStringLiteral(
            "%1 of %2 Compatibility exposure(s) still require recovery")
                           .arg(batch.remainingExposureCount)
                           .arg(batch.requestedCount);
    }

    return batch;
}


PrivacyStillItemTransactionResult
PrivacyStillItemTransactionEngine::recoverCompatibility(
    const PrivacyStorageRoot& publicRoot,
    const PrivacyTransaction& transaction)
{
    const auto fail = [&](PrivacyStillItemTransactionStatus status,
                          const QString& detail)
    {
        return failure(status, transaction.uuid, transaction.itemUuid, detail);
    };
    PrivacyJournalRecord record;
    QString groupUuid;

    if ((transaction.type !=
         PrivacyTransactionType::CompatibilityUnlock) ||
        !decodeCompatibilityPayload(
            transaction.payloadData, &record, &groupUuid) ||
        (record.transactionUuid != transaction.uuid) ||
        (record.transactionType != transaction.type) ||
        (record.categoryUuid != transaction.categoryUuid) ||
        (record.rootUuid != publicRoot.uuid) || record.assets.isEmpty())
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Compatibility recovery payload is not exact"));
    }

    PrivacyJournalRootExpectation expectation;
    expectation.rootUuid = record.rootUuid;
    expectation.markerUuid = publicRoot.markerUuid;
    expectation.identitySha256 = record.rootIdentitySha256;
    expectation.device = record.rootDevice;
    expectation.inode = record.rootInode;

    if (!sameRootExpectation(publicRoot, expectation))
    {
        return fail(PrivacyStillItemTransactionStatus::RootUnavailable,
                    QStringLiteral("Compatibility recovery root is not exact"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return fail(PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot load Compatibility recovery snapshot"));
    }

    const PrivacyItem* const item = itemForUuid(snapshot, transaction.itemUuid);

    if (!item || (item->categoryUuid != transaction.categoryUuid))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Compatibility recovery item is missing"));
    }

    QByteArray journalHash;
    QString detail;

    {
        if ((transaction.state == PrivacyTransactionState::Created) ||
            (transaction.state == PrivacyTransactionState::Prepared))
        {
            for (const PrivacyJournalAsset& asset : record.assets)
            {
                const QString publicPath = absolutePath(
                    publicRoot, asset.publicRelativePath);

                if (asset.proxy.presence ==
                    PrivacyJournalExpectedPresence::Present)
                {
                    PrivacyJournalObjectFact current;

                    if (!stableFileFact(publicPath, nullptr, &current) ||
                        !sameFact(current, asset.proxy))
                    {
                        return fail(
                            PrivacyStillItemTransactionStatus::RecoveryRequired,
                            QStringLiteral(
                                "pre-exposure public proxy is not exact"));
                    }
                }
                else
                {
                    const QFileInfo info(publicPath);

                    if (info.exists() || info.isSymLink())
                    {
                        return fail(
                            PrivacyStillItemTransactionStatus::RecoveryRequired,
                            QStringLiteral(
                                "pre-exposure associated path is not absent"));
                    }
                }

                if (!removeExactFile(publicRoot, expectation,
                                     asset.stagedRelativePath,
                                     asset.original, true))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::CleanupPending,
                        QStringLiteral(
                            "cancelled exposure stage cleanup is pending"));
                }
            }

            const PrivacyTransactionJournal* databaseJournal =
                databaseJournalFor(snapshot, transaction.uuid, record.rootUuid);

            if (!databaseJournal)
            {
                return fail(
                    PrivacyStillItemTransactionStatus::JournalFailure,
                    QStringLiteral("Compatibility database journal is missing"));
            }

            PrivacyJournalRecord predecessor = recordAt(
                record, static_cast<PrivacyJournalStage>(databaseJournal->stage));

            if (databaseJournal->stage ==
                static_cast<int>(PrivacyJournalStage::Created))
            {
                const PrivacyJournalRecord created = recordAt(
                    record, PrivacyJournalStage::Created);

                if (!d->advanceJournal(publicRoot, expectation, created, created,
                                       &journalHash, &detail))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::JournalFailure,
                        detail);
                }

                predecessor = created;
            }

            const PrivacyJournalRecord complete = recordAt(
                record, PrivacyJournalStage::Complete);

            if (!d->advanceJournal(publicRoot, expectation, predecessor,
                                   complete, &journalHash, &detail))
            {
                return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                            detail);
            }

            PrivacyTransaction completed = transaction;
            completed.state = PrivacyTransactionState::Complete;
            completed.generation = transaction.generation + 1;
            completed.payloadData = encodeCompatibilityPayload(
                complete, groupUuid);
            completed.updatedAt = QDateTime::currentDateTimeUtc();

            if (completed.payloadData.isEmpty() ||
                !d->persistence.compareAndUpdateTransaction(
                    completed, transaction.state, transaction.generation))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot cancel pre-exposure transaction"));
            }

            if (!d->runtime.publishCompatibilityExposure(
                    item->imageId, item->uuid, false))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                    QStringLiteral("cannot clear cancelled exposure gate"));
            }

            PrivacyStillItemTransactionResult result;
            result.status =
                PrivacyStillItemTransactionStatus::CompatibilityRelocked;
            result.transactionUuid = transaction.uuid;
            result.itemUuid = transaction.itemUuid;
            return result;
        }

        PrivacyTransaction exposed = transaction;
        PrivacyJournalRecord publicVerified = recordAt(
            record, PrivacyJournalStage::PublicStateVerified);

        if ((transaction.state == PrivacyTransactionState::Applying) &&
            (record.stage != PrivacyJournalStage::ProtectedCopyVerified))
        {
            return fail(
                PrivacyStillItemTransactionStatus::RecoveryRequired,
                QStringLiteral("Compatibility Unlock Applying payload is not exact"));
        }

        if ((transaction.state != PrivacyTransactionState::Applying) &&
            (transaction.state != PrivacyTransactionState::Exposed) &&
            (transaction.state !=
             PrivacyTransactionState::NeedsReconciliation))
        {
            return fail(
                PrivacyStillItemTransactionStatus::RecoveryRequired,
                QStringLiteral(
                    "Compatibility Unlock has not reached password-free recovery"));
        }

        if (!d->runtime.publishCompatibilityExposure(item->imageId,
                                                      item->uuid, true))
        {
            return fail(
                PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                QStringLiteral("cannot publish recovered exposure gate"));
        }

        if ((transaction.state == PrivacyTransactionState::Exposed) ||
            (transaction.state ==
             PrivacyTransactionState::NeedsReconciliation))
        {
            PrivacyJournalError loadError = PrivacyJournalError::None;
            std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
                PrivacyTransactionJournalStore::open(
                    publicRoot.configuredPath, expectation, &loadError,
                    &detail);

            if (!journalStore)
            {
                return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                            detail);
            }

            const PrivacyJournalLoadResult loaded = journalStore->load(
                transaction.uuid);
            const bool allowedStage =
                loaded.hasRecord &&
                ((loaded.record.stage ==
                  PrivacyJournalStage::PublicStateVerified) ||
                 (loaded.record.stage ==
                  PrivacyJournalStage::ReconciliationRequired) ||
                 (loaded.record.stage == PrivacyJournalStage::Complete));
            const PrivacyJournalRecord expectedLoaded = allowedStage
                ? recordAt(publicVerified, loaded.record.stage)
                : PrivacyJournalRecord();
            const QByteArray expectedBytes = allowedStage
                ? PrivacyTransactionJournalCodec::encode(expectedLoaded)
                : QByteArray();

            if ((loaded.disposition !=
                 PrivacyJournalLoadDisposition::Loaded) ||
                !loaded.authoritative || !allowedStage ||
                expectedBytes.isEmpty() ||
                (loaded.canonicalBytes != expectedBytes) ||
                (loaded.sha256 != PrivacyTransactionJournalCodec::sha256(
                     expectedBytes)))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::JournalFailure,
                    QStringLiteral(
                        "Compatibility exposure journal facts changed"));
            }

            const bool guardStage =
                ((loaded.record.stage ==
                  PrivacyJournalStage::ReconciliationRequired) ||
                 (loaded.record.stage == PrivacyJournalStage::Complete));

            if (guardStage)
            {
                const PrivacyStillItemTransactionResult guarded =
                    PrivacyCompatibilityExposureGuardEngine::relock(
                        publicRoot, expectation, transaction.uuid);

                if (!guarded.succeeded())
                {
                    if ((guarded.status ==
                         PrivacyStillItemTransactionStatus::
                             ReconciliationRequired) &&
                        (transaction.state ==
                         PrivacyTransactionState::Exposed))
                    {
                        PrivacyTransaction pending = transaction;
                        pending.state =
                            PrivacyTransactionState::NeedsReconciliation;
                        pending.generation = transaction.generation + 1;
                        pending.payloadData = encodeCompatibilityPayload(
                            recordAt(
                                publicVerified,
                                PrivacyJournalStage::ReconciliationRequired),
                            groupUuid);
                        pending.updatedAt = QDateTime::currentDateTimeUtc();

                        if (pending.payloadData.isEmpty() ||
                            !d->persistence.compareAndUpdateTransaction(
                                pending, PrivacyTransactionState::Exposed,
                                transaction.generation))
                        {
                            return fail(
                                PrivacyStillItemTransactionStatus::
                                    PersistenceFailure,
                                QStringLiteral(
                                    "cannot publish guard reconciliation"));
                        }
                    }

                    return fail(guarded.status, guarded.detail);
                }

                const PrivacyJournalRecord complete = recordAt(
                    publicVerified, PrivacyJournalStage::Complete);

                if (!d->advanceJournal(publicRoot, expectation,
                                       publicVerified, complete,
                                       &journalHash, &detail))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::JournalFailure,
                        detail);
                }

                PrivacyTransaction completed = transaction;
                completed.state = PrivacyTransactionState::Complete;
                completed.generation = transaction.generation + 1;
                completed.payloadData = encodeCompatibilityPayload(
                    complete, groupUuid);
                completed.updatedAt = QDateTime::currentDateTimeUtc();

                if (completed.payloadData.isEmpty() ||
                    !d->persistence.compareAndUpdateTransaction(
                        completed, transaction.state,
                        transaction.generation))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral(
                            "cannot complete guard-relocked exposure"));
                }

                if (!d->runtime.publishCompatibilityExposure(
                        item->imageId, item->uuid, false))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::
                            RuntimePublicationFailure,
                        QStringLiteral(
                            "cannot clear guard-relocked exposure gate"));
                }

                PrivacyStillItemTransactionResult result;
                result.status =
                    PrivacyStillItemTransactionStatus::CompatibilityRelocked;
                result.transactionUuid = transaction.uuid;
                result.itemUuid = transaction.itemUuid;
                return result;
            }
        }

        if (transaction.state == PrivacyTransactionState::Applying)
        {
            PrivacyJournalError loadError = PrivacyJournalError::None;
            std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
                PrivacyTransactionJournalStore::open(
                    publicRoot.configuredPath, expectation, &loadError,
                    &detail);

            if (!journalStore)
            {
                return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                            detail);
            }

            const PrivacyJournalLoadResult loaded = journalStore->load(
                transaction.uuid);

            if (!transitionJournalBoundToPayload(snapshot, record, loaded))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::JournalFailure,
                    QStringLiteral(
                        "Compatibility Unlock transition journal is not authoritative"));
            }

            const QList<PrivacyAsset> mappedAssets = assetsForItem(
                snapshot, item->uuid);
            QList<PrivacyPublicTransitionRequest> transitions;
            QString primaryPublicPath;

            if (mappedAssets.size() != loaded.record.assets.size())
            {
                return fail(
                    PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral("Compatibility Unlock asset set changed"));
            }

            for (const PrivacyJournalAsset& journalAsset : loaded.record.assets)
            {
                const auto mappedIt = std::find_if(
                    mappedAssets.cbegin(), mappedAssets.cend(),
                    [&journalAsset](const PrivacyAsset& candidate)
                    {
                        return ((candidate.role == journalAsset.role) &&
                                (candidate.ordinal == journalAsset.ordinal));
                    });
                mode_t installedMode = 0;

                if ((mappedIt == mappedAssets.cend()) ||
                    !decodePortableMode(mappedIt->portableAttributes,
                                        &installedMode))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral(
                            "Compatibility Unlock mode evidence is missing"));
                }

                PrivacyPublicTransitionRequest transition;
                transition.absoluteRootPath = publicRoot.configuredPath;
                transition.rootExpectation = expectation;
                transition.journalRecord = loaded.record;
                transition.authoritativeJournalSha256 = loaded.sha256;
                transition.itemUuid = item->uuid;
                transition.role = journalAsset.role;
                transition.ordinal = journalAsset.ordinal;
                transition.mode =
                    (journalAsset.proxy.presence ==
                     PrivacyJournalExpectedPresence::Present)
                        ? PrivacyPublicTransitionMode::ExchangePresent
                        : PrivacyPublicTransitionMode::InstallAbsent;
                transition.currentFact =
                    PrivacyPublicTransitionFactKind::Proxy;
                transition.installedFact =
                    PrivacyPublicTransitionFactKind::Original;
                transition.installedUnixMode = static_cast<int>(installedMode);
                transitions << transition;

                if ((journalAsset.role == PrivacyAsset::PrimaryMediaRole) &&
                    (journalAsset.ordinal == 0))
                {
                    primaryPublicPath = absolutePath(
                        publicRoot, journalAsset.publicRelativePath);
                }
            }

            if (primaryPublicPath.isEmpty() ||
                !d->cache.begin(item->imageId, primaryPublicPath, false, false))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                    QStringLiteral(
                        "cannot begin recovered Compatibility Unlock cache gate"));
            }

            const PrivacyPublicTransitionResult transitioned =
                d->transition.executeBatch(transitions);

            if (!transitioned.succeeded())
            {
                return fail(
                    PrivacyStillItemTransactionStatus::PublicTransitionFailure,
                    transitioned.detail);
            }

            if (!d->advanceJournal(publicRoot, expectation, record,
                                   publicVerified, &journalHash, &detail))
            {
                return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                            detail);
            }

            for (const PrivacyJournalAsset& asset : publicVerified.assets)
            {
                PrivacyJournalObjectFact current;

                if (!stableFileFact(
                        absolutePath(publicRoot, asset.publicRelativePath),
                        nullptr, &current) || !sameFact(current, asset.original))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::RecoveryRequired,
                        QStringLiteral(
                            "Compatibility exposure is mixed or changed"));
                }
            }

            if (!d->cache.finish(item->imageId, primaryPublicPath, false, true))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::CacheTransitionFailure,
                    QStringLiteral(
                        "cannot finish recovered Compatibility Unlock cache gate"));
            }

            exposed.state = PrivacyTransactionState::Exposed;
            exposed.generation = transaction.generation + 1;
            exposed.payloadData = encodeCompatibilityPayload(
                publicVerified, groupUuid);
            exposed.updatedAt = QDateTime::currentDateTimeUtc();

            if (exposed.payloadData.isEmpty() ||
                !d->persistence.compareAndUpdateTransaction(
                    exposed, PrivacyTransactionState::Applying,
                    transaction.generation))
            {
                return fail(
                    PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("cannot publish recovered exposure"));
            }
        }

        const PrivacyStillItemTransactionResult guarded =
            PrivacyCompatibilityExposureGuardEngine::relock(
                publicRoot, expectation, transaction.uuid);

        if (!guarded.succeeded())
        {
            if ((guarded.status ==
                 PrivacyStillItemTransactionStatus::ReconciliationRequired) &&
                (exposed.state == PrivacyTransactionState::Exposed))
            {
                PrivacyTransaction pending = exposed;
                pending.state =
                    PrivacyTransactionState::NeedsReconciliation;
                pending.generation = exposed.generation + 1;
                pending.payloadData = encodeCompatibilityPayload(
                    recordAt(
                        publicVerified,
                        PrivacyJournalStage::ReconciliationRequired),
                    groupUuid);
                pending.updatedAt = QDateTime::currentDateTimeUtc();

                if (pending.payloadData.isEmpty() ||
                    !d->persistence.compareAndUpdateTransaction(
                        pending, PrivacyTransactionState::Exposed,
                        exposed.generation))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral(
                            "cannot publish guarded reconciliation"));
                }
            }

            return fail(guarded.status, guarded.detail);
        }

        const PrivacyJournalRecord complete = recordAt(
            publicVerified, PrivacyJournalStage::Complete);

        if (!d->advanceJournal(publicRoot, expectation,
                               publicVerified, complete,
                               &journalHash, &detail))
        {
            return fail(PrivacyStillItemTransactionStatus::JournalFailure,
                        detail);
        }

        PrivacyTransaction completed = exposed;
        completed.state = PrivacyTransactionState::Complete;
        completed.generation = exposed.generation + 1;
        completed.payloadData = encodeCompatibilityPayload(
            complete, groupUuid);
        completed.updatedAt = QDateTime::currentDateTimeUtc();

        if (completed.payloadData.isEmpty() ||
            !d->persistence.compareAndUpdateTransaction(
                completed, exposed.state, exposed.generation))
        {
            return fail(
                PrivacyStillItemTransactionStatus::PersistenceFailure,
                QStringLiteral("cannot complete guarded exposure"));
        }

        if (!d->runtime.publishCompatibilityExposure(
                item->imageId, item->uuid, false))
        {
            return fail(
                PrivacyStillItemTransactionStatus::RuntimePublicationFailure,
                QStringLiteral(
                    "cannot clear guarded exposure gate"));
        }

        PrivacyStillItemTransactionResult result;
        result.status =
            PrivacyStillItemTransactionStatus::CompatibilityRelocked;
        result.transactionUuid = transaction.uuid;
        result.itemUuid = transaction.itemUuid;
        return result;
    }
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
        (transaction->payloadFormatVersion != 1))
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired, {},
                    QStringLiteral("recoverable still transaction is missing"));
    }

    if (transaction->type == PrivacyTransactionType::CompatibilityUnlock)
    {
        return recoverCompatibility(publicRoot, *transaction);
    }

    if ((transaction->type != PrivacyTransactionType::ProtectItem) &&
        (transaction->type != PrivacyTransactionType::UnprotectItem))
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
        record.assets.isEmpty() ||
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

    const auto journalAssetIt = std::find_if(
        record.assets.cbegin(), record.assets.cend(),
        [](const PrivacyJournalAsset& candidate)
        {
            return ((candidate.role == PrivacyAsset::PrimaryMediaRole) &&
                    (candidate.ordinal == 0));
        });

    if (journalAssetIt == record.assets.cend())
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    transaction->itemUuid,
                    QStringLiteral("durable primary payload is missing"));
    }

    const PrivacyJournalAsset& journalAsset = *journalAssetIt;

    const QString expectedPayloadPath =
        (transaction->type == PrivacyTransactionType::ProtectItem)
            ? protectArchiveStageRelativePath(journalAsset.publicRelativePath,
                                              transactionUuid)
            : journalAsset.containerRelativePath;

    if (payloadPath != expectedPayloadPath)
    {
        return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                    journalAsset.itemUuid,
                    QStringLiteral("durable archive payload path is not exact"));
    }

    const PrivacyItem* item = itemForUuid(snapshot, journalAsset.itemUuid);
    qlonglong imageId = item ? item->imageId : -1;
    PrivacyItem teardownItem;

    if (!item && (transaction->type == PrivacyTransactionType::UnprotectItem) &&
        (transaction->state == PrivacyTransactionState::Applying))
    {
        PrivacyContainer teardownContainer;
        QList<PrivacyAsset> teardownAssets;
        QString priorProtectTransactionUuid;

        if (!decodeTeardownSnapshot(metadata, &teardownItem, &teardownContainer,
                                    &teardownAssets,
                                    &priorProtectTransactionUuid) ||
            (teardownItem.uuid != journalAsset.itemUuid) ||
            (teardownItem.categoryUuid != transaction->categoryUuid) ||
            (teardownAssets.size() != record.assets.size()))
        {
            return fail(PrivacyStillItemTransactionStatus::RecoveryRequired,
                        journalAsset.itemUuid,
                        QStringLiteral("detached durable teardown is not exact"));
        }

        for (const PrivacyAsset& teardownAsset : std::as_const(teardownAssets))
        {
            if (teardownAsset.publicRootUuid != publicRoot.uuid)
            {
                return fail(
                    PrivacyStillItemTransactionStatus::RecoveryRequired,
                    journalAsset.itemUuid,
                    QStringLiteral("detached durable root is not exact"));
            }
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
        PrivacyAssetInventoryBridgeItemResult bridgeItem;
        bridgeItem.imageId = imageId;
        bridgeItem.inventory.status = PrivacyInventoryStatus::Ready;

        for (const PrivacyJournalAsset& recoveryAsset : record.assets)
        {
            PrivacyInventoryAsset inventoryAsset;
            inventoryAsset.role = static_cast<PrivacyInventoryAssetRole>(
                recoveryAsset.role);
            inventoryAsset.ordinal = recoveryAsset.ordinal;
            inventoryAsset.location.root.uuid = publicRoot.uuid;
            inventoryAsset.location.root.absolutePath = publicRoot.configuredPath;
            inventoryAsset.location.relativePath = recoveryAsset.publicRelativePath;
            inventoryAsset.evidence.type = PrivacyInventoryFileType::Regular;
            inventoryAsset.evidence.identityComplete = true;
            inventoryAsset.evidence.deviceId = record.rootDevice;
            inventoryAsset.evidence.inode =
                (static_cast<quint64>(
                     static_cast<quint32>(recoveryAsset.role)) << 32) |
                (static_cast<quint64>(
                     static_cast<quint32>(recoveryAsset.ordinal)) + 1);
            inventoryAsset.evidence.linkCount =
                recoveryAsset.original.linkCount;
            inventoryAsset.evidence.byteSize = recoveryAsset.original.size;

            if (created)
            {
#if defined(Q_OS_UNIX)
                const QString publicPath = absolutePath(
                    publicRoot, recoveryAsset.publicRelativePath);
                struct stat status = {};
                const QByteArray encodedPath = QFile::encodeName(publicPath);

                if (publicPath.isEmpty() ||
                    (::lstat(encodedPath.constData(), &status) != 0) ||
                    !S_ISREG(status.st_mode) || S_ISLNK(status.st_mode) ||
                    (status.st_nlink < 1))
                {
                    return fail(
                        PrivacyStillItemTransactionStatus::SourceChanged,
                        journalAsset.itemUuid,
                        QStringLiteral(
                            "durable Protect asset is not regular"));
                }

                inventoryAsset.evidence.deviceId =
                    static_cast<quint64>(status.st_dev);
                inventoryAsset.evidence.inode =
                    static_cast<quint64>(status.st_ino);
                inventoryAsset.evidence.linkCount =
                    static_cast<quint64>(status.st_nlink);
                inventoryAsset.evidence.byteSize =
                    static_cast<qlonglong>(status.st_size);
#else
                return fail(PrivacyStillItemTransactionStatus::RootUnavailable,
                            journalAsset.itemUuid,
                            QStringLiteral("durable recovery is unsupported"));
#endif
            }

            bridgeItem.inventory.requiredAssets << inventoryAsset;
        }

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
