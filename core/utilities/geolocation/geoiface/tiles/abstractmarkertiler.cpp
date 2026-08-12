/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : An abstract base class for tiling of markers
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "abstractmarkertiler.h"

// Qt includes

#include <QPair>
#include <QScopedPointer>

// Local includes

#include "digikam_debug.h"
#include "geoifacecommon.h"

namespace Digikam
{

class Q_DECL_HIDDEN AbstractMarkerTiler::Private
{
public:

    Private() = default;

public:

    QScopedPointer<AbstractMarkerTiler::Tile> rootTile;
    bool                                      isDirty = true;
};

AbstractMarkerTiler::AbstractMarkerTiler(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

AbstractMarkerTiler::~AbstractMarkerTiler()
{
    delete d;
}

AbstractMarkerTiler::Tile* AbstractMarkerTiler::rootTile()
{
    if (isDirty())
    {
        regenerateTiles();
    }

    return d->rootTile.data();
}

bool AbstractMarkerTiler::isDirty() const
{
    return d->isDirty;
}

void AbstractMarkerTiler::setDirty(const bool state)
{
    if (state && !d->isDirty)
    {
        d->isDirty = true;

        Q_EMIT signalTilesOrSelectionChanged();
    }
    else
    {
        d->isDirty = state;
    }
}

void AbstractMarkerTiler::resetRootTile()
{
    d->rootTile.reset(tileNew());
}

void AbstractMarkerTiler::onIndicesClicked(const ClickInfo& clickInfo)
{
    Q_UNUSED(clickInfo)
}

void AbstractMarkerTiler::onIndicesMoved(const TileIndex::List& tileIndicesList,
                                         const GeoCoordinates& targetCoordinates,
                                         const QPersistentModelIndex& targetSnapIndex)
{
    Q_UNUSED(tileIndicesList);
    Q_UNUSED(targetCoordinates);
    Q_UNUSED(targetSnapIndex);
}

AbstractMarkerTiler::TilerFlags AbstractMarkerTiler::tilerFlags() const
{
    return FlagNull;
}

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN AbstractMarkerTiler::NonEmptyIterator::Private
{
public:

    Private() = default;

    AbstractMarkerTiler*                model   = nullptr;
    int                                 level   = 0;

    QList<QPair<TileIndex, TileIndex> > boundsList;

    TileIndex                           startIndex;
    TileIndex                           endIndex;
    TileIndex                           currentIndex;

    bool                                atEnd   = false;
};

AbstractMarkerTiler::NonEmptyIterator::~NonEmptyIterator()
{
    delete d;
}

AbstractMarkerTiler::NonEmptyIterator::NonEmptyIterator(AbstractMarkerTiler* const model,
                                                        const int level)
    : d(new Private)
{
    d->model = model;

    GEOIFACE_ASSERT(level <= TileIndex::MaxLevel);

    d->level = level;

    TileIndex startIndex;
    TileIndex endIndex;

    for (int i = 0 ; i <= level ; ++i)
    {
        startIndex.appendLinearIndex(0);
        endIndex.appendLinearIndex(TileIndex::Tiling * TileIndex::Tiling - 1);
    }
/*
    qCDebug(DIGIKAM_GEOIFACE_LOG) << d->startIndexLinear << d->endIndexLinear;
*/
    d->boundsList << QPair<TileIndex, TileIndex>(startIndex, endIndex);

    initializeNextBounds();
}

AbstractMarkerTiler::NonEmptyIterator::NonEmptyIterator(AbstractMarkerTiler* const model,
                                                        const int level,
                                                        const TileIndex& startIndex,
                                                        const TileIndex& endIndex)
    : d(new Private)
{
    d->model = model;

    GEOIFACE_ASSERT(level <= TileIndex::MaxLevel);

    d->level = level;

    GEOIFACE_ASSERT(startIndex.level() == level);
    GEOIFACE_ASSERT(endIndex.level()   == level);

    d->boundsList << QPair<TileIndex, TileIndex>(startIndex, endIndex);

    initializeNextBounds();
}

AbstractMarkerTiler::NonEmptyIterator::NonEmptyIterator(AbstractMarkerTiler* const model,
                                                        const int level,
                                                        const GeoCoordinates::PairList& normalizedMapBounds)
    : d(new Private)
{
    d->model = model;

    GEOIFACE_ASSERT(level <= TileIndex::MaxLevel);

    d->level = level;

    // store the coordinates of the bounds as indices:

    for (int i = 0 ; i < normalizedMapBounds.count() ; ++i)
    {
        GeoCoordinates::Pair currentBounds = normalizedMapBounds.at(i);

        GEOIFACE_ASSERT(currentBounds.first.lat() < currentBounds.second.lat());
        GEOIFACE_ASSERT(currentBounds.first.lon() < currentBounds.second.lon());

        const TileIndex startIndex = TileIndex::fromCoordinates(currentBounds.first,  d->level);
        const TileIndex endIndex   = TileIndex::fromCoordinates(currentBounds.second, d->level);
/*
        qCDebug(DIGIKAM_GEOIFACE_LOG) << currentBounds.first.geoUrl()
                                      << startIndex
                                      << currentBounds.second.geoUrl()
                                      << endIndex;
*/
        d->boundsList << QPair<TileIndex, TileIndex>(startIndex, endIndex);
    }

    initializeNextBounds();
}

bool AbstractMarkerTiler::NonEmptyIterator::initializeNextBounds()
{
    if (d->boundsList.isEmpty())
    {
        d->atEnd = true;

        return false;
    }

    QPair<TileIndex, TileIndex> nextBounds = d->boundsList.takeFirst();
    d->startIndex                          = nextBounds.first;
    d->endIndex                            = nextBounds.second;

    GEOIFACE_ASSERT(d->startIndex.level() == d->level);
    GEOIFACE_ASSERT(d->endIndex.level()   == d->level);

    d->currentIndex.clear();
    d->currentIndex.appendLinearIndex(-1);

    // make sure the root tile is split, using linear index -1 is ok here as
    // this is handled in Tile::getChild

    d->model->getTile(d->currentIndex, true);

    nextIndex();

    return d->atEnd;
}

TileIndex AbstractMarkerTiler::NonEmptyIterator::nextIndex()
{
    if (d->atEnd)
    {
        return d->currentIndex;
    }

    Q_FOREVER
    {
        const int currentLevel = d->currentIndex.level();
/*
        qCDebug(DIGIKAM_GEOIFACE_LOG) << d->level
                                      << currentLevel
                                      << d->currentIndex;
*/
        // determine the limits in the current level:

        int limitLatBL   = 0;
        int limitLonBL   = 0;
        int limitLatTR   = TileIndex::Tiling-1;
        int limitLonTR   = TileIndex::Tiling-1;

        {
            int compareLevel = currentLevel - 1;

            // check limit on the left side:

            bool onLimit = true;

            for (int i = 0 ; onLimit && (i <= compareLevel) ; ++i)
            {
                onLimit = d->currentIndex.indexLat(i) == d->startIndex.indexLat(i);
            }

            if (onLimit)
            {
                limitLatBL = d->startIndex.indexLat(currentLevel);
            }

            // check limit on the bottom side:

            onLimit = true;

            for (int i = 0 ; onLimit && (i <= compareLevel) ; ++i)
            {
                onLimit = d->currentIndex.indexLon(i) == d->startIndex.indexLon(i);
            }

            if (onLimit)
            {
                limitLonBL = d->startIndex.indexLon(currentLevel);
            }

            // check limit on the right side:

            onLimit = true;

            for (int i = 0 ; onLimit && (i <= compareLevel) ; ++i)
            {
                onLimit = d->currentIndex.indexLat(i) == d->endIndex.indexLat(i);
            }

            if (onLimit)
            {
                limitLatTR = d->endIndex.indexLat(currentLevel);
            }

            // check limit on the top side:

            onLimit = true;

            for (int i = 0 ; onLimit && (i <= compareLevel) ; ++i)
            {
                onLimit = d->currentIndex.indexLon(i) == d->endIndex.indexLon(i);
            }

            if (onLimit)
            {
                limitLonTR = d->endIndex.indexLon(currentLevel);
            }
        }

        // get the parent tile of the current level

        int levelLinearIndex = d->currentIndex.lastIndex();

        d->currentIndex.oneUp();

        Tile* const tile     = d->model->getTile(d->currentIndex, true);

        if (!tile)
        {
            // this can only happen if the model has changed during iteration. handle gracefully...

            d->currentIndex.oneUp();

            continue;
        }
        else
        {
            d->currentIndex.appendLinearIndex(levelLinearIndex);
        }

        // helper function to check if index is inside bounds of the current level

        auto inBounds = [&](int idx)
                        {
                            int currentLat = idx / TileIndex::Tiling;
                            int currentLon = idx % TileIndex::Tiling;

                            return (
                                    (currentLat >= limitLatBL) &&
                                    (currentLat <= limitLatTR) &&
                                    (currentLon >= limitLonBL) &&
                                    (currentLon <= limitLonTR)
                                   );
                        };

        // get the next linear index on the level which is in Bounds

        levelLinearIndex = tile->nextNonEmptyIndex(levelLinearIndex);

        while ((levelLinearIndex != -1) && !inBounds(levelLinearIndex))
        {
            levelLinearIndex = tile->nextNonEmptyIndex(levelLinearIndex);
        }

        if (levelLinearIndex == -1)
        {
            // no index in bound anymore on this level

            if (currentLevel == 0)
            {
                // we are at the end!
                // are there other bounds to iterate over?

                initializeNextBounds();

                // initializeNextBounds() call nextIndex which updates d->currentIndex, if possible:

                return d->currentIndex;
            }

            // we need to go one level up, trim the indices:

            d->currentIndex.oneUp();

            continue;
        }

        // save the new position:

        d->currentIndex.oneUp();
        d->currentIndex.appendLinearIndex(levelLinearIndex);

        // are we at the target level?

        if (currentLevel == d->level)
        {
            // yes, return the current index:

            return d->currentIndex;
        }

        // go one level down:

        d->currentIndex.appendLinearIndex(-1);

        // make sure the current level is split,using linear index -1 is ok here
        // as this is handled in Tile::getChild

        d->model->getTile(d->currentIndex, true);
    }
}

TileIndex AbstractMarkerTiler::NonEmptyIterator::currentIndex() const
{
    return d->currentIndex;
}

bool AbstractMarkerTiler::NonEmptyIterator::atEnd() const
{
    return d->atEnd;
}

AbstractMarkerTiler* AbstractMarkerTiler::NonEmptyIterator::model() const
{
    return d->model;
}

// -------------------------------------------------------------------------

AbstractMarkerTiler::Tile::~Tile()
{
    for (auto* const tile : std::as_const(children))
    {
        if (tile)
        {
            delete tile;
        }
    }
}

int AbstractMarkerTiler::Tile::maxChildCount()
{
    return (TileIndex::Tiling * TileIndex::Tiling);
}

AbstractMarkerTiler::Tile* AbstractMarkerTiler::Tile::getChild(const int linearIndex)
{
    if (children.isEmpty())
    {
        return nullptr;
    }

    if ((linearIndex < 0) || (linearIndex >= maxChildCount()))
    {
        return nullptr;
    }

    return children.at(linearIndex);
}

AbstractMarkerTiler::Tile* AbstractMarkerTiler::Tile::addChild(const int linearIndex, Tile* tilePointer)
{
    if ((tilePointer == nullptr) && children.isEmpty())
    {
        return nullptr;
    }

    prepareForChildren();

    GEOIFACE_ASSERT(!children[linearIndex]);

    if (!children[linearIndex])
    {
        nonEmptyIndices.insert(std::upper_bound(nonEmptyIndices.begin(), nonEmptyIndices.end(), linearIndex), linearIndex);
    }

    children[linearIndex] = tilePointer;

    return tilePointer;
}

void AbstractMarkerTiler::Tile::deleteChild(Tile* const childTile,
                                            const int knownLinearIndex)
{
    if (children.isEmpty())
    {
        return;
    }

    int tileIndex = knownLinearIndex;

    if (tileIndex < 0)
    {
        tileIndex = std::distance(children.begin(), std::find(children.begin(), children.end(), childTile));
    }

    if (children[tileIndex])
    {
        nonEmptyIndices.erase(std::lower_bound(nonEmptyIndices.begin(), nonEmptyIndices.end(), tileIndex));
    }

    delete children[tileIndex];
    children[tileIndex] = nullptr;
}

bool AbstractMarkerTiler::Tile::childrenEmpty() const
{
    return children.empty();
}

void AbstractMarkerTiler::Tile::prepareForChildren()
{
    if (!children.isEmpty())
    {
        return;
    }

    children = QVector<Tile*>(maxChildCount(), nullptr);
}

int AbstractMarkerTiler::Tile::nextNonEmptyIndex(int linearIndex) const
{
    auto it = std::upper_bound(nonEmptyIndices.begin(), nonEmptyIndices.end(), linearIndex);

    if (it != nonEmptyIndices.end())
    {
        return *it;
    }

    return (-1);
}

} // namespace Digikam

#include "moc_abstractmarkertiler.cpp"
