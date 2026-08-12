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

#pragma once

// Local includes

#include "dbengineparameters.h"
#include "coredbbackend.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CoreDbPrivilegesChecker
{

public:

    explicit CoreDbPrivilegesChecker(const DbEngineParameters& parameters);
    ~CoreDbPrivilegesChecker() = default;

    bool checkPrivileges(QStringList& insufficientRights);
    bool checkPriv(CoreDbBackend& dbBackend, const QString& dbActionName);

private:

    DbEngineParameters m_parameters;
};

} // namespace Digikam
