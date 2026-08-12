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

#include "GeoDataVec2.h"

// Local includes

#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataVec2Private
{
public:

    GeoDataVec2Private();

    static GeoDataVec2::Unit parseUnits(const QString& value);

public:

    GeoDataVec2::Unit  m_xunit;
    GeoDataVec2::Unit  m_yunit;
};

GeoDataVec2Private::GeoDataVec2Private()
    : m_xunit(GeoDataVec2::Fraction),
      m_yunit(GeoDataVec2::Fraction)
{
}

GeoDataVec2::GeoDataVec2()
    : d(new GeoDataVec2Private)
{
}

GeoDataVec2::GeoDataVec2(qreal x, qreal y, const QString& xunits, const QString& yunits)
    : d(new GeoDataVec2Private)
{
    setX(x);
    setY(y);
    d->m_xunit = GeoDataVec2Private::parseUnits(xunits);
    d->m_yunit = GeoDataVec2Private::parseUnits(yunits);
}

GeoDataVec2::Unit GeoDataVec2Private::parseUnits(const QString& value)
{
    if (value == QLatin1String("fraction"))
    {
        return GeoDataVec2::Fraction;
    }

    if (value == QLatin1String("pixels"))
    {
        return GeoDataVec2::Pixels;
    }

    if (value == QLatin1String("insetPixels"))
    {
        return GeoDataVec2::InsetPixels;
    }

    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Warning: Unknown units value " << value << " - falling back to default 'fraction'";
    return GeoDataVec2::Fraction;
}

GeoDataVec2::GeoDataVec2(const Marble::GeoDataVec2& other) :
    QPointF(other), d(new GeoDataVec2Private(*other.d))
{
}

GeoDataVec2& GeoDataVec2::operator=(const GeoDataVec2& other)
{
    QPointF::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataVec2::operator==(const GeoDataVec2& other) const
{
    return x() == other.x() && y() == other.y() &&
           d->m_xunit == other.d->m_xunit && d->m_yunit == other.d->m_yunit;
}

bool GeoDataVec2::operator!=(const GeoDataVec2& other) const
{
    return !this->operator==(other);
}

GeoDataVec2::~GeoDataVec2()
{
    delete d;
}

GeoDataVec2::Unit GeoDataVec2::xunit() const
{
    return d->m_xunit;
}

void GeoDataVec2::setXunits(Unit xunit)
{
    d->m_xunit = xunit;
}

GeoDataVec2::Unit GeoDataVec2::yunit() const
{
    return d->m_yunit;
}

void GeoDataVec2::setYunits(Unit yunit)
{
    d->m_yunit = yunit;
}

} // namespace Marble
