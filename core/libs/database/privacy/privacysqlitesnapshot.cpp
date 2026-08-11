/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacysqlitesnapshot.h"

// SQLite includes

#include <sqlite3.h>

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QUuid>

namespace Digikam
{

namespace
{

QString sqliteError(sqlite3* database, const QString& fallback)
{
    return database ? QString::fromUtf8(sqlite3_errmsg(database)) : fallback;
}

bool passesIntegrityCheck(sqlite3* database, QString* error)
{
    sqlite3_stmt* statement = nullptr;
    const int prepareResult = sqlite3_prepare_v2(
        database, "PRAGMA integrity_check(1);", -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        *error = sqliteError(database, QLatin1String("Could not validate the database snapshot"));
        return false;
    }

    const int stepResult = sqlite3_step(statement);
    const bool valid = (stepResult == SQLITE_ROW) &&
                       (QString::fromUtf8(reinterpret_cast<const char*>(
                            sqlite3_column_text(statement, 0))) == QLatin1String("ok"));
    sqlite3_finalize(statement);

    if (!valid)
    {
        *error = QLatin1String("The completed database snapshot failed its integrity check");
    }

    return valid;
}

} // namespace

PrivacySqliteSnapshotResult PrivacySqliteSnapshot::create(
    const QString& sourcePath,
    const QString& destinationPath,
    const Progress& progress,
    const IsCanceled& isCanceled)
{
    PrivacySqliteSnapshotResult result;
    const QFileInfo sourceInfo(sourcePath);
    const QFileInfo destinationInfo(destinationPath);

    if (!sourceInfo.exists() || !sourceInfo.isFile() || !sourceInfo.isReadable())
    {
        result.error = QLatin1String("The source SQLite database is not readable");
        return result;
    }

    if (destinationInfo.exists())
    {
        result.error = QLatin1String("The snapshot destination already exists");
        return result;
    }

    if (!QDir().mkpath(destinationInfo.absolutePath()))
    {
        result.error = QLatin1String("The snapshot destination directory could not be created");
        return result;
    }

    const QString partialPath = destinationPath + QLatin1String(".partial-") +
                                QUuid::createUuid().toString(QUuid::WithoutBraces);
    sqlite3* source = nullptr;
    sqlite3* destination = nullptr;
    sqlite3_backup* backup = nullptr;
    int busyAttempts = 0;
    int backupResult = SQLITE_ERROR;

    const QByteArray sourceName = QFile::encodeName(sourceInfo.absoluteFilePath());
    const QByteArray destinationName = QFile::encodeName(partialPath);

    if (sqlite3_open_v2(sourceName.constData(), &source,
                        SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX, nullptr) != SQLITE_OK)
    {
        result.error = sqliteError(source, QLatin1String("Could not open the source database"));
    }
    else if (sqlite3_open_v2(destinationName.constData(), &destination,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE,
                             nullptr) != SQLITE_OK)
    {
        result.error = sqliteError(destination,
                                   QLatin1String("Could not create the database snapshot"));
    }
    else
    {
        sqlite3_busy_timeout(source, 2500);
        sqlite3_busy_timeout(destination, 2500);
        backup = sqlite3_backup_init(destination, "main", source, "main");

        if (!backup)
        {
            result.error = sqliteError(destination,
                                       QLatin1String("Could not initialize the database snapshot"));
        }
        else
        {
            do
            {
                if (isCanceled && isCanceled())
                {
                    result.canceled = true;
                    break;
                }

                backupResult = sqlite3_backup_step(backup, 256);
                result.totalPages = sqlite3_backup_pagecount(backup);
                result.copiedPages = result.totalPages - sqlite3_backup_remaining(backup);

                if (progress)
                {
                    progress(result.copiedPages, result.totalPages);
                }

                if ((backupResult == SQLITE_BUSY) || (backupResult == SQLITE_LOCKED))
                {
                    if (++busyAttempts > 80)
                    {
                        break;
                    }

                    QThread::msleep(25);
                }
                else
                {
                    busyAttempts = 0;
                }
            }
            while (backupResult != SQLITE_DONE);

            const int finishResult = sqlite3_backup_finish(backup);
            backup = nullptr;

            if (!result.canceled &&
                ((backupResult != SQLITE_DONE) || (finishResult != SQLITE_OK)))
            {
                result.error = sqliteError(destination,
                                           QLatin1String("The database snapshot could not be completed"));
            }
            else if (!result.canceled && passesIntegrityCheck(destination, &result.error))
            {
                result.success = true;
            }
        }
    }

    if (backup)
    {
        sqlite3_backup_finish(backup);
    }

    if (destination)
    {
        sqlite3_close(destination);
    }

    if (source)
    {
        sqlite3_close(source);
    }

    if (result.success)
    {
        if (!QFile::rename(partialPath, destinationInfo.absoluteFilePath()))
        {
            result.success = false;
            result.error = QLatin1String("The validated database snapshot could not be published");
            QFile::remove(partialPath);
        }
    }
    else
    {
        QFile::remove(partialPath);

        if (result.canceled)
        {
            result.error = QLatin1String("The database snapshot was canceled");
        }
    }

    return result;
}

} // namespace Digikam
