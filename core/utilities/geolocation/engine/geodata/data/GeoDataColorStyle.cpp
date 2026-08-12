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

#include "GeoDataColorStyle.h"

// C++ includes

#include <cstdlib>

// Qt includes

#include <QDataStream>
#include <QColor>
#include <QRandomGenerator>

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataColorStylePrivate
{
public:

    GeoDataColorStylePrivate()
        : m_color(Qt::white),
          m_colorMode(GeoDataColorStyle::Normal)
    {
    }

    /// stores the current color
    QColor     m_color;

    /// stores random color
    QColor     m_randomColor;

    /// stores the current color mode
    GeoDataColorStyle::ColorMode  m_colorMode;
};

GeoDataColorStyle::GeoDataColorStyle()
    : d(new GeoDataColorStylePrivate)
{
}

GeoDataColorStyle::GeoDataColorStyle(const GeoDataColorStyle& other)
    : GeoDataObject(other),
      d(new GeoDataColorStylePrivate(*other.d))
{
}

GeoDataColorStyle::~GeoDataColorStyle()
{
    delete d;
}

GeoDataColorStyle& GeoDataColorStyle::operator=(const GeoDataColorStyle& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataColorStyle::operator==(const GeoDataColorStyle& other) const
{
    return equals(other) && d->m_color == other.d->m_color &&
           d->m_colorMode == other.d->m_colorMode;
}

bool GeoDataColorStyle::operator!=(const GeoDataColorStyle& other) const
{
    return !this->operator==(other);
}

const char* GeoDataColorStyle::nodeType() const
{
    return GeoDataTypes::GeoDataColorStyleType;
}

void GeoDataColorStyle::setColor(const QColor& value)
{
    d->m_color = value;

    qreal red = d->m_color.redF();
    qreal green = d->m_color.greenF();
    qreal blue = d->m_color.blueF();
    d->m_randomColor = d->m_color;
    qreal const randMax = RAND_MAX;
    d->m_randomColor.setRedF(red * (QRandomGenerator::global()->generate() / randMax));
    d->m_randomColor.setGreenF(green * (QRandomGenerator::global()->generate() / randMax));
    d->m_randomColor.setBlueF(blue * (QRandomGenerator::global()->generate() / randMax));
}

QColor GeoDataColorStyle::color() const
{
    return d->m_color;
}

QColor GeoDataColorStyle::paintedColor() const
{
    return d->m_colorMode == Normal ? d->m_color : d->m_randomColor;
}

void GeoDataColorStyle::setColorMode(ColorMode colorMode)
{
    d->m_colorMode = colorMode;
}

GeoDataColorStyle::ColorMode GeoDataColorStyle::colorMode() const
{
    return d->m_colorMode;
}

void GeoDataColorStyle::pack(QDataStream& stream) const
{
    GeoDataObject::pack(stream);

    stream << d->m_color;
    // FIXME: Why is not colorMode saved?
    //    stream << m_colorMode;
}

void GeoDataColorStyle::unpack(QDataStream& stream)
{
    GeoDataObject::unpack(stream);

    stream >> d->m_color;
    // FIXME: Why is not colorMode saved?
    //    stream >> m_colorMode;
}

QString Marble::GeoDataColorStyle::contrastColor(const QColor& color)
{
    return color.valueF() > 0.85 ? QStringLiteral("black") : QStringLiteral("white");
}

} // namespace Marble
