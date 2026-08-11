/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacystrongrecoverymanifest.h"

// C++ includes

#include <algorithm>
#include <cmath>
#include <limits>

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QUuid>

#ifdef Q_OS_UNIX

#   include <fcntl.h>
#   include <unistd.h>

#endif

namespace Digikam
{

namespace
{

constexpr qsizetype MaximumManifestBytes       = 1024 * 1024;
constexpr int       MaximumItemCount           = 4096;
constexpr int       MaximumMemberCount         = 256;
constexpr qsizetype MaximumOriginalNameBytes   = 255;
constexpr qsizetype MaximumPathBytes           = 1024;
constexpr qsizetype MaximumPortableAttributes  = 64 * 1024;

void setError(PrivacyStrongRecoveryManifestError* const error,
              PrivacyStrongRecoveryManifestError value)
{
    if (error)
    {
        *error = value;
    }
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

bool isSafeRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) ||
        path.contains(QChar::Null) || path.contains(QLatin1Char('\\')) ||
        (path != path.normalized(QString::NormalizationForm_C)))
    {
        return false;
    }

    const QByteArray utf8 = path.toUtf8();

    if (utf8.isEmpty() || (utf8.size() > MaximumPathBytes))
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
    }

    return true;
}

QJsonValue epochMilliseconds(const QDateTime& value)
{
    return value.isValid()
         ? QJsonValue(QString::number(value.toUTC().toMSecsSinceEpoch()))
         : QJsonValue(QJsonValue::Null);
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

QJsonObject portableAttributesObject(const QByteArray& attributes)
{
    QJsonObject portable;
    portable.insert(QLatin1String("encoding"), QLatin1String("base64"));
    portable.insert(QLatin1String("version"), 1);
    portable.insert(QLatin1String("data"),
                    QString::fromLatin1(attributes.toBase64()));
    return portable;
}

bool parsePortableAttributes(const QJsonObject& portable,
                             QByteArray* const attributes)
{
    if (!attributes)
    {
        return false;
    }

    int version = 0;
    const QByteArray encoded =
        portable.value(QLatin1String("data")).toString().toLatin1();
    const QByteArray decoded =
        QByteArray::fromBase64(encoded, QByteArray::AbortOnBase64DecodingErrors);

    if ((portable.value(QLatin1String("encoding")).toString() !=
         QLatin1String("base64")) ||
        !jsonInteger(portable.value(QLatin1String("version")), &version) ||
        (version != 1) ||
        (decoded.size() > MaximumPortableAttributes) ||
        (decoded.toBase64() != encoded))
    {
        return false;
    }

    *attributes = decoded;
    return true;
}

bool validHexSha256(const QString& hex)
{
    if (hex.size() != 64)
    {
        return false;
    }

    return std::all_of(hex.cbegin(), hex.cend(), [](const QChar character)
    {
        return ((character >= QLatin1Char('0')) &&
                (character <= QLatin1Char('9'))) ||
               ((character >= QLatin1Char('a')) &&
                (character <= QLatin1Char('f')));
    });
}

bool fsyncFile(const QString& path)
{
#ifdef Q_OS_UNIX
    const int descriptor = ::open(QFile::encodeName(path).constData(),
                                  O_RDONLY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return false;
    }

    const bool synced = (::fsync(descriptor) == 0);
    ::close(descriptor);
    return synced;
#else
    Q_UNUSED(path);
    return true;
#endif
}

bool fsyncDirectory(const QString& path)
{
#ifdef Q_OS_UNIX
    const int descriptor = ::open(QFile::encodeName(path).constData(),
                                  O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return false;
    }

    const bool synced = (::fsync(descriptor) == 0);
    ::close(descriptor);
    return synced;
#else
    Q_UNUSED(path);
    return true;
#endif
}

} // namespace

bool PrivacyStrongRecoveryMember::isValid() const
{
    return ((role > 0) && (ordinal >= 0) && (size >= 0) &&
            (hashAlgorithm == QLatin1String("sha256")) &&
            validHexSha256(sha256Hex) && (unixMode > 0) &&
            ((unixMode & 0170000) == 0100000) &&
            isSafeOriginalName(originalName) &&
            isSafeRelativePath(vaultRelativePath) &&
            isSafeRelativePath(publicRelativePath) &&
            (portableAttributes.size() <= MaximumPortableAttributes));
}

