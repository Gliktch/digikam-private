/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycasualarchive.h"

// C++ includes

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

// Qt includes

#include <QCryptographicHash>
#include <QByteArrayView>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QIODevice>
#include <QSet>
#include <QStringList>
#include <QTimeZone>
#include <QUuid>

// libzip includes

#include <zip.h>

#ifdef Q_OS_UNIX
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

#ifdef Q_OS_LINUX
#   include <linux/fs.h>
#   include <sys/syscall.h>
#endif

namespace Digikam
{

namespace
{

constexpr int MaximumMemberCount               = 256;
constexpr qsizetype MaximumMemberNameBytes     = 1024;
constexpr qsizetype MaximumOriginalNameBytes   = 255;
constexpr qsizetype MaximumPortableAttributes  = 64 * 1024;
constexpr qsizetype MaximumManifestBytes        = 1024 * 1024;
constexpr qsizetype IoChunkBytes                = 1024 * 1024;
constexpr qlonglong FixedManifestMtime          = 315532800; // 1980-01-01 UTC
constexpr quint32 ManifestUnixMode              = 0100600;

const QString ArchiveSuffix = QStringLiteral(".digikam-private.zip");
const QString ArchiveCommentPrefix =
    QStringLiteral("digiKam Private casual-v1; password=utf8-nfc-v1");
const QString ArchiveCommentRecoveryMarker = QStringLiteral("; recovery=");
const QString ManifestName = QStringLiteral("digikam-private/recovery-v1.json");

bool isCanonicalUuid(const QString& value);

bool encodeArchiveComment(const QString& recoverySetUuid, QByteArray* comment)
{
    if (!isCanonicalUuid(recoverySetUuid) || !comment)
    {
        return false;
    }

    *comment = (ArchiveCommentPrefix + ArchiveCommentRecoveryMarker +
                recoverySetUuid).toUtf8();
    return (comment->size() <= 65535);
}

bool decodeArchiveComment(const QByteArray& comment,
                          QString* recoverySetUuid)
{
    if (!comment.startsWith(ArchiveCommentPrefix.toUtf8()))
    {
        return false;
    }

    const QString text = QString::fromUtf8(comment);
    const int markerOffset = ArchiveCommentPrefix.size();

    if (!text.mid(markerOffset).startsWith(ArchiveCommentRecoveryMarker))
    {
        return false;
    }

    const QString identity = text.mid(markerOffset +
                                      ArchiveCommentRecoveryMarker.size());

    if (!isCanonicalUuid(identity))
    {
        return false;
    }

    if (recoverySetUuid)
    {
        *recoverySetUuid = identity;
    }

    return true;
}

struct RewriteSource
{
    zip_file_t*  file = nullptr;
    zip_uint64_t size = 0;
    zip_uint64_t offset = 0;
    bool         failed = false;
};

zip_int64_t rewriteSourceCallback(void* state, void* data, zip_uint64_t len,
                                  zip_source_cmd_t cmd)
{
    RewriteSource* const source = static_cast<RewriteSource*>(state);

    switch (cmd)
    {
        case ZIP_SOURCE_OPEN:
        {
            return 0;
        }

        case ZIP_SOURCE_READ:
        {
            if (source->failed || (len == 0))
            {
                return 0;
            }

            const zip_int64_t count = zip_fread(source->file, data, len);

            if (count < 0)
            {
                source->failed = true;
                return -1;
            }

            source->offset += static_cast<zip_uint64_t>(count);
            return count;
        }

        case ZIP_SOURCE_CLOSE:
        {
            return 0;
        }

        case ZIP_SOURCE_STAT:
        {
            zip_stat_t* const stat = static_cast<zip_stat_t*>(data);
            zip_stat_init(stat);
            stat->valid = ZIP_STAT_SIZE;
            stat->size  = source->size;
            return 0;
        }

        case ZIP_SOURCE_TELL:
        {
            return static_cast<zip_int64_t>(source->offset);
        }

        case ZIP_SOURCE_SEEK:
        {
            const zip_source_args_seek* const args =
                static_cast<const zip_source_args_seek*>(data);
            zip_uint64_t target = 0;

            switch (args->whence)
            {
                case SEEK_SET:
                {
                    target = static_cast<zip_uint64_t>(args->offset);
                    break;
                }

                case SEEK_CUR:
                {
                    if (args->offset < 0)
                    {
                        return -1;
                    }

                    target = source->offset +
                             static_cast<zip_uint64_t>(args->offset);
                    break;
                }

                case SEEK_END:
                {
                    if (args->offset < 0)
                    {
                        return -1;
                    }

                    target = source->size +
                             static_cast<zip_uint64_t>(args->offset);
                    break;
                }

                default:
                {
                    return -1;
                }
            }

            if (target < source->offset)
            {
                return -1;
            }

            QByteArray discardBuffer(64 * 1024, Qt::Uninitialized);
            zip_uint64_t remaining = target - source->offset;

            while (remaining > 0)
            {
                const zip_uint64_t wanted = static_cast<zip_uint64_t>(
                    qMin<qsizetype>(discardBuffer.size(),
                                    static_cast<qsizetype>(remaining)));
                const zip_int64_t count =
                    zip_fread(source->file, discardBuffer.data(), wanted);

                if (count <= 0)
                {
                    source->failed = true;
                    return -1;
                }

                source->offset += static_cast<zip_uint64_t>(count);
                remaining -= static_cast<zip_uint64_t>(count);
            }

            return 0;
        }

        case ZIP_SOURCE_SUPPORTS:
        {
            return ZIP_SOURCE_SUPPORTS_SEEKABLE;
        }

        case ZIP_SOURCE_ERROR:
        case ZIP_SOURCE_FREE:
        {
            return 0;
        }

        default:
        {
            return -1;
        }
    }
}

struct PreparedMember
{
    PrivacyCasualArchiveMember input;
    QByteArray                 archiveName;
    QByteArray                 sha256;
    qlonglong                  size = -1;
    quint32                    unixMode = 0;
    quint64                    device = 0;
    quint64                    inode = 0;
    qlonglong                  sourceMtimeSeconds = -1;
    qlonglong                  sourceMtimeNanoseconds = -1;
};

struct ExpectedMember
{
    QString    archiveName;
    QString    originalName;
    int        role = 0;
    int        ordinal = -1;
    qlonglong  size = -1;
    QByteArray sha256;
    quint32    unixMode = 0;
};

void setError(PrivacyCasualArchiveError* const error,
              PrivacyCasualArchiveError value)
{
    if (error)
    {
        *error = value;
    }
}

bool cancelled(const PrivacyCasualArchiveEngine::CancellationCheck& check)
{
    return (check && check());
}

bool isCanonicalUuid(const QString& value)
{
    const QUuid uuid(value);

    return (!uuid.isNull() &&
            (value == uuid.toString(QUuid::WithoutBraces)));
}

bool isSafeOriginalName(const QString& name)
{
    if (name.isEmpty() || (name == QLatin1String(".")) ||
        (name == QLatin1String("..")) ||
        (name != name.normalized(QString::NormalizationForm_C)) ||
        name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\')) ||
        name.contains(QChar::Null))
    {
        return false;
    }

    for (const QChar character : name)
    {
        if (character.category() == QChar::Other_Control)
        {
            return false;
        }
    }

    const QByteArray utf8 = name.toUtf8();

    return (!utf8.isEmpty() && (utf8.size() <= MaximumOriginalNameBytes) &&
            (QString::fromUtf8(utf8) == name));
}

bool isSafeArchiveMemberPath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) ||
        path.contains(QChar::Null) || path.contains(QLatin1Char('\\')) ||
        (path != path.normalized(QString::NormalizationForm_C)))
    {
        return false;
    }

    const QByteArray utf8 = path.toUtf8();

    if (utf8.isEmpty() || (utf8.size() > MaximumMemberNameBytes))
    {
        return false;
    }

    const QStringList parts = path.split(QLatin1Char('/'));

    for (const QString& part : parts)
    {
        if (part.isEmpty() || (part == QLatin1String(".")) ||
            (part == QLatin1String("..")))
        {
            return false;
        }

        for (const QChar character : part)
        {
            if (character.category() == QChar::Other_Control)
            {
                return false;
            }
        }
    }

    return true;
}

bool isSafeAbsoluteFilePath(const QString& path, bool mustExist)
{
    if (path.isEmpty() || !QDir::isAbsolutePath(path) ||
        (QDir::cleanPath(path) != path) || path.contains(QChar::Null))
    {
        return false;
    }

    const QFileInfo parentInfo(QFileInfo(path).absolutePath());

    if (!parentInfo.isDir() || parentInfo.isSymLink() ||
        (parentInfo.canonicalFilePath() != parentInfo.absoluteFilePath()))
    {
        return false;
    }

    const QFileInfo info(path);

    if (!mustExist)
    {
        return (!info.exists() && !info.isSymLink());
    }

    return (info.isFile() && !info.isSymLink() &&
            (info.canonicalFilePath() == info.absoluteFilePath()));
}

bool safeDestination(const QString& path)
{
    if (!path.endsWith(ArchiveSuffix))
    {
        return false;
    }

    return QFileInfo::exists(path) ? isSafeAbsoluteFilePath(path, true)
                                   : isSafeAbsoluteFilePath(path, false);
}

bool safeStagingPath(const QString& stagingPath, const QString& finalPath,
                     bool mustExist)
{
    const QFileInfo stagingInfo(stagingPath);
    const QFileInfo finalInfo(finalPath);

    return (stagingInfo.absolutePath() == finalInfo.absolutePath() &&
            stagingInfo.fileName().startsWith(
                QLatin1String(".digikam-private-stage-")) &&
            stagingInfo.fileName().endsWith(QLatin1String(".zip")) &&
            isSafeAbsoluteFilePath(stagingPath, mustExist));
}

