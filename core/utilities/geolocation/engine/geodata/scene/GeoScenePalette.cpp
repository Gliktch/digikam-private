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

#include "GeoScenePalette.h"

// Local includes

#include "GeoSceneTypes.h"

namespace Marble
{

GeoScenePalette::GeoScenePalette(const QString& type,
                                 const QString& file)
    : m_type(type),
      m_file(file)
{
}

QString GeoScenePalette::type() const
{
    return m_type;
}

void GeoScenePalette::setType(const QString& type)
{
    m_type = type;
}

QString GeoScenePalette::file() const
{
    return m_file;
}

void GeoScenePalette::setFile(const QString& file)
{
    m_file = file;
}

bool GeoScenePalette::operator==(const GeoScenePalette& rhs) const
{
    return m_file == rhs.m_file && m_type == rhs.m_type;
}

const char* GeoScenePalette::nodeType() const
{
    return GeoSceneTypes::GeoScenePaletteType;
}

} // namespace Marble
