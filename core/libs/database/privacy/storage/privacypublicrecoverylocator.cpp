/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacypublicrecoverylocator.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <QStringList>
#include <QUuid>

// C++ includes

#include <algorithm>

namespace Digikam
{

namespace
{

const QString LocatorFormat = QLatin1String("digikam-private-recovery-locator");
const QString MetadataDirectory = QLatin1String(".digikam-private");
constexpr int LocatorFormatVersion = 1;
constexpr int MaximumPlaceholderIdentityBytes = 64;
constexpr int MaximumEntryCount = 4096;
constexpr qsizetype MaximumLocatorBytes = 8 * 1024 * 1024;

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() &&
            (uuid == parsed.toString(QUuid::WithoutBraces)));
}

bool isValidRelativePublicPath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) ||
        path.contains(QLatin1Char('\0')))
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

bool backendName(PrivacyBackend backend, QString* name)
{
    if (!name)
    {
        return false;
    }

    switch (backend)
    {
        case PrivacyBackend::Casual:
            *name = QLatin1String("casual");
            return true;
        case PrivacyBackend::Strong:
            *name = QLatin1String("strong");
            return true;
    }

    return false;
}

bool backendFromName(const QString& name, PrivacyBackend* backend)
{
    if (!backend)
    {
        return false;
    }

    if (name == QLatin1String("casual"))
    {
        *backend = PrivacyBackend::Casual;
        return true;
    }

    if (name == QLatin1String("strong"))
    {
        *backend = PrivacyBackend::Strong;
        return true;
    }

    return false;
}

} // namespace

bool PrivacyPublicRecoveryLocatorEntry::isValid() const
{
    const bool validBackend =
        ((backend == PrivacyBackend::Casual) ||
         (backend == PrivacyBackend::Strong));
    const bool validPlaceholderSize = (expectedPlaceholderSize >= 0);
    const bool validPlaceholderHash =
        (expectedPlaceholderSha256.size() == 32);
    const bool validPlaceholderIdentity =
        ((placeholderIdentity == QLatin1String("generic-v1")) ||
         (placeholderIdentity == QLatin1String("blur-v1")));

    return (isCanonicalUuid(recoverySetUuid) &&
            validBackend &&
            isValidRelativePublicPath(publicRelativePath) &&
            validPlaceholderIdentity &&
            (placeholderIdentity.toUtf8().size() <=
             MaximumPlaceholderIdentityBytes) &&
            validPlaceholderSize &&
            validPlaceholderHash);
}

QString PrivacyPublicRecoveryLocatorCodec::relativePath()
{
    return QLatin1String(".digikam-private/recovery-locator-v1.json");
}

QByteArray PrivacyPublicRecoveryLocatorCodec::encode(
    const QList<PrivacyPublicRecoveryLocatorEntry>& entries,
    PrivacyPublicRecoveryLocatorError* const error)
{
    const auto fail = [error](PrivacyPublicRecoveryLocatorError value)
    {
        if (error)
        {
            *error = value;
        }

        return QByteArray();
    };

    if (error)
    {
        *error = PrivacyPublicRecoveryLocatorError::None;
    }

    if (entries.size() > MaximumEntryCount)
    {
        return fail(PrivacyPublicRecoveryLocatorError::Invalid);
    }

    QJsonArray jsonEntries;

    for (const PrivacyPublicRecoveryLocatorEntry& entry : entries)
    {
        if (!entry.isValid())
        {
            return fail(PrivacyPublicRecoveryLocatorError::Invalid);
        }

        QString backend;

        if (!backendName(entry.backend, &backend))
        {
            return fail(PrivacyPublicRecoveryLocatorError::Invalid);
        }

        QJsonObject object;
        object.insert(QLatin1String("recoverySetUuid"), entry.recoverySetUuid);
        object.insert(QLatin1String("backend"), backend);
        object.insert(QLatin1String("publicRelativePath"),
                      entry.publicRelativePath);
        object.insert(QLatin1String("placeholderIdentity"),
                      entry.placeholderIdentity);
        object.insert(QLatin1String("expectedPlaceholderSize"),
                      QString::number(entry.expectedPlaceholderSize));
        object.insert(QLatin1String("expectedPlaceholderSha256"),
                      QString::fromLatin1(
                          entry.expectedPlaceholderSha256.toHex()));
        jsonEntries.append(object);
    }

    QJsonObject root;
    root.insert(QLatin1String("format"), LocatorFormat);
    root.insert(QLatin1String("formatVersion"), LocatorFormatVersion);
    root.insert(QLatin1String("entries"), jsonEntries);
    const QByteArray bytes =
        QJsonDocument(root).toJson(QJsonDocument::Compact);

    if (bytes.isEmpty() || (bytes.size() > MaximumLocatorBytes))
    {
        return fail(PrivacyPublicRecoveryLocatorError::Invalid);
    }

    return bytes;
}

