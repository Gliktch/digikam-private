/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Core database convenience object for transactions.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredbtransaction.h"

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"

namespace Digikam
{

CoreDbTransaction::CoreDbTransaction()
{
    CoreDbAccess access;
    access.backend()->beginTransaction();
}

CoreDbTransaction::CoreDbTransaction(CoreDbAccess* const access)
    : m_access(access)
{
    m_access->backend()->beginTransaction();
}

CoreDbTransaction::~CoreDbTransaction()
{
    if (m_access)
    {
        m_access->backend()->commitTransaction();
    }
    else
    {
        CoreDbAccess access;
        access.backend()->commitTransaction();
    }
}

} // namespace Digikam
