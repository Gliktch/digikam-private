/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Face database schema updater
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael dot miller at msn dot com>
 * SPDX-FileCopyrightText: 2020      by Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facedbschemaupdater.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dbenginebackend.h"
#include "facedbaccess.h"
#include "facedb.h"
#include "identityprovider.h"

namespace Digikam
{

int FaceDbSchemaUpdater::schemaVersion()
{
    return 6;
}

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN FaceDbSchemaUpdater::Private
{
public:

    Private() = default;

public:

    bool                    setError                = false;

    int                     currentVersion          = 0;
    int                     currentRequiredVersion  = 0;

    FaceDbAccess*           dbAccess                = nullptr;

    InitializationObserver* observer                = nullptr;
};

FaceDbSchemaUpdater::FaceDbSchemaUpdater(FaceDbAccess* const dbAccess)
    : d(new Private)
{
    d->dbAccess = dbAccess;
}

FaceDbSchemaUpdater::~FaceDbSchemaUpdater()
{
    delete d;
}

void FaceDbSchemaUpdater::setObserver(InitializationObserver* const observer)
{
    d->observer = observer;
}

bool FaceDbSchemaUpdater::update()
{
    bool success = startUpdates();

    // Even on failure, try to set current version - it may have incremented.

    if (d->currentVersion)
    {
        d->dbAccess->db()->setSetting(QLatin1String("DBFaceVersion"),
                                      QString::number(d->currentVersion));
    }

    if (d->currentRequiredVersion)
    {
        d->dbAccess->db()->setSetting(QLatin1String("DBFaceVersionRequired"),
                                      QString::number(d->currentRequiredVersion));
    }

    return success;
}

bool FaceDbSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?

    QStringList tables = d->dbAccess->backend()->tables();

    if (tables.contains(QLatin1String("Identities"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file.

        QString version         = d->dbAccess->db()->setting(QLatin1String("DBFaceVersion"));
        QString versionRequired = d->dbAccess->db()->setting(QLatin1String("DBFaceVersionRequired"));

        qCDebug(DIGIKAM_FACEDB_LOG) << "Face database: have a structure version " << version;

        // Mini schema update.

        if (version.isEmpty() && d->dbAccess->parameters().isSQLite())
        {
            version = d->dbAccess->db()->setting(QLatin1String("DBVersion"));
        }

        // We absolutely require the DBFaceVersion setting.

        if (version.isEmpty())
        {
            // Something is damaged. Give up.

            qCWarning(DIGIKAM_FACEDB_LOG) << "DBFaceVersion not available! Giving up schema upgrading.";

            QString errorMsg = i18n("The database is not valid: "
                                    "the \"DBFaceVersion\" setting does not exist. "
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

        // Current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.

        d->currentVersion = version.toInt();

        if (d->currentVersion > schemaVersion())
        {
            // Trying to open a database with a more advanced than this FaceDbSchemaUpdater supports.

            if (!versionRequired.isEmpty() && (versionRequired.toInt() <= schemaVersion()))
            {
                // Version required may be less than current version.

                return true;
            }

            else
            {
                QString errorMsg = i18n("The database has been used with a more recent version of digiKam "
                                        "and has been updated to a database schema which cannot be used with this version. "
                                        "(This means this digiKam version is too old, or the database format is to recent.) "
                                        "Please use the more recent version of digiKam that you used before.");

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
        qCDebug(DIGIKAM_FACEDB_LOG) << "Face database: no database file available";

        DbEngineParameters parameters = d->dbAccess->parameters();

        // No legacy handling: start with a fresh db.

        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n%1",
                                    d->dbAccess->backend()->lastError());
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

bool FaceDbSchemaUpdater::makeUpdates()
{
    if (d->currentVersion < schemaVersion())
    {
        if (d->currentVersion == 1)
        {
            updateV1ToV2();
        }

        else if (d->currentVersion == 2)
        {
            updateV2ToV6();
        }

        else if (d->currentVersion == 3)
        {
            updateV3ToV6();
        }

        else if (d->currentVersion == 4)
        {
            updateV4ToV6();
        }

        else if (d->currentVersion == 5)
        {
            updateV5ToV6();
        }
    }

    return true;
}

bool FaceDbSchemaUpdater::createDatabase()
{
    if (
        createTables()   &&
        createIndices()  &&
        createTriggers() &&
        populateDbSettings()        // cppcheck-suppress knownConditionTrueFalse
       )
    {
        d->currentVersion         = schemaVersion();
        d->currentRequiredVersion = 5;

        return true;
    }

    else
    {
        return false;
    }
}

bool FaceDbSchemaUpdater::createTables()
{
    // The creation order is important because of the foreign keys in MySQL.

    return (
            d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceDB")))             &&
            d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceDBFaceMatrices")))
           );
}

bool FaceDbSchemaUpdater::createIndices()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceIndices")));
}

bool FaceDbSchemaUpdater::createTriggers()
{
    return d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceTriggers")));
}

bool FaceDbSchemaUpdater::populateDbSettings()
{
    d->dbAccess->db()->setSetting(QLatin1String("TrainingVersion"), IdentityProvider::FaceTrainingVersion);
    d->dbAccess->db()->setSetting(QLatin1String("ExtractorModel"),  IdentityProvider::ExtractorModel);

    return true;
}

bool FaceDbSchemaUpdater::updateV1ToV2()
{
    /*
        if (!d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction("UpdateDBSchemaFromV1ToV2")))
        {
            qCCritical(DIGIKAM_FACEDB_LOG) << "Schema upgrade in DB from V1 to V2 failed!";

            return false;
        }
    */

    d->currentVersion         = 2;
    d->currentRequiredVersion = 1;

    return true;
}

bool FaceDbSchemaUpdater::updateV2ToV6()
{
    if (!(d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("CreateFaceDBFaceMatrices")))))
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "fail to recreate FaceMatrices table";

        return false;
    }

    if (!createIndices())
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "fail to create FaceMatrices index";

        return false;
    }

    d->currentVersion         = schemaVersion();
    d->currentRequiredVersion = 5;

    return true;
}

bool FaceDbSchemaUpdater::updateV3ToV6()
{
    return updateV2ToV6();
}

bool FaceDbSchemaUpdater::updateV4ToV6()
{
    if (!(d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("RenameFaceDBFaceMatrices_contextcolumn_V5")))))
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "fail to rename FaceMatrices column";

        return false;
    }

    if (!createIndices())
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "fail to create FaceMatrices index";

        return false;
    }

    d->currentVersion         = schemaVersion();
    d->currentRequiredVersion = 5;

    return true;
}

bool FaceDbSchemaUpdater::updateV5ToV6()
{
    if (!(d->dbAccess->backend()->execDBAction(d->dbAccess->backend()->getDBAction(QLatin1String("DropKDTreeTable_v6")))))
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "fail to drop KDTree table";

        return false;
    }

    d->currentVersion         = schemaVersion();
    d->currentRequiredVersion = 5;

    return true;
}

} // namespace Digikam
