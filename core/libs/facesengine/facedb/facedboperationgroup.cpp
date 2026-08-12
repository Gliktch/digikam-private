/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-12
 * Description : Convenience object for grouping database operations
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facedboperationgroup.h"

// Qt includes

#include <QElapsedTimer>

// Local includes

#include "facedbaccess.h"
#include "facedbbackend.h"

namespace Digikam
{

class Q_DECL_HIDDEN FaceDbOperationGroup::Private
{
public:

    Private() = default;

public:

    FaceDbAccess* dbAccess      = nullptr;
    bool          acquired      = false;
    QElapsedTimer timeAcquired;
    int           maxTime       = 0;

public:

    bool needsTransaction() const
    {
        return FaceDbAccess().parameters().isSQLite();
    }

    void acquire()
    {
        if (dbAccess)
        {
            acquired = dbAccess->backend()->beginTransaction();
        }

        else
        {
            FaceDbAccess dbAccess;
            acquired = dbAccess.backend()->beginTransaction();
        }

        timeAcquired.start();
    }

    void release()
    {
        if (acquired)
        {
            if (dbAccess)
            {
                dbAccess->backend()->commitTransaction();
            }

            else
            {
                FaceDbAccess dbAccess;
                dbAccess.backend()->commitTransaction();
            }
        }
    }
};

// -----------------------------------------------------------------------

FaceDbOperationGroup::FaceDbOperationGroup()
    : d(new Private)
{
    if (d->needsTransaction())
    {
        d->acquire();
    }
}

FaceDbOperationGroup::FaceDbOperationGroup(FaceDbAccess* const dbAccess)
    : d(new Private)
{
    d->dbAccess = dbAccess;

    if (d->needsTransaction())
    {
        d->acquire();
    }
}

FaceDbOperationGroup::~FaceDbOperationGroup()
{
    d->release();
    delete d;
}

void FaceDbOperationGroup::lift()
{
    if (d->acquired)
    {
        d->release();

        if (d->dbAccess)
        {
            FaceDbAccessUnlock unlock(d->dbAccess);
        }

        d->acquire();
    }
}

void FaceDbOperationGroup::setMaximumTime(int msecs)
{
    d->maxTime = msecs;
}

void FaceDbOperationGroup::resetTime()
{
    d->timeAcquired.start();
}

void FaceDbOperationGroup::allowLift()
{
    if (d->maxTime && (d->timeAcquired.elapsed() > d->maxTime))
    {
        lift();
    }
}

} // namespace Digikam
