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

#include <Qt>

// Local includes

#include "MarbleGraphicsItem.h"
#include "digikam_export.h"

class QPointF;

namespace Marble
{

class GeoDataCoordinates;
class BillboardGraphicsItemPrivate;

/**
 * @brief Base class for all 2D labels (a.k.a. billboards) in 3D space.
 *
 * A BillboardGraphicsItem represents a point of interest on a planet with a geo location.
 * As a result, it may appear more than once on the map, depending on the projection and
 * the zoom level.
 */
class DIGIKAM_EXPORT BillboardGraphicsItem : public MarbleGraphicsItem
{
public:

    BillboardGraphicsItem();

    GeoDataCoordinates coordinate()                             const;
    void setCoordinate(const GeoDataCoordinates& coordinates);

    Qt::Alignment alignment()                                   const;
    void setAlignment(Qt::Alignment alignment);

    /**
     * @brief Returns the absolute screen positions of the item.
     *
     * All elements indicate the position of the topleft corner in screen coordinates and are
     * based on the viewport of the last paintEvent(). Note that the result list may contain
     * an unbound number of elements, including zero, depending on the item's visibility,
     * the projection and zoom level.
     * @since 0.26.0
     */
    QVector<QPointF> positions()                                const;

    /**
     * @brief Returns the absolute screen bounding boxes of the item.
     * @since 0.26.0
     */
    QVector<QRectF> boundingRects()                             const;

    /**
     * Returns the rect of one representation of the object that is at the given position.
     */
    QRectF containsRect(const QPointF& point)                   const;

private:

    Q_DECLARE_PRIVATE(BillboardGraphicsItem)
};

} // Marble namespace