bool syncDirectory(const QString& path)
{
#ifdef Q_OS_UNIX
    const QByteArray encoded = QFile::encodeName(path);
    const int fd = ::open(encoded.constData(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (fd < 0)
    {
        return false;
    }

    const bool result = (::fsync(fd) == 0);
    ::close(fd);

    return result;
#else
    Q_UNUSED(path);

    return true;
#endif
}

bool syncFile(const QString& path)
{
#ifdef Q_OS_UNIX
    const QByteArray encoded = QFile::encodeName(path);
    const int fd = ::open(encoded.constData(), O_RDWR | O_CLOEXEC | O_NOFOLLOW);

    if (fd < 0)
    {
        return false;
    }

    const bool result = (::fsync(fd) == 0);
    ::close(fd);

    return result;
#else
    QFile file(path);

    return (file.open(QIODevice::ReadWrite) && file.flush());
#endif
}

bool hashFile(const QString& path, QByteArray* const digest,
              qlonglong* const size,
              const PrivacyCasualArchiveEngine::CancellationCheck& isCancelled)
{
    if (!digest || !size || !isSafeAbsoluteFilePath(path, true))
    {
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
    qlonglong total = 0;

#ifdef Q_OS_UNIX
    const QByteArray encoded = QFile::encodeName(path);
    const int fd = ::open(encoded.constData(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);

    if (fd < 0)
    {
        return false;
    }

    struct stat statBuffer = {};

    if ((::fstat(fd, &statBuffer) != 0) || !S_ISREG(statBuffer.st_mode))
    {
        ::close(fd);

        return false;
    }
#else
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }
#endif

    while (true)
    {
        if (cancelled(isCancelled))
        {
#ifdef Q_OS_UNIX
            ::close(fd);
#endif
            return false;
        }

#ifdef Q_OS_UNIX
        const qint64 count = ::read(fd, buffer.data(), static_cast<size_t>(buffer.size()));
#else
        const qint64 count = file.read(buffer.data(), buffer.size());
#endif

        if (count < 0)
        {
#ifdef Q_OS_UNIX
            ::close(fd);
#endif
            return false;
        }

        if (count == 0)
        {
            break;
        }

        hash.addData(QByteArrayView(buffer.constData(), count));
        total += count;
    }

#ifdef Q_OS_UNIX
    ::close(fd);
#endif

    *digest = hash.result();
    *size   = total;

    return true;
}

QJsonValue epochMilliseconds(const QDateTime& value)
{
    return value.isValid() ? QJsonValue(QString::number(value.toUTC().toMSecsSinceEpoch()))
                           : QJsonValue(QJsonValue::Null);
}

QByteArray createManifest(const PrivacyCasualArchiveRequest& request,
                          const QList<PreparedMember>& members)
{
    QJsonObject root;
    root.insert(QLatin1String("format"), QLatin1String("digikam-private-casual"));
    root.insert(QLatin1String("formatVersion"), 1);
    root.insert(QLatin1String("passwordEncoding"), QLatin1String("utf8-nfc-v1"));
    root.insert(QLatin1String("categoryUuid"), request.categoryUuid);
    root.insert(QLatin1String("containerUuid"), request.containerUuid);
    root.insert(QLatin1String("itemUuid"), request.itemUuid);

    QJsonArray memberArray;

    for (const PreparedMember& member : members)
    {
        QJsonObject portable;
        portable.insert(QLatin1String("encoding"), QLatin1String("base64"));
        portable.insert(QLatin1String("version"), 1);
        portable.insert(QLatin1String("data"),
                        QString::fromLatin1(member.input.portableAttributes.toBase64()));

        QJsonObject object;
        object.insert(QLatin1String("path"), member.input.protectedRelativePath);
        object.insert(QLatin1String("originalName"), member.input.originalName);
        object.insert(QLatin1String("role"), member.input.role);
        object.insert(QLatin1String("ordinal"), member.input.ordinal);
        object.insert(QLatin1String("size"), QString::number(member.size));
        object.insert(QLatin1String("hashAlgorithm"), QLatin1String("sha256"));
        object.insert(QLatin1String("hash"), QString::fromLatin1(member.sha256.toHex()));
        object.insert(QLatin1String("creationTimeUtcMs"),
                      epochMilliseconds(member.input.originalCreationDate));
        object.insert(QLatin1String("modificationTimeUtcMs"),
                      epochMilliseconds(member.input.originalModificationDate));
        object.insert(QLatin1String("portableAttributes"), portable);
        object.insert(QLatin1String("unixMode"), QString::number(member.unixMode));
        memberArray.append(object);
    }

    root.insert(QLatin1String("members"), memberArray);

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

bool decimalString(const QJsonValue& value, qlonglong* const result,
                   bool nullable = false)
{
    if (nullable && value.isNull())
    {
        return true;
    }

    if (!value.isString() || !result)
    {
        return false;
    }

    bool okay = false;
    const qlonglong parsed = value.toString().toLongLong(&okay, 10);

    if (!okay || (QString::number(parsed) != value.toString()))
    {
        return false;
    }

    *result = parsed;

    return true;
}

bool jsonInteger(const QJsonValue& value, int* const result)
{
    if (!result || !value.isDouble())
    {
        return false;
    }

    const double number = value.toDouble();

    if (!std::isfinite(number) ||
        (number < static_cast<double>(std::numeric_limits<int>::min())) ||
        (number > static_cast<double>(std::numeric_limits<int>::max())))
    {
        return false;
    }

    const int integer = static_cast<int>(number);

    if (number != static_cast<double>(integer))
    {
        return false;
    }

    *result = integer;

    return true;
}

bool parseManifest(const QByteArray& bytes,
                   QList<ExpectedMember>* const members,
                   QJsonObject* const semanticObject)
{
    if (!members || bytes.isEmpty() || (bytes.size() > MaximumManifestBytes))
    {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);

    if ((parseError.error != QJsonParseError::NoError) || !document.isObject())
    {
        return false;
    }

    const QJsonObject root = document.object();
    int formatVersion = 0;

    if ((root.value(QLatin1String("format")).toString() !=
         QLatin1String("digikam-private-casual")) ||
        !jsonInteger(root.value(QLatin1String("formatVersion")),
                     &formatVersion) || (formatVersion != 1) ||
        (root.value(QLatin1String("passwordEncoding")).toString() !=
         QLatin1String("utf8-nfc-v1")) ||
        !isCanonicalUuid(root.value(QLatin1String("categoryUuid")).toString()) ||
        !isCanonicalUuid(root.value(QLatin1String("containerUuid")).toString()) ||
        !isCanonicalUuid(root.value(QLatin1String("itemUuid")).toString()) ||
        !root.value(QLatin1String("members")).isArray())
    {
        return false;
    }

    const QJsonArray array = root.value(QLatin1String("members")).toArray();

    if (array.isEmpty() || (array.size() > MaximumMemberCount))
    {
        return false;
    }

    QSet<QString> collisionKeys;
    QList<ExpectedMember> parsedMembers;

    for (const QJsonValue& value : array)
    {
        if (!value.isObject())
        {
            return false;
        }

        const QJsonObject object = value.toObject();
        ExpectedMember member;
        member.archiveName = object.value(QLatin1String("path")).toString();
        member.originalName = object.value(QLatin1String("originalName")).toString();
        qlonglong size      = -1;
        qlonglong mode      = -1;

        if (!jsonInteger(object.value(QLatin1String("role")), &member.role) ||
            !jsonInteger(object.value(QLatin1String("ordinal")), &member.ordinal) ||
            !isSafeOriginalName(member.originalName) || (member.role <= 0) ||
            (member.ordinal < 0) ||
            (member.archiveName != PrivacyCasualArchiveEngine::expectedMemberPath(
                 member.role, member.ordinal, member.originalName)) ||
            !decimalString(object.value(QLatin1String("size")), &size) ||
            (size < 0) ||
            !decimalString(object.value(QLatin1String("unixMode")), &mode) ||
            (mode < 0) || (mode > 0xffff) ||
            ((mode & 0170000) != 0100000) ||
            (object.value(QLatin1String("hashAlgorithm")).toString() !=
             QLatin1String("sha256")))
        {
            return false;
        }

        const QByteArray hash = object.value(QLatin1String("hash")).toString().toLatin1();

        if ((hash.size() != 64) ||
            std::any_of(hash.cbegin(), hash.cend(), [](char character)
            {
                return !(((character >= '0') && (character <= '9')) ||
                         ((character >= 'a') && (character <= 'f')));
            }))
        {
            return false;
        }

        qlonglong ignored = 0;

        if (!decimalString(object.value(QLatin1String("creationTimeUtcMs")),
                           &ignored, true) ||
            !decimalString(object.value(QLatin1String("modificationTimeUtcMs")),
                           &ignored, true))
        {
            return false;
        }

        const QJsonObject portable =
            object.value(QLatin1String("portableAttributes")).toObject();
        int portableVersion = 0;
        const QByteArray encodedAttributes =
            portable.value(QLatin1String("data")).toString().toLatin1();
        const QByteArray decodedAttributes =
            QByteArray::fromBase64(encodedAttributes, QByteArray::AbortOnBase64DecodingErrors);

        if ((portable.value(QLatin1String("encoding")).toString() !=
             QLatin1String("base64")) ||
            !jsonInteger(portable.value(QLatin1String("version")),
                         &portableVersion) || (portableVersion != 1) ||
            (decodedAttributes.size() > MaximumPortableAttributes) ||
            (decodedAttributes.toBase64() != encodedAttributes))
        {
            return false;
        }

        const QString collisionKey = member.archiveName.toCaseFolded();

        if (collisionKeys.contains(collisionKey))
        {
            return false;
        }

        collisionKeys.insert(collisionKey);
        member.size     = size;
        member.sha256   = QByteArray::fromHex(hash);
        member.unixMode = static_cast<quint32>(mode);
        parsedMembers << member;
    }

    *members = parsedMembers;

    if (semanticObject)
    {
        *semanticObject = root;
    }

    return true;
}

bool parsePortableManifest(const QByteArray& bytes,
                           PrivacyCasualArchiveManifest* const manifest)
{
    if (!manifest)
    {
        return false;
    }

    QList<ExpectedMember> validated;
    QJsonObject semantic;

    if (!parseManifest(bytes, &validated, &semantic) || validated.isEmpty())
    {
        return false;
    }

    PrivacyCasualArchiveManifest decoded;
    decoded.format = semantic.value(QLatin1String("format")).toString();
    decoded.formatVersion =
        semantic.value(QLatin1String("formatVersion")).toInt();
    decoded.passwordEncoding =
        semantic.value(QLatin1String("passwordEncoding")).toString();
    decoded.categoryUuid =
        semantic.value(QLatin1String("categoryUuid")).toString();
    decoded.containerUuid =
        semantic.value(QLatin1String("containerUuid")).toString();
    decoded.itemUuid = semantic.value(QLatin1String("itemUuid")).toString();

    const QJsonArray array = semantic.value(QLatin1String("members")).toArray();

    for (const QJsonValue& value : array)
    {
        if (!value.isObject())
        {
            return false;
        }

        const QJsonObject object = value.toObject();
        PrivacyCasualArchiveManifestMember member;
        member.protectedRelativePath =
            object.value(QLatin1String("path")).toString();
        member.originalName =
            object.value(QLatin1String("originalName")).toString();
        member.role = object.value(QLatin1String("role")).toInt();
        member.ordinal = object.value(QLatin1String("ordinal")).toInt();
        member.hashAlgorithm =
            object.value(QLatin1String("hashAlgorithm")).toString();
        member.sha256 = QByteArray::fromHex(
            object.value(QLatin1String("hash")).toString().toLatin1());

        qlonglong size = -1;

        if (!decimalString(object.value(QLatin1String("size")), &size))
        {
            return false;
        }

        member.size = size;
        qlonglong mode = -1;

        if (!decimalString(object.value(QLatin1String("unixMode")), &mode))
        {
            return false;
        }

        member.unixMode = static_cast<quint32>(mode);
        qlonglong creation = 0;
        qlonglong modification = 0;

        if (decimalString(object.value(QLatin1String("creationTimeUtcMs")),
                          &creation, true))
        {
            if (!object.value(QLatin1String("creationTimeUtcMs")).isNull())
            {
                member.creationTimeUtc =
                    QDateTime::fromMSecsSinceEpoch(creation, QTimeZone::UTC);
            }
        }

        if (decimalString(object.value(QLatin1String("modificationTimeUtcMs")),
                          &modification, true))
        {
            if (!object.value(QLatin1String("modificationTimeUtcMs")).isNull())
            {
                member.modificationTimeUtc =
                    QDateTime::fromMSecsSinceEpoch(modification, QTimeZone::UTC);
            }
        }

        member.portableAttributes = QByteArray::fromBase64(
            object.value(QLatin1String("portableAttributes"))
                .toObject().value(QLatin1String("data")).toString().toLatin1(),
            QByteArray::AbortOnBase64DecodingErrors);

        if (!member.isValid() ||
            (member.protectedRelativePath !=
             PrivacyCasualArchiveEngine::expectedMemberPath(
                 member.role, member.ordinal, member.originalName)))
        {
            return false;
        }

        decoded.members << member;
    }

    if (decoded.members.size() != validated.size())
    {
        return false;
    }

    *manifest = decoded;
    return true;
}

bool prepareMember(const PrivacyCasualArchiveMember& input,
                   PreparedMember* const prepared,
                   const PrivacyCasualArchiveEngine::CancellationCheck& isCancelled,
                   PrivacyCasualArchiveError* const error)
{
    if (!prepared || !isSafeOriginalName(input.originalName) ||
        (input.role <= 0) || (input.ordinal < 0) ||
        (input.protectedRelativePath != PrivacyCasualArchiveEngine::expectedMemberPath(
             input.role, input.ordinal, input.originalName)))
    {
        setError(error, PrivacyCasualArchiveError::InvalidMemberName);

        return false;
    }

    if ((input.portableAttributes.size() > MaximumPortableAttributes) ||
        !isSafeAbsoluteFilePath(input.sourcePath, true))
    {
        setError(error, PrivacyCasualArchiveError::UnsafeSource);

        return false;
    }

#ifdef Q_OS_UNIX
    const QByteArray encodedPath = QFile::encodeName(input.sourcePath);
    const int fd = ::open(encodedPath.constData(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);

    if (fd < 0)
    {
        setError(error, PrivacyCasualArchiveError::UnsafeSource);

        return false;
    }

    struct stat initialStat = {};

    if ((::fstat(fd, &initialStat) != 0) || !S_ISREG(initialStat.st_mode))
    {
        ::close(fd);
        setError(error, PrivacyCasualArchiveError::UnsafeSource);

        return false;
    }

    if (((input.expectedDevice != 0) &&
         (static_cast<quint64>(initialStat.st_dev) != input.expectedDevice)) ||
        ((input.expectedInode != 0) &&
         (static_cast<quint64>(initialStat.st_ino) != input.expectedInode)) ||
        ((input.expectedLinkCount != 0) &&
         (static_cast<quint64>(initialStat.st_nlink) != input.expectedLinkCount)) ||
        ((input.expectedSize >= 0) &&
         (static_cast<qlonglong>(initialStat.st_size) != input.expectedSize)))
    {
        ::close(fd);
        setError(error, PrivacyCasualArchiveError::SourceChanged);
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
    qlonglong total = 0;

    while (true)
    {
        if (cancelled(isCancelled))
        {
            ::close(fd);
            setError(error, PrivacyCasualArchiveError::Cancelled);

            return false;
        }

        const ssize_t count = ::read(fd, buffer.data(), static_cast<size_t>(buffer.size()));

        if (count < 0)
        {
            ::close(fd);
            setError(error, PrivacyCasualArchiveError::SourceReadFailed);

            return false;
        }

        if (count == 0)
        {
            break;
        }

        hash.addData(QByteArrayView(buffer.constData(),
                                    static_cast<qsizetype>(count)));
        total += count;
    }

    struct stat finalStat = {};

    if ((::fstat(fd, &finalStat) != 0) || (total != initialStat.st_size) ||
        (initialStat.st_dev != finalStat.st_dev) ||
        (initialStat.st_ino != finalStat.st_ino) ||
        (initialStat.st_size != finalStat.st_size) ||
        (initialStat.st_mtime != finalStat.st_mtime))
    {
        ::close(fd);
        setError(error, PrivacyCasualArchiveError::SourceChanged);

        return false;
    }

    ::close(fd);

    if (!input.expectedSha256.isEmpty() &&
        (hash.result() != input.expectedSha256))
    {
        setError(error, PrivacyCasualArchiveError::SourceChanged);
        return false;
    }

    prepared->device             = static_cast<quint64>(initialStat.st_dev);
    prepared->inode              = static_cast<quint64>(initialStat.st_ino);
    prepared->sourceMtimeSeconds = static_cast<qlonglong>(initialStat.st_mtime);
#ifdef Q_OS_LINUX
    prepared->sourceMtimeNanoseconds = static_cast<qlonglong>(initialStat.st_mtim.tv_nsec);
#endif
    prepared->unixMode           = static_cast<quint32>(initialStat.st_mode & 0xffffU);
    prepared->size               = total;
    prepared->sha256             = hash.result();
#else
    if (!hashFile(input.sourcePath, &prepared->sha256, &prepared->size, isCancelled))
    {
        setError(error, cancelled(isCancelled) ? PrivacyCasualArchiveError::Cancelled
                                               : PrivacyCasualArchiveError::SourceReadFailed);

        return false;
    }

    prepared->unixMode = ManifestUnixMode;
#endif

    prepared->input       = input;

    if (!prepared->input.originalModificationDate.isValid())
    {
        prepared->input.originalModificationDate =
            QDateTime::fromSecsSinceEpoch(prepared->sourceMtimeSeconds,
                                         QTimeZone::UTC);
    }

    prepared->archiveName = input.protectedRelativePath.toUtf8();

    return true;
}

zip_source_t* sourceForPreparedMember(zip_t* const archive,
                                      const PreparedMember& member,
                                      PrivacyCasualArchiveError* const error)
{
#ifdef Q_OS_UNIX
    const QByteArray encodedPath = QFile::encodeName(member.input.sourcePath);
    const int fd = ::open(encodedPath.constData(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);

    if (fd < 0)
    {
        setError(error, PrivacyCasualArchiveError::UnsafeSource);

        return nullptr;
    }

    struct stat statBuffer = {};

    if ((::fstat(fd, &statBuffer) != 0) || !S_ISREG(statBuffer.st_mode) ||
        (static_cast<quint64>(statBuffer.st_dev) != member.device) ||
        (static_cast<quint64>(statBuffer.st_ino) != member.inode) ||
        (statBuffer.st_size != member.size) ||
        (statBuffer.st_mtime != member.sourceMtimeSeconds) ||
        ((member.input.expectedLinkCount != 0) &&
         (static_cast<quint64>(statBuffer.st_nlink) !=
          member.input.expectedLinkCount)) ||
        (static_cast<quint32>(statBuffer.st_mode & 0xffffU) != member.unixMode))
    {
        ::close(fd);
        setError(error, PrivacyCasualArchiveError::SourceChanged);

        return nullptr;
    }

#ifdef Q_OS_LINUX
    if (statBuffer.st_mtim.tv_nsec != member.sourceMtimeNanoseconds)
    {
        ::close(fd);
        setError(error, PrivacyCasualArchiveError::SourceChanged);

        return nullptr;
    }
#endif

    FILE* const file = ::fdopen(fd, "rb");

    if (!file)
    {
        ::close(fd);
        setError(error, PrivacyCasualArchiveError::SourceReadFailed);

        return nullptr;
    }

    zip_source_t* const source = zip_source_filep(archive, file, 0, -1);

    if (!source)
    {
        ::fclose(file);
        setError(error, PrivacyCasualArchiveError::ArchiveWriteFailed);
    }

    return source;
#else
    const QByteArray encodedPath = QFile::encodeName(member.input.sourcePath);
    zip_source_t* const source = zip_source_file(archive, encodedPath.constData(), 0, -1);

    if (!source)
    {
        setError(error, PrivacyCasualArchiveError::ArchiveWriteFailed);
    }

    return source;
#endif
}

int cancelZip(zip_t*, void* state)
{
    const auto* const check =
        static_cast<const PrivacyCasualArchiveEngine::CancellationCheck*>(state);

    return (check && *check && (*check)()) ? 1 : 0;
}

bool configureEntry(zip_t* const archive, zip_uint64_t index,
                    time_t mtime, quint32 unixMode,
                    const PrivacyPassword& password)
{
    if ((zip_set_file_compression(archive, index, ZIP_CM_STORE, 0) != 0) ||
        (zip_file_set_mtime(archive, index, mtime, 0) != 0) ||
        (zip_file_set_external_attributes(
             archive, index, 0, ZIP_OPSYS_UNIX,
             static_cast<zip_uint32_t>(unixMode << 16)) != 0))
    {
        return false;
    }

    return password.withUtf8CString([archive, index](const char* value)
    {
        return (zip_file_set_encryption(archive, index,
                                        ZIP_EM_TRAD_PKWARE, value) == 0);
    });
}

bool readEncryptedEntry(zip_t* const archive, zip_uint64_t index,
                        const PrivacyPassword& password, qlonglong maximumBytes,
                        const PrivacyCasualArchiveEngine::CancellationCheck& isCancelled,
                        QByteArray* const output, QByteArray* const digest,
                        qlonglong* const size,
                        PrivacyCasualArchiveError* const error)
{
    bool result = false;

    const bool invoked = password.withUtf8CString(
        [&](const char* passwordBytes)
        {
            zip_file_t* const file =
                zip_fopen_index_encrypted(archive, index, 0, passwordBytes);

            if (!file)
            {
                setError(error, PrivacyCasualArchiveError::DecryptionFailed);

                return false;
            }

            QCryptographicHash hash(QCryptographicHash::Sha256);
            QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
            QByteArray collected;
            qlonglong total = 0;

            while (true)
            {
                if (cancelled(isCancelled))
                {
                    setError(error, PrivacyCasualArchiveError::Cancelled);
                    break;
                }

                const zip_int64_t count =
                    zip_fread(file, buffer.data(), static_cast<zip_uint64_t>(buffer.size()));

                if (count < 0)
                {
                    setError(error, PrivacyCasualArchiveError::DecryptionFailed);
                    break;
                }

                if (count == 0)
                {
                    result = true;
                    break;
                }

                if ((total > maximumBytes) || (count > (maximumBytes - total)))
                {
                    setError(error, PrivacyCasualArchiveError::SizeMismatch);
                    break;
                }

                hash.addData(QByteArrayView(buffer.constData(),
                                            static_cast<qsizetype>(count)));

                if (output)
                {
                    collected.append(buffer.constData(), static_cast<qsizetype>(count));
                }

                total += count;
            }

            if (zip_fclose(file) != 0)
            {
                result = false;
                setError(error, PrivacyCasualArchiveError::DecryptionFailed);
            }

            if (result)
            {
                if (output)
                {
                    *output = std::move(collected);
                }

                if (digest)
                {
                    *digest = hash.result();
                }

                if (size)
                {
                    *size = total;
                }
            }

            return result;
        });

    if (!invoked && (error && (*error == PrivacyCasualArchiveError::None)))
    {
        setError(error, PrivacyCasualArchiveError::InvalidPassword);
    }

    return (invoked && result);
}

bool streamEncryptedEntry(zip_t* const archive, zip_uint64_t index,
                          const PrivacyPassword& password,
                          const ExpectedMember& expected,
                          const PrivacyCasualArchiveEngine::CancellationCheck& isCancelled,
                          QIODevice* const destination,
                          PrivacyCasualArchiveError* const error)
{
    if (!destination || !destination->isOpen() || !destination->isWritable())
    {
        setError(error, PrivacyCasualArchiveError::DestinationWriteFailed);
        return false;
    }

    bool result = false;
    const bool invoked = password.withUtf8CString(
        [&](const char* passwordBytes)
        {
            zip_file_t* const file =
                zip_fopen_index_encrypted(archive, index, 0, passwordBytes);

            if (!file)
            {
                setError(error, PrivacyCasualArchiveError::DecryptionFailed);
                return false;
            }

            QCryptographicHash hash(QCryptographicHash::Sha256);
            QByteArray buffer(IoChunkBytes, Qt::Uninitialized);
            qlonglong total = 0;

            while (total < expected.size)
            {
                if (cancelled(isCancelled))
                {
                    setError(error, PrivacyCasualArchiveError::Cancelled);
                    break;
                }

                const zip_uint64_t wanted = static_cast<zip_uint64_t>(
                    std::min<qlonglong>(buffer.size(), expected.size - total));
                const zip_int64_t count = zip_fread(file, buffer.data(), wanted);

                if (count <= 0)
                {
                    setError(error, (count < 0)
                                    ? PrivacyCasualArchiveError::DecryptionFailed
                                    : PrivacyCasualArchiveError::SizeMismatch);
                    break;
                }

                qint64 written = 0;

                while (written < count)
                {
                    const qint64 amount = destination->write(
                        buffer.constData() + written, count - written);

                    if (amount <= 0)
                    {
                        setError(error, PrivacyCasualArchiveError::DestinationWriteFailed);
                        break;
                    }

                    written += amount;
                }

                if (written != count)
                {
                    break;
                }

                hash.addData(QByteArrayView(buffer.constData(),
                                            static_cast<qsizetype>(count)));
                total += count;
            }

            if ((total == expected.size) && !cancelled(isCancelled))
            {
                char extra = 0;
                const zip_int64_t extraCount = zip_fread(file, &extra, 1);
                result = ((extraCount == 0) && (hash.result() == expected.sha256));

                if (!result)
                {
                    setError(error, (extraCount == 0)
                                    ? PrivacyCasualArchiveError::HashMismatch
                                    : PrivacyCasualArchiveError::SizeMismatch);
                }
            }

            if (zip_fclose(file) != 0)
            {
                result = false;
                setError(error, PrivacyCasualArchiveError::DecryptionFailed);
            }

            return result;
        });

    if (!invoked && error && (*error == PrivacyCasualArchiveError::None))
    {
        setError(error, PrivacyCasualArchiveError::InvalidPassword);
    }

    return (invoked && result);
}

bool readManifestForResume(
    const QString& archivePath, const PrivacyPassword& password,
    const PrivacyCasualArchiveEngine::CancellationCheck& isCancelled,
    QByteArray* const manifest, PrivacyCasualArchiveError* const error)
{
    if (!manifest || !isSafeAbsoluteFilePath(archivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);

        return false;
    }

    int zipError = 0;
    const QByteArray encodedPath = QFile::encodeName(archivePath);
    zip_t* const archive = zip_open(encodedPath.constData(),
                                    ZIP_RDONLY | ZIP_CHECKCONS, &zipError);

    if (!archive)
    {
        setError(error, PrivacyCasualArchiveError::ArchiveOpenFailed);

        return false;
    }

    const QByteArray encodedManifestName = ManifestName.toUtf8();
    const zip_int64_t index = zip_name_locate(archive,
                                               encodedManifestName.constData(),
                                               ZIP_FL_ENC_UTF_8);
    bool result = false;

    if (index < 0)
    {
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);
    }
    else
    {
        result = readEncryptedEntry(archive,
                                    static_cast<zip_uint64_t>(index), password,
                                    MaximumManifestBytes, isCancelled, manifest,
                                    nullptr, nullptr, error);
    }

    zip_discard(archive);

    return result;
}

bool verifyArchive(const QString& archivePath, const QByteArray& expectedManifest,
                   const PrivacyPassword& password,
                   const PrivacyCasualArchiveEngine::CancellationCheck& isCancelled,
                   const QString& expectedRecoverySetUuid,
                   PrivacyCasualArchiveError* const error)
{
    QList<ExpectedMember> expectedMembers;
    QJsonObject expectedSemantic;

    if (!parseManifest(expectedManifest, &expectedMembers, &expectedSemantic) ||
        !isSafeAbsoluteFilePath(archivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);

        return false;
    }

    int zipError = 0;
    const QByteArray encodedPath = QFile::encodeName(archivePath);
    zip_t* archive = zip_open(encodedPath.constData(), ZIP_RDONLY | ZIP_CHECKCONS, &zipError);

    if (!archive)
    {
        setError(error, PrivacyCasualArchiveError::ArchiveOpenFailed);

        return false;
    }

    const auto discard = [&archive]()
    {
        if (archive)
        {
            zip_discard(archive);
            archive = nullptr;
        }
    };

    int commentLength = 0;
    const char* const comment = zip_get_archive_comment(archive, &commentLength, 0);
    QString commentRecoverySetUuid;

    if (!comment ||
        !decodeArchiveComment(QByteArray(comment, commentLength),
                              &commentRecoverySetUuid) ||
        (!expectedRecoverySetUuid.isEmpty() &&
         (commentRecoverySetUuid != expectedRecoverySetUuid)) ||
        (zip_get_num_entries(archive, 0) != (expectedMembers.size() + 1)))
    {
        discard();
        setError(error, PrivacyCasualArchiveError::ArchivePolicyViolation);

        return false;
    }

    QHash<QString, zip_uint64_t> indices;
    QSet<QString> collisionKeys;
    const zip_int64_t count = zip_get_num_entries(archive, 0);

    for (zip_uint64_t index = 0 ; index < static_cast<zip_uint64_t>(count) ; ++index)
    {
        zip_stat_t stat;
        zip_stat_init(&stat);
        const char* const rawName = zip_get_name(archive, index, ZIP_FL_ENC_STRICT);

        if (!rawName ||
            (zip_stat_index(archive, index, ZIP_FL_ENC_STRICT, &stat) != 0) ||
            !(stat.valid & ZIP_STAT_SIZE) || !(stat.valid & ZIP_STAT_COMP_METHOD) ||
            !(stat.valid & ZIP_STAT_ENCRYPTION_METHOD) ||
            (stat.comp_method != ZIP_CM_STORE) ||
            (stat.encryption_method != ZIP_EM_TRAD_PKWARE))
        {
            discard();
            setError(error, PrivacyCasualArchiveError::ArchivePolicyViolation);

            return false;
        }

        const QString name = QString::fromUtf8(rawName);
        const QString collisionKey = name.normalized(QString::NormalizationForm_C).toCaseFolded();

        if ((name.toUtf8() != QByteArray(rawName)) ||
            !isSafeArchiveMemberPath(name) || indices.contains(name) ||
            collisionKeys.contains(collisionKey))
        {
            discard();
            setError(error, PrivacyCasualArchiveError::DuplicateMember);

            return false;
        }

        indices.insert(name, index);
        collisionKeys.insert(collisionKey);
    }

    if (!indices.contains(ManifestName))
    {
        discard();
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);

        return false;
    }

    zip_uint8_t manifestHostSystem = 0;
    zip_uint32_t manifestAttributes = 0;

    if ((zip_file_get_external_attributes(archive, indices.value(ManifestName), 0,
                                          &manifestHostSystem,
                                          &manifestAttributes) != 0) ||
        (manifestHostSystem != ZIP_OPSYS_UNIX) ||
        ((manifestAttributes >> 16) != ManifestUnixMode))
    {
        discard();
        setError(error, PrivacyCasualArchiveError::ArchivePolicyViolation);

        return false;
    }

    QByteArray manifestBytes;

    if (!readEncryptedEntry(archive, indices.value(ManifestName), password,
                            MaximumManifestBytes, isCancelled, &manifestBytes,
                            nullptr, nullptr, error))
    {
        discard();

        return false;
    }

    QList<ExpectedMember> actualMembers;
    QJsonObject actualSemantic;

    if (!parseManifest(manifestBytes, &actualMembers, &actualSemantic) ||
        (actualSemantic != expectedSemantic))
    {
        discard();
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);

        return false;
    }

    for (const ExpectedMember& member : expectedMembers)
    {
        if (cancelled(isCancelled))
        {
            discard();
            setError(error, PrivacyCasualArchiveError::Cancelled);

            return false;
        }

        if (!indices.contains(member.archiveName))
        {
            discard();
            setError(error, PrivacyCasualArchiveError::ManifestInvalid);

            return false;
        }

        const zip_uint64_t index = indices.value(member.archiveName);
        zip_stat_t stat;
        zip_stat_init(&stat);
        zip_uint8_t hostSystem = 0;
        zip_uint32_t attributes = 0;

        if ((zip_stat_index(archive, index, 0, &stat) != 0) ||
            !(stat.valid & ZIP_STAT_SIZE) ||
            (stat.size != static_cast<zip_uint64_t>(member.size)) ||
            (zip_file_get_external_attributes(archive, index, 0,
                                              &hostSystem, &attributes) != 0) ||
            (hostSystem != ZIP_OPSYS_UNIX) ||
            ((attributes >> 16) != member.unixMode))
        {
            discard();
            setError(error, PrivacyCasualArchiveError::ArchivePolicyViolation);

            return false;
        }

        QByteArray digest;
        qlonglong readSize = -1;

        if (!readEncryptedEntry(archive, index, password, member.size,
                                isCancelled, nullptr, &digest, &readSize, error))
        {
            discard();

            return false;
        }

        if (readSize != member.size)
        {
            discard();
            setError(error, PrivacyCasualArchiveError::SizeMismatch);

            return false;
        }

        if (digest != member.sha256)
        {
            discard();
            setError(error, PrivacyCasualArchiveError::HashMismatch);

            return false;
        }
    }

    discard();

    return true;
}

bool removeKnownStage(const QString& stagePath, const QString& finalPath)
{
    if (!QFileInfo::exists(stagePath))
    {
        return true;
    }

    return (safeStagingPath(stagePath, finalPath, true) && QFile::remove(stagePath));
}

enum class PublishResult
{
    Success,
    SuccessWithStagingRemaining,
    Conflict,
    Failure
};

PublishResult publishWithoutReplacement(const QString& source, const QString& destination)
{
    const QByteArray encodedSource      = QFile::encodeName(source);
    const QByteArray encodedDestination = QFile::encodeName(destination);

#ifdef Q_OS_LINUX
    if (::syscall(SYS_renameat2, AT_FDCWD, encodedSource.constData(),
                  AT_FDCWD, encodedDestination.constData(), RENAME_NOREPLACE) == 0)
    {
        return PublishResult::Success;
    }

    if (errno == EEXIST)
    {
        return PublishResult::Conflict;
    }

    if ((errno != ENOSYS) && (errno != EINVAL))
    {
        return PublishResult::Failure;
    }
#endif

#ifdef Q_OS_UNIX
    if (::link(encodedSource.constData(), encodedDestination.constData()) != 0)
    {
        return (errno == EEXIST) ? PublishResult::Conflict : PublishResult::Failure;
    }

    if (::unlink(encodedSource.constData()) != 0)
    {
        return PublishResult::SuccessWithStagingRemaining;
    }

    return PublishResult::Success;
#else
    return QFile::rename(source, destination) ? PublishResult::Success
                                               : (QFileInfo::exists(destination)
                                                  ? PublishResult::Conflict
                                                  : PublishResult::Failure);
#endif
}

enum class ReplaceResult
{
    Success,
    ExistingMismatchRestored,
    ExchangeFailed,
    RollbackFailed,
    OldArchiveRemaining
};

ReplaceResult replaceWithExpectedArchive(
    const QString& source, const QString& destination,
    const QByteArray& expectedSourceHash, qlonglong expectedSourceSize,
    const QByteArray& expectedDestinationHash)
{
#ifdef Q_OS_LINUX
    const QByteArray encodedSource      = QFile::encodeName(source);
    const QByteArray encodedDestination = QFile::encodeName(destination);

    if (::syscall(SYS_renameat2, AT_FDCWD, encodedSource.constData(),
                  AT_FDCWD, encodedDestination.constData(), RENAME_EXCHANGE) != 0)
    {
        return ReplaceResult::ExchangeFailed;
    }

    QByteArray displacedHash;
    QByteArray installedHash;
    qlonglong displacedSize = -1;
    qlonglong installedSize = -1;
    const bool expectedObjects =
        hashFile(source, &displacedHash, &displacedSize, {}) &&
        hashFile(destination, &installedHash, &installedSize, {}) &&
        (displacedHash == expectedDestinationHash) &&
        (installedHash == expectedSourceHash) &&
        (installedSize == expectedSourceSize);

    if (!expectedObjects)
    {
        if (::syscall(SYS_renameat2, AT_FDCWD, encodedSource.constData(),
                      AT_FDCWD, encodedDestination.constData(),
                      RENAME_EXCHANGE) != 0)
        {
            return ReplaceResult::RollbackFailed;
        }

        return ReplaceResult::ExistingMismatchRestored;
    }

    return QFile::remove(source) ? ReplaceResult::Success
                                 : ReplaceResult::OldArchiveRemaining;
#else
    Q_UNUSED(source);
    Q_UNUSED(destination);
    Q_UNUSED(expectedSourceHash);
    Q_UNUSED(expectedSourceSize);
    Q_UNUSED(expectedDestinationHash);

    return ReplaceResult::ExchangeFailed;
#endif
}

} // namespace

PrivacyCasualArchiveStage::PrivacyCasualArchiveStage(
    PrivacyCasualArchiveStage&& other) noexcept
    : m_stagingPath(std::move(other.m_stagingPath)),
      m_finalArchivePath(std::move(other.m_finalArchivePath)),
      m_archiveSize(other.m_archiveSize),
      m_archiveSha256(std::move(other.m_archiveSha256)),
      m_expectedManifest(std::move(other.m_expectedManifest)),
      m_recoverySetUuid(std::move(other.m_recoverySetUuid))
{
    other.clear();
}

bool PrivacyCasualArchiveStage::isValid() const
{
    return (!m_stagingPath.isEmpty() && !m_finalArchivePath.isEmpty() &&
            (m_archiveSize >= 0) && (m_archiveSha256.size() == 32) &&
            !m_expectedManifest.isEmpty() && !m_recoverySetUuid.isEmpty());
}

QString PrivacyCasualArchiveStage::stagingPath() const
{
    return m_stagingPath;
}

QString PrivacyCasualArchiveStage::finalArchivePath() const
{
    return m_finalArchivePath;
}

qlonglong PrivacyCasualArchiveStage::archiveSize() const
{
    return m_archiveSize;
}

QByteArray PrivacyCasualArchiveStage::archiveSha256() const
{
    return m_archiveSha256;
}

void PrivacyCasualArchiveStage::clear()
{
    m_stagingPath.clear();
    m_finalArchivePath.clear();
    m_archiveSize = -1;
    m_archiveSha256.clear();
    m_expectedManifest.clear();
    m_recoverySetUuid.clear();
}

QString PrivacyCasualArchiveEngine::manifestMemberName()
{
    return ManifestName;
}

QString PrivacyCasualArchiveEngine::expectedMemberPath(
    int role, int ordinal, const QString& originalName)
{
    return QString::fromLatin1("digikam-private/assets/%1/%2/%3")
        .arg(role).arg(ordinal).arg(originalName);
}

bool PrivacyCasualArchiveEngine::checkCapabilities(
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if ((QLatin1String(zip_libzip_version()) != QLatin1String("1.11.4")) ||
        !zip_encryption_method_supported(ZIP_EM_TRAD_PKWARE, 1) ||
        !zip_encryption_method_supported(ZIP_EM_TRAD_PKWARE, 0))
    {
        setError(error, PrivacyCasualArchiveError::UnsupportedLibzip);

        return false;
    }

    return true;
}

bool PrivacyCasualArchiveManifestMember::isValid() const
{
    return (isSafeOriginalName(originalName) &&
            (role > 0) && (ordinal >= 0) &&
            (hashAlgorithm == QLatin1String("sha256")) &&
            (sha256.size() == 32) && (size >= 0) &&
            (portableAttributes.size() <= MaximumPortableAttributes) &&
            ((unixMode & 0170000) == 0100000));
}

bool PrivacyCasualArchiveManifest::isValid() const
{
    return ((format == QLatin1String("digikam-private-casual")) &&
            (formatVersion == 1) &&
            (passwordEncoding == QLatin1String("utf8-nfc-v1")) &&
            isCanonicalUuid(categoryUuid) &&
            isCanonicalUuid(containerUuid) &&
            isCanonicalUuid(itemUuid) && !members.isEmpty() &&
            (members.size() <= MaximumMemberCount));
}

PrivacyCasualArchiveIdentity PrivacyCasualArchiveEngine::readPublicIdentity(
    const QString& archivePath,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);
    PrivacyCasualArchiveIdentity identity;

    if (!isSafeAbsoluteFilePath(archivePath, true) ||
        !archivePath.endsWith(ArchiveSuffix))
    {
        setError(error, PrivacyCasualArchiveError::UnsafeDestination);
        return identity;
    }

    int zipError = 0;
    zip_t* archive = zip_open(
        QFile::encodeName(archivePath).constData(),
        ZIP_RDONLY | ZIP_CHECKCONS, &zipError);

    if (!archive)
    {
        setError(error, PrivacyCasualArchiveError::ArchiveOpenFailed);
        return identity;
    }

    int commentLength = 0;
    const char* const comment =
        zip_get_archive_comment(archive, &commentLength, 0);
    QString recoverySetUuid;
    const bool validComment =
        comment && decodeArchiveComment(QByteArray(comment, commentLength),
                                        &recoverySetUuid);
    zip_discard(archive);

    if (!validComment)
    {
        setError(error, PrivacyCasualArchiveError::ArchivePolicyViolation);
        return identity;
    }

    identity.valid = true;
    identity.format = QLatin1String("digiKam Private casual-v1");
    identity.passwordEncoding = QLatin1String("utf8-nfc-v1");
    identity.recoverySetUuid = recoverySetUuid;

    return identity;
}

PrivacyCasualArchiveIdentity PrivacyCasualArchiveEngine::inspectIdentity(
    const QString& archivePath,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);
    PrivacyCasualArchiveIdentity identity =
        readPublicIdentity(archivePath, error);

    if (!identity.valid)
    {
        return identity;
    }

    QByteArray sha256;
    qlonglong size = -1;

    if (!hashFile(archivePath, &sha256, &size, {}))
    {
        setError(error, PrivacyCasualArchiveError::SourceReadFailed);
        return PrivacyCasualArchiveIdentity();
    }

    identity.archiveSize = size;
    identity.sha256 = sha256;

    return identity;
}

bool PrivacyCasualArchiveEngine::verifyAndReadManifest(
    const QString& archivePath,
    const PrivacyPassword& password,
    qlonglong expectedArchiveSize,
    const QByteArray& expectedArchiveSha256,
    PrivacyCasualArchiveManifest* const manifest,
    const CancellationCheck& isCancelled,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if (!manifest || !password.isValid() ||
        (expectedArchiveSize < 0) ||
        (expectedArchiveSha256.size() != 32) ||
        !isSafeAbsoluteFilePath(archivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);
        return false;
    }

    QByteArray archiveHash;
    qlonglong archiveSize = -1;

    if (!hashFile(archivePath, &archiveHash, &archiveSize, isCancelled) ||
        (archiveHash != expectedArchiveSha256) ||
        (archiveSize != expectedArchiveSize))
    {
        setError(error, cancelled(isCancelled)
                        ? PrivacyCasualArchiveError::Cancelled
                        : PrivacyCasualArchiveError::ExistingArchiveMismatch);
        return false;
    }

    QByteArray manifestBytes;

    if (!readManifestForResume(archivePath, password, isCancelled,
                               &manifestBytes, error) ||
        !verifyArchive(archivePath, manifestBytes, password, isCancelled,
                       QString(), error))
    {
        return false;
    }

    if (!parsePortableManifest(manifestBytes, manifest))
    {
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);
        return false;
    }

    return true;
}

