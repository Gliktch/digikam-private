/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-20
 * Description : GPS search marker tiler
 *
 * SPDX-FileCopyrightText: 2010      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>
#include <QMetaType>
#include <QItemSelectionModel>

// Local includes

#include "abstractmarkertiler.h"
#include "mapwidget.h"
#include "digikam_export.h"
#include "itemposition.h"
#include "coredbchangesets.h"
#include "itemlister.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "iteminfo.h"
#include "thumbnailloadthread.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"
#include "coredbwatch.h"
#include "coredbfields.h"
#include "itemalbummodel.h"
#include "itemfiltermodel.h"

namespace Digikam
{

class GPSItemInfo;

class GPSMarkerTiler : public AbstractMarkerTiler
{
    Q_OBJECT

public:

    class MyTile;

    explicit GPSMarkerTiler(QObject* const parent,
                            ItemFilterModel* const imageFilterModel,
                            QItemSelectionModel* const selectionModel);
    ~GPSMarkerTiler()                                                                               override;

    Tile* tileNew()                                                                                 override;
    void prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level) override;
    void regenerateTiles()                                                                          override;
    AbstractMarkerTiler::Tile* getTile(const TileIndex& tileIndex, const bool stopIfEmpty)          override;
    int getTileMarkerCount(const TileIndex& tileIndex)                                              override;
    int getTileSelectedCount(const TileIndex& tileIndex)                                            override;

    QVariant getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey)             override;
    QVariant bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey)     override;
    QPixmap pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)                 override;
    bool indicesEqual(const QVariant& a, const QVariant& b) const                                   override;
    GeoGroupState getTileGroupState(const TileIndex& tileIndex)                                     override;
    GeoGroupState getGlobalGroupState()                                                             override;

    void onIndicesClicked(const ClickInfo& clickInfo)                                               override;

    void setActive(const bool state)                                                                override;

    void setRegionSelection(const GeoCoordinates::Pair& sel);
    void removeCurrentRegionSelection();
    void setPositiveFilterIsActive(const bool state);

Q_SIGNALS:

    void signalModelFilteredImages(const QList<qlonglong>& imagesId);

public Q_SLOTS:

    void slotNewModelData(const QList<ItemInfo>& infoList);

private Q_SLOTS:

    /// @todo Do we monitor all signals of the source models?
    void slotMapImagesJobResult();
    void slotMapImagesJobData(const QList<ItemListerRecord>& records);
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);
    void slotImageChange(const ImageChangeset& changeset);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:

    QList<qlonglong> getTileMarkerIds(const TileIndex& tileIndex);
    GeoGroupState getImageState(const qlonglong imageId);
    void removeMarkerFromTileAndChildren(const qlonglong imageId,
                                         const TileIndex& markerTileIndex);
    void addMarkerToTileAndChildren(const qlonglong imageId,
                                    const TileIndex& markerTileIndex);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
