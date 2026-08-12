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

#pragma once

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CoreDbAccess;

/**
 * Convenience class: You can create a CoreDbTransaction object for a scope for which
 * you want to declare a database commit.
 * Equivalent to calling beginTransaction and commitTransaction on the album db.
 */
class DIGIKAM_DATABASE_EXPORT CoreDbTransaction
{
public:

    /**
     * Retrieve a CoreDbAccess object each time when constructing and destructing.
     */
    CoreDbTransaction();

    /**
     * Use an existing CoreDbAccess object, which must live as long as this object exists.
     */
    explicit CoreDbTransaction(CoreDbAccess* const access);
    ~CoreDbTransaction();

private:

    /// @note disabled
    CoreDbTransaction(const CoreDbTransaction&)            = delete;
    CoreDbTransaction& operator=(const CoreDbTransaction&) = delete;

    CoreDbAccess* m_access = nullptr;
};

} // namespace Digikam