PrivacyCasualArchiveStage PrivacyCasualArchiveEngine::stageArchive(
    const PrivacyCasualArchiveRequest& request, const PrivacyPassword& password,
    const CancellationCheck& isCancelled,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);
    PrivacyCasualArchiveStage stage;

    if (!checkCapabilities(error))
    {
        return stage;
    }

    if (!password.isValid())
    {
        setError(error, PrivacyCasualArchiveError::InvalidPassword);

        return stage;
    }

    const bool exactStageRequested = !request.stagingArchivePath.isEmpty();

    if (!safeDestination(request.finalArchivePath) ||
        (exactStageRequested &&
         (!safeStagingPath(request.stagingArchivePath,
                           request.finalArchivePath, false) ||
          (QFileInfo(request.stagingArchivePath).absolutePath() !=
           QFileInfo(request.finalArchivePath).absolutePath()))) ||
        !isCanonicalUuid(request.categoryUuid) ||
        !isCanonicalUuid(request.containerUuid) ||
        !isCanonicalUuid(request.itemUuid) ||
        !isCanonicalUuid(request.recoverySetUuid) ||
        request.members.isEmpty() || (request.members.size() > MaximumMemberCount))
    {
        setError(error, safeDestination(request.finalArchivePath)
                        ? PrivacyCasualArchiveError::InvalidRequest
                        : PrivacyCasualArchiveError::UnsafeDestination);

        return stage;
    }

    QList<PreparedMember> members;
    QSet<QString> collisionKeys;

    for (const PrivacyCasualArchiveMember& input : request.members)
    {
        PreparedMember member;

        if (!prepareMember(input, &member, isCancelled, error))
        {
            return stage;
        }

        const QString collisionKey =
            input.protectedRelativePath.normalized(
                QString::NormalizationForm_C).toCaseFolded();

        if (collisionKeys.contains(collisionKey))
        {
            setError(error, PrivacyCasualArchiveError::DuplicateMember);

            return stage;
        }

        collisionKeys.insert(collisionKey);
        members << member;
    }

    std::sort(members.begin(), members.end(),
              [](const PreparedMember& left, const PreparedMember& right)
              {
                  return (left.archiveName < right.archiveName);
              });

    const QByteArray manifest = createManifest(request, members);

    if (manifest.isEmpty() || (manifest.size() > MaximumManifestBytes))
    {
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);

        return stage;
    }

    const QString directory = QFileInfo(request.finalArchivePath).absolutePath();
    const QString stagingPath = exactStageRequested
                              ? request.stagingArchivePath
                              : directory + QLatin1String("/.digikam-private-stage-") +
                                QUuid::createUuid().toString(QUuid::WithoutBraces) +
                                QLatin1String(".zip");
    int openError = 0;
    const QByteArray encodedStaging = QFile::encodeName(stagingPath);
    zip_t* archive = zip_open(encodedStaging.constData(), ZIP_CREATE | ZIP_EXCL,
                              &openError);

    if (!archive)
    {
        setError(error, PrivacyCasualArchiveError::StagingCreateFailed);

        return stage;
    }

    if (isCancelled)
    {
        zip_register_cancel_callback_with_state(archive, cancelZip, nullptr,
                                                const_cast<CancellationCheck*>(&isCancelled));
    }

    QByteArray comment;
    const bool commentOkay = encodeArchiveComment(request.recoverySetUuid,
                                                  &comment);
    bool writeOkay =
        commentOkay &&
        (zip_set_archive_comment(archive, comment.constData(),
                                 static_cast<zip_uint16_t>(comment.size())) == 0);

    for (const PreparedMember& member : std::as_const(members))
    {
        if (!writeOkay || cancelled(isCancelled))
        {
            writeOkay = false;
            break;
        }

        zip_source_t* const source = sourceForPreparedMember(archive, member, error);

        if (!source)
        {
            writeOkay = false;
            break;
        }

        const zip_int64_t index =
            zip_file_add(archive, member.archiveName.constData(), source,
                         ZIP_FL_ENC_UTF_8);

        if (index < 0)
        {
            zip_source_free(source);
            writeOkay = false;
            break;
        }

        const time_t mtime = member.input.originalModificationDate.isValid()
                           ? static_cast<time_t>(
                                 member.input.originalModificationDate.toSecsSinceEpoch())
                           : static_cast<time_t>(member.sourceMtimeSeconds);
        writeOkay = configureEntry(archive, static_cast<zip_uint64_t>(index),
                                   mtime, member.unixMode, password);
    }

    if (writeOkay)
    {
        zip_source_t* const source =
            zip_source_buffer(archive, manifest.constData(),
                              static_cast<zip_uint64_t>(manifest.size()), 0);

        if (!source)
        {
            writeOkay = false;
        }
        else
        {
            const QByteArray manifestName = ManifestName.toUtf8();
            const zip_int64_t index =
                zip_file_add(archive, manifestName.constData(), source,
                             ZIP_FL_ENC_UTF_8);

            if (index < 0)
            {
                zip_source_free(source);
                writeOkay = false;
            }
            else
            {
                writeOkay = configureEntry(archive, static_cast<zip_uint64_t>(index),
                                            FixedManifestMtime,
                                            ManifestUnixMode, password);
            }
        }
    }

    if (!writeOkay || cancelled(isCancelled))
    {
        zip_discard(archive);
        removeKnownStage(stagingPath, request.finalArchivePath);
        setError(error, cancelled(isCancelled) ? PrivacyCasualArchiveError::Cancelled
                                               : PrivacyCasualArchiveError::ArchiveWriteFailed);

        return stage;
    }

    if (zip_close(archive) != 0)
    {
        zip_discard(archive);
        removeKnownStage(stagingPath, request.finalArchivePath);
        setError(error, cancelled(isCancelled) ? PrivacyCasualArchiveError::Cancelled
                                               : PrivacyCasualArchiveError::ArchiveWriteFailed);

        return stage;
    }

    if (!QFile::setPermissions(stagingPath, QFileDevice::ReadOwner |
                                           QFileDevice::WriteOwner) ||
        !syncFile(stagingPath) || !syncDirectory(directory))
    {
        removeKnownStage(stagingPath, request.finalArchivePath);
        setError(error, PrivacyCasualArchiveError::DurabilityFailed);

        return stage;
    }

    if (!verifyArchive(stagingPath, manifest, password, isCancelled,
                       request.recoverySetUuid, error))
    {
        removeKnownStage(stagingPath, request.finalArchivePath);

        return stage;
    }

    QByteArray archiveHash;
    qlonglong archiveSize = -1;

    if (!hashFile(stagingPath, &archiveHash, &archiveSize, isCancelled))
    {
        removeKnownStage(stagingPath, request.finalArchivePath);
        setError(error, cancelled(isCancelled) ? PrivacyCasualArchiveError::Cancelled
                                               : PrivacyCasualArchiveError::SourceReadFailed);

        return stage;
    }

    stage.m_stagingPath      = stagingPath;
    stage.m_finalArchivePath = request.finalArchivePath;
    stage.m_archiveSize      = archiveSize;
    stage.m_archiveSha256    = archiveHash;
    stage.m_expectedManifest = manifest;
    stage.m_recoverySetUuid  = request.recoverySetUuid;

    return stage;
}

