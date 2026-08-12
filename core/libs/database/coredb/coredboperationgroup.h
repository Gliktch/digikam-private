/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-12
 * Description : Core database convenience object for grouping operations
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CoreDbAccess;

/**
 * When you intend to execute a number of write operations to the database,
 * group them while holding a CoreDbOperationGroup.
 * For some database systems (SQLite), keeping a transaction across write operations
 * occurring in short time results in enormous speedup (800x).
 * For system that do not need this optimization, this class is a no-op.
 */
class DIGIKAM_DATABASE_EXPORT CoreDbOperationGroup
{
public:

    /**
     * Retrieve a CoreDbAccess object each time when constructing and destructing.
     */
    CoreDbOperationGroup();

    /**
     * Use an existing CoreDbAccess object, which must live as long as this object exists.
     */
    explicit CoreDbOperationGroup(CoreDbAccess* const access);
    ~CoreDbOperationGroup();

    /**
     * This will - if a transaction is held - commit the transaction and acquire a new one.
     * This may improve concurrent access.
     */
    void lift();

    void setMaximumTime(int msecs);

    /**
     * Resets to 0 the time used by allowLift()
     */
    void resetTime();

    /**
     * Allows to lift(). The transaction will be lifted if the time set by setMaximumTime()
     * has expired.
     */
    void allowLift();

private:

    /// @note disabled
    CoreDbOperationGroup(const CoreDbOperationGroup&)            = delete;
    CoreDbOperationGroup& operator=(const CoreDbOperationGroup&) = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
