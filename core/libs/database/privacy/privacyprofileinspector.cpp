/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprofileinspector.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

// KDE includes

#include <KConfigGroup>
#include <KSharedConfig>

namespace Digikam
{

namespace
{

QString sqliteFilePath(const QString& configuredPath, const QString& fileName)
{
    const QFileInfo configured(configuredPath);

    if (!configured.isFile() &&
        !configuredPath.endsWith(QLatin1String(".db"), Qt::CaseInsensitive))
    {
        return QDir(configuredPath).filePath(fileName);
    }

    return configured.absoluteFilePath();
}

QVariant singleValue(QSqlDatabase& database, const QString& statement)
{
    QSqlQuery query(database);

    if (query.exec(statement) && query.next())
    {
        return query.value(0);
    }

    return QVariant();
}

bool hasCompletePrivacySchema(const QSqlDatabase& database)
{
    const QStringList required = {
        QLatin1String("PrivacyCategories"),
        QLatin1String("PrivacyCredentials"),
        QLatin1String("PrivacyStorageRoots"),
        QLatin1String("PrivacyStores"),
        QLatin1String("PrivacyStoreBindings"),
        QLatin1String("PrivacyItems"),
        QLatin1String("PrivacyContainers"),
        QLatin1String("PrivacyAssets"),
        QLatin1String("PrivacyDerivatives"),
        QLatin1String("PrivacyTransactions"),
        QLatin1String("PrivacyTransactionJournals")
    };
    const QStringList tables = database.tables();

    for (const QString& table : required)
    {
        if (!tables.contains(table, Qt::CaseInsensitive))
        {
            return false;
        }
    }

    return true;
}

void addFileFacts(const QString& path, qint64* bytes, QDateTime* latest)
{
    const QFileInfo info(path);

    if (!info.exists() || !info.isFile())
    {
        return;
    }

    *bytes += info.size();

    if (!latest->isValid() || (info.lastModified() > *latest))
    {
        *latest = info.lastModified();
    }
}

} // namespace

bool PrivacyProfileSummary::isUsable() const
{
    return integrityOk &&
           ((schemaKind == PrivacyProfileSchemaKind::Stock) ||
            (schemaKind == PrivacyProfileSchemaKind::PrivateP1));
}

bool PrivacyProfileSummary::isPrivateProfile() const
{
    return (schemaKind == PrivacyProfileSchemaKind::PrivateP1);
}

QString PrivacyProfileInspector::defaultStockSettingsPath()
{
    return QDir::home().filePath(QLatin1String(".config/digikamrc"));
}

PrivacyProfileSummary PrivacyProfileInspector::inspectSettingsFile(const QString& settingsPath)
{
    PrivacyProfileSummary result;
    const QFileInfo settingsInfo(settingsPath);
    result.settingsPath = settingsInfo.absoluteFilePath();

    if (!settingsInfo.exists() || !settingsInfo.isFile() || !settingsInfo.isReadable())
    {
        result.error = QLatin1String("The selected digiKam settings file is not readable");
        return result;
    }

    const KSharedConfig::Ptr config = KSharedConfig::openConfig(
        result.settingsPath, KConfig::SimpleConfig);
    const KConfigGroup group(config, QLatin1String("Database Settings"));
    const QString databaseType = group.readEntry(QLatin1String("Database Type"), QString());

    if (databaseType != QLatin1String("QSQLITE"))
    {
        result.schemaKind = PrivacyProfileSchemaKind::Unsupported;
        result.databaseType = databaseType;
        result.error = databaseType.isEmpty()
                     ? QLatin1String("The settings file does not identify a database backend")
                     : QLatin1String("This database backend is not supported by the v1 profile importer");
        return result;
    }

    const QString coreSetting = group.readPathEntry(QLatin1String("Database Name"), QString());
    const QString thumbsSetting = group.readPathEntry(
        QLatin1String("Database Name Thumbnails"), coreSetting);

    if (coreSetting.isEmpty())
    {
        result.error = QLatin1String("The settings file does not contain a database location");
        return result;
    }

    result = inspectCoreDatabase(
        sqliteFilePath(coreSetting, QLatin1String("digikam4.db")),
        settingsInfo.absoluteFilePath(),
        sqliteFilePath(thumbsSetting, QLatin1String("thumbnails-digikam.db")));

    return result;
}

PrivacyProfileSummary PrivacyProfileInspector::inspectDatabaseFolder(const QString& databaseFolder)
{
    const QDir directory(databaseFolder);

    if (!directory.exists())
    {
        PrivacyProfileSummary result;
        result.error = QLatin1String("The selected database folder does not exist");
        return result;
    }

    return inspectCoreDatabase(
        directory.filePath(QLatin1String("digikam4.db")),
        QString(),
        directory.filePath(QLatin1String("thumbnails-digikam.db")));
}

PrivacyProfileSummary PrivacyProfileInspector::inspectCoreDatabase(
    const QString& databasePath,
    const QString& settingsPath,
    const QString& thumbnailDatabasePath)
{
    PrivacyProfileSummary result;
    result.settingsPath = settingsPath;
    result.databasePath = QFileInfo(databasePath).absoluteFilePath();
    result.thumbnailDatabasePath = thumbnailDatabasePath;
    result.databaseType = QLatin1String("SQLite");

    const QFileInfo databaseInfo(result.databasePath);

    if (!databaseInfo.exists() || !databaseInfo.isFile() || !databaseInfo.isReadable())
    {
        result.error = QLatin1String("The digiKam database is not readable");
        return result;
    }

    const QString connectionName = QLatin1String("privacy-profile-inspect-") +
                                   QUuid::createUuid().toString(QUuid::WithoutBraces);

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
        database.setConnectOptions(QLatin1String("QSQLITE_OPEN_READONLY"));
        database.setDatabaseName(result.databasePath);

        if (!database.open())
        {
            result.error = database.lastError().text();
        }
        else
        {
            const QVariant version = singleValue(
                database,
                QLatin1String("SELECT value FROM Settings WHERE keyword='DBVersion';"));

            if (!version.isValid())
            {
                result.error = QLatin1String("The database has no schema version");
            }
            else
            {
                result.schemaVersion = version.toInt();
                result.versionLabel = versionLabel(result.schemaVersion);
                const QString flavor = singleValue(
                    database,
                    QLatin1String("SELECT value FROM Settings WHERE keyword='DBSchemaFlavor';"))
                                           .toString();
                const int flavorVersion = singleValue(
                    database,
                    QLatin1String("SELECT value FROM Settings WHERE keyword='DBSchemaFlavorVersion';"))
                                                 .toInt();
                const int baseVersion = singleValue(
                    database,
                    QLatin1String("SELECT value FROM Settings WHERE keyword='DBSchemaBaseVersion';"))
                                               .toInt();

                if ((result.schemaVersion == 101) &&
                    (flavor == QLatin1String("digikam-private")) &&
                    (flavorVersion == 1) &&
                    (baseVersion == 17) &&
                    hasCompletePrivacySchema(database))
                {
                    result.schemaKind = PrivacyProfileSchemaKind::PrivateP1;
                }
                else if ((result.schemaVersion >= 4) && (result.schemaVersion <= 17) &&
                         flavor.isEmpty())
                {
                    result.schemaKind = PrivacyProfileSchemaKind::Stock;
                }
                else
                {
                    result.schemaKind = PrivacyProfileSchemaKind::Unsupported;
                    result.error = QLatin1String("The database schema is unsupported or ambiguous");
                }

                if (result.schemaKind != PrivacyProfileSchemaKind::Unsupported)
                {
                    result.activeItemCount = singleValue(
                        database,
                        QLatin1String("SELECT COUNT(*) FROM Images WHERE status=1;"))
                                                     .toLongLong();
                    result.totalItemCount = singleValue(
                        database,
                        QLatin1String("SELECT COUNT(*) FROM Images;"))
                                                    .toLongLong();

                    QSqlQuery roots(database);

                    if (roots.exec(QLatin1String(
                            "SELECT specificPath FROM AlbumRoots WHERE status=0 ORDER BY id;")))
                    {
                        while (roots.next())
                        {
                            result.collectionRoots << roots.value(0).toString();
                        }
                    }

                    if (result.schemaKind == PrivacyProfileSchemaKind::PrivateP1)
                    {
                        result.protectedItemCount = singleValue(
                            database,
                            QLatin1String("SELECT COUNT(*) FROM PrivacyItems;"))
                                                         .toLongLong();
                        result.privacyCategoryCount = singleValue(
                            database,
                            QLatin1String("SELECT COUNT(*) FROM PrivacyCategories;"))
                                                         .toInt();
                        result.incompletePrivacyTransactionCount = singleValue(
                            database,
                            QLatin1String("SELECT COUNT(*) FROM PrivacyTransactions "
                                          "WHERE state<>7;"))
                                                         .toInt();
                    }

                    const QString check = singleValue(
                        database, QLatin1String("PRAGMA quick_check(1);"))
                                              .toString();
                    result.integrityOk = (check == QLatin1String("ok"));

                    if (!result.integrityOk)
                    {
                        result.error = QLatin1String("The database integrity check failed");
                    }
                }
            }

            database.close();
        }
    }

    QSqlDatabase::removeDatabase(connectionName);

    addFileFacts(result.databasePath, &result.databaseBytes, &result.latestModification);
    addFileFacts(result.databasePath + QLatin1String("-wal"),
                 &result.databaseBytes, &result.latestModification);
    addFileFacts(result.databasePath + QLatin1String("-shm"),
                 &result.databaseBytes, &result.latestModification);

    return result;
}

QString PrivacyProfileInspector::versionLabel(int schemaVersion)
{
    switch (schemaVersion)
    {
        case 101:
            return QLatin1String("digiKam Private P1");
        case 17:
            return QLatin1String("digiKam 9.1");
        case 16:
            return QLatin1String("digiKam 9.0");
        case 15:
            return QLatin1String("digiKam 7.10");
        default:
            return (schemaVersion >= 4) && (schemaVersion <= 14)
                 ? QLatin1String("digiKam <7.5")
                 : QLatin1String("Unknown");
    }
}

} // namespace Digikam
