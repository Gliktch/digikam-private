/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprofilepreflight.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace Digikam
{

namespace
{

const QLatin1String markerDirectoryName(".digikam-private");
const QLatin1String markerFileName("root-marker-v1.json");
const QLatin1String rootMarkerKind("digikam-private-root-marker-v1");

struct RootRecord
{
    QString uuid;
    QString configuredPath;
    QString markerUuid;
};

struct StoreRecord
{
    QString uuid;
    QString rootUuid;
    QString cipherRelativePath;
    QString configRelativePath;
};

struct ContainerRecord
{
    QString  uuid;
    QString  rootUuid;
    QString  storeUuid;
    QString  objectRelativePath;
    qlonglong protectedSize = -1;
};

bool verifyMarkerFile(const QString& path,
                      const QString& expectedMarkerUuid,
                      QString* const error)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        *error = QString::fromLatin1("missing managed-root marker: %1").arg(path);
        return false;
    }

    const QByteArray data = file.readAll();
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);

    if (!document.isObject() || (parseError.error != QJsonParseError::NoError))
    {
        *error = QString::fromLatin1("invalid managed-root marker: %1").arg(path);
        return false;
    }

    const QJsonObject object = document.object();

    if ((object.value(QLatin1String("kind")).toString() != rootMarkerKind) ||
        (!expectedMarkerUuid.isEmpty() &&
         (object.value(QLatin1String("markerUuid")).toString() != expectedMarkerUuid)))
    {
        *error = QString::fromLatin1("managed-root marker identity mismatch: %1").arg(path);
        return false;
    }

    return true;
}

bool verifyRegularFileWithSize(const QString& path,
                               qlonglong expectedSize,
                               const QLatin1String& missingMessage,
                               const QLatin1String& sizeMessage,
                               QString* const error)
{
    const QFileInfo info(path);

    if (!info.exists() || !info.isFile() || !info.isReadable())
    {
        *error = missingMessage + path;
        return false;
    }

    if ((expectedSize >= 0) && (info.size() != expectedSize))
    {
        *error = sizeMessage + path;
        return false;
    }

    return true;
}

QString markerPathForRoot(const QString& configuredPath)
{
    return QDir(configuredPath).filePath(
        QDir(markerDirectoryName).filePath(markerFileName));
}

} // namespace

