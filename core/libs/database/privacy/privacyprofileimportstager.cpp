/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprofileimportstager.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUuid>

// Local includes

#include "coredbcopymanager.h"
#include "dbengineparameters.h"
#include "privacyprofilemerge.h"
#include "privacyprofilepreflight.h"
#include "privacysqlitesnapshot.h"

namespace Digikam
{

namespace
{

bool directoryIsEmpty(const QDir& directory)
{
    return directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty();
}

bool thumbnailDatabaseIsCompatible(const QString& path)
{
    const QString connectionName = QLatin1String("privacy-thumbnail-inspect-") +
                                   QUuid::createUuid().toString(QUuid::WithoutBraces);
    bool compatible = false;

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
        database.setConnectOptions(QLatin1String("QSQLITE_OPEN_READONLY"));
        database.setDatabaseName(path);

        if (database.open())
        {
            QSqlQuery version(database);
            QSqlQuery integrity(database);
            compatible = version.exec(QLatin1String(
                             "SELECT value FROM Settings WHERE keyword='DBVersion';")) &&
                         version.next() &&
                         (version.value(0).toInt() >= 1) &&
                         (version.value(0).toInt() <= 3) &&
                         integrity.exec(QLatin1String("PRAGMA quick_check(1);")) &&
                         integrity.next() &&
                         (integrity.value(0).toString() == QLatin1String("ok"));
            database.close();
        }
    }

    QSqlDatabase::removeDatabase(connectionName);
    return compatible;
}

void reportProgress(const PrivacyProfileImportStager::Progress& progress,
                    const QString& step,
                    int current,
                    int total)
{
    if (progress)
    {
        progress(step, current, total);
    }
}

} // namespace