PrivacyCasualArchiveStage PrivacyCasualArchiveEngine::rewriteArchive(
    const PrivacyCasualArchiveRequest& request,
    const PrivacyPassword& oldPassword, const PrivacyPassword& newPassword,
    const CancellationCheck& isCancelled,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);
    PrivacyCasualArchiveStage stage;

    if (!checkCapabilities(error))
    {
        return stage;
    }

    if (!oldPassword.isValid() || !newPassword.isValid())
    {
        setError(error, PrivacyCasualArchiveError::InvalidPassword);
        return stage;
    }

    if (!isCanonicalUuid(request.categoryUuid) ||
        !isCanonicalUuid(request.containerUuid) ||
        !isCanonicalUuid(request.itemUuid) ||
        !isCanonicalUuid(request.recoverySetUuid) ||
        !safeDestination(request.finalArchivePath) ||
        !QFileInfo(request.finalArchivePath).isFile())
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);
        return stage;
    }

    const bool exactStageRequested = !request.stagingArchivePath.isEmpty();
    const QString directory = QFileInfo(request.finalArchivePath).absolutePath();
    const QString stagingPath = exactStageRequested
        ? request.stagingArchivePath
        : directory + QLatin1String("/.digikam-private-stage-") +
          QUuid::createUuid().toString(QUuid::WithoutBraces) +
          QLatin1String(".zip");

    if (exactStageRequested &&
        (!safeStagingPath(request.stagingArchivePath,
                          request.finalArchivePath, false) ||
         (QFileInfo(request.stagingArchivePath).absolutePath() != directory)))
    {
        setError(error, PrivacyCasualArchiveError::UnsafeDestination);
        return stage;
    }

    QByteArray oldManifest;

    if (!readManifestForResume(request.finalArchivePath, oldPassword,
                               isCancelled, &oldManifest, error) ||
        !verifyArchive(request.finalArchivePath, oldManifest, oldPassword,
                       isCancelled, request.recoverySetUuid, error))
    {
        return stage;
    }

    int zipError = 0;
    zip_t* oldArchive = zip_open(
        QFile::encodeName(request.finalArchivePath).constData(),
        ZIP_RDONLY | ZIP_CHECKCONS, &zipError);

    if (!oldArchive)
    {
        setError(error, PrivacyCasualArchiveError::ArchiveOpenFailed);
        return stage;
    }

    zip_t* newArchive = zip_open(QFile::encodeName(stagingPath).constData(),
                                 ZIP_CREATE | ZIP_EXCL, &zipError);

    if (!newArchive)
    {
        zip_discard(oldArchive);
        setError(error, PrivacyCasualArchiveError::StagingCreateFailed);
        return stage;
    }

    if (isCancelled)
    {
        zip_register_cancel_callback_with_state(
            newArchive, cancelZip, nullptr,
            const_cast<CancellationCheck*>(&isCancelled));
    }

    std::vector<std::unique_ptr<RewriteSource>> sources;

    const auto fail = [&](PrivacyCasualArchiveError value)
    {
        for (const std::unique_ptr<RewriteSource>& owned : sources)
        {
            if (owned->file)
            {
                zip_fclose(owned->file);
            }
        }

        if (newArchive)
        {
            zip_discard(newArchive);
            newArchive = nullptr;
        }

        if (oldArchive)
        {
            zip_discard(oldArchive);
            oldArchive = nullptr;
        }

        removeKnownStage(stagingPath, request.finalArchivePath);
        setError(error, cancelled(isCancelled)
                            ? PrivacyCasualArchiveError::Cancelled
                            : value);
        return PrivacyCasualArchiveStage();
    };

    bool oldPasswordSet = false;
    bool newPasswordSet = false;
    oldPassword.withUtf8CString(
        [oldArchive, &oldPasswordSet](const char* value)
        {
            oldPasswordSet = (zip_set_default_password(oldArchive, value) == 0);
            return true;
        });
    newPassword.withUtf8CString(
        [newArchive, &newPasswordSet](const char* value)
        {
            newPasswordSet = (zip_set_default_password(newArchive, value) == 0);
            return true;
        });

    if (!oldPasswordSet || !newPasswordSet)
    {
        return fail(PrivacyCasualArchiveError::InvalidPassword);
    }

    int commentLength = 0;
    const char* const comment =
        zip_get_archive_comment(oldArchive, &commentLength, 0);

    QString oldCommentRecovery;

    if (!comment ||
        !decodeArchiveComment(QByteArray(comment, commentLength),
                              &oldCommentRecovery) ||
        (oldCommentRecovery != request.recoverySetUuid) ||
        (zip_set_archive_comment(
             newArchive, comment,
             static_cast<zip_uint16_t>(commentLength)) != 0))
    {
        return fail(PrivacyCasualArchiveError::ArchiveWriteFailed);
    }

    const zip_int64_t entryCount = zip_get_num_entries(oldArchive, 0);

    for (zip_int64_t oldIndex = 0 ; oldIndex < entryCount ; ++oldIndex)
    {
        if (cancelled(isCancelled))
        {
            return fail(PrivacyCasualArchiveError::Cancelled);
        }

        const char* const rawName = zip_get_name(
            oldArchive, static_cast<zip_uint64_t>(oldIndex), ZIP_FL_ENC_STRICT);

        if (!rawName)
        {
            return fail(PrivacyCasualArchiveError::ArchivePolicyViolation);
        }

        zip_stat_t stat;
        zip_stat_init(&stat);

        if (zip_stat_index(oldArchive,
                           static_cast<zip_uint64_t>(oldIndex), 0, &stat) != 0)
        {
            return fail(PrivacyCasualArchiveError::ArchivePolicyViolation);
        }

        zip_uint8_t hostSystem = 0;
        zip_uint32_t attributes = 0;
        zip_file_get_external_attributes(
            oldArchive, static_cast<zip_uint64_t>(oldIndex), 0,
            &hostSystem, &attributes);

        std::unique_ptr<RewriteSource> sourceState(new RewriteSource);
        sourceState->file = zip_fopen_index(
            oldArchive, static_cast<zip_uint64_t>(oldIndex), 0);
        sourceState->size =
            (stat.valid & ZIP_STAT_SIZE) ? stat.size : 0;

        if (!sourceState->file)
        {
            return fail(PrivacyCasualArchiveError::DecryptionFailed);
        }

        zip_source_t* const source = zip_source_function(
            newArchive, rewriteSourceCallback, sourceState.get());

        if (!source)
        {
            return fail(PrivacyCasualArchiveError::ArchiveWriteFailed);
        }

        const zip_int64_t newIndex = zip_file_add(
            newArchive, rawName, source, ZIP_FL_ENC_UTF_8);

        if (newIndex < 0)
        {
            zip_source_free(source);
            return fail(PrivacyCasualArchiveError::ArchiveWriteFailed);
        }

        const time_t mtime = (stat.valid & ZIP_STAT_MTIME)
                           ? static_cast<time_t>(stat.mtime)
                           : 0;
        bool configured = false;
        newPassword.withUtf8CString(
            [newArchive, newIndex, mtime, hostSystem, attributes,
             &configured](const char* value)
            {
                configured =
                    (zip_set_file_compression(
                         newArchive, static_cast<zip_uint64_t>(newIndex),
                         ZIP_CM_STORE, 0) == 0) &&
                    (zip_file_set_mtime(
                         newArchive, static_cast<zip_uint64_t>(newIndex),
                         mtime, 0) == 0) &&
                    (zip_file_set_external_attributes(
                         newArchive, static_cast<zip_uint64_t>(newIndex),
                         0, hostSystem, attributes) == 0) &&
                    (zip_file_set_encryption(
                         newArchive, static_cast<zip_uint64_t>(newIndex),
                         ZIP_EM_TRAD_PKWARE, value) == 0);
                return true;
            });

        if (!configured)
        {
            return fail(PrivacyCasualArchiveError::ArchiveWriteFailed);
        }

        sources.push_back(std::move(sourceState));
    }

    if (zip_close(newArchive) != 0)
    {
        newArchive = nullptr;
        return fail(PrivacyCasualArchiveError::ArchiveWriteFailed);
    }

    newArchive = nullptr;

    for (const std::unique_ptr<RewriteSource>& owned : sources)
    {
        if (owned->file)
        {
            zip_fclose(owned->file);
        }
    }

    sources.clear();
    zip_discard(oldArchive);
    oldArchive = nullptr;

    if (!QFile::setPermissions(stagingPath,
                               QFileDevice::ReadOwner |
                               QFileDevice::WriteOwner) ||
        !syncFile(stagingPath) || !syncDirectory(directory))
    {
        removeKnownStage(stagingPath, request.finalArchivePath);
        setError(error, PrivacyCasualArchiveError::DurabilityFailed);
        return stage;
    }

    if (!verifyArchive(stagingPath, oldManifest, newPassword,
                       isCancelled, request.recoverySetUuid, error))
    {
        removeKnownStage(stagingPath, request.finalArchivePath);
        return stage;
    }

    QByteArray archiveHash;
    qlonglong archiveSize = -1;

    if (!hashFile(stagingPath, &archiveHash, &archiveSize, isCancelled))
    {
        removeKnownStage(stagingPath, request.finalArchivePath);
        setError(error, cancelled(isCancelled)
                            ? PrivacyCasualArchiveError::Cancelled
                            : PrivacyCasualArchiveError::SourceReadFailed);
        return stage;
    }

    stage.m_stagingPath      = stagingPath;
    stage.m_finalArchivePath = request.finalArchivePath;
    stage.m_archiveSize      = archiveSize;
    stage.m_archiveSha256    = archiveHash;
    stage.m_expectedManifest = oldManifest;
    stage.m_recoverySetUuid  = request.recoverySetUuid;

    return stage;
}