bool PrivacyPublicRecoveryLocatorCodec::decode(
    const QByteArray& bytes,
    QList<PrivacyPublicRecoveryLocatorEntry>* const entries,
    PrivacyPublicRecoveryLocatorError* const error)
{
    const auto fail = [error](PrivacyPublicRecoveryLocatorError value)
    {
        if (error)
        {
            *error = value;
        }

        return false;
    };

    if (error)
    {
        *error = PrivacyPublicRecoveryLocatorError::None;
    }

    if (!entries || bytes.isEmpty() || (bytes.size() > MaximumLocatorBytes))
    {
        return fail(PrivacyPublicRecoveryLocatorError::Invalid);
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(bytes, &parseError);

    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject())
    {
        return fail(PrivacyPublicRecoveryLocatorError::Invalid);
    }

    const QJsonObject root = document.object();

    if ((root.value(QLatin1String("format")).toString() != LocatorFormat) ||
        (root.value(QLatin1String("formatVersion")).toInt() !=
         LocatorFormatVersion))
    {
        return fail(PrivacyPublicRecoveryLocatorError::Invalid);
    }

    const QJsonArray jsonEntries =
        root.value(QLatin1String("entries")).toArray();

    if (jsonEntries.size() > MaximumEntryCount)
    {
        return fail(PrivacyPublicRecoveryLocatorError::Invalid);
    }

    QList<PrivacyPublicRecoveryLocatorEntry> decoded;
    QSet<QString> seenPaths;

    for (const QJsonValue& value : jsonEntries)
    {
        if (!value.isObject())
        {
            return fail(PrivacyPublicRecoveryLocatorError::Invalid);
        }

        const QJsonObject object = value.toObject();
        PrivacyPublicRecoveryLocatorEntry entry;
        entry.recoverySetUuid =
            object.value(QLatin1String("recoverySetUuid")).toString();

        if (!backendFromName(
                object.value(QLatin1String("backend")).toString(),
                &entry.backend))
        {
            return fail(PrivacyPublicRecoveryLocatorError::Invalid);
        }

        entry.publicRelativePath =
            object.value(QLatin1String("publicRelativePath")).toString();
        entry.placeholderIdentity =
            object.value(QLatin1String("placeholderIdentity")).toString();

        bool sizeOkay = false;
        entry.expectedPlaceholderSize =
            object.value(QLatin1String("expectedPlaceholderSize"))
                .toString().toLongLong(&sizeOkay);
        const QByteArray hashHex =
            object.value(QLatin1String("expectedPlaceholderSha256"))
                .toString().toLatin1();
        entry.expectedPlaceholderSha256 =
            QByteArray::fromHex(hashHex);

        if (!entry.isValid() ||
            (entry.expectedPlaceholderSha256.toHex() != hashHex) ||
            !sizeOkay ||
            seenPaths.contains(entry.publicRelativePath))
        {
            return fail(PrivacyPublicRecoveryLocatorError::Invalid);
        }

        seenPaths.insert(entry.publicRelativePath);
        decoded << entry;
    }

    *entries = decoded;
    return true;
}

bool PrivacyPublicRecoveryLocatorStore::load(
    const QString& collectionRoot,
    QList<PrivacyPublicRecoveryLocatorEntry>* const entries,
    PrivacyPublicRecoveryLocatorError* const error)
{
    const auto fail = [error](PrivacyPublicRecoveryLocatorError value)
    {
        if (error)
        {
            *error = value;
        }

        return false;
    };

    if (error)
    {
        *error = PrivacyPublicRecoveryLocatorError::None;
    }

    if (!entries)
    {
        return fail(PrivacyPublicRecoveryLocatorError::Invalid);
    }

    entries->clear();
    const QFileInfo rootInfo(collectionRoot);

    if (!rootInfo.isDir() || rootInfo.isSymLink())
    {
        return fail(PrivacyPublicRecoveryLocatorError::UnsafePath);
    }

    const QString path = QDir(collectionRoot).filePath(
        PrivacyPublicRecoveryLocatorCodec::relativePath());
    const QFileInfo info(path);

    if (!info.exists())
    {
        return true;
    }

    if (!info.isFile() || info.isSymLink() ||
        (info.size() > MaximumLocatorBytes))
    {
        return fail(PrivacyPublicRecoveryLocatorError::UnsafePath);
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return fail(PrivacyPublicRecoveryLocatorError::IoFailure);
    }

    return PrivacyPublicRecoveryLocatorCodec::decode(
        file.readAll(), entries, error);
}

