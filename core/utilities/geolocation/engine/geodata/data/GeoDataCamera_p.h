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

// Qt includes

#include <QAtomicInt>

// Local includes

#include "GeoDataCoordinates.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataCameraPrivate
{
public:

    GeoDataCameraPrivate()
        : m_coordinates (),
          m_roll        (0.0),
          m_heading     (0.0),
          m_tilt        (0.0),
          ref           (1)
    {
    }

    GeoDataCoordinates  m_coordinates;
    qreal               m_roll          = 0.0;
    qreal               m_heading       = 0.0;
    qreal               m_tilt          = 0.0;

    QAtomicInt          ref             = 1;
};

} // namespace Marble
