/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database engine configuration
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbengineconfig.h"

// Qt includes

#include <QtGlobal>
#include <QStandardPaths>

// Local includes

#include "digikam_dbconfig.h"
#include "dbengineconfigloader.h"

namespace Digikam
{

Q_GLOBAL_STATIC_WITH_ARGS(
                          DbEngineConfigSettingsLoader,
                          dbcoreloader,
                          (
                           QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QLatin1String("digikam/database/dbconfig.xml")),
                           dbcoreconfig_xml_version
                          )
                         )

DbEngineConfigSettings DbEngineConfig::element(const QString& databaseType)
{
    // Unprotected read-only access? Usually accessed under CoreDbAccess protection anyway

    return dbcoreloader->databaseConfigs.value(databaseType);
}

bool DbEngineConfig::checkReadyForUse()
{
    return dbcoreloader->isValid;
}

QString DbEngineConfig::errorMessage()
{
    return dbcoreloader->errorMessage;
}

} // namespace Digikam
