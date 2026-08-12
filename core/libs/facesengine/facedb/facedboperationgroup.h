/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-12
 * Description : Convenience object for grouping face database operations
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

namespace Digikam
{

class FaceDbAccess;

/**
 * @brief When you intend to execute a number of write operations to the database,
 * group them while holding a FaceDbOperationGroup.
 * For some database systems (SQLite), keeping a transaction across write operations
 * occurring in short time results in enormous speedup (800x).
 * For system that do not need this optimization, this class is a no-op.
 */
class FaceDbOperationGroup
{
public:

    /**
     * @brief Retrieve a FaceDbAccess object each time when constructing and destructing.
     */
    FaceDbOperationGroup();

    /**
     * @brief Use an existing FaceDbAccess object, which must live as long as this object exists.
     */
    explicit FaceDbOperationGroup(FaceDbAccess* const dbAccess);

    ~FaceDbOperationGroup();

    /**
     * @brief This will - if a transaction is held - commit the transaction and acquire a new one.
     * This may improve concurrent access.
     */
    void lift();

    void setMaximumTime(int msecs);

    /**
     * @brief Resets to 0 the time used by allowLift().
     */
    void resetTime();

    /**
     * @brief Allows to lift(). The transaction will be lifted if the time set by setMaximumTime()
     * has expired.
     */
    void allowLift();

private:

    /// @note disabled
    FaceDbOperationGroup(const FaceDbOperationGroup&)            = delete;
    FaceDbOperationGroup& operator=(const FaceDbOperationGroup&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
