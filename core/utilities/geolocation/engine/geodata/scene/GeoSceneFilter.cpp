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

#include "GeoSceneFilter.h"

// Local includes

#include "GeoScenePalette.h"
#include "GeoSceneTypes.h"

namespace Marble
{

GeoSceneFilter::GeoSceneFilter(const QString& name)
    : m_name(name),
      m_type(QLatin1String("none"))
{
}

GeoSceneFilter::~GeoSceneFilter()
{
    qDeleteAll(m_palette);
}

QString GeoSceneFilter::name() const
{
    return m_name;
}

void GeoSceneFilter::setName(const QString& name)
{
    m_name = name;
}

QString GeoSceneFilter::type() const
{
    return m_type;
}

void GeoSceneFilter::setType(const QString& type)
{
    m_type = type;
}

QList<const GeoScenePalette*> GeoSceneFilter::palette() const
{
    return m_palette;
}

void GeoSceneFilter::addPalette(const GeoScenePalette* palette)
{
    m_palette.append(palette);
}

int GeoSceneFilter::removePalette(const GeoScenePalette* palette)
{
    return m_palette.removeAll(palette);
}

const char* GeoSceneFilter::nodeType() const
{
    return GeoSceneTypes::GeoSceneFilterType;
}

} // namespace Marble
