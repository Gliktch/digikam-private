/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database engine element configuration settings
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>

// Local includes

#include "dbengineaction.h"

namespace Digikam
{

class DbEngineConfigSettings
{

public:

    DbEngineConfigSettings() = default;

    QString                       databaseID;
    QString                       hostName;
    QString                       port;
    QString                       connectOptions;
    QString                       databaseName;
    QString                       userName;
    QString                       password;
    QMap<QString, DbEngineAction> sqlStatements;
};

} // namespace Digikam
