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

/**
 * This file contains the headers for AzimuthalProjection.
 */

#pragma once

// Local includes

#include "AbstractProjection.h"

namespace Marble
{

class AzimuthalProjectionPrivate;

/**
 * @short A base class for the Gnomonic and Orthographic (Globe) projections in Marble
 */
class AzimuthalProjection : public AbstractProjection
{
    // Not a QObject so far because we don't need to send signals.

public:

    AzimuthalProjection();

    ~AzimuthalProjection()                                                    override;

public:

    bool repeatableX()                                                  const override
    {
        return false;
    }

    bool traversablePoles()                                             const override
    {
        return true;
    }
    bool traversableDateLine()                                          const override
    {
        return true;
    }

    SurfaceType surfaceType()                                           const override
    {
        return Azimuthal;
    }

    PreservationType preservationType()                                 const override
    {
        return NoPreservation;
    }

    bool isClippedToSphere()                                            const override;

    qreal clippingRadius()                                              const override;

    bool  mapCoversViewport(const ViewportParams* viewport)             const override;

    bool screenCoordinates(const GeoDataLineString& lineString,
                           const ViewportParams* viewport,
                           QVector<QPolygonF*>& polygons)               const override;

    using AbstractProjection::screenCoordinates;

    QPainterPath mapShape(const ViewportParams* viewport)               const override;

    GeoDataLatLonAltBox latLonAltBox(const QRect& screenRect,
                                     const ViewportParams* viewport)    const override;

protected:

    explicit AzimuthalProjection(AzimuthalProjectionPrivate* dd);

private:

    Q_DECLARE_PRIVATE(AzimuthalProjection)
    Q_DISABLE_COPY(AzimuthalProjection)
};

} // namespace Marble
