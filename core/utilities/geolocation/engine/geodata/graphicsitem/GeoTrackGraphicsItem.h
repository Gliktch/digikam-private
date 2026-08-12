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

#include "GeoLineStringGraphicsItem.h"

namespace Marble
{

class GeoDataPlacemark;
class GeoDataTrack;

class DIGIKAM_EXPORT GeoTrackGraphicsItem : public GeoLineStringGraphicsItem
{

public:

    explicit GeoTrackGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataTrack* track);

    void paint(GeoPainter* painter, const ViewportParams* viewport, const QString& layer, int tileZoomLevel) override;

private:

    void setTrack(const GeoDataTrack* track);
    void update();

private:

    const GeoDataTrack* m_track = nullptr;
};

} // namespace Marble