PrivacyCasualArchiveStage PrivacyCasualArchiveEngine::resumeStagedArchive(
    const QString& stagingPath, const QString& finalArchivePath,
    qlonglong expectedArchiveSize, const QByteArray& expectedArchiveSha256,
    const QString& expectedRecoverySetUuid,
    const PrivacyPassword& password, const CancellationCheck& isCancelled,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);
    PrivacyCasualArchiveStage stage;

    if (!checkCapabilities(error))
    {
        return stage;
    }

    if (!password.isValid())
    {
        setError(error, PrivacyCasualArchiveError::InvalidPassword);

        return stage;
    }

    if ((expectedArchiveSize < 0) || (expectedArchiveSha256.size() != 32) ||
        !isCanonicalUuid(expectedRecoverySetUuid) ||
        !safeDestination(finalArchivePath) ||
        !safeStagingPath(stagingPath, finalArchivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);

        return stage;
    }

    QByteArray currentHash;
    qlonglong currentSize = -1;

    if (!hashFile(stagingPath, &currentHash, &currentSize, isCancelled))
    {
        setError(error, cancelled(isCancelled)
                        ? PrivacyCasualArchiveError::Cancelled
                        : PrivacyCasualArchiveError::SourceReadFailed);

        return stage;
    }

    if ((currentSize != expectedArchiveSize) ||
        (currentHash != expectedArchiveSha256))
    {
        setError(error, PrivacyCasualArchiveError::HashMismatch);

        return stage;
    }

    QByteArray manifest;

    if (!readManifestForResume(stagingPath, password, isCancelled,
                               &manifest, error) ||
        !verifyArchive(stagingPath, manifest, password, isCancelled,
                       expectedRecoverySetUuid, error))
    {
        return stage;
    }

    stage.m_stagingPath      = stagingPath;
    stage.m_finalArchivePath = finalArchivePath;
    stage.m_archiveSize      = currentSize;
    stage.m_archiveSha256    = currentHash;
    stage.m_expectedManifest = manifest;
    stage.m_recoverySetUuid  = expectedRecoverySetUuid;

    return stage;
}

