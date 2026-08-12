/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database engine action
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineActionElement
{
public:

    DbEngineActionElement() = default;

    QString mode;
    int     order       = 0;
    QString statement;
};

// ---------------------------------------------------------------

class DIGIKAM_EXPORT DbEngineAction
{
public:

    DbEngineAction() = default;

    QString                      name;
    QString                      mode;
    QList<DbEngineActionElement> dbActionElements;
};

} // namespace Digikam
