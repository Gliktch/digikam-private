/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : class GroupStateComputer
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "groupstatecomputer.h"
#include "geoifacetypes.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN GroupStateComputer::Private
{
public:

    Private() = default;

    GeoGroupState state     = SelectedNone;
    GeoGroupState stateMask = SelectedNone;
};

GroupStateComputer::GroupStateComputer()
    : d(new Private)
{
}

GroupStateComputer::~GroupStateComputer()
{
}

GeoGroupState GroupStateComputer::getState() const
{
    return d->state;
}

void GroupStateComputer::clear()
{
    d->state     = SelectedNone;
    d->stateMask = SelectedNone;
}

void GroupStateComputer::addState(const GeoGroupState state)
{
    addSelectedState(state);
    addFilteredPositiveState(state);
    addRegionSelectedState(state);
}

void GroupStateComputer::addSelectedState(const GeoGroupState state)
{
    if (!(d->stateMask & SelectedMask))
    {
        d->state     |= state;
        d->stateMask |= SelectedMask;
    }
    else
    {
        if      ((state&SelectedMask) == SelectedAll)
        {
            d->state |= SelectedAll;
        }
        else if ((d->state&SelectedMask) == SelectedAll)
        {
            d->state |= SelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void GroupStateComputer::addFilteredPositiveState(const GeoGroupState state)
{
    if (!(d->stateMask & FilteredPositiveMask))
    {
        d->state     |= state;
        d->stateMask |= FilteredPositiveMask;
    }
    else
    {
        if      ((state&FilteredPositiveMask) == FilteredPositiveAll)
        {
            d->state |= FilteredPositiveAll;
        }
        else if ((d->state&FilteredPositiveMask) == FilteredPositiveAll)
        {
            d->state |= FilteredPositiveSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

void GroupStateComputer::addRegionSelectedState(const GeoGroupState state)
{
    if (!(d->stateMask & RegionSelectedMask))
    {
        d->state     |= state;
        d->stateMask |= RegionSelectedMask;
    }
    else
    {
        if      ((state&RegionSelectedMask) == RegionSelectedAll)
        {
            d->state |= RegionSelectedAll;
        }
        else if ((d->state&RegionSelectedMask) == RegionSelectedAll)
        {
            d->state |= RegionSelectedSome;
        }
        else
        {
            d->state |= state;
        }
    }
}

} // namespace Digikam
