/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "GeoDataAnimatedUpdate.h"

// Local includes

#include "GeoDataUpdate.h"
#include "GeoDataTypes.h"
#include "GeoDataAbstractView.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataAnimatedUpdatePrivate
{
public:

    double          m_duration;
    double          m_delayedStart;
    GeoDataUpdate*  m_update = nullptr;

    GeoDataAnimatedUpdatePrivate();
};

GeoDataAnimatedUpdatePrivate::GeoDataAnimatedUpdatePrivate() :
    m_duration(0.0), m_delayedStart(0), m_update(nullptr)
{
}

GeoDataAnimatedUpdate::GeoDataAnimatedUpdate() : d(new GeoDataAnimatedUpdatePrivate)
{
}

GeoDataAnimatedUpdate::GeoDataAnimatedUpdate(const Marble::GeoDataAnimatedUpdate& other) :
    GeoDataTourPrimitive(other), d(new GeoDataAnimatedUpdatePrivate(*other.d))
{
}

GeoDataAnimatedUpdate& GeoDataAnimatedUpdate::operator=(const GeoDataAnimatedUpdate& other)
{
    GeoDataTourPrimitive::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataAnimatedUpdate::operator==(const GeoDataAnimatedUpdate& other) const
{
    if ((!d->m_update && other.d->m_update) || (d->m_update && !other.d->m_update))
    {
        return false;
    }

    else if (d->m_update && other.d->m_update)
    {
        return d->m_duration == other.d->m_duration && *(d->m_update) == *(other.d->m_update);
    }

    return d->m_duration == other.d->m_duration;
}

bool GeoDataAnimatedUpdate::operator!=(const GeoDataAnimatedUpdate& other) const
{
    return !this->operator==(other);
}

GeoDataAnimatedUpdate::~GeoDataAnimatedUpdate()
{
    delete d;
}

const char* GeoDataAnimatedUpdate::nodeType() const
{
    return GeoDataTypes::GeoDataAnimatedUpdateType;
}

const GeoDataUpdate* GeoDataAnimatedUpdate::update() const
{
    return d->m_update;
}

GeoDataUpdate* GeoDataAnimatedUpdate::update()
{
    return d->m_update;
}

void GeoDataAnimatedUpdate::setUpdate(GeoDataUpdate* update)
{
    delete d->m_update;
    d->m_update = update;

    if (d->m_update)
    {
        d->m_update->setParent(this);
    }
}

double GeoDataAnimatedUpdate::duration() const
{
    return d->m_duration;
}

void GeoDataAnimatedUpdate::setDuration(double duration)
{
    d->m_duration = duration;
}

double GeoDataAnimatedUpdate::delayedStart() const
{
    return d->m_delayedStart;
}

void GeoDataAnimatedUpdate::setDelayedStart(double delayedStart)
{
    d->m_delayedStart = delayedStart;
}

} // namespace Marble
