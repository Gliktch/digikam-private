/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbsdbschemaupdater.h"

// Qt includes

#include <QFileInfo>
#include <QFile>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "collectionscannerobserver.h"
#include "dbenginebackend.h"
#include "coredbtransaction.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"

namespace Digikam
{

int ThumbsDbSchemaUpdater::schemaVersion()
{
    return 3;
}

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ThumbsDbSchemaUpdater::Private
{

public:

    Private() = default;

public:

    bool                     setError               = false;

    int                      currentVersion         = 0;
    int                      currentRequiredVersion = 0;

    ThumbsDbAccess*          dbAccess               = nullptr;

    InitializationObserver*  observer               = nullptr;
};

ThumbsDbSchemaUpdater::ThumbsDbSchemaUpdater(ThumbsDbAccess* const dbAccess)
    : d(new Private)
{
    d->dbAccess = dbAccess;
}

ThumbsDbSchemaUpdater::~ThumbsDbSchemaUpdater()
{
    delete d;
}

bool ThumbsDbSchemaUpdater::update()
{
    bool success = startUpdates();

    // even on failure, try to set current version - it may have incremented

    if (d->currentVersion)
    {
        d->dbAccess->db()->setSetting(QLatin1String("DBThumbnailsVersion"),
                                      QString::number(d->currentVersion));
    }

    if (d->currentRequiredVersion)
    {
        d->dbAccess->db()->setSetting(QLatin1String("DBThumbnailsVersionRequired"),
                                      QString::number(d->currentRequiredVersion));
    }

    return success;
}

void ThumbsDbSchemaUpdater::setObserver(InitializationObserver* const observer)
{
    d->observer = observer;
}

bool ThumbsDbSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?

    QStringList tables = d->dbAccess->backend()->tables();

    if (tables.contains(QLatin1String("Thumbnails"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file

        QString version         = d->dbAccess->db()->getSetting(QLatin1String("DBThumbnailsVersion"));
        QString versionRequired = d->dbAccess->db()->getSetting(QLatin1String("DBThumbnailsVersionRequired"));
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: have a structure version " << version;

        // mini schema update

        if (version.isEmpty() && d->dbAccess->parameters().isSQLite())
        {
            version = d->dbAccess->db()->getSetting(QLatin1String("DBVersion"));
        }

        if (version.isEmpty() && d->dbAccess->parameters().isMySQL())
        {
            version         = d->dbAccess->db()->getLegacySetting(QLatin1String("DBThumbnailsVersion"));
            versionRequired = d->dbAccess->db()->getLegacySetting(QLatin1String("DBThumbnailsVersionRequired"));
        }

        // We absolutely require the DBThumbnailsVersion setting

        if (version.isEmpty())
        {
            // Something is damaged. Give up.

            qCCritical(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: database version not available! Giving up schema upgrading.";

            QString errorMsg = i18n("The database is not valid: "
                                    "the \"DBThumbnailsVersion\" setting does not exist. "
                                    "The current database schema version cannot be verified. "
                                    "Try to start with an empty database. ");
            d->dbAccess->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.

        d->currentVersion = version.toInt();

        if (d->currentVersion > schemaVersion())
        {
            // trying to open a database with a more advanced than this ThumbsDbSchemaUpdater supports

            if (!versionRequired.isEmpty() && (versionRequired.toInt() <= schemaVersion()))
            {
                // version required may be less than current version

                return true;
            }
            else
            {
                QString errorMsg = i18n("The database has been used with a more recent version of digiKam "
                                        "and has been updated to a database schema which cannot be used with this version. "
                                        "(This means this digiKam version is too old, or the database format is to recent) "
                                        "Please use the more recent version of digikam that you used before. ");
                d->dbAccess->setLastError(errorMsg);

                if (d->observer)
                {
                    d->observer->error(errorMsg);
                    d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
                }

                return false;
            }
        }
        else
        {
            return makeUpdates();
        }
    }
    else
    {
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: no database file available";

        DbEngineParameters parameters = d->dbAccess->parameters();

        // No legacy handling: start with a fresh db

        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n ") +
                               d->dbAccess->backend()->lastError();
            d->dbAccess->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        return true;
    }
}

bool ThumbsDbSchemaUpdater::makeUpdates()
{
    if (d->currentVersion < schemaVersion())
    {
        if (d->currentVersion == 1)
        {
            updateV1ToV2();
        }

        if (d->currentVersion <= 2)
        {
            updateV2ToV3();
        }
    }

    return true;
}

bool ThumbsDbSchemaUpdater::createDatabase()
{
    if (
        createTables()  &&
        createIndices() &&
        createTriggers()
       )
    {
        d->currentVersion         = schemaVersion();
        d->currentRequiredVersion = 1;

        return true;
    }
    else
    {
        return false;
    }
}

bool ThumbsDbSchemaUpdater::createTables()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateThumbnailsDB")));
}

bool ThumbsDbSchemaUpdater::createIndices()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateThumbnailsDBIndices")));
}

bool ThumbsDbSchemaUpdater::createTriggers()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateThumbnailsDBTrigger")));
}

bool ThumbsDbSchemaUpdater::updateV1ToV2()
{
    if (!d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("UpdateThumbnailsDBSchemaFromV1ToV2"))))
    {
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: schema upgrade from V1 to V2 failed!";
        return false;
    }

    d->currentVersion         = 2;
    d->currentRequiredVersion = 1;

    return true;
}

bool ThumbsDbSchemaUpdater::updateV2ToV3()
{
    if (!d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("UpdateThumbnailsDBSchemaFromV2ToV3"))))
    {
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: schema upgrade from V2 to V3 failed!";

        return false;
    }

    d->currentVersion         = 3;
    d->currentRequiredVersion = 1;

    return true;
}

} // namespace Digikam
