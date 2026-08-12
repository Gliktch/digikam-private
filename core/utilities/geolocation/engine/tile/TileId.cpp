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

#include "TileId.h"

// Local includes

#include "GeoDataCoordinates.h"
#include "digikam_debug.h"

namespace Marble
{

TileId::TileId(QString const& mapThemeId, int zoomLevel, int tileX, int tileY)
    : m_mapThemeIdHash(qHash(mapThemeId)),
      m_zoomLevel     (zoomLevel),
      m_tileX         (tileX),
      m_tileY         (tileY)
{
}

TileId::TileId(uint mapThemeIdHash, int zoomLevel, int tileX, int tileY)
    : m_mapThemeIdHash(mapThemeIdHash),
      m_zoomLevel     (zoomLevel),
      m_tileX         (tileX),
      m_tileY         (tileY)
{
}

TileId::TileId()
    : m_mapThemeIdHash(0),
      m_zoomLevel     (0),
      m_tileX         (0),
      m_tileY         (0)
{
}

TileId TileId::fromCoordinates(const GeoDataCoordinates& _coords, int _zoomLevel)
{
    if (_zoomLevel < 0)
    {
        return TileId();
    }

    const int maxLat = 90  * 1000000;
    const int maxLon = 180 * 1000000;
    int lat          = GeoDataCoordinates::normalizeLat(_coords.latitude(GeoDataCoordinates::Degree),
                                                        GeoDataCoordinates::Degree) * 1000000;
    int lon          = GeoDataCoordinates::normalizeLon(_coords.longitude(GeoDataCoordinates::Degree),
                                                        GeoDataCoordinates::Degree) * 1000000;
    int x            = 0;
    int y            = 0;

    for (int i = 0 ; i < _zoomLevel ; ++i)
    {
        const int deltaLat = maxLat >> i;

        if (lat <= (maxLat - deltaLat))
        {
            y   += 1 << (_zoomLevel - i - 1);
            lat += deltaLat;
        }

        const int deltaLon = maxLon >> i;

        if (lon >= (maxLon - deltaLon))
        {
            x += 1 << (_zoomLevel - i - 1);
        }
        else
        {
            lon += deltaLon;
        }
    }

    return TileId(0, _zoomLevel, x, y);
}

} // namespace Marble

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug dbg, const Marble::TileId& id)
{
    return dbg << QString::fromUtf8("Marble::TileId(%1, %2, %3, %4)").arg(id.mapThemeIdHash())
           .arg(id.zoomLevel())
           .arg(id.x())
           .arg(id.y());
}

#endif
