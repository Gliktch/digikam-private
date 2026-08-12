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

#include "GeoDataOverlay_p.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataScreenOverlayPrivate : public GeoDataOverlayPrivate
{
public:

    GeoDataScreenOverlayPrivate();

    GeoDataVec2  m_overlayXY;
    GeoDataVec2  m_screenXY;
    GeoDataVec2  m_rotationXY;
    GeoDataVec2  m_size;
    qreal        m_rotation;
};

GeoDataScreenOverlayPrivate::GeoDataScreenOverlayPrivate()
    : m_rotation(0.0)
{
}

} // namespace Marble
