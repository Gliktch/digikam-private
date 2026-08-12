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

#include "GeoGraphicsItem.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataLinearRing;
class GeoDataPlacemark;
class GeoDataPolygon;
class GeoDataBuilding;

class DIGIKAM_EXPORT AbstractGeoPolygonGraphicsItem : public GeoGraphicsItem
{
protected:

    AbstractGeoPolygonGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataPolygon* polygon);
    AbstractGeoPolygonGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataLinearRing* ring);
    AbstractGeoPolygonGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataBuilding* building);
    ~AbstractGeoPolygonGraphicsItem()                                                 override;

public:

    const GeoDataLatLonAltBox& latLonAltBox()                                   const override;
    void paint(GeoPainter* painter, const ViewportParams* viewport,
               const QString& layer, int tileZoomLevel)                               override;
    bool contains(const QPoint& screenPosition,
                  const ViewportParams* viewport)                               const override;

    void setLinearRing(const GeoDataLinearRing* ring);
    void setPolygon(const GeoDataPolygon* polygon);

public:

    static const void* s_previousStyle;

protected:

    bool configurePainter(GeoPainter* painter, const ViewportParams& viewport)  const;

    inline const GeoDataPolygon* polygon()                                      const
    {
        return m_polygon;
    }

    inline const GeoDataLinearRing* ring()                                      const
    {
        return m_ring;
    }

    inline const GeoDataBuilding* building()                                    const
    {
        return m_building;
    }

    static int extractElevation(const GeoDataPlacemark& placemark);

private:

    QPixmap texture(const QString& path, const QColor& color)                   const;

private:

    const GeoDataPolygon* m_polygon         = nullptr;
    const GeoDataLinearRing* m_ring         = nullptr;
    const GeoDataBuilding* const m_building = nullptr;
};

} // namespace Marble
