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

#include "GeoDataPolyStyle.h"

// Qt includes

#include <QDataStream>
#include <QImage>

// Local includes

#include "GeoDataTypes.h"
#include "MarbleDirs.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataPolyStylePrivate
{
public:

    GeoDataPolyStylePrivate()
        : m_fill(true)
        , m_outline(true)
        , m_brushStyle(Qt::SolidPattern)
        , m_colorIndex(0)
    {
    }

    /// whether to fill the polygon
    bool            m_fill;

    /// whether to draw the outline
    bool            m_outline;
    Qt::BrushStyle  m_brushStyle;

    /// The value of colorIndex will be mapped to a color for brush
    quint8          m_colorIndex;
    QString         m_texturePath;
    QImage          m_textureImage;
};

GeoDataPolyStyle::GeoDataPolyStyle()
    : d(new GeoDataPolyStylePrivate)
{
}

GeoDataPolyStyle::GeoDataPolyStyle(const GeoDataPolyStyle& other)
    : GeoDataColorStyle(other), d(new GeoDataPolyStylePrivate(*other.d))
{
}

GeoDataPolyStyle::GeoDataPolyStyle(const QColor& color)
    : d(new GeoDataPolyStylePrivate)
{
    setColor(color);
}

GeoDataPolyStyle::~GeoDataPolyStyle()
{
    delete d;
}

GeoDataPolyStyle& GeoDataPolyStyle::operator=(const GeoDataPolyStyle& other)
{
    GeoDataColorStyle::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataPolyStyle::operator==(const GeoDataPolyStyle& other) const
{
    if (GeoDataColorStyle::operator!=(other))
    {
        return false;
    }

    return d->m_fill == other.d->m_fill &&
           d->m_outline == other.d->m_outline &&
           d->m_brushStyle == other.d->m_brushStyle;
}

bool GeoDataPolyStyle::operator!=(const GeoDataPolyStyle& other) const
{
    return !this->operator==(other);
}

const char* GeoDataPolyStyle::nodeType() const
{
    return GeoDataTypes::GeoDataPolyStyleType;
}

void GeoDataPolyStyle::setFill(bool fill)
{
    d->m_fill = fill;
}

bool GeoDataPolyStyle::fill() const
{
    return d->m_fill;
}

void GeoDataPolyStyle::setOutline(bool outline)
{
    d->m_outline = outline;
}

bool GeoDataPolyStyle::outline() const
{
    return d->m_outline;
}

void GeoDataPolyStyle::setBrushStyle(const Qt::BrushStyle style)
{
    d->m_brushStyle = style;
}

Qt::BrushStyle GeoDataPolyStyle::brushStyle() const
{
    return d->m_brushStyle;
}

void GeoDataPolyStyle::setColorIndex(quint8 colorIndex)
{
    d->m_colorIndex = colorIndex;
}

quint8 GeoDataPolyStyle::colorIndex() const
{
    return d->m_colorIndex;
}

void GeoDataPolyStyle::setTexturePath(const QString& texturePath)
{
    d->m_texturePath = texturePath;
    d->m_textureImage = QImage();
}

QString GeoDataPolyStyle::texturePath() const
{
    return d->m_texturePath;
}

QImage GeoDataPolyStyle::textureImage() const
{
    if (!d->m_textureImage.isNull())
    {
        return d->m_textureImage;
    }

    else if (!d->m_texturePath.isEmpty())
    {
        d->m_textureImage = QImage(resolvePath(d->m_texturePath));
    }

    return d->m_textureImage;
}

void GeoDataPolyStyle::pack(QDataStream& stream) const
{
    GeoDataColorStyle::pack(stream);

    stream << d->m_fill;
    stream << d->m_outline;
    stream << d->m_colorIndex;
}

void GeoDataPolyStyle::unpack(QDataStream& stream)
{
    GeoDataColorStyle::unpack(stream);

    stream >> d->m_fill;
    stream >> d->m_outline;
    stream >> d->m_colorIndex;
}

} // namespace Marble
