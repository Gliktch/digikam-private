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

#include "GeoDataPoint.h"
#include "GeoDataPoint_p.h"

// Local includes

#include "MarbleGlobal.h"
#include "GeoDataTypes.h"
#include "GeoDataLatLonAltBox.h"
#include "digikam_debug.h"

namespace Marble
{

GeoDataPoint::GeoDataPoint(qreal lon, qreal lat, qreal alt,
                           GeoDataCoordinates::Unit unit)
    : GeoDataGeometry(new GeoDataPointPrivate)
{
    Q_D(GeoDataPoint);
    d->m_coordinates = GeoDataCoordinates(lon, lat, alt, unit);
    d->m_latLonAltBox = GeoDataLatLonAltBox(d->m_coordinates);
}

GeoDataPoint::GeoDataPoint(const GeoDataPoint& other)
    : GeoDataGeometry(other)
{
    Q_D(GeoDataPoint);
    const GeoDataPointPrivate* const otherD = other.d_func();

    d->m_coordinates = otherD->m_coordinates;
    d->m_latLonAltBox = otherD->m_latLonAltBox;
}

GeoDataPoint::GeoDataPoint(const GeoDataCoordinates& other)
    : GeoDataGeometry(new GeoDataPointPrivate)
{
    Q_D(GeoDataPoint);
    d->m_coordinates = other;
    d->m_latLonAltBox = GeoDataLatLonAltBox(d->m_coordinates);
}

GeoDataPoint::GeoDataPoint()
    : GeoDataGeometry(new GeoDataPointPrivate)
{
    // nothing to do
}

GeoDataPoint::~GeoDataPoint()
{
    // nothing to do
}

EnumGeometryId GeoDataPoint::geometryId() const
{
    return GeoDataPointId;
}

GeoDataGeometry* GeoDataPoint::copy() const
{
    return new GeoDataPoint(*this);
}

bool GeoDataPoint::operator==(const GeoDataPoint& other) const
{
    return equals(other) &&
           coordinates() == other.coordinates();
}

bool GeoDataPoint::operator!=(const GeoDataPoint& other) const
{
    return !this->operator==(other);
}

void GeoDataPoint::setCoordinates(const GeoDataCoordinates& coordinates)
{
    detach();

    Q_D(GeoDataPoint);
    d->m_coordinates = coordinates;
    d->m_latLonAltBox = GeoDataLatLonAltBox(d->m_coordinates);
}

const GeoDataCoordinates& GeoDataPoint::coordinates() const
{
    Q_D(const GeoDataPoint);
    return d->m_coordinates;
}

const char* GeoDataPoint::nodeType() const
{
    return GeoDataTypes::GeoDataPointType;
}

void GeoDataPoint::detach()
{
    GeoDataGeometry::detach();
}

void GeoDataPoint::pack(QDataStream& stream) const
{
    Q_D(const GeoDataPoint);
    d->m_coordinates.pack(stream);
    // TODO: what about m_latLonAltBox and base class?
}

void GeoDataPoint::unpack(QDataStream& stream)
{
    Q_D(GeoDataPoint);
    d->m_coordinates.unpack(stream);
    // TODO: what about m_latLonAltBox and base class?
}

} // namespace Marble
