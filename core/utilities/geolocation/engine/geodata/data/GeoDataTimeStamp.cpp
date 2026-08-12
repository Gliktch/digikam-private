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

#include "GeoDataTimeStamp.h"
#include "GeoDataTimeStamp_p.h"

// Qt includes

#include <QDataStream>

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

GeoDataTimeStamp::GeoDataTimeStamp()
    : GeoDataTimePrimitive(),
      d(new GeoDataTimeStampPrivate)
{
}

GeoDataTimeStamp::GeoDataTimeStamp(const GeoDataTimeStamp& other)
    : GeoDataTimePrimitive(other),
      d(new GeoDataTimeStampPrivate(*other.d))
{
}

GeoDataTimeStamp::~GeoDataTimeStamp()
{
    delete d;
}

GeoDataTimeStamp& GeoDataTimeStamp::operator=(const GeoDataTimeStamp& other)
{
    GeoDataTimePrimitive::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataTimeStamp::operator==(const GeoDataTimeStamp& other) const
{
    return equals(other) &&
           d->m_resolution == other.d->m_resolution &&
           d->m_when == other.d->m_when;
}

bool GeoDataTimeStamp::operator!=(const GeoDataTimeStamp& other) const
{
    return !this->operator==(other);
}


const char* GeoDataTimeStamp::nodeType() const
{
    return GeoDataTypes::GeoDataTimeStampType;
}

QDateTime GeoDataTimeStamp::when() const
{
    return d->m_when;
}

void GeoDataTimeStamp::setWhen(const QDateTime& when)
{
    d->m_when = when;
}

void GeoDataTimeStamp::setResolution(GeoDataTimeStamp::TimeResolution resolution)
{
    d->m_resolution = resolution;
}

GeoDataTimeStamp::TimeResolution GeoDataTimeStamp::resolution() const
{
    return d->m_resolution;
}

void GeoDataTimeStamp::pack(QDataStream& stream) const
{
    GeoDataTimePrimitive::pack(stream);

    stream << d->m_when;
}

void GeoDataTimeStamp::unpack(QDataStream& stream)
{
    GeoDataTimePrimitive::unpack(stream);

    stream >> d->m_when;
}

} // namespace Marble
