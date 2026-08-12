/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Core database access wrapper.
 *
 * SPDX-FileCopyrightText: 2016 by Swati Lodha <swatilodha27 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * The DbEngineAccess class provides access to the database:
 * Create an instance of this class on the stack to retrieve a pointer to the database.
 */
class DIGIKAM_EXPORT DbEngineAccess
{
public:

    /**
     * Checks the availability of drivers. Must be used in children class.
     * Return true if low level drivers are ready to use, else false with
     * an error string of the problem.
     */
    static bool checkReadyForUse(QString& error);
};

} // namespace Digikam
