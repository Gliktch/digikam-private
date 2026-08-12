/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Tile index used in the tiling classes
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tileindex.h"

// Local includes

#include "geoifacecommon.h"

namespace
{
    static_assert (Digikam::TileIndex::Tiling == 10,
                   "the constants below expect 10x10 tile splits");

    static_assert (Digikam::TileIndex::MaxLevel == 9,
                   "the constants below expect 10x10 tile splits with max level 9");

    constexpr qint64 MaxLevelTileSplits       = 10000000000LL;
    constexpr double MaxLevelTileSplitsFactor = 1.0 / static_cast<double>(MaxLevelTileSplits);
}


namespace Digikam
{

int TileIndex::indexCount() const
{
    return m_indicesCount;
}

int TileIndex::level() const
{
    return (m_indicesCount > 0 ? m_indicesCount - 1 : 0);
}

void TileIndex::clear()
{
    m_indicesCount = 0;
}

void TileIndex::appendLinearIndex(const int newIndex)
{
    GEOIFACE_ASSERT(m_indicesCount + 1 <= MaxIndexCount);

    m_indices[m_indicesCount] = newIndex;
    m_indicesCount++;
}

int TileIndex::linearIndex(const int getLevel) const
{
    GEOIFACE_ASSERT(getLevel<=level());

    return m_indices[getLevel];
}

int TileIndex::at(const int getLevel) const
{
    GEOIFACE_ASSERT(getLevel<=level());

    return m_indices[getLevel];
}

int TileIndex::lastIndex() const
{
    GEOIFACE_ASSERT(m_indicesCount>0);

    return m_indices[m_indicesCount-1];
}

int TileIndex::indexLat(const int getLevel) const
{
    return (linearIndex(getLevel) / Tiling);
}

int TileIndex::indexLon(const int getLevel) const
{
    return (linearIndex(getLevel) % Tiling);
}

QPoint TileIndex::latLonIndex(const int getLevel) const
{
    return QPoint(indexLon(getLevel), indexLat(getLevel));
}

void TileIndex::latLonIndex(const int getLevel, int* const latIndex, int* const lonIndex) const
{
    GEOIFACE_ASSERT(getLevel <= level());

    *latIndex = indexLat(getLevel);
    *lonIndex = indexLon(getLevel);

    GEOIFACE_ASSERT(*latIndex < Tiling);
    GEOIFACE_ASSERT(*lonIndex < Tiling);
}

void TileIndex::appendLatLonIndex(const int latIndex, const int lonIndex)
{
    appendLinearIndex(latIndex*Tiling + lonIndex);
}

QIntList TileIndex::toIntList() const
{
    QIntList result;

    for (int i = 0 ; i < m_indicesCount ; ++i)
    {
        result << m_indices[i];
    }

    return result;
}

TileIndex TileIndex::fromIntList(const QIntList& intList)
{
    TileIndex result;

    for (int i = 0 ; i < intList.count() ; ++i)
    {
        result.appendLinearIndex(intList.at(i));
    }

    return result;
}

bool TileIndex::indicesEqual(const TileIndex& a, const TileIndex& b, const int upToLevel)
{
    GEOIFACE_ASSERT(a.level() >= upToLevel);
    GEOIFACE_ASSERT(b.level() >= upToLevel);

    for (int i = 0 ; i <= upToLevel ; ++i)
    {
        if (a.linearIndex(i) != b.linearIndex(i))
        {
            return false;
        }
    }

    return true;
}

TileIndex TileIndex::mid(const int first, const int len) const
{
    GEOIFACE_ASSERT(first + (len - 1) <= m_indicesCount);

    TileIndex result;

    for (int i = first ; i < first + len ; ++i)
    {
        result.appendLinearIndex(m_indices[i]);
    }

    return result;
}

void TileIndex::oneUp()
{
    GEOIFACE_ASSERT(m_indicesCount > 0);

    m_indicesCount--;
}

QList<QIntList> TileIndex::listToIntListList(const QList<TileIndex>& tileIndexList)
{
    QList<QIntList> result;

    for (int i = 0 ; i < tileIndexList.count() ; ++i)
    {
        result << tileIndexList.at(i).toIntList();
    }

    return result;
}

TileIndex TileIndex::fromCoordinates(const Digikam::GeoCoordinates& coordinate, const int getLevel)
{
    GEOIFACE_ASSERT(getLevel <= MaxLevel);

    if (!coordinate.hasCoordinates())
    {
        return TileIndex();
    }

    qint64 tileLat, tileLon;
    {
        // this is the only place where rounding happens

        tileLat = static_cast<qint64>(((coordinate.lat() + 90.0)  / 180.0) * MaxLevelTileSplits);
        tileLon = static_cast<qint64>(((coordinate.lon() + 180.0) / 360.0) * MaxLevelTileSplits);

        // the very last tile includes it's upper bound

        tileLat = std::min(tileLat, MaxLevelTileSplits-1);
        tileLon = std::min(tileLon, MaxLevelTileSplits-1);

        // guard against bogus input

        tileLat = std::max(tileLat, static_cast<qint64>(0));
        tileLon = std::max(tileLon, static_cast<qint64>(0));
     }

    // every calculation below is on integers so no rounding issues

    TileIndex tileIndex;

    for (int i = 0 ; i <= TileIndex::MaxLevel ; ++i)
    {
        tileIndex.m_indices[TileIndex::MaxLevel-i] = (tileLat % Tiling) * Tiling + tileLon % Tiling;
        tileLat /= Tiling;
        tileLon /= Tiling;
    }

    tileIndex.m_indicesCount = getLevel + 1;

    return tileIndex;
}

GeoCoordinates TileIndex::toCoordinates() const
{
    return toCoordinates(CornerPosition::CornerSW);
}

GeoCoordinates TileIndex::toCoordinates(const CornerPosition ofCorner) const
{
    qint64 tileLat = 0;
    qint64 tileLon = 0;

    for (int l = 0 ; l <= MaxLevel ; ++l)
    {
        tileLat *= Tiling;
        tileLon *= Tiling;

        if (l < m_indicesCount)
        {
            tileLat += indexLat(l);
            tileLon += indexLon(l);
        }

        if (
            (l + 1 == m_indicesCount)               &&
            (
             (ofCorner == CornerPosition::CornerNE) ||
             (ofCorner == CornerPosition::CornerNW)
            )
           )
        {
            tileLat += 1;
        }

        if (
            (l + 1 == m_indicesCount)               &&
            (
             (ofCorner == CornerPosition::CornerNE) ||
             (ofCorner == CornerPosition::CornerSE)
            )
           )
        {
            tileLon += 1;
        }
    }

    return Digikam::GeoCoordinates((tileLat -  MaxLevelTileSplits / 2) * MaxLevelTileSplitsFactor * 180.0,
                                   (tileLon -  MaxLevelTileSplits / 2) * MaxLevelTileSplitsFactor * 360.0);
}

} // namespace Digikam

QDebug operator<<(QDebug debug, const Digikam::TileIndex& tileIndex)
{
    debug << tileIndex.toIntList();

    return debug;
}
