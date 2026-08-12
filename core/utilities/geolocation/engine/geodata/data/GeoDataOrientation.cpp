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

#include "GeoDataOrientation.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataOrientationPrivate
{
public:

    double m_heading;

    double m_tilt;

    double m_roll;

    GeoDataOrientationPrivate();
};

GeoDataOrientationPrivate::GeoDataOrientationPrivate()
    : m_heading(0),
      m_tilt(0),
      m_roll(0)
{
    // nothing to do
}

GeoDataOrientation::GeoDataOrientation()
    : d(new GeoDataOrientationPrivate)
{
    // nothing to do
}

GeoDataOrientation::GeoDataOrientation(const Marble::GeoDataOrientation& other) :
    GeoDataObject(other), d(new GeoDataOrientationPrivate(*other.d))
{
    // nothing to do
}

GeoDataOrientation& GeoDataOrientation::operator=(const GeoDataOrientation& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataOrientation::operator==(const GeoDataOrientation& other) const
{
    return equals(other) &&
           d->m_heading == other.d->m_heading &&
           d->m_roll == other.d->m_roll &&
           d->m_tilt == other.d->m_tilt;
}

bool GeoDataOrientation::operator!=(const GeoDataOrientation& other) const
{
    return !this->operator==(other);
}

GeoDataOrientation::~GeoDataOrientation()
{
    delete d;
}

const char* GeoDataOrientation::nodeType() const
{
    return GeoDataTypes::GeoDataOrientationType;
}

double GeoDataOrientation::heading() const
{
    return d->m_heading;
}

void GeoDataOrientation::setHeading(double heading)
{
    d->m_heading = heading;
}

double GeoDataOrientation::tilt() const
{
    return d->m_tilt;
}

void GeoDataOrientation::setTilt(double tilt)
{
    d->m_tilt = tilt;
}

double GeoDataOrientation::roll() const
{
    return d->m_roll;
}

void GeoDataOrientation::setRoll(double roll)
{
    d->m_roll = roll;
}

} // namespace Marble
