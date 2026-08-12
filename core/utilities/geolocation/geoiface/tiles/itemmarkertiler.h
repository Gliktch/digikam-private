/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-17
 * Description : A marker tiler operating on item models
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QItemSelection>

// Local includes

#include "digikam_export.h"
#include "abstractmarkertiler.h"

namespace Digikam
{
class GeoModelHelper;

class DIGIKAM_EXPORT ItemMarkerTiler : public AbstractMarkerTiler
{
    Q_OBJECT

private:

    class MyTile;

public:

    explicit ItemMarkerTiler(GeoModelHelper* const modelHelper, QObject* const parent = nullptr);
    ~ItemMarkerTiler()                                                                                    override;

    TilerFlags tilerFlags()                                                                         const override;
    Tile* tileNew()                                                                                       override;
    void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level)       override;
    void regenerateTiles()                                                                                override;
    Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty)                                     override;
    int getTileMarkerCount(const TileIndex& tileIndex)                                                    override;
    int getTileSelectedCount(const TileIndex& tileIndex)                                                  override;

    QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey)                   override;
    QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey)           override;
    QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)                       override;
    bool indicesEqual(const QVariant& a, const QVariant& b)                                         const override;
    GeoGroupState getTileGroupState(const TileIndex& tileIndex)                                           override;
    GeoGroupState getGlobalGroupState()                                                                   override;

    void onIndicesClicked(const ClickInfo& clickInfo)                                                     override;
    void onIndicesMoved(const TileIndex::List& tileIndicesList,
                                const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex)                             override;

    void setMarkerGeoModelHelper(GeoModelHelper* const modelHelper);
    void removeMarkerIndexFromGrid(const QModelIndex& markerIndex, const bool ignoreSelection = false);
    void addMarkerIndexToGrid(const QPersistentModelIndex& markerIndex);

    void setActive(const bool state)                                                                      override;

private Q_SLOTS:

    void slotSourceModelRowsInserted(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelRowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end);
    void slotSourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotSourceModelReset();
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void slotThumbnailAvailableForIndex(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void slotSourceModelLayoutChanged();

private:

    QList<QPersistentModelIndex> getTileMarkerIndices(const TileIndex& tileIndex);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