bool PrivacyCasualArchiveEngine::verifyStagedArchive(
    const PrivacyCasualArchiveStage& stage, const PrivacyPassword& password,
    const CancellationCheck& isCancelled,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if (!stage.isValid() ||
        !safeStagingPath(stage.m_stagingPath, stage.m_finalArchivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);

        return false;
    }

    return verifyArchive(stage.m_stagingPath, stage.m_expectedManifest,
                         password, isCancelled, stage.m_recoverySetUuid,
                         error);
}

bool PrivacyCasualArchiveEngine::publishExactPreparedStage(
    const QString& stagingPath, const QString& finalArchivePath,
    qlonglong expectedArchiveSize, const QByteArray& expectedArchiveSha256,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if ((expectedArchiveSize < 0) || (expectedArchiveSha256.size() != 32) ||
        !safeDestination(finalArchivePath) ||
        !safeStagingPath(stagingPath, finalArchivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);
        return false;
    }

    QByteArray currentHash;
    qlonglong currentSize = -1;

    if (!hashFile(stagingPath, &currentHash, &currentSize, {}) ||
        (currentHash != expectedArchiveSha256) ||
        (currentSize != expectedArchiveSize))
    {
        setError(error, PrivacyCasualArchiveError::HashMismatch);
        return false;
    }

    const PublishResult result = publishWithoutReplacement(stagingPath,
                                                            finalArchivePath);

    if ((result != PublishResult::Success) &&
        (result != PublishResult::SuccessWithStagingRemaining))
    {
        setError(error, (result == PublishResult::Conflict)
                        ? PrivacyCasualArchiveError::PublicationConflict
                        : PrivacyCasualArchiveError::PublicationFailed);
        return false;
    }

    if (result == PublishResult::SuccessWithStagingRemaining)
    {
        setError(error, PrivacyCasualArchiveError::DurabilityUncertain);
        return false;
    }

    if (!syncDirectory(QFileInfo(finalArchivePath).absolutePath()))
    {
        setError(error, PrivacyCasualArchiveError::DurabilityUncertain);
        return false;
    }

    return true;
}

