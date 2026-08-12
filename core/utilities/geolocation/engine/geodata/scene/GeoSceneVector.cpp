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

#include "GeoSceneVector.h"

// Local includes

#include "GeoSceneTypes.h"

namespace Marble
{

GeoSceneVector::GeoSceneVector(const QString& name)
    : GeoSceneAbstractDataset(name),
      m_sourceFile(),
      m_feature(),
      m_pen(),
      m_brush()
{
}

const char* GeoSceneVector::nodeType() const
{
    return GeoSceneTypes::GeoSceneVectorType;
}

QString GeoSceneVector::sourceFile() const
{
    return m_sourceFile;
}

void GeoSceneVector::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

QString GeoSceneVector::feature() const
{
    return m_feature;
}

void GeoSceneVector::setFeature(const QString& feature)
{
    m_feature = feature;
}

QPen GeoSceneVector::pen() const
{
    return m_pen;
}

void GeoSceneVector::setPen(const QPen& pen)
{
    m_pen = pen;
}

QBrush GeoSceneVector::brush() const
{
    return m_brush;
}

void GeoSceneVector::setBrush(const QBrush& brush)
{
    m_brush = brush;
}

} // namespace Marble