PrivacyProfilePreflightResult PrivacyProfilePreflight::verify(
    const QString& p1DatabasePath)
{
    PrivacyProfilePreflightResult result;
    const QFileInfo databaseInfo(p1DatabasePath);

    if (!databaseInfo.exists() || !databaseInfo.isFile() || !databaseInfo.isReadable())
    {
        result.error = QString::fromLatin1(
            "the P1 database is not readable: %1").arg(p1DatabasePath);
        return result;
    }

    const QString connectionName = QLatin1String("privacy-profile-preflight-") +
                                   QUuid::createUuid().toString(QUuid::WithoutBraces);

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"),
                                                          connectionName);
        database.setConnectOptions(QLatin1String("QSQLITE_OPEN_READONLY"));
        database.setDatabaseName(p1DatabasePath);

        if (!database.open())
        {
            result.error = database.lastError().text();
        }
        else
        {
            QHash<QString, RootRecord> roots;
            QSqlQuery query(database);

            if (query.exec(QLatin1String(
                    "SELECT uuid, configuredPath, markerUuid "
                    "FROM PrivacyStorageRoots ORDER BY uuid;")))
            {
                while (query.next())
                {
                    RootRecord record;
                    record.uuid = query.value(0).toString();
                    record.configuredPath = query.value(1).toString();
                    record.markerUuid = query.value(2).toString();
                    roots.insert(record.uuid, record);
                }
            }
            else
            {
                result.error = QString::fromLatin1(
                    "the P1 database has no readable privacy storage roots: %1")
                                   .arg(query.lastError().text());
            }

            for (auto it = roots.cbegin(); result.error.isEmpty() && (it != roots.cend()); ++it)
            {
                QString markerError;

                if (verifyMarkerFile(markerPathForRoot(it.value().configuredPath),
                                     it.value().markerUuid,
                                     &markerError))
                {
                    ++result.verifiedRootCount;
                }
                else
                {
                    ++result.failedRootCount;
                    result.error = markerError;
                }
            }

            QHash<QString, StoreRecord> stores;

            if (result.error.isEmpty() &&
                query.exec(QLatin1String(
                    "SELECT uuid, rootUuid, cipherRelativePath, configRelativePath "
                    "FROM PrivacyStores ORDER BY uuid;")))
            {
                while (query.next())
                {
                    StoreRecord record;
                    record.uuid = query.value(0).toString();
                    record.rootUuid = query.value(1).toString();
                    record.cipherRelativePath = query.value(2).toString();
                    record.configRelativePath = query.value(3).toString();
                    stores.insert(record.uuid, record);
                }
            }
            else if (result.error.isEmpty())
            {
                result.error = QString::fromLatin1(
                    "the P1 database has no readable privacy stores: %1")
                                   .arg(query.lastError().text());
            }

            for (auto it = stores.cbegin(); result.error.isEmpty() && (it != stores.cend()); ++it)
            {
                const RootRecord root = roots.value(it.value().rootUuid);

                if (root.uuid.isEmpty())
                {
                    ++result.failedStoreCount;
                    result.error = QString::fromLatin1(
                        "privacy store references an unknown root: %1")
                                       .arg(it.value().uuid);
                    break;
                }

                const QString cipherDirectory = QDir(root.configuredPath).filePath(
                    it.value().cipherRelativePath);
                const QFileInfo directoryInfo(cipherDirectory);

                if (!directoryInfo.exists() || !directoryInfo.isDir())
                {
                    ++result.failedStoreCount;
                    result.error = QString::fromLatin1(
                        "missing store directory: %1").arg(cipherDirectory);
                    break;
                }

                if (!it.value().configRelativePath.isEmpty())
                {
                    const QString configPath = QDir(cipherDirectory).filePath(
                        it.value().configRelativePath);
                    const QFileInfo configInfo(configPath);

                    if (!configInfo.exists() || !configInfo.isFile())
                    {
                        ++result.failedStoreCount;
                        result.error = QString::fromLatin1(
                            "missing store configuration: %1").arg(configPath);
                        break;
                    }
                }

                ++result.verifiedStoreCount;
            }

            QHash<QString, ContainerRecord> containers;

            if (result.error.isEmpty() &&
                query.exec(QLatin1String(
                    "SELECT uuid, rootUuid, storeUuid, objectRelativePath, protectedSize "
                    "FROM PrivacyContainers ORDER BY uuid;")))
            {
                while (query.next())
                {
                    ContainerRecord record;
                    record.uuid = query.value(0).toString();
                    record.rootUuid = query.value(1).toString();
                    record.storeUuid = query.value(2).toString();
                    record.objectRelativePath = query.value(3).toString();
                    record.protectedSize = query.value(4).toLongLong();
                    containers.insert(record.uuid, record);
                }
            }
            else if (result.error.isEmpty())
            {
                result.error = QString::fromLatin1(
                    "the P1 database has no readable privacy containers: %1")
                                   .arg(query.lastError().text());
            }

            for (auto it = containers.cbegin(); result.error.isEmpty() && (it != containers.cend()); ++it)
            {
                QString objectPath;

                if (!it.value().storeUuid.isEmpty())
                {
                    const StoreRecord store = stores.value(it.value().storeUuid);

                    if (store.uuid.isEmpty())
                    {
                        ++result.failedContainerCount;
                        result.error = QString::fromLatin1(
                            "protected object references an unknown store: %1")
                                           .arg(it.value().uuid);
                        break;
                    }

                    const RootRecord root = roots.value(store.rootUuid);
                    objectPath = QDir(root.configuredPath).filePath(
                        store.cipherRelativePath + QLatin1Char('/') +
                        it.value().objectRelativePath);
                }
                else if (!it.value().rootUuid.isEmpty())
                {
                    const RootRecord root = roots.value(it.value().rootUuid);
                    objectPath = QDir(root.configuredPath).filePath(
                        it.value().objectRelativePath);
                }
                else
                {
                    ++result.failedContainerCount;
                    result.error = QString::fromLatin1(
                        "protected object has no root or store reference: %1")
                                       .arg(it.value().uuid);
                    break;
                }

                if (verifyRegularFileWithSize(
                        objectPath, it.value().protectedSize,
                        QLatin1String("missing protected object: "),
                        QLatin1String("protected object size mismatch: "),
                        &result.error))
                {
                    ++result.verifiedContainerCount;
                }
                else
                {
                    ++result.failedContainerCount;
                    break;
                }
            }

            if (result.error.isEmpty() &&
                query.exec(QLatin1String(
                    "SELECT asset.publicRelativePath, asset.proxySize, root.configuredPath "
                    "FROM PrivacyAssets AS asset "
                    "JOIN PrivacyStorageRoots AS root ON (root.uuid = asset.publicRootUuid) "
                    "WHERE asset.proxySize > 0;")))
            {
                while (query.next())
                {
                    const QString proxyPath = QDir(query.value(2).toString()).filePath(
                        query.value(0).toString());
                    const qlonglong proxySize = query.value(1).toLongLong();

                    if (verifyRegularFileWithSize(
                            proxyPath, proxySize,
                            QLatin1String("missing public proxy: "),
                            QLatin1String("public proxy size mismatch: "),
                            &result.error))
                    {
                        ++result.verifiedContainerCount;
                    }
                    else
                    {
                        ++result.failedContainerCount;
                        break;
                    }
                }
            }
            else if (result.error.isEmpty())
            {
                result.error = QString::fromLatin1(
                    "the P1 database has no readable privacy assets: %1")
                                   .arg(query.lastError().text());
            }

            database.close();
        }
    }

    QSqlDatabase::removeDatabase(connectionName);
    result.success = result.error.isEmpty();
    return result;
}

} // namespace Digikam