bool PrivacyCasualArchiveEngine::restoreMember(
    const PrivacyCasualArchiveRestoreRequest& request,
    const PrivacyPassword& password, QIODevice* const destination,
    const CancellationCheck& isCancelled,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if (!checkCapabilities(error) || !password.isValid() || !destination ||
        !destination->isOpen() || !destination->isWritable() ||
        !isSafeAbsoluteFilePath(request.archivePath, true) ||
        !isCanonicalUuid(request.categoryUuid) ||
        !isCanonicalUuid(request.containerUuid) ||
        !isCanonicalUuid(request.itemUuid) ||
        !isCanonicalUuid(request.recoverySetUuid) ||
        !isSafeOriginalName(request.originalName) || (request.role <= 0) ||
        (request.ordinal < 0) ||
        (request.protectedRelativePath != expectedMemberPath(
             request.role, request.ordinal, request.originalName)) ||
        (request.expectedArchiveSize < 0) ||
        (request.expectedArchiveSha256.size() != 32) ||
        (request.expectedMemberSize < 0) ||
        (request.expectedMemberSha256.size() != 32))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);
        return false;
    }

    QByteArray archiveHash;
    qlonglong archiveSize = -1;

    if (!hashFile(request.archivePath, &archiveHash, &archiveSize, isCancelled) ||
        (archiveHash != request.expectedArchiveSha256) ||
        (archiveSize != request.expectedArchiveSize))
    {
        setError(error, cancelled(isCancelled)
                        ? PrivacyCasualArchiveError::Cancelled
                        : PrivacyCasualArchiveError::ExistingArchiveMismatch);
        return false;
    }

    QByteArray manifest;

    if (!readManifestForResume(request.archivePath, password, isCancelled,
                               &manifest, error) ||
        !verifyArchive(request.archivePath, manifest, password,
                       isCancelled, request.recoverySetUuid, error))
    {
        return false;
    }

    QList<ExpectedMember> members;
    QJsonObject semantic;

    if (!parseManifest(manifest, &members, &semantic) ||
        (semantic.value(QLatin1String("categoryUuid")).toString() !=
         request.categoryUuid) ||
        (semantic.value(QLatin1String("containerUuid")).toString() !=
         request.containerUuid) ||
        (semantic.value(QLatin1String("itemUuid")).toString() !=
         request.itemUuid))
    {
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);
        return false;
    }

    const auto memberIt = std::find_if(
        members.cbegin(), members.cend(),
        [&request](const ExpectedMember& member)
        {
            return ((member.archiveName == request.protectedRelativePath) &&
                    (member.originalName == request.originalName) &&
                    (member.role == request.role) &&
                    (member.ordinal == request.ordinal) &&
                    (member.size == request.expectedMemberSize) &&
                    (member.sha256 == request.expectedMemberSha256));
        });

    if (memberIt == members.cend())
    {
        setError(error, PrivacyCasualArchiveError::ManifestInvalid);
        return false;
    }

    int zipError = 0;
    const QByteArray encodedPath = QFile::encodeName(request.archivePath);
    zip_t* const archive = zip_open(encodedPath.constData(),
                                    ZIP_RDONLY | ZIP_CHECKCONS, &zipError);

    if (!archive)
    {
        setError(error, PrivacyCasualArchiveError::ArchiveOpenFailed);
        return false;
    }

    const QByteArray encodedMember = request.protectedRelativePath.toUtf8();
    const zip_int64_t index = zip_name_locate(archive, encodedMember.constData(),
                                               ZIP_FL_ENC_UTF_8);
    const bool restored = (index >= 0) && streamEncryptedEntry(
        archive, static_cast<zip_uint64_t>(index), password, *memberIt,
        isCancelled, destination, error);
    zip_discard(archive);

    QByteArray finalArchiveHash;
    qlonglong finalArchiveSize = -1;
    const bool archiveStillExact = restored &&
        hashFile(request.archivePath, &finalArchiveHash, &finalArchiveSize,
                 isCancelled) &&
        (finalArchiveHash == request.expectedArchiveSha256) &&
        (finalArchiveSize == request.expectedArchiveSize);

    if (restored && !archiveStillExact)
    {
        setError(error, PrivacyCasualArchiveError::ExistingArchiveMismatch);
    }
    else if (!restored && error && (*error == PrivacyCasualArchiveError::None))
    {
        setError(error, PrivacyCasualArchiveError::MemberReadFailed);
    }

    return archiveStillExact;
}

