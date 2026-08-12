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

#include "GeoDataScale.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataScalePrivate
{
public:

    double m_x;

    double m_y;

    double m_z;

    GeoDataScalePrivate();
};

GeoDataScalePrivate::GeoDataScalePrivate()
    : m_x(1),
      m_y(1),
      m_z(1)
{
    // nothing to do
}

GeoDataScale::GeoDataScale()
    : d(new GeoDataScalePrivate)
{
    // nothing to do
}

GeoDataScale::GeoDataScale(const Marble::GeoDataScale& other)
    : GeoDataObject(other),
      d(new GeoDataScalePrivate(*other.d))
{
    // nothing to do
}

GeoDataScale& GeoDataScale::operator=(const GeoDataScale& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}


bool GeoDataScale::operator==(const GeoDataScale& other) const
{
    return equals(other) &&
           d->m_x == other.d->m_x &&
           d->m_y == other.d->m_y &&
           d->m_z == other.d->m_z;
}

bool GeoDataScale::operator!=(const GeoDataScale& other) const
{
    return !this->operator==(other);
}

GeoDataScale::~GeoDataScale()
{
    delete d;
}

const char* GeoDataScale::nodeType() const
{
    return GeoDataTypes::GeoDataScaleType;
}

double GeoDataScale::x() const
{
    return d->m_x;
}

void GeoDataScale::setX(double x)
{
    d->m_x = x;
}

double GeoDataScale::y() const
{
    return d->m_y;
}

void GeoDataScale::setY(double y)
{
    d->m_y = y;
}

double GeoDataScale::z() const
{
    return d->m_z;
}

void GeoDataScale::setZ(double z)
{
    d->m_z = z;
}

} // namespace Marble
