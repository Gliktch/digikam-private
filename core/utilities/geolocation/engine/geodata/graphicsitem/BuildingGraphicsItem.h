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

#include "AbstractGeoPolygonGraphicsItem.h"
#include "GeoDataCoordinates.h"

class QPointF;

namespace Marble
{

class DIGIKAM_EXPORT BuildingGraphicsItem : public AbstractGeoPolygonGraphicsItem
{
public:

    BuildingGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataBuilding* building);
    ~BuildingGraphicsItem()                                                                           override;

public:

    void paint(GeoPainter* painter, const ViewportParams* viewport,
               const QString& layer, int tileZoomLevel)                                               override;

private:

    void paintFrame(GeoPainter* painter, const ViewportParams* viewport);

    void paintRoof(GeoPainter* painter, const ViewportParams* viewport);
    bool configurePainterForFrame(GeoPainter* painter)                                          const;
    void initializeBuildingPainting(const GeoPainter* painter, const ViewportParams* viewport,
                                    bool& drawAccurate3D, bool& isCameraAboveBuilding)          const;
    void updatePolygons(const ViewportParams& viewport,
                        QVector<QPolygonF*>& outlinePolygons,
                        QVector<QPolygonF*>& innerPolygons,
                        bool& hasInnerBoundaries)                                               const;

    QPointF buildingOffset(const QPointF& point, const ViewportParams* viewport,
                           bool* isCameraAboveBuilding = nullptr)                               const;

    static QPointF centroid(const QPolygonF& polygon, double& area);
    static void screenPolygons(
                               const ViewportParams& viewport,
                               const GeoDataPolygon* polygon,
                               QVector<QPolygonF*>& polygons,
                               QVector<QPolygonF*>& outlines
                              );

    bool contains(const QPoint& screenPosition, const ViewportParams* viewport)                 const override;

private:

    QVector<QPolygonF*> m_cachedOuterPolygons;
    QVector<QPolygonF*> m_cachedInnerPolygons;
    QVector<QPolygonF*> m_cachedOuterRoofPolygons;
    QVector<QPolygonF*> m_cachedInnerRoofPolygons;
    bool                m_hasInnerBoundaries        = false;
};

} // namespace Marble
