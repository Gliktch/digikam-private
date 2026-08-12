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

#include "GeoSceneGeodata.h"

// Local includes

#include "GeoSceneTypes.h"
#include "digikam_debug.h"

namespace Marble
{
GeoSceneGeodata::GeoSceneGeodata(const QString& name)
    : GeoSceneAbstractDataset(name),
      m_sourceFile(QString()),
      m_alpha(1.0),
      m_pen(QPen(Qt::NoPen)),
      m_brush(QBrush(Qt::transparent)),
      m_renderOrder(0)
{
}

GeoSceneGeodata::~GeoSceneGeodata()
{
}

const char* GeoSceneGeodata::nodeType() const
{
    return GeoSceneTypes::GeoSceneGeodataType;
}

bool GeoSceneGeodata::operator==(const GeoSceneGeodata& other) const
{
    return m_sourceFile == other.sourceFile()
           && m_pen == other.pen()
           && m_brush == other.brush();
}

QString GeoSceneGeodata::property() const
{
    return m_property;
}

void GeoSceneGeodata::setProperty(const QString& property)
{
    m_property = property;
}

QString GeoSceneGeodata::sourceFile() const
{
    return m_sourceFile;
}

void GeoSceneGeodata::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

QString GeoSceneGeodata::colorize() const
{
    return m_colorize;
}

void GeoSceneGeodata::setColorize(const QString& colorize)
{
    m_colorize = colorize;
}

QPen GeoSceneGeodata::pen() const
{
    return m_pen;
}

void GeoSceneGeodata::setAlpha(qreal alpha)
{
    m_alpha = alpha;
}

qreal GeoSceneGeodata::alpha() const
{
    return m_alpha;
}

void GeoSceneGeodata::setPen(const QPen& pen)
{
    m_pen = pen;
}

QBrush GeoSceneGeodata::brush() const
{
    return m_brush;
}

void GeoSceneGeodata::setBrush(const QBrush& brush)
{
    m_brush = brush;
}

int GeoSceneGeodata::renderOrder() const
{
    return m_renderOrder;
}

void GeoSceneGeodata::setRenderOrder(int renderOrder)
{
    m_renderOrder = renderOrder;
}

QVector<QColor> GeoSceneGeodata::colors() const
{
    return m_colors;
}

void GeoSceneGeodata::setColors(const QVector<QColor>& colors)
{
    m_colors = colors;
}

} // namespace Marble
