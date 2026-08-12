/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Core database copy manager for migration operations.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredbcopymanager.h"

// Qt includes

#include <QSqlError>
#include <QSqlDriver>
#include <QSqlRecord>

// KDE Includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dbenginebackend.h"
#include "dbengineparameters.h"
#include "coredb.h"
#include "coredbschemaupdater.h"

namespace Digikam
{

void CoreDbCopyManager::stopProcessing()
{
    m_isStopProcessing = true;
}

void CoreDbCopyManager::copyDatabases(const DbEngineParameters& fromDBParameters,
                                      const DbEngineParameters& toDBParameters)
{
    m_isStopProcessing = false;
    DbEngineLocking fromLocking;
    CoreDbBackend fromDBbackend(&fromLocking, QLatin1String("MigrationFromDatabase"));

    if (!fromDBbackend.open(fromDBParameters))
    {
        Q_EMIT finished(CoreDbCopyManager::failed, i18n("Error while opening the source database."));

        return;
    }

    DbEngineLocking toLocking;
    CoreDbBackend toDBbackend(&toLocking, QLatin1String("MigrationToDatabase"));

    if (!toDBbackend.open(toDBParameters))
    {
        Q_EMIT finished(CoreDbCopyManager::failed, i18n("Error while opening the target database."));

        fromDBbackend.close();

        return;
    }

    // Order may be important, array class must _not_ be sorted

    const QStringList tables = QStringList()
        << QLatin1String("AlbumRoots")
        << QLatin1String("Albums")
        << QLatin1String("Images")

        // Virtual table used to allow population of Albums.icon after Images migration

        << QLatin1String("AlbumsExtra")
        << QLatin1String("ImageInformation")
        << QLatin1String("ImageMetadata")
        << QLatin1String("ImagePositions")
        << QLatin1String("ImageComments")
        << QLatin1String("ImageCopyright")
        << QLatin1String("Tags")

        // Virtual table used to allow population of Tags.icon after Images migration

        << QLatin1String("TagsExtra")
        << QLatin1String("TagsTree")
        << QLatin1String("TagProperties")
        << QLatin1String("ImageTagProperties")
        << QLatin1String("ImageTags")
        << QLatin1String("ImageProperties")
        << QLatin1String("ImageHistory")
        << QLatin1String("ImageRelations")
        << QLatin1String("Searches")
        << QLatin1String("DownloadHistory")
        << QLatin1String("VideoMetadata")
        << QLatin1String("Settings")
    ;

    const int tablesSize = tables.size();

    // Run any database specific preparatory cleanup prior to dropping tables.

    DbEngineAction action                        = toDBbackend.getDBAction(QString::fromUtf8("Migrate_Cleanup_Prepare"));
    BdEngineBackend::QueryState queryStateResult = toDBbackend.execDBAction(action);

    // Accept SQL error because the foreign key may not exist.

    if (queryStateResult == BdEngineBackend::ConnectionError)
    {
        Q_EMIT finished(CoreDbCopyManager::failed, i18n("Error while preparing the target database."));
    }

    // Delete all tables

    for (int i = (tablesSize - 1) ; i >= 0 ; --i)
    {
        if (
            m_isStopProcessing ||
            (toDBbackend.execDirectSql(QString::fromUtf8("DROP TABLE IF EXISTS %1;").arg(tables[i])) != BdEngineBackend::NoErrors)
           )
        {
            Q_EMIT finished(CoreDbCopyManager::failed, i18n("Error while scrubbing the target database."));

            fromDBbackend.close();
            toDBbackend.close();

            return;
        }
    }

    // Then create the schema

    CoreDB              albumDB(&toDBbackend);
    CoreDbSchemaUpdater updater(&albumDB, &toDBbackend, toDBParameters);

    Q_EMIT stepStarted(i18n("Create Schema..."));

    if (m_isStopProcessing || !updater.update())
    {
        Q_EMIT finished(CoreDbCopyManager::failed, i18n("Error while creating the database schema."));

        fromDBbackend.close();
        toDBbackend.close();

        return;
    }

    // loop copying the tables, stop if an error is met

    for (int i = 0 ; i < tablesSize ; ++i)
    {
        Q_EMIT stepStarted(i18n(QString::fromUtf8("Copy %1...").arg(tables[i]).toLatin1().constData()));

        // Now perform the copy action

        if (
            m_isStopProcessing ||
            !copyTable(fromDBbackend, QString::fromUtf8("Migrate_Read_%1").arg(tables[i]),
                       toDBbackend, QString::fromUtf8("Migrate_Write_%1").arg(tables[i]))
           )
        {
            handleClosing(m_isStopProcessing, fromDBbackend, toDBbackend);

            return;
        }
    }

    fromDBbackend.close();
    toDBbackend.close();

    Q_EMIT smallStepStarted(1, 1);
    Q_EMIT finished(CoreDbCopyManager::success, QString());
}

bool CoreDbCopyManager::copyTable(CoreDbBackend& fromDBbackend,
                                  const QString& fromActionName,
                                  CoreDbBackend& toDBbackend,
                                  const QString& toActionName)
{
    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: trying to copy contents from DB with ActionName: [" << fromActionName
                                << "] to DB with ActionName [" << toActionName << "]";

    QMap<QString, QVariant> bindingMap;

    // now perform the copy action

    QList<QString> columnNames;
    QSqlQuery      result        = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;
    int            resultSize    = -1;
    bool           isForwardOnly = result.isForwardOnly();

    if (result.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        resultSize=result.size();
    }
    else
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: driver do not support query size. "
                                       "We try to go to the last row and back to the current.";

        result.last();

        // Now get the current row. If this is not possible, a value lower than 0 will be returned.
        // To not confuse the log reading user, we reset this value to 0.

        resultSize = (result.at() < 0) ? 0 : result.at();

        // Avoid a misleading error message, query is redone if isForwardOnly

        if ( ! isForwardOnly)
        {
            result.first();
        }
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: result size: ["<< resultSize << "]";

    // If the sql query is forward only - perform the query again.
    // This is not atomic, so it can be tend to different results between
    // real database entries copied and shown at the progressbar.

    if (isForwardOnly)
    {
        result.finish();
        result = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;
    }

    int columnCount = result.record().count();

    for (int i = 0 ; i < columnCount ; ++i)
    {
/*
        qCDebug(DIGIKAM_COREDB_LOG) << "Column: ["<< result.record().fieldName(i) << "]";
*/
        columnNames.append(result.record().fieldName(i));
    }

    int resultCounter = 0;

    while (result.next())
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: query isOnValidRow [" << result.isValid()
                                    << "] isActive [" << result.isActive()
                                    << "] result size: [" << result.size() << "]";

        if (m_isStopProcessing)
        {
            return false;
        }

        // Send a signal to the GUI to entertain the user

        Q_EMIT smallStepStarted(resultCounter++, resultSize);

        // Read the values from the fromDB into a hash

        QMap<QString, QVariant> tempBindingMap;
        int i = 0;

        for (QString columnName : std::as_const(columnNames))
        {
            columnName.remove(QLatin1String(" COLLATE utf8_general_ci"));

            QVariant varValue = result.value(i);

            if (
                (columnName == QLatin1String("modificationDate")) ||
                (columnName == QLatin1String("digitizationDate")) ||
                (columnName == QLatin1String("creationDate"))     ||
                (columnName == QLatin1String("filedate"))         ||
                (columnName == QLatin1String("date"))
               )
            {
                // The "date" column can be a QDate or QDateTime.

                if (varValue.toString().size() > 10)
                {
                    varValue = QVariant(toDBbackend.asDBDateTime(varValue.toDateTime()));
                }
            }

            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: column: ["
                                        << columnName << "] value ["
                                        << varValue << "]";

            tempBindingMap.insert(columnName.insert(0, QLatin1Char(':')), varValue);
            ++i;
        }

        // Insert the previous requested values to the DB

        DbEngineAction action                        = toDBbackend.getDBAction(toActionName);
        BdEngineBackend::QueryState queryStateResult = toDBbackend.execDBAction(action, tempBindingMap);

        if (
            (queryStateResult != BdEngineBackend::NoErrors) &&
            toDBbackend.lastSQLError().isValid()            &&
            !toDBbackend.lastSQLError().nativeErrorCode().isEmpty()
           )
        {
            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: error while converting table data. Details:"
                                        << toDBbackend.lastSQLError();
            QString errorMsg = i18n("Error while converting the database.\n Details: %1",
                                    toDBbackend.lastSQLError().databaseText());

            Q_EMIT finished(CoreDbCopyManager::failed, errorMsg);

            return false;
        }
    }

    return true;
}

void CoreDbCopyManager::handleClosing(bool isStopThread,
                                      CoreDbBackend& fromDBbackend,
                                      CoreDbBackend& toDBbackend)
{
    if (isStopThread)
    {
        Q_EMIT finished(CoreDbCopyManager::canceled, QLatin1String(""));
    }

    fromDBbackend.close();
    toDBbackend.close();
}

} // namespace Digikam

#include "moc_coredbcopymanager.cpp"
