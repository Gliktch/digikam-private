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

#include "AbstractProjection_p.h"

namespace Marble
{

class CylindricalProjection;

class Q_DECL_HIDDEN CylindricalProjectionPrivate : public AbstractProjectionPrivate
{
public:

    explicit CylindricalProjectionPrivate(CylindricalProjection* parent);

    /**
     * This method tessellates a line segment in a way that the line segment
     * follows great circles. The count parameter specifies the
     * number of nodes generated for the polygon. If the
     * clampToGround flag is added the polygon contains count + 2
     * nodes as the clamped down start and end node get added.
     */
    int tessellateLineSegment(const GeoDataCoordinates& aCoords,
                              qreal ax, qreal ay,
                              const GeoDataCoordinates& bCoords,
                              qreal bx, qreal by,
                              const QVector<QPolygonF*>& polygons,
                              const ViewportParams* viewport,
                              TessellationFlags f = TessellationFlags(),
                              int mirrorCount = 0,
                              qreal repeatDistance = 0)                     const;

    int processTessellation(const GeoDataCoordinates& previousCoords,
                            const GeoDataCoordinates& currentCoords,
                            int count,
                            const QVector<QPolygonF*>& polygons,
                            const ViewportParams* viewport,
                            TessellationFlags f = TessellationFlags(),
                            int mirrorCount = 0,
                            qreal repeatDistance = 0)                       const;

    bool lineStringToPolygon(const GeoDataLineString& lineString,
                             const ViewportParams* viewport,
                             QVector<QPolygonF*>& polygons)                 const;

    void repeatPolygons(const ViewportParams* viewport,
                        QVector<QPolygonF*>& polygons)                      const;

    qreal repeatDistance(const ViewportParams* viewport)                    const;

public:

    static int crossDateLine(const GeoDataCoordinates& aCoord,
                             const GeoDataCoordinates& bCoord,
                             qreal bx,
                             qreal by,
                             const QVector<QPolygonF*>& polygons,
                             int mirrorCount = 0,
                             qreal repeatDistance = 0);

    static void translatePolygons(const QVector<QPolygonF*>& polygons,
                                  QVector<QPolygonF*>& translatedPolygons,
                                  qreal xOffset);

public:

    Q_DECLARE_PUBLIC(CylindricalProjection)

protected:

    CylindricalProjection* const q_ptr = nullptr;
};

} // namespace Marble
