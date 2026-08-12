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

#include <QHash>

// Local includes

#include "digikam_export.h"

class QString;

namespace Marble
{

class GeoDataCoordinates;

class DIGIKAM_EXPORT TileId
{
public:

    TileId(QString const& mapThemeId, int zoomLevel, int tileX, int tileY);
    TileId(uint mapThemeIdHash, int zoomLevel, int tileX, int tileY);
    TileId();

    int zoomLevel() const;
    int x() const;
    int y() const;
    uint mapThemeIdHash() const;

    bool operator==(TileId const& rhs) const;
    bool operator<(TileId const& rhs) const;

    static TileId fromCoordinates(const GeoDataCoordinates& _coords, int _zoomLevel);

private:

    uint m_mapThemeIdHash = 0;
    int  m_zoomLevel      = 0;
    int  m_tileX          = 0;
    int  m_tileY          = 0;
};

uint qHash(TileId const&);

// inline definitions

inline int TileId::zoomLevel() const
{
    return m_zoomLevel;
}

inline int TileId::x() const
{
    return m_tileX;
}

inline int TileId::y() const
{
    return m_tileY;
}

inline uint TileId::mapThemeIdHash() const
{
    return m_mapThemeIdHash;
}

inline bool TileId::operator==(TileId const& rhs) const
{
    return (
               (m_zoomLevel      == rhs.m_zoomLevel)
            && (m_tileX          == rhs.m_tileX)
            && (m_tileY          == rhs.m_tileY)
            && (m_mapThemeIdHash == rhs.m_mapThemeIdHash)
           );
}

inline bool TileId::operator<(TileId const& rhs) const
{
    if      (m_zoomLevel < rhs.m_zoomLevel)
    {
        return true;
    }
    else if ((m_zoomLevel == rhs.m_zoomLevel)
             && (m_tileX < rhs.m_tileX))
    {
        return true;
    }
    else if ((m_zoomLevel == rhs.m_zoomLevel)
             && (m_tileX == rhs.m_tileX)
             && (m_tileY < rhs.m_tileY))
    {
        return true;
    }
    else if ((m_zoomLevel == rhs.m_zoomLevel)
             && (m_tileX == rhs.m_tileX)
             && (m_tileY == rhs.m_tileY)
             && (m_mapThemeIdHash < rhs.m_mapThemeIdHash))
    {
        return true;
    }

    return false;
}

inline uint qHash(TileId const& tid)
{
    const quint64 tmp =   ((quint64)(tid.zoomLevel()) << 36)
                        + ((quint64)(tid.x()) << 18)
                        +  (quint64)(tid.y());

    return (::qHash(tmp) ^ tid.mapThemeIdHash());
}

} // namespace Marble

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug, const Marble::TileId&);

#endif