bool PrivacyStrongRecoveryItem::isValid() const
{
    if (!isCanonicalUuid(itemUuid) || !isCanonicalUuid(containerUuid) ||
        (generation < 0) || members.isEmpty() ||
        (members.size() > MaximumMemberCount))
    {
        return false;
    }

    QSet<QString> collisionKeys;

    for (const PrivacyStrongRecoveryMember& member : members)
    {
        if (!member.isValid())
        {
            return false;
        }

        const QString key = member.vaultRelativePath.toCaseFolded();

        if (collisionKeys.contains(key))
        {
            return false;
        }

        collisionKeys.insert(key);
    }

    return true;
}

bool PrivacyStrongRecoveryManifest::isValid() const
{
    if ((format != QLatin1String("digikam-private-strong")) ||
        (formatVersion != 1) ||
        (passwordEncoding != QLatin1String("utf8-nfc-v1")) ||
        !isCanonicalUuid(categoryUuid) || !isCanonicalUuid(storeUuid) ||
        categoryName.isEmpty() ||
        (categoryName != categoryName.normalized(QString::NormalizationForm_C)) ||
        (presentationMode <= 0) || (unlockedThumbnailMode <= 0) ||
        (tagVisibilityMode <= 0) || (currentCredentialGeneration < 0) ||
        (items.size() > MaximumItemCount))
    {
        return false;
    }

    QSet<QString> globalCollisions;

    for (const PrivacyStrongRecoveryItem& item : items)
    {
        if (!item.isValid())
        {
            return false;
        }

        for (const PrivacyStrongRecoveryMember& member : item.members)
        {
            const QString key = member.vaultRelativePath.toCaseFolded();

            if (globalCollisions.contains(key))
            {
                return false;
            }

            globalCollisions.insert(key);
        }
    }

    return true;
}

QString PrivacyStrongRecoveryManifestCodec::relativePath()
{
    return QLatin1String("digikam-private/recovery-v1.json");
}

QByteArray PrivacyStrongRecoveryManifestCodec::encode(
    const PrivacyStrongRecoveryManifest& manifest,
    PrivacyStrongRecoveryManifestError* error)
{
    if (!manifest.isValid())
    {
        setError(error, PrivacyStrongRecoveryManifestError::Invalid);
        return {};
    }

    QJsonObject root;
    root.insert(QLatin1String("format"), manifest.format);
    root.insert(QLatin1String("formatVersion"), manifest.formatVersion);
    root.insert(QLatin1String("passwordEncoding"), manifest.passwordEncoding);
    root.insert(QLatin1String("categoryUuid"), manifest.categoryUuid);
    root.insert(QLatin1String("categoryName"), manifest.categoryName);
    root.insert(QLatin1String("presentationMode"), manifest.presentationMode);
    root.insert(QLatin1String("unlockedThumbnailMode"),
                manifest.unlockedThumbnailMode);
    root.insert(QLatin1String("tagVisibilityMode"),
                manifest.tagVisibilityMode);
    root.insert(QLatin1String("currentCredentialGeneration"),
                QString::number(manifest.currentCredentialGeneration));
    root.insert(QLatin1String("storeUuid"), manifest.storeUuid);

    QJsonArray items;

    for (const PrivacyStrongRecoveryItem& item : manifest.items)
    {
        QJsonObject itemObject;
        itemObject.insert(QLatin1String("itemUuid"), item.itemUuid);
        itemObject.insert(QLatin1String("containerUuid"), item.containerUuid);
        itemObject.insert(QLatin1String("generation"),
                          QString::number(item.generation));

        QJsonArray members;

        for (const PrivacyStrongRecoveryMember& member : item.members)
        {
            QJsonObject memberObject;
            memberObject.insert(QLatin1String("vaultPath"),
                                member.vaultRelativePath);
            memberObject.insert(QLatin1String("publicRelativePath"),
                                member.publicRelativePath);
            memberObject.insert(QLatin1String("originalName"),
                                member.originalName);
            memberObject.insert(QLatin1String("role"), member.role);
            memberObject.insert(QLatin1String("ordinal"), member.ordinal);
            memberObject.insert(QLatin1String("hashAlgorithm"),
                                member.hashAlgorithm);
            memberObject.insert(QLatin1String("hash"), member.sha256Hex);
            memberObject.insert(QLatin1String("size"),
                                QString::number(member.size));
            memberObject.insert(QLatin1String("creationTimeUtcMs"),
                                epochMilliseconds(member.creationTimeUtc));
            memberObject.insert(QLatin1String("modificationTimeUtcMs"),
                                epochMilliseconds(member.modificationTimeUtc));
            memberObject.insert(QLatin1String("portableAttributes"),
                                portableAttributesObject(
                                    member.portableAttributes));
            memberObject.insert(QLatin1String("unixMode"),
                                QString::number(member.unixMode));
            members.append(memberObject);
        }

        itemObject.insert(QLatin1String("members"), members);
        items.append(itemObject);
    }

    root.insert(QLatin1String("items"), items);
    const QByteArray bytes =
        QJsonDocument(root).toJson(QJsonDocument::Compact);

    if (bytes.isEmpty() || (bytes.size() > MaximumManifestBytes))
    {
        setError(error, PrivacyStrongRecoveryManifestError::Invalid);
        return {};
    }

    setError(error, PrivacyStrongRecoveryManifestError::None);
    return bytes;
}

