/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - private containers.
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemscanner_p.h"

namespace Digikam
{

LessThanByProximityToSubject::LessThanByProximityToSubject(const ItemInfo& subject)
    : subject(subject)
{
}

bool LessThanByProximityToSubject::operator()(const ItemInfo& a, const ItemInfo& b)
{
    if (a.isNull() || b.isNull())
    {
        // both null: false
        // only a null: a greater than b (null infos at end of list)
        //  (a && b) || (a && !b) = a
        // only b null: a less than b

        if (a.isNull())
        {
            return false;
        }

        return true;
    }

    if (a == b)
    {
        return false;
    }

    // same collection

    if (a.albumId() != b.albumId())
    {
        // same album

        if (a.albumId() == subject.albumId())
        {
            return true;
        }

        if (b.albumId() == subject.albumId())
        {
            return false;
        }

        if (a.albumRootId() != b.albumRootId())
        {
            // different collection

            if (a.albumRootId() == subject.albumRootId())
            {
                return true;
            }

            if (b.albumRootId() == subject.albumRootId())
            {
                return false;
            }
        }
    }

    if (a.modDateTime() != b.modDateTime())
    {
        return (a.modDateTime() < b.modDateTime());
    }

    if (a.name() != b.name())
    {
        return qAbs(a.name().compare(subject.name())) < qAbs(b.name().compare(subject.name()));
    }

    // last resort

    return (a.id() < b.id());
}

// ---------------------------------------------------------------------------

ItemScanner::Private::Private()
    : metadata(new DMetadata)
{
    timer.start();
}

ItemScanner::Private::~Private()
{
    delete metadata;
}

} // namespace Digikam
