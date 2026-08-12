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

#include "GeoDataLookAt.h"
#include "GeoDataLookAt_p.h"

// Local includes

#include "GeoDataTypes.h"
#include "digikam_debug.h"

namespace Marble
{

GeoDataLookAt::GeoDataLookAt()
    : GeoDataAbstractView(),
      d                  (new GeoDataLookAtPrivate)
{
}

GeoDataLookAt::GeoDataLookAt(const GeoDataLookAt& other)
    : GeoDataAbstractView(),
      d                  (other.d)
{
    d->ref.ref();
}

GeoDataLookAt& GeoDataLookAt::operator=(const GeoDataLookAt& other)
{
    GeoDataAbstractView::operator=(other);
    qAtomicAssign(d, other.d);

    return *this;
}

bool GeoDataLookAt::operator==(const GeoDataLookAt& other) const
{
    return (
            equals(other)                                &&
            (d->m_coordinates == other.d->m_coordinates) &&
            (d->m_range       == other.d->m_range)
           );
}

bool GeoDataLookAt::operator!=(const GeoDataLookAt& other) const
{
    return (!this->operator==(other));
}

GeoDataLookAt::~GeoDataLookAt()
{
    if (!d->ref.deref())
    {
        delete d;
    }
}

GeoDataAbstractView* GeoDataLookAt::copy() const
{
    return new GeoDataLookAt(*this);
}

void GeoDataLookAt::setCoordinates(const GeoDataCoordinates& coordinates)
{
    d->m_coordinates = coordinates;
}

const char* GeoDataLookAt::nodeType() const
{
    return GeoDataTypes::GeoDataLookAtType;
}

void GeoDataLookAt::setAltitude(qreal altitude)
{
    detach();
    d->m_coordinates.setAltitude(altitude);
}

qreal GeoDataLookAt::altitude() const
{
    return d->m_coordinates.altitude();
}

void GeoDataLookAt::setLatitude(qreal latitude, GeoDataCoordinates::Unit unit)
{
    detach();
    d->m_coordinates.setLatitude(latitude, unit);
}

qreal GeoDataLookAt::latitude(GeoDataCoordinates::Unit unit) const
{
    return d->m_coordinates.latitude(unit);
}

void GeoDataLookAt::setLongitude(qreal longitude, GeoDataCoordinates::Unit unit)
{
    detach();
    d->m_coordinates.setLongitude(longitude, unit);
}

qreal GeoDataLookAt::longitude(GeoDataCoordinates::Unit unit) const
{
    return d->m_coordinates.longitude(unit);
}

GeoDataCoordinates GeoDataLookAt::coordinates() const
{
    return d->m_coordinates;
}

void GeoDataLookAt::setRange(qreal range)
{
    detach();
    d->m_range = range;
}

qreal GeoDataLookAt::range() const
{
    return d->m_range;
}

void GeoDataLookAt::detach()
{
    qAtomicDetach(d);
}

} // namespace Marble