bool PrivacyStrongRecoveryManifestCodec::decode(
    const QByteArray& bytes,
    PrivacyStrongRecoveryManifest* manifest,
    PrivacyStrongRecoveryManifestError* error)
{
    if (!manifest || bytes.isEmpty() || (bytes.size() > MaximumManifestBytes))
    {
        setError(error, PrivacyStrongRecoveryManifestError::Invalid);
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);

    if ((parseError.error != QJsonParseError::NoError) || !document.isObject())
    {
        setError(error, PrivacyStrongRecoveryManifestError::Invalid);
        return false;
    }

    const QJsonObject root = document.object();
    PrivacyStrongRecoveryManifest decoded;
    int formatVersion = 0;
    qlonglong credentialGeneration = -1;

    if ((root.value(QLatin1String("format")).toString() !=
         QLatin1String("digikam-private-strong")) ||
        !jsonInteger(root.value(QLatin1String("formatVersion")),
                     &formatVersion) ||
        (formatVersion != 1) ||
        (root.value(QLatin1String("passwordEncoding")).toString() !=
         QLatin1String("utf8-nfc-v1")) ||
        !isCanonicalUuid(root.value(QLatin1String("categoryUuid")).toString()) ||
        !isCanonicalUuid(root.value(QLatin1String("storeUuid")).toString()) ||
        !jsonInteger(root.value(QLatin1String("presentationMode")),
                     &decoded.presentationMode) ||
        !jsonInteger(root.value(QLatin1String("unlockedThumbnailMode")),
                     &decoded.unlockedThumbnailMode) ||
        !jsonInteger(root.value(QLatin1String("tagVisibilityMode")),
                     &decoded.tagVisibilityMode) ||
        !decimalString(root.value(QLatin1String("currentCredentialGeneration")),
                       &credentialGeneration) ||
        !root.value(QLatin1String("items")).isArray())
    {
        setError(error, PrivacyStrongRecoveryManifestError::Invalid);
        return false;
    }

    decoded.format = QLatin1String("digikam-private-strong");
    decoded.formatVersion = 1;
    decoded.passwordEncoding = QLatin1String("utf8-nfc-v1");
    decoded.categoryUuid = root.value(QLatin1String("categoryUuid")).toString();
    decoded.categoryName = root.value(QLatin1String("categoryName")).toString();
    decoded.storeUuid = root.value(QLatin1String("storeUuid")).toString();
    decoded.currentCredentialGeneration = credentialGeneration;

    const QJsonArray itemArray = root.value(QLatin1String("items")).toArray();

    if (itemArray.size() > MaximumItemCount)
    {
        setError(error, PrivacyStrongRecoveryManifestError::Invalid);
        return false;
    }

    for (const QJsonValue& itemValue : itemArray)
    {
        if (!itemValue.isObject())
        {
            setError(error, PrivacyStrongRecoveryManifestError::Invalid);
            return false;
        }

        const QJsonObject itemObject = itemValue.toObject();
        PrivacyStrongRecoveryItem item;
        qlonglong generation = -1;

        if (!isCanonicalUuid(itemObject.value(QLatin1String("itemUuid")).toString()) ||
            !isCanonicalUuid(itemObject.value(
                 QLatin1String("containerUuid")).toString()) ||
            !decimalString(itemObject.value(QLatin1String("generation")),
                           &generation) ||
            !itemObject.value(QLatin1String("members")).isArray())
        {
            setError(error, PrivacyStrongRecoveryManifestError::Invalid);
            return false;
        }

        item.itemUuid = itemObject.value(QLatin1String("itemUuid")).toString();
        item.containerUuid =
            itemObject.value(QLatin1String("containerUuid")).toString();
        item.generation = generation;
        const QJsonArray memberArray =
            itemObject.value(QLatin1String("members")).toArray();

        if (memberArray.isEmpty() || (memberArray.size() > MaximumMemberCount))
        {
            setError(error, PrivacyStrongRecoveryManifestError::Invalid);
            return false;
        }

        for (const QJsonValue& memberValue : memberArray)
        {
            if (!memberValue.isObject())
            {
                setError(error, PrivacyStrongRecoveryManifestError::Invalid);
                return false;
            }

            const QJsonObject memberObject = memberValue.toObject();
            PrivacyStrongRecoveryMember member;
            qlonglong size = -1;
            qlonglong mode = -1;
            qlonglong ignored = 0;

            if (!jsonInteger(memberObject.value(QLatin1String("role")),
                             &member.role) ||
                !jsonInteger(memberObject.value(QLatin1String("ordinal")),
                             &member.ordinal) ||
                !decimalString(memberObject.value(QLatin1String("size")),
                               &size) ||
                !decimalString(memberObject.value(QLatin1String("unixMode")),
                               &mode) ||
                (mode < 0) || (mode > 0xffff) ||
                ((mode & 0170000) != 0100000) ||
                (memberObject.value(QLatin1String("hashAlgorithm")).toString() !=
                 QLatin1String("sha256")) ||
                !decimalString(
                    memberObject.value(QLatin1String("creationTimeUtcMs")),
                    &ignored, true) ||
                !decimalString(
                    memberObject.value(QLatin1String("modificationTimeUtcMs")),
                    &ignored, true))
            {
                setError(error, PrivacyStrongRecoveryManifestError::Invalid);
                return false;
            }

            member.vaultRelativePath =
                memberObject.value(QLatin1String("vaultPath")).toString();
            member.publicRelativePath =
                memberObject.value(QLatin1String("publicRelativePath")).toString();
            member.originalName =
                memberObject.value(QLatin1String("originalName")).toString();
            member.hashAlgorithm =
                memberObject.value(QLatin1String("hashAlgorithm")).toString();
            member.sha256Hex =
                memberObject.value(QLatin1String("hash")).toString();
            member.size = size;
            member.unixMode = static_cast<quint32>(mode);

            const QString creation =
                memberObject.value(QLatin1String("creationTimeUtcMs")).toString();
            const QString modification =
                memberObject.value(QLatin1String("modificationTimeUtcMs")).toString();

            if (!creation.isEmpty())
            {
                bool okay = false;
                const qlonglong creationMs = creation.toLongLong(&okay);

                if (okay)
                {
                    member.creationTimeUtc =
                        QDateTime::fromMSecsSinceEpoch(creationMs, Qt::UTC);
                }
            }

            if (!modification.isEmpty())
            {
                bool okay = false;
                const qlonglong modificationMs = modification.toLongLong(&okay);

                if (okay)
                {
                    member.modificationTimeUtc =
                        QDateTime::fromMSecsSinceEpoch(modificationMs, Qt::UTC);
                }
            }

            if (!parsePortableAttributes(
                    memberObject.value(
                        QLatin1String("portableAttributes")).toObject(),
                    &member.portableAttributes) ||
                !member.isValid())
            {
                setError(error, PrivacyStrongRecoveryManifestError::Invalid);
                return false;
            }

            item.members << member;
        }

        if (!item.isValid())
        {
            setError(error, PrivacyStrongRecoveryManifestError::Invalid);
            return false;
        }

        decoded.items << item;
    }

    if (!decoded.isValid())
    {
        setError(error, PrivacyStrongRecoveryManifestError::Invalid);
        return false;
    }

    *manifest = decoded;
    setError(error, PrivacyStrongRecoveryManifestError::None);
    return true;
}

