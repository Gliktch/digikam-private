/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-24
 * Description : Core database privileges checker
 *
 * SPDX-FileCopyrightText: 2010 by Holger Foerster <hamsi2k at freenet dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredbchecker.h"

// Qt includes

#include <QSqlDatabase>
#include <QSqlError>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

CoreDbPrivilegesChecker::CoreDbPrivilegesChecker(const DbEngineParameters& parameters)
    : m_parameters(parameters)
{
}

bool CoreDbPrivilegesChecker::checkPrivileges(QStringList& insufficientRights)
{
    bool result = true;
    DbEngineLocking fromLocking;
    CoreDbBackend fromDBbackend(&fromLocking, QLatin1String("PrivilegesCheckDatabase"));

    if (!fromDBbackend.open(m_parameters))
    {
        return false;
    }

    // After a crash at the start of digiKam, it may happen that the table
    // to check the rights remains. Because the next time the start of the
    // rights check is constantly failing, we remove the table before
    // every start and do not check for errors.

    (void)checkPriv(fromDBbackend, QLatin1String("CheckPriv_Cleanup"));

    if      (!checkPriv(fromDBbackend, QLatin1String("CheckPriv_CREATE_TABLE")))
    {
        insufficientRights.append(QLatin1String("CREATE TABLE"));
        result = false;
    }
    else if (!checkPriv(fromDBbackend, QLatin1String("CheckPriv_ALTER_TABLE")))
    {
        insufficientRights.append(QLatin1String("ALTER TABLE"));
        result = false;
    }
    else if (!checkPriv(fromDBbackend, QLatin1String("CheckPriv_CREATE_TRIGGER")))
    {
        insufficientRights.append(QLatin1String("CREATE TRIGGER"));
        result = false;
    }
    else if (!checkPriv(fromDBbackend, QLatin1String("CheckPriv_DROP_TRIGGER")))
    {
        insufficientRights.append(QLatin1String("DROP TRIGGER"));
        result = false;
    }
    else if (!checkPriv(fromDBbackend, QLatin1String("CheckPriv_DROP_TABLE")))
    {
        insufficientRights.append(QLatin1String("DROP TABLE"));
        result = false;
    }
    else if (!checkPriv(fromDBbackend, QLatin1String("CheckPriv_Cleanup")))
    {
        insufficientRights.append(QLatin1String("DROP TABLE PrivCheck"));
        result = false;
    }

    return result;
}

bool CoreDbPrivilegesChecker::checkPriv(CoreDbBackend& dbBackend, const QString& dbActionName)
{
    BdEngineBackend::QueryState queryStateResult = dbBackend.execDBAction(dbActionName);

    if (
        (queryStateResult != BdEngineBackend::NoErrors)       &&
        dbBackend.lastSQLError().isValid()                    &&
        !dbBackend.lastSQLError().nativeErrorCode().isEmpty()
       )
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: error while creating a trigger. Details:"
                                    << dbBackend.lastSQLError();
        return false;
    }

    return true;
}

} // namespace Digikam
