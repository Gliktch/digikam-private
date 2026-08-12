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

#include "GeoSceneEquirectTileProjection.h"

// Local includes

#include "GeoDataLatLonBox.h"

namespace Marble
{

GeoSceneEquirectTileProjection::GeoSceneEquirectTileProjection()
{
}

GeoSceneEquirectTileProjection::~GeoSceneEquirectTileProjection()
{
}

GeoSceneAbstractTileProjection::Type GeoSceneEquirectTileProjection::type() const
{
    return Equirectangular;
}

static inline
unsigned int lowerBoundTileIndex(qreal baseTileIndex)
{
    const qreal floorBaseTileIndex = floor(baseTileIndex);
    unsigned int tileIndex = static_cast<unsigned int>(floorBaseTileIndex);
    return (baseTileIndex == floorBaseTileIndex) ? tileIndex - 1 : tileIndex;
}

static inline
unsigned int upperBoundTileIndex(qreal baseTileIndex)
{
    return (unsigned int)floor(baseTileIndex);
}

static inline
qreal baseTileXFromLon(qreal lon, unsigned int tileCount)
{
    return 0.5 * (lon / M_PI + 1.0) * tileCount;
}

static inline
qreal baseTileYFromLat(qreal lat, unsigned int tileCount)
{
    return (0.5 - lat / M_PI) * tileCount;
}

// on tile borders selects the tile to the east
static inline
unsigned int eastBoundTileXFromLon(qreal lon, unsigned int tileCount)
{
    // special casing tile-map end
    if (lon == M_PI)
    {
        return 0;
    }

    return upperBoundTileIndex(baseTileXFromLon(lon, tileCount));
}

// on tile borders selects the tile to the west
static inline
unsigned int westBoundTileXFromLon(qreal lon, unsigned int tileCount)
{
    // special casing tile-map end
    if (lon == -M_PI)
    {
        return tileCount - 1;
    }

    return lowerBoundTileIndex(baseTileXFromLon(lon, tileCount));
}

// on tile borders selects the tile to the south
static inline
unsigned int southBoundTileYFromLat(qreal lat, unsigned int tileCount)
{
    // special casing tile-map end
    if (lat == -M_PI * 0.5)
    {
        return 0;
    }

    return upperBoundTileIndex(baseTileYFromLat(lat, tileCount));
}

// on tile borders selects the tile to the north
static inline
unsigned int northBoundTileYFromLat(qreal lat, unsigned int tileCount)
{
    // special casing tile-map end
    if (lat == M_PI * 0.5)
    {
        return tileCount - 1;
    }

    return lowerBoundTileIndex(baseTileYFromLat(lat, tileCount));
}

QRect GeoSceneEquirectTileProjection::tileIndexes(const GeoDataLatLonBox& latLonBox, int zoomLevel) const
{
    const unsigned int xTileCount = (1 << zoomLevel) * levelZeroColumns();

    const int westX =   eastBoundTileXFromLon(latLonBox.west(),  xTileCount);
    const int eastX =   westBoundTileXFromLon(latLonBox.east(),  xTileCount);

    const unsigned int yTileCount = (1 << zoomLevel) * levelZeroRows();

    const int northY = southBoundTileYFromLat(latLonBox.north(), yTileCount);
    const int southY = northBoundTileYFromLat(latLonBox.south(), yTileCount);

    return QRect(QPoint(westX, northY), QPoint(eastX, southY));
}

GeoDataLatLonBox GeoSceneEquirectTileProjection::geoCoordinates(int zoomLevel, int x, int y) const
{
    const qreal radiusX = (1 << zoomLevel) * levelZeroColumns() / 2.0;
    const qreal radiusY = (1 << zoomLevel) * levelZeroRows() / 2.0;

    const qreal west  = (x - radiusX) / radiusX * M_PI;
    const qreal north = (radiusY - y) / radiusY * M_PI / 2.0;

    const qreal east  = ((x + 1) - radiusX) / radiusX * M_PI;
    const qreal south = (radiusY - (y + 1)) / radiusY * M_PI / 2.0;

    return GeoDataLatLonBox(north, south, east, west);
}

} // namespace Marble
