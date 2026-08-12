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

#include "GeoPolygonGraphicsItem.h"

// Local includes

#include "BuildingGraphicsItem.h"
#include "GeoDataPlacemark.h"
#include "StyleBuilder.h"

namespace Marble
{

AbstractGeoPolygonGraphicsItem* GeoPolygonGraphicsItem::createGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataPolygon* polygon)
{
    return new GeoPolygonGraphicsItem(placemark, polygon);
}

AbstractGeoPolygonGraphicsItem* GeoPolygonGraphicsItem::createGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataLinearRing* ring)
{
    return new GeoPolygonGraphicsItem(placemark, ring);
}

AbstractGeoPolygonGraphicsItem* GeoPolygonGraphicsItem::createGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataBuilding* building)
{
    return new BuildingGraphicsItem(placemark, building);
}

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataPolygon* polygon) :
    AbstractGeoPolygonGraphicsItem(placemark, polygon)
{
    const int elevation = extractElevation(*placemark);
    setZValue(zValue() + elevation);

    const GeoDataPlacemark::GeoDataVisualCategory visualCategory = placemark->visualCategory();
    const QString paintLayer = QLatin1String("Polygon/") + StyleBuilder::visualCategoryName(visualCategory);
    setPaintLayers(QStringList(paintLayer));
}

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem(const GeoDataPlacemark* placemark, const GeoDataLinearRing* ring) :
    AbstractGeoPolygonGraphicsItem(placemark, ring)
{
    const int elevation = extractElevation(*placemark);
    setZValue(zValue() + elevation);

    const GeoDataPlacemark::GeoDataVisualCategory visualCategory = placemark->visualCategory();
    const QString paintLayer = QLatin1String("Polygon/") + StyleBuilder::visualCategoryName(visualCategory);
    setPaintLayers(QStringList(paintLayer));
}

} // namespace Marble
