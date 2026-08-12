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

#include "coredboperationgroup.h"

// Qt includes

#include <QElapsedTimer>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"

namespace Digikam
{

class Q_DECL_HIDDEN CoreDbOperationGroup::Private
{
public:

    Private() = default;

public:

    CoreDbAccess* access   = nullptr;
    bool          acquired = false;
    QElapsedTimer timeAcquired;
    int           maxTime  = 0;

public:

    bool needsTransaction() const
    {
        return CoreDbAccess::parameters().isSQLite();
    }

    void acquire()
    {
        if (access)
        {
            acquired = access->backend()->beginTransaction();
        }
        else
        {
            CoreDbAccess access;
            acquired = access.backend()->beginTransaction();
        }

        timeAcquired.start();
    }

    void release()
    {
        if (acquired)
        {
            if (access)
            {
                access->backend()->commitTransaction();
            }
            else
            {
                CoreDbAccess access;
                access.backend()->commitTransaction();
            }
        }
    }
};

CoreDbOperationGroup::CoreDbOperationGroup()
    : d(new Private)
{
    if (d->needsTransaction())
    {
        d->acquire();
    }
}

CoreDbOperationGroup::CoreDbOperationGroup(CoreDbAccess* const access)
    : d(new Private)
{
    d->access = access;

    if (d->needsTransaction())
    {
        d->acquire();
    }
}

CoreDbOperationGroup::~CoreDbOperationGroup()
{
    d->release();
    delete d;
}

void CoreDbOperationGroup::lift()
{
    if (d->acquired)
    {
        d->release();

        if (d->access)
        {
            CoreDbAccessUnlock unlock(d->access);
        }

        d->acquire();
    }
}

void CoreDbOperationGroup::setMaximumTime(int msecs)
{
    d->maxTime = msecs;
}

void CoreDbOperationGroup::resetTime()
{
    d->timeAcquired.start();
}

void CoreDbOperationGroup::allowLift()
{
    if (d->maxTime && d->timeAcquired.elapsed() > d->maxTime)
    {
        lift();
    }
}

} // namespace Digikam