bool PrivacyStrongRecoveryManifestStore::load(
    const QString& vaultPlaintextRoot,
    PrivacyStrongRecoveryManifest* manifest,
    PrivacyStrongRecoveryManifestError* error)
{
    if (!manifest || vaultPlaintextRoot.isEmpty() ||
        !QFileInfo(vaultPlaintextRoot).isDir() ||
        QFileInfo(vaultPlaintextRoot).isSymLink())
    {
        setError(error, PrivacyStrongRecoveryManifestError::UnsafePath);
        return false;
    }

    const QString path = QDir(vaultPlaintextRoot).filePath(
        PrivacyStrongRecoveryManifestCodec::relativePath());
    const QFileInfo info(path);

    if (!info.exists())
    {
        setError(error, PrivacyStrongRecoveryManifestError::Unavailable);
        return false;
    }

    if (info.isSymLink() || !info.isFile() ||
        (info.size() <= 0) || (info.size() > MaximumManifestBytes))
    {
        setError(error, PrivacyStrongRecoveryManifestError::UnsafePath);
        return false;
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        setError(error, PrivacyStrongRecoveryManifestError::IoFailure);
        return false;
    }

    const QByteArray bytes = file.readAll();
    file.close();

    return PrivacyStrongRecoveryManifestCodec::decode(bytes, manifest, error);
}

