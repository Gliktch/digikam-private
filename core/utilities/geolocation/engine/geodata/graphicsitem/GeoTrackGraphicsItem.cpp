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

#include "GeoTrackGraphicsItem.h"

// Local includes

#include "GeoDataLineString.h"
#include "GeoDataPlacemark.h"
#include "GeoDataTrack.h"
#include "StyleBuilder.h"
#include "digikam_debug.h"

namespace Marble
{

GeoTrackGraphicsItem::GeoTrackGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataTrack* track) :
    GeoLineStringGraphicsItem(placemark, track->lineString())
{
    setTrack(track);

    if (placemark)
    {
        QString const paintLayer = QLatin1String("Track/") + StyleBuilder::visualCategoryName(placemark->visualCategory());
        setPaintLayers(QStringList() << paintLayer);
    }
}

void GeoTrackGraphicsItem::setTrack(const GeoDataTrack* track)
{
    m_track = track;
    update();
}

void GeoTrackGraphicsItem::paint(GeoPainter* painter, const ViewportParams* viewport, const QString& layer, int tileZoomLevel)
{
    Q_UNUSED(layer);
    Q_UNUSED(tileZoomLevel);
    update();
    GeoLineStringGraphicsItem::paint(painter, viewport, layer, tileZoomLevel);
}

void GeoTrackGraphicsItem::update()
{
    setLineString(m_track->lineString());
}

} // namespace Marble
