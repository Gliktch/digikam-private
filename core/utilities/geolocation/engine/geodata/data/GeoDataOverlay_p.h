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

#include <QImage>
#include <QColor>

// Local includes

#include "GeoDataFeature_p.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataOverlayPrivate : public GeoDataFeaturePrivate
{
public:

    QColor          m_color;

    int             m_drawOrder;

    mutable QImage  m_image;

    QString         m_iconPath;

public:

    GeoDataOverlayPrivate()
        : m_color(Qt::white),
          m_drawOrder(0)
    {
    }

    GeoDataOverlayPrivate(const GeoDataOverlayPrivate& other)
        : GeoDataFeaturePrivate(other),
          m_color(Qt::white),
          m_drawOrder(0)
    {
    }
};

} // namespace Marble