PrivacyProfileImportStageResult PrivacyProfileImportStager::stage(
    const PrivacyProfileSummary& source,
    const QString& stageDirectory,
    const Progress& progress,
    const IsCanceled& isCanceled,
    const QString& additiveTargetP1Path)
{
    PrivacyProfileImportStageResult result;
    result.stageDirectory = QFileInfo(stageDirectory).absoluteFilePath();

    if (!source.isUsable())
    {
        result.error = QLatin1String("The selected source profile is not importable");
        return result;
    }

    if (source.isPrivateProfile() && (source.incompletePrivacyTransactionCount > 0))
    {
        result.error = QLatin1String("The source profile has incomplete privacy transactions");
        return result;
    }

    QDir stage(result.stageDirectory);

    if ((!stage.exists() && !QDir().mkpath(result.stageDirectory)) ||
        !directoryIsEmpty(stage))
    {
        result.error = QLatin1String("The profile-import staging directory is not empty or writable");
        return result;
    }

    const QString sourceSnapshotPath = stage.filePath(QLatin1String("source-core.db"));
    const QString candidateDirectory = stage.filePath(QLatin1String("candidate"));

    if (!QDir().mkpath(candidateDirectory))
    {
        result.error = QLatin1String("The candidate profile directory could not be created");
        return result;
    }

    result.candidateCoreDatabasePath =
        QDir(candidateDirectory).filePath(QLatin1String("digikam4.db"));
    reportProgress(progress, QLatin1String("Snapshot source database"), 0, 1);
    const PrivacySqliteSnapshotResult snapshot = PrivacySqliteSnapshot::create(
        source.databasePath, sourceSnapshotPath,
        [progress](int copied, int total)
        {
            reportProgress(progress, QLatin1String("Snapshot source database"), copied, total);
        },
        isCanceled);

    if (!snapshot.success)
    {
        result.canceled = snapshot.canceled;
        result.error = snapshot.error;
        return result;
    }

    const PrivacyProfileSummary captured =
        PrivacyProfileInspector::inspectCoreDatabase(sourceSnapshotPath);

    if (!captured.isUsable() ||
        (captured.schemaKind != source.schemaKind) ||
        (captured.schemaVersion != source.schemaVersion))
    {
        result.error = QLatin1String("The captured source database did not match the inspected profile");
        return result;
    }

    if (captured.isPrivateProfile() && (captured.protectedItemCount > 0))
    {
        const PrivacyProfilePreflightResult preflight =
            PrivacyProfilePreflight::verify(sourceSnapshotPath);

        if (!preflight.success)
        {
            result.error = QLatin1String("Protected-store validation failed: ") +
                           preflight.error;
            return result;
        }
    }

    if (isCanceled && isCanceled())
    {
        result.canceled = true;
        result.error = QLatin1String("Profile import staging was canceled");
        return result;
    }

    if (!additiveTargetP1Path.isEmpty())
    {
        if (source.schemaKind != PrivacyProfileSchemaKind::Stock)
        {
            result.error = QLatin1String(
                "Only a stock catalogue can be merged additively into a private profile");
            return result;
        }

        const QString convertedSourcePath = QDir(candidateDirectory).filePath(
            QLatin1String("source-converted.db"));
        reportProgress(progress, QLatin1String("Convert stock catalogue to P1"), 0, 1);
        const DbEngineParameters from = DbEngineParameters::parametersForSQLite(
            sourceSnapshotPath);
        const DbEngineParameters to = DbEngineParameters::parametersForSQLite(
            convertedSourcePath);
        CoreDbCopyManager copyManager;
        int finishState = CoreDbCopyManager::failed;
        QString copyError = QLatin1String(
            "The stock catalogue conversion did not finish");

        QObject::connect(&copyManager, &CoreDbCopyManager::finished,
                         [&finishState, &copyError](int state, const QString& error)
                         {
                             finishState = state;
                             copyError = error;
                         });
        QObject::connect(&copyManager, &CoreDbCopyManager::smallStepStarted,
                         [progress](int current, int total)
                         {
                             reportProgress(progress,
                                            QLatin1String("Convert stock catalogue to P1"),
                                            current, total);
                         });

        copyManager.copyDatabases(from, to);

        if (finishState != CoreDbCopyManager::success)
        {
            result.canceled = (finishState == CoreDbCopyManager::canceled);
            result.error = copyError.isEmpty()
                         ? QLatin1String("The stock catalogue could not be converted to P1")
                         : copyError;
            return result;
        }

        reportProgress(progress, QLatin1String("Copy active private profile"), 0, 1);
        const PrivacySqliteSnapshotResult targetSnapshot = PrivacySqliteSnapshot::create(
            additiveTargetP1Path, result.candidateCoreDatabasePath,
            [progress](int copied, int total)
            {
                reportProgress(progress, QLatin1String("Copy active private profile"),
                               copied, total);
            },
            isCanceled);

        if (!targetSnapshot.success)
        {
            result.canceled = targetSnapshot.canceled;
            result.error = targetSnapshot.error;
            return result;
        }

        reportProgress(progress, QLatin1String("Merge stock catalogue"), 0, 1);
        const PrivacyProfileMergeResult merged = PrivacyProfileMerge::mergeStockIntoP1(
            convertedSourcePath, result.candidateCoreDatabasePath,
            progress, isCanceled);

        if (!merged.success)
        {
            result.canceled = merged.canceled;
            result.error = merged.error;
            return result;
        }

        result.addedItemCount = merged.addedItemCount;
        result.skippedExistingItemCount = merged.skippedExistingItemCount;
        result.warnings << merged.warnings;

        // Preserve the target thumbnail database (existing thumbnails stay);
        // thumbnails for newly merged items are rebuilt by the next scan.
        const QString targetThumbnailPath = QDir(
            QFileInfo(additiveTargetP1Path).absolutePath()).filePath(
            QLatin1String("thumbnails-digikam.db"));

        if (QFileInfo::exists(targetThumbnailPath) &&
            thumbnailDatabaseIsCompatible(targetThumbnailPath))
        {
            result.candidateThumbnailDatabasePath = QDir(candidateDirectory).filePath(
                QLatin1String("thumbnails-digikam.db"));
            reportProgress(progress, QLatin1String("Copy active thumbnails"), 0, 1);
            const PrivacySqliteSnapshotResult thumbnails = PrivacySqliteSnapshot::create(
                targetThumbnailPath, result.candidateThumbnailDatabasePath,
                [progress](int copied, int total)
                {
                    reportProgress(progress, QLatin1String("Copy active thumbnails"),
                                   copied, total);
                },
                isCanceled);

            if (thumbnails.success)
            {
                result.thumbnailDatabaseIncluded = true;
            }
            else if (thumbnails.canceled)
            {
                result.canceled = true;
                result.error = thumbnails.error;
                return result;
            }
            else
            {
                result.candidateThumbnailDatabasePath.clear();
                result.warnings << QLatin1String(
                    "The active thumbnail database could not be captured and will be rebuilt");
            }
        }
        else if (QFileInfo::exists(targetThumbnailPath))
        {
            result.warnings << QLatin1String(
                "The active thumbnail database is incompatible and will be rebuilt");
        }
    }
    else if (source.schemaKind == PrivacyProfileSchemaKind::Stock)
    {
        reportProgress(progress, QLatin1String("Convert stock catalogue to P1"), 0, 1);
        const DbEngineParameters from = DbEngineParameters::parametersForSQLite(
            sourceSnapshotPath);
        const DbEngineParameters to = DbEngineParameters::parametersForSQLite(
            result.candidateCoreDatabasePath);
        CoreDbCopyManager copyManager;
        int finishState = CoreDbCopyManager::failed;
        QString copyError = QLatin1String(
            "The stock catalogue conversion did not finish");

        QObject::connect(&copyManager, &CoreDbCopyManager::finished,
                         [&finishState, &copyError](int state, const QString& error)
                         {
                             finishState = state;
                             copyError = error;
                         });
        QObject::connect(&copyManager, &CoreDbCopyManager::smallStepStarted,
                         [progress](int current, int total)
                         {
                             reportProgress(progress,
                                            QLatin1String("Convert stock catalogue to P1"),
                                            current, total);
                         });

        copyManager.copyDatabases(from, to);

        if (finishState != CoreDbCopyManager::success)
        {
            result.canceled = (finishState == CoreDbCopyManager::canceled);
            result.error = copyError.isEmpty()
                         ? QLatin1String("The stock catalogue could not be converted to P1")
                         : copyError;
            return result;
        }
    }
    else if (!QFile::rename(sourceSnapshotPath, result.candidateCoreDatabasePath))
    {
        result.error = QLatin1String("The P1 candidate database could not be staged");
        return result;
    }

    result.candidateSummary = PrivacyProfileInspector::inspectCoreDatabase(
        result.candidateCoreDatabasePath);

    if (!result.candidateSummary.isPrivateProfile() ||
        !result.candidateSummary.integrityOk)
    {
        result.error = QLatin1String("The staged candidate is not a valid P1 database");
        return result;
    }

    const QFileInfo thumbnailInfo(source.thumbnailDatabasePath);

    if (thumbnailInfo.exists() && thumbnailInfo.isFile() && thumbnailInfo.isReadable())
    {
        if (!thumbnailDatabaseIsCompatible(source.thumbnailDatabasePath))
        {
            result.warnings << QLatin1String(
                "The source thumbnail database is incompatible and will be rebuilt");
        }
        else
        {
            result.candidateThumbnailDatabasePath = QDir(candidateDirectory).filePath(
                QLatin1String("thumbnails-digikam.db"));
            reportProgress(progress, QLatin1String("Snapshot thumbnail database"), 0, 1);
            const PrivacySqliteSnapshotResult thumbnails = PrivacySqliteSnapshot::create(
                source.thumbnailDatabasePath,
                result.candidateThumbnailDatabasePath,
                [progress](int copied, int total)
                {
                    reportProgress(progress,
                                   QLatin1String("Snapshot thumbnail database"),
                                   copied, total);
                },
                isCanceled);

            if (thumbnails.success)
            {
                result.thumbnailDatabaseIncluded = true;
            }
            else if (thumbnails.canceled)
            {
                result.canceled = true;
                result.error = thumbnails.error;
                return result;
            }
            else
            {
                result.candidateThumbnailDatabasePath.clear();
                result.warnings << QLatin1String(
                    "The thumbnail database could not be captured and will be rebuilt");
            }
        }
    }

    result.success = true;
    reportProgress(progress, QLatin1String("Profile import candidate ready"), 1, 1);
    return result;
}

} // namespace Digikam
