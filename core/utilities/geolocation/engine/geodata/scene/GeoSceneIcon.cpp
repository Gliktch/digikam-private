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

#include "GeoSceneIcon.h"

// Local includes

#include "GeoSceneTypes.h"

namespace Marble
{

GeoSceneIcon::GeoSceneIcon()
    : m_color()
{
}

GeoSceneIcon::~GeoSceneIcon()
{
}

QString GeoSceneIcon::pixmap() const
{
    return m_pixmap;
}

void GeoSceneIcon::setPixmap(const QString& pixmap)
{
    m_pixmap = pixmap;
}

QColor GeoSceneIcon::color() const
{
    return m_color;
}

void GeoSceneIcon::setColor(const QColor& color)
{
    m_color = color;
}

const char* GeoSceneIcon::nodeType() const
{
    return GeoSceneTypes::GeoSceneIconType;
}

} // namespace Marble
