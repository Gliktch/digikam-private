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
 * This file contains the headers for CylindricalProjection.
 */

#pragma once

// Local includes

#include "AbstractProjection.h"

namespace Marble
{

class CylindricalProjectionPrivate;
class AbstractProjectionPrivate;

/**
 * @short A base class for the Equirectangular and Mercator projections in Marble
 */
class CylindricalProjection : public AbstractProjection
{
    /// @note Not a QObject so far because we don't need to send signals.

public:

    CylindricalProjection();

    ~CylindricalProjection()                                      override;

    bool repeatableX()                                      const override
    {
        return true;
    };

    bool traversablePoles()                                 const override
    {
        return false;
    }
    bool traversableDateLine()                              const override
    {
        return false;
    }

    SurfaceType surfaceType()                               const override
    {
        return Cylindrical;
    }

    bool screenCoordinates(const GeoDataLineString& lineString,
                           const ViewportParams* viewport,
                           QVector<QPolygonF*>& polygons)   const override;

    using AbstractProjection::screenCoordinates;

    QPainterPath mapShape(const ViewportParams* viewport)   const override;

protected:

    explicit CylindricalProjection(CylindricalProjectionPrivate* dd);

private:

    Q_DECLARE_PRIVATE(CylindricalProjection)
    Q_DISABLE_COPY(CylindricalProjection)
};

} // namespace Marble
