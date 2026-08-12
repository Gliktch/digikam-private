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

// Local includes

#include "GeoDataPoint.h"
#include "GeoGraphicsItem.h"
#include "digikam_export.h"

namespace Marble
{

class DIGIKAM_EXPORT GeoPhotoGraphicsItem : public GeoGraphicsItem
{
public:

    explicit GeoPhotoGraphicsItem(const GeoDataFeature* feature);

    void setPoint(const GeoDataPoint& point);

    GeoDataPoint point() const;

    void paint(GeoPainter* painter, const ViewportParams* viewport, const QString& layer, int tileZoomLevel) override;

    const GeoDataLatLonAltBox& latLonAltBox() const override;

    bool contains(const QPoint& point, const ViewportParams* viewport) const override;

protected:

    GeoDataPoint m_point;
    QImage       m_photo;
};

} // namespace Marble