bool PrivacyCasualArchiveEngine::publishNew(
    PrivacyCasualArchiveStage* const stage,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if (!stage || !stage->isValid() ||
        !safeStagingPath(stage->m_stagingPath, stage->m_finalArchivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);

        return false;
    }

    QByteArray currentHash;
    qlonglong currentSize = -1;

    if (!hashFile(stage->m_stagingPath, &currentHash, &currentSize, {}) ||
        (currentHash != stage->m_archiveSha256) ||
        (currentSize != stage->m_archiveSize))
    {
        setError(error, PrivacyCasualArchiveError::HashMismatch);

        return false;
    }

    const PublishResult result =
        publishWithoutReplacement(stage->m_stagingPath, stage->m_finalArchivePath);

    if ((result != PublishResult::Success) &&
        (result != PublishResult::SuccessWithStagingRemaining))
    {
        setError(error, (result == PublishResult::Conflict)
                        ? PrivacyCasualArchiveError::PublicationConflict
                        : PrivacyCasualArchiveError::PublicationFailed);

        return false;
    }

    const QString directory = QFileInfo(stage->m_finalArchivePath).absolutePath();

    if (result == PublishResult::SuccessWithStagingRemaining)
    {
        setError(error, PrivacyCasualArchiveError::DurabilityUncertain);

        return false;
    }

    stage->clear();

    if (!syncDirectory(directory))
    {
        setError(error, PrivacyCasualArchiveError::DurabilityUncertain);

        return false;
    }

    return true;
}

bool PrivacyCasualArchiveEngine::publishReplacement(
    PrivacyCasualArchiveStage* const stage,
    const QByteArray& expectedExistingSha256,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if (!stage || !stage->isValid() || (expectedExistingSha256.size() != 32) ||
        !safeStagingPath(stage->m_stagingPath, stage->m_finalArchivePath, true) ||
        !isSafeAbsoluteFilePath(stage->m_finalArchivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);

        return false;
    }

    QByteArray stagedHash;
    QByteArray existingHash;
    qlonglong stagedSize = -1;
    qlonglong existingSize = -1;

    if (!hashFile(stage->m_stagingPath, &stagedHash, &stagedSize, {}) ||
        (stagedHash != stage->m_archiveSha256) ||
        (stagedSize != stage->m_archiveSize))
    {
        setError(error, PrivacyCasualArchiveError::HashMismatch);

        return false;
    }

    if (!hashFile(stage->m_finalArchivePath, &existingHash, &existingSize, {}) ||
        (existingHash != expectedExistingSha256))
    {
        setError(error, PrivacyCasualArchiveError::ExistingArchiveMismatch);

        return false;
    }

    const QString directory = QFileInfo(stage->m_finalArchivePath).absolutePath();
    const ReplaceResult result = replaceWithExpectedArchive(
        stage->m_stagingPath, stage->m_finalArchivePath,
        stage->m_archiveSha256, stage->m_archiveSize,
        expectedExistingSha256);

    if (result == ReplaceResult::ExistingMismatchRestored)
    {
        if (!syncDirectory(directory))
        {
            setError(error, PrivacyCasualArchiveError::DurabilityUncertain);

            return false;
        }

        setError(error, PrivacyCasualArchiveError::ExistingArchiveMismatch);

        return false;
    }

    if (result == ReplaceResult::ExchangeFailed)
    {
        setError(error, PrivacyCasualArchiveError::PublicationFailed);

        return false;
    }

    if ((result == ReplaceResult::RollbackFailed) ||
        (result == ReplaceResult::OldArchiveRemaining))
    {
        stage->clear();
        syncDirectory(directory);
        setError(error, PrivacyCasualArchiveError::DurabilityUncertain);

        return false;
    }

    stage->clear();

    if (!syncDirectory(directory))
    {
        setError(error, PrivacyCasualArchiveError::DurabilityUncertain);

        return false;
    }

    return true;
}

bool PrivacyCasualArchiveEngine::discardStaged(
    PrivacyCasualArchiveStage* const stage,
    PrivacyCasualArchiveError* const error) const
{
    setError(error, PrivacyCasualArchiveError::None);

    if (!stage || !stage->isValid() ||
        !safeStagingPath(stage->m_stagingPath, stage->m_finalArchivePath, true))
    {
        setError(error, PrivacyCasualArchiveError::InvalidRequest);

        return false;
    }

    QByteArray currentHash;
    qlonglong currentSize = -1;

    if (!hashFile(stage->m_stagingPath, &currentHash, &currentSize, {}) ||
        (currentHash != stage->m_archiveSha256) ||
        (currentSize != stage->m_archiveSize))
    {
        setError(error, PrivacyCasualArchiveError::ExistingArchiveMismatch);

        return false;
    }

    const QString directory = QFileInfo(stage->m_stagingPath).absolutePath();

    if (!QFile::remove(stage->m_stagingPath))
    {
        setError(error, PrivacyCasualArchiveError::PublicationFailed);

        return false;
    }

    stage->clear();

    if (!syncDirectory(directory))
    {
        setError(error, PrivacyCasualArchiveError::DurabilityUncertain);

        return false;
    }

    return true;
}

} // namespace Digikam
