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

#include "GeoDataLocation.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataLocationPrivate
{
public:

    GeoDataCoordinates m_coordinates;

    GeoDataLocationPrivate();
};

GeoDataLocationPrivate::GeoDataLocationPrivate()
    : m_coordinates()
{
    // nothing to do
}

GeoDataLocation::GeoDataLocation() : d(new GeoDataLocationPrivate)
{
    // nothing to do
}

GeoDataLocation::GeoDataLocation(const Marble::GeoDataLocation& other) :
    GeoDataObject(other), d(new GeoDataLocationPrivate(*other.d))
{
    // nothing to do
}

GeoDataLocation& GeoDataLocation::operator=(const GeoDataLocation& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}


bool GeoDataLocation::operator==(const GeoDataLocation& other) const
{
    return equals(other) &&
           d->m_coordinates == other.d->m_coordinates;
}

bool GeoDataLocation::operator!=(const GeoDataLocation& other) const
{
    return !this->operator==(other);
}

GeoDataLocation::~GeoDataLocation()
{
    delete d;
}

const char* GeoDataLocation::nodeType() const
{
    return GeoDataTypes::GeoDataLocationType;
}

qreal GeoDataLocation::altitude() const
{
    return d->m_coordinates.altitude();
}

void GeoDataLocation::setAltitude(qreal altitude)
{

    d->m_coordinates.setAltitude(altitude);
}

qreal GeoDataLocation::latitude(GeoDataCoordinates::Unit unit) const
{
    return d->m_coordinates.latitude(unit);
}

void GeoDataLocation::setLatitude(qreal latitude, GeoDataCoordinates::Unit unit)
{
    d->m_coordinates.setLatitude(latitude, unit);
}

qreal GeoDataLocation::longitude(GeoDataCoordinates::Unit unit) const
{
    return d->m_coordinates.longitude(unit);
}

void GeoDataLocation::setLongitude(qreal longitude, GeoDataCoordinates::Unit unit)
{
    d->m_coordinates.setLongitude(longitude, unit);
}

} // namespace Marble