bool PrivacyPublicRecoveryLocatorStore::commit(
    const QString& collectionRoot,
    const QList<PrivacyPublicRecoveryLocatorEntry>& entries,
    PrivacyPublicRecoveryLocatorError* const error)
{
    const auto fail = [error](PrivacyPublicRecoveryLocatorError value)
    {
        if (error)
        {
            *error = value;
        }

        return false;
    };

    if (error)
    {
        *error = PrivacyPublicRecoveryLocatorError::None;
    }

    const QFileInfo rootInfo(collectionRoot);

    if (!rootInfo.isDir() || rootInfo.isSymLink())
    {
        return fail(PrivacyPublicRecoveryLocatorError::UnsafePath);
    }

    const QString metadataDirectory =
        QDir(collectionRoot).filePath(MetadataDirectory);
    const QFileInfo metadataInfo(metadataDirectory);

    if (metadataInfo.exists() &&
        (!metadataInfo.isDir() || metadataInfo.isSymLink()))
    {
        return fail(PrivacyPublicRecoveryLocatorError::UnsafePath);
    }

    if (!metadataInfo.exists())
    {
        if (!QDir().mkpath(metadataDirectory))
        {
            return fail(PrivacyPublicRecoveryLocatorError::IoFailure);
        }

        QFile::setPermissions(metadataDirectory,
                              QFileDevice::ReadOwner |
                              QFileDevice::WriteOwner |
                              QFileDevice::ExeOwner);
    }

    const QByteArray bytes =
        PrivacyPublicRecoveryLocatorCodec::encode(entries, error);

    if (bytes.isEmpty())
    {
        return false;
    }

    const QString fileName =
        QString::fromLatin1("recovery-locator-v1.json");
    QSaveFile file(QDir(metadataDirectory).filePath(fileName));
    file.setDirectWriteFallback(false);

    if (!file.open(QIODevice::WriteOnly))
    {
        return fail(PrivacyPublicRecoveryLocatorError::IoFailure);
    }

    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    if ((file.write(bytes) != bytes.size()) || !file.commit())
    {
        return fail(PrivacyPublicRecoveryLocatorError::IoFailure);
    }

    return true;
}

bool PrivacyPublicRecoveryLocatorMaintenance::recordProtectedProxy(
    const PrivacyStorageRoot& publicRoot,
    const PrivacyItem& item,
    const PrivacyCategory& category,
    const PrivacyAsset& primaryAsset,
    QString* const error)
{
    const auto fail = [error](const QString& detail)
    {
        if (error)
        {
            *error = detail;
        }

        return false;
    };

    if (!publicRoot.isValid() ||
        (publicRoot.kind != PrivacyStorageRootKind::AlbumRoot) ||
        !item.isValid() || !category.isValid() ||
        (category.recoverySetUuid.isEmpty()) ||
        !primaryAsset.isValid() ||
        (primaryAsset.role != PrivacyAsset::PrimaryMediaRole) ||
        (primaryAsset.ordinal != 0) ||
        (primaryAsset.publicRootUuid != publicRoot.uuid) ||
        (item.expectedProxySize < 0) || item.expectedProxyHash.isEmpty())
    {
        return fail(QStringLiteral("invalid protected proxy locator facts"));
    }

    const QByteArray proxySha256 =
        QByteArray::fromHex(item.expectedProxyHash.toLatin1());

    if (proxySha256.size() != 32)
    {
        return fail(QStringLiteral("invalid protected proxy hash"));
    }

    const QString placeholderIdentity =
        (category.presentationMode == PrivacyPresentationMode::Blur)
            ? QLatin1String("blur-v1")
            : QLatin1String("generic-v1");
    QList<PrivacyPublicRecoveryLocatorEntry> entries;
    PrivacyPublicRecoveryLocatorError locatorError =
        PrivacyPublicRecoveryLocatorError::None;

    if (!PrivacyPublicRecoveryLocatorStore::load(
            publicRoot.configuredPath, &entries, &locatorError))
    {
        return fail(QStringLiteral("recovery locator could not be read"));
    }

    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                       [&primaryAsset](const PrivacyPublicRecoveryLocatorEntry& entry)
                       {
                           return (entry.publicRelativePath ==
                                   primaryAsset.publicRelativePath);
                       }),
        entries.end());

    PrivacyPublicRecoveryLocatorEntry entry;
    entry.recoverySetUuid = category.recoverySetUuid;
    entry.backend = category.backend;
    entry.publicRelativePath = primaryAsset.publicRelativePath;
    entry.placeholderIdentity = placeholderIdentity;
    entry.expectedPlaceholderSize = item.expectedProxySize;
    entry.expectedPlaceholderSha256 = proxySha256;

    if (!entry.isValid())
    {
        return fail(QStringLiteral("recovery locator entry is invalid"));
    }

    entries << entry;

    if (!PrivacyPublicRecoveryLocatorStore::commit(
            publicRoot.configuredPath, entries, &locatorError))
    {
        return fail(QStringLiteral("recovery locator could not be written"));
    }

    return true;
}

