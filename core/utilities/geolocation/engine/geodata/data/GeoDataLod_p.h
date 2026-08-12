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

#pragma once

// Local includes

#include "GeoDataLod.h"
#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataLodPrivate
{
public:

    GeoDataLodPrivate()
        : m_minLodPixels(0),
          m_maxLodPixels(-1),
          m_minFadeExtent(0),
          m_maxFadeExtent(0)
    {
    }

    qreal m_minLodPixels;
    qreal m_maxLodPixels;
    qreal m_minFadeExtent;
    qreal m_maxFadeExtent;
};

} // namespace Marble