bool PrivacyStrongRecoveryManifestStore::commit(
    const QString& vaultPlaintextRoot,
    const PrivacyStrongRecoveryManifest& manifest,
    PrivacyStrongRecoveryManifestError* error)
{
    if (vaultPlaintextRoot.isEmpty() ||
        !QFileInfo(vaultPlaintextRoot).isDir() ||
        QFileInfo(vaultPlaintextRoot).isSymLink())
    {
        setError(error, PrivacyStrongRecoveryManifestError::UnsafePath);
        return false;
    }

    const QByteArray bytes =
        PrivacyStrongRecoveryManifestCodec::encode(manifest, error);

    if (bytes.isEmpty())
    {
        return false;
    }

    const QString relative = PrivacyStrongRecoveryManifestCodec::relativePath();
    const QString directory = QDir(vaultPlaintextRoot).filePath(
        QFileInfo(relative).path());
    const QString finalPath = QDir(vaultPlaintextRoot).filePath(relative);
    const QString temporaryPath = finalPath + QLatin1String(".tmp");

    if (!QDir().mkpath(directory))
    {
        setError(error, PrivacyStrongRecoveryManifestError::IoFailure);
        return false;
    }

    if (QFileInfo(temporaryPath).isSymLink() ||
        QFileInfo(finalPath).isSymLink())
    {
        setError(error, PrivacyStrongRecoveryManifestError::UnsafePath);
        return false;
    }

    QFile temporary(temporaryPath);

    if (!temporary.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        setError(error, PrivacyStrongRecoveryManifestError::IoFailure);
        return false;
    }

    if ((temporary.write(bytes) != bytes.size()) || !temporary.flush())
    {
        temporary.close();
        QFile::remove(temporaryPath);
        setError(error, PrivacyStrongRecoveryManifestError::IoFailure);
        return false;
    }

    temporary.close();

    if (!fsyncFile(temporaryPath))
    {
        QFile::remove(temporaryPath);
        setError(error, PrivacyStrongRecoveryManifestError::IoFailure);
        return false;
    }

    bool renamed = false;

#ifdef Q_OS_UNIX
    renamed = (::rename(QFile::encodeName(temporaryPath).constData(),
                        QFile::encodeName(finalPath).constData()) == 0);
#else
    QFile::remove(finalPath);
    renamed = QFile::rename(temporaryPath, finalPath);
#endif

    if (!renamed || !fsyncDirectory(directory))
    {
        QFile::remove(temporaryPath);
        setError(error, PrivacyStrongRecoveryManifestError::IoFailure);
        return false;
    }

    setError(error, PrivacyStrongRecoveryManifestError::None);
    return true;
}

} // namespace Digikam