bool PrivacyPublicRecoveryLocatorMaintenance::removePublicPaths(
    const PrivacyStorageRoot& publicRoot,
    const QStringList& publicRelativePaths,
    QString* const error)
{
    const auto fail = [error](const QString& detail)
    {
        if (error)
        {
            *error = detail;
        }

        return false;
    };

    if (!publicRoot.isValid() ||
        (publicRoot.kind != PrivacyStorageRootKind::AlbumRoot))
    {
        return fail(QStringLiteral("invalid public root"));
    }

    if (publicRelativePaths.isEmpty())
    {
        return true;
    }

    QList<PrivacyPublicRecoveryLocatorEntry> entries;
    PrivacyPublicRecoveryLocatorError locatorError =
        PrivacyPublicRecoveryLocatorError::None;

    if (!PrivacyPublicRecoveryLocatorStore::load(
            publicRoot.configuredPath, &entries, &locatorError))
    {
        return fail(QStringLiteral("recovery locator could not be read"));
    }

    QSet<QString> removed;

    for (const QString& path : publicRelativePaths)
    {
        removed.insert(path);
    }

    const int previousCount = entries.size();
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                       [&removed](const PrivacyPublicRecoveryLocatorEntry& entry)
                       {
                           return removed.contains(entry.publicRelativePath);
                       }),
        entries.end());

    if (entries.size() == previousCount)
    {
        return true;
    }

    if (!PrivacyPublicRecoveryLocatorStore::commit(
            publicRoot.configuredPath, entries, &locatorError))
    {
        return fail(QStringLiteral("recovery locator could not be written"));
    }

    return true;
}

bool PrivacyPublicRecoveryLocatorMaintenance::retargetProxy(
    const PrivacyStorageRoot& publicRoot,
    const QString& publicRelativePath,
    const QString& newRecoverySetUuid,
    PrivacyBackend newBackend,
    QString* const error)
{
    const auto fail = [error](const QString& detail)
    {
        if (error)
        {
            *error = detail;
        }

        return false;
    };

    if (!publicRoot.isValid() ||
        (publicRoot.kind != PrivacyStorageRootKind::AlbumRoot) ||
        publicRelativePath.isEmpty() ||
        !isCanonicalUuid(newRecoverySetUuid) ||
        ((newBackend != PrivacyBackend::Casual) &&
         (newBackend != PrivacyBackend::Strong)))
    {
        return fail(QStringLiteral("invalid proxy retarget facts"));
    }

    QList<PrivacyPublicRecoveryLocatorEntry> entries;
    PrivacyPublicRecoveryLocatorError locatorError =
        PrivacyPublicRecoveryLocatorError::None;

    if (!PrivacyPublicRecoveryLocatorStore::load(
            publicRoot.configuredPath, &entries, &locatorError))
    {
        return fail(QStringLiteral("recovery locator could not be read"));
    }

    bool changed = false;

    for (PrivacyPublicRecoveryLocatorEntry& entry : entries)
    {
        if (entry.publicRelativePath == publicRelativePath)
        {
            entry.recoverySetUuid = newRecoverySetUuid;
            entry.backend = newBackend;
            changed = true;

            if (!entry.isValid())
            {
                return fail(QStringLiteral("recovery locator entry is invalid"));
            }
        }
    }

    if (!changed)
    {
        return true;
    }

    if (!PrivacyPublicRecoveryLocatorStore::commit(
            publicRoot.configuredPath, entries, &locatorError))
    {
        return fail(QStringLiteral("recovery locator could not be written"));
    }

    return true;
}

} // namespace Digikam
