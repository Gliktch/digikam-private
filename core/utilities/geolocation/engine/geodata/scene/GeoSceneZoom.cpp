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

#include "GeoSceneZoom.h"

// Local includes

#include "GeoSceneTypes.h"

namespace Marble
{

GeoSceneZoom::GeoSceneZoom()
    : GeoNode(),
      // Arbitrary defaults, as used in Marble
      m_minimum(900),
      m_maximum(2500),
      m_discrete(false)
{
}

int GeoSceneZoom::minimum() const
{
    return m_minimum;
}

void GeoSceneZoom::setMinimum(int minimum)
{
    m_minimum = minimum;
}

int GeoSceneZoom::maximum() const
{
    return m_maximum;
}

void GeoSceneZoom::setMaximum(int maximum)
{
    m_maximum = maximum;
}

bool GeoSceneZoom::discrete() const
{
    return m_discrete;
}

void GeoSceneZoom::setDiscrete(bool discrete)
{
    m_discrete = discrete;
}

const char* GeoSceneZoom::nodeType() const
{
    return GeoSceneTypes::GeoSceneZoomType;
}

} // namespace Marble
