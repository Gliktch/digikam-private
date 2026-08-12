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

#include "gpsmarkertiler.h"

// Qt includes

#include <QPair>
#include <QRectF>
#include <QTimer>

// Local includes

#include "groupstatecomputer.h"
#include "gpsiteminfosorter.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "digikam_debug.h"
#include "dbjobsmanager.h"

/// @todo Actually use this definition!
typedef QPair<Digikam::TileIndex, int> MapPair;

Q_DECLARE_METATYPE(MapPair)

namespace Digikam
{

/**
 * @class GPSMarkerTiler
 *
 * @brief Marker model for storing data needed to display markers on the map. The data is retrieved from Digikam's database.
 */

class Q_DECL_HIDDEN GPSMarkerTiler::MyTile : public Tile
{
public:

    MyTile()           = default;
    ~MyTile() override = default;

    QList<qlonglong> imagesId;
};

class Q_DECL_HIDDEN GPSMarkerTiler::Private
{
public:

    class Q_DECL_HIDDEN InternalJobs
    {
    public:

        InternalJobs() = default;

        int                level        = 0;
        GPSDBJobsThread*   jobThread    = nullptr;
        QList<GPSItemInfo> dataFromDatabase;
    };

public:

    Private() = default;

public:

    QList<InternalJobs>           jobs;
    ThumbnailLoadThread*          thumbnailLoadThread       = nullptr;
    QHash<qlonglong, QVariant>    thumbnailMap;
    QList<QRectF>                 rectList;
    bool                          activeState               = true;
    QHash<qlonglong, GPSItemInfo> imagesHash;
    ItemFilterModel*              imageFilterModel          = nullptr;
    ItemAlbumModel*               imageAlbumModel           = nullptr;
    QItemSelectionModel*          selectionModel            = nullptr;
    GeoCoordinates::Pair          currentRegionSelection;
    GeoGroupState                 mapGlobalGroupState       = SelectedNone;
};

/**
 * @brief Constructor
 * @param parent            The parent object
 * @param imageFilterModel  The image filter instance
 * @param selectionModel    The selection model instance
 */
GPSMarkerTiler::GPSMarkerTiler(QObject* const parent,
                               ItemFilterModel* const imageFilterModel,
                               QItemSelectionModel* const selectionModel)
    : AbstractMarkerTiler(parent),
      d                  (new Private)
{
    resetRootTile();

    d->thumbnailLoadThread = new ThumbnailLoadThread(this);
    d->imageFilterModel    = imageFilterModel;
    d->imageAlbumModel     = qobject_cast<ItemAlbumModel*>(imageFilterModel->sourceModel());
    d->selectionModel      = selectionModel;

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChange(ImageChangeset)), Qt::QueuedConnection);

    connect(d->imageAlbumModel, SIGNAL(imageInfosAdded(QList<ItemInfo>)),
            this, SLOT(slotNewModelData(QList<ItemInfo>)));

    connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
}

/**
 * @brief Destructor
 */
GPSMarkerTiler::~GPSMarkerTiler()
{
    delete d;
}

void GPSMarkerTiler::regenerateTiles()
{
}

/**
 * @brief Requests all images inside a given rectangle from the database.
 *
 * This function calls the database for the images found inside a rectangle
 * defined by upperLeft and lowerRight points. The images are returned from
 * the database in batches.
 *
 * @param upperLeft The North-West point.
 * @param lowerRight The South-East point.
 * @param level The requested tiling level.
 */
void GPSMarkerTiler::prepareTiles(const GeoCoordinates& upperLeft, const GeoCoordinates& lowerRight, int level)
{
    const QRectF worldRect(-90, -180, 180, 360);

    qreal lat1         = upperLeft.lat();
    qreal lng1         = upperLeft.lon();
    qreal lat2         = lowerRight.lat();
    qreal lng2         = lowerRight.lon();
    auto requestedRect = worldRect.intersected(QRectF(lat1, lng1, lat2 - lat1, lng2 - lng1));

    for (int i = 0 ; i < d->rectList.count() ; ++i)
    {
        // is there a rect that contains the requested one?

        const QRectF& currentRect = d->rectList.at(i);

        if (currentRect.contains(requestedRect))
        {
            return;
        }

        // and remove rects that are contained in the requested one

        if (requestedRect.contains(currentRect))
        {
            std::swap(d->rectList[i], d->rectList.back());
            d->rectList.removeLast();

            // we removed one entry. we have to subtract one from the index

            --i;
        }
    }

    // grow the rect a bit such that we don't have to request many small ones while panning

    qreal marginW = requestedRect.width()  * 0.05;
    qreal marginH = requestedRect.height() * 0.05;
    requestedRect = requestedRect.marginsAdded(QMarginsF(marginW, marginH, marginW, marginH));
    requestedRect = worldRect.intersected(requestedRect);
    requestedRect.getCoords(&lat1, &lng1, &lat2, &lng2);

    for (int i = 0 ; i < d->rectList.count() ; ++i)
    {
        qreal rectLat1, rectLng1, rectLat2, rectLng2;
        const QRectF currentRect = d->rectList.at(i);
        currentRect.getCoords(&rectLat1, &rectLng1, &rectLat2, &rectLng2);

        if      (currentRect.contains(lat1, lng1))
        {
            if (currentRect.contains(lat2, lng1))
            {
                lng1 = rectLng2;
                break;
            }
        }
        else if (currentRect.contains(lat2, lng1))
        {
            if (currentRect.contains(lat2, lng2))
            {
                lat2 = rectLat1;
                break;
            }
        }
        else if (currentRect.contains(lat2, lng2))
        {
            if (currentRect.contains(lat1, lng2))
            {
                lng2 = rectLng1;
                break;
            }
        }
        else if (currentRect.contains(lat1, lng2))
        {
            if (currentRect.contains(lat1, lng1))
            {
                lat1 = rectLat2;
                break;
            }
        }
    }

    requestedRect = QRectF(lat1, lng1, lat2 - lat1, lng2 - lng1);
    d->rectList.append(requestedRect);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Listing" << lat1 << lat2 << lng1 << lng2;

    GPSDBJobInfo jobInfo;
    jobInfo.setLat1(lat1);
    jobInfo.setLat2(lat2);
    jobInfo.setLng1(lng1);
    jobInfo.setLng2(lng2);

    GPSDBJobsThread* const currentJob = DBJobsManager::instance()->startGPSJobThread(jobInfo);

    Private::InternalJobs currentJobInfo;

    currentJobInfo.jobThread          = currentJob;
    currentJobInfo.level              = level;

    d->jobs.append(currentJobInfo);

    connect(currentJob, SIGNAL(signalFinished()),
            this, SLOT(slotMapImagesJobResult()));

    connect(currentJob, SIGNAL(data(QList<ItemListerRecord>)),
            this, SLOT(slotMapImagesJobData(QList<ItemListerRecord>)));
}

/**
 * @brief Returns a pointer to a tile.
 *
 * @param tileIndex The index of a tile.
 * @param stopIfEmpty Determines whether child tiles are also created for empty tiles.
 */
AbstractMarkerTiler::Tile* GPSMarkerTiler::getTile(const TileIndex& tileIndex, const bool stopIfEmpty)
{
    Q_ASSERT(tileIndex.level() <= TileIndex::MaxLevel);

    MyTile* tile = static_cast<MyTile*>(rootTile());

    for (int level = 0 ; level < tileIndex.indexCount() ; ++level)
    {
        const int currentIndex = tileIndex.linearIndex(level);
        MyTile* childTile      = nullptr;

        if (tile->childrenEmpty())
        {
            for (int i = 0 ; i < tile->imagesId.count() ; ++i)
            {
                const int currentImageId          = tile->imagesId.at(i);
                const GPSItemInfo currentItemInfo = d->imagesHash[currentImageId];
                const TileIndex markerTileIndex   = TileIndex::fromCoordinates(currentItemInfo.coordinates, level);
                const int newTileIndex            = markerTileIndex.lastIndex();
                MyTile* const newTile1            = static_cast<MyTile*>(tile->getChild(newTileIndex));

                if (newTile1 == nullptr)
                {
                    MyTile* const newTile2 = static_cast<MyTile*>(tileNew());
                    newTile2->imagesId.append(currentImageId);
                    tile->addChild(newTileIndex, newTile2);
                }
                else
                {
                    if (!newTile1->imagesId.contains(currentImageId))
                    {
                        newTile1->imagesId.append(currentImageId);
                    }
                }
            }
        }

        childTile = static_cast<MyTile*>(tile->getChild(currentIndex));

        if (childTile == nullptr)
        {
            if (stopIfEmpty)
            {
                // there will be no markers in this tile, therefore stop

                return nullptr;
            }

            childTile = static_cast<MyTile*>(tileNew());
            tile->addChild(currentIndex, childTile);
        }

        tile = childTile;
    }

    return tile;
}

int GPSMarkerTiler::getTileMarkerCount(const TileIndex& tileIndex)
{
    MyTile* const tile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (tile)
    {
        return tile->imagesId.count();
    }

    return 0;
}

int GPSMarkerTiler::getTileSelectedCount(const TileIndex& tileIndex)
{
    Q_UNUSED(tileIndex)

    return 0;
}

/**
 * @brief This function finds the best representative marker from a tile of markers.
 *
 * @param tileIndex Index of the tile from which the best marker should be found.
 * @param sortKey Sets the criteria for selecting the representative thumbnail, a combination of the SortOptions bits.
 *
 * @return Returns the internally used index of the marker.
 */
QVariant GPSMarkerTiler::getTileRepresentativeMarker(const TileIndex& tileIndex, const int sortKey)
{
    MyTile* const tile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!tile)
    {
        return QVariant();
    }

    if (tile->imagesId.isEmpty())
    {
        return QVariant();
    }

    GPSItemInfo bestMarkerInfo         = d->imagesHash.value(tile->imagesId.first());
    GeoGroupState bestMarkerGroupState = getImageState(bestMarkerInfo.id);

    for (int i = 1 ; i < tile->imagesId.count() ; ++i)
    {
        const GPSItemInfo currentMarkerInfo         = d->imagesHash.value(tile->imagesId.at(i));
        const GeoGroupState currentMarkerGroupState = getImageState(currentMarkerInfo.id);

        if (GPSItemInfoSorter::fitsBetter(bestMarkerInfo,
                                          bestMarkerGroupState,
                                          currentMarkerInfo,
                                          currentMarkerGroupState,
                                          getGlobalGroupState(),
                                          GPSItemInfoSorter::SortOptions(sortKey)))
        {
            bestMarkerInfo       = currentMarkerInfo;
            bestMarkerGroupState = currentMarkerGroupState;
        }
    }

    const QPair<TileIndex, int> returnedMarker(tileIndex, bestMarkerInfo.id);

    return QVariant::fromValue(returnedMarker);
}

/**
 * @brief This function finds the best representative marker from a group of markers. This is needed to display a thumbnail for a marker group.
 *
 * @param indices A list containing markers, obtained by getTileRepresentativeMarker.
 * @param sortKey Sets the criteria for selecting the representative thumbnail, a combination of the SortOptions bits.
 *
 * @return Returns the internally used index of the marker.
 */
QVariant GPSMarkerTiler::bestRepresentativeIndexFromList(const QList<QVariant>& indices, const int sortKey)
{
    if (indices.isEmpty())
    {
        return QVariant();
    }

    const QPair<TileIndex, int> firstIndex = indices.first().value<QPair<TileIndex, int> >();
    GPSItemInfo bestMarkerInfo             = d->imagesHash.value(firstIndex.second);
    GeoGroupState bestMarkerGroupState     = getImageState(firstIndex.second);
    TileIndex bestMarkerTileIndex          = firstIndex.first;

    for (int i = 1 ; i < indices.count() ; ++i)
    {
        const QPair<TileIndex, int> currentIndex = indices.at(i).value<QPair<TileIndex, int> >();

        GPSItemInfo currentMarkerInfo            = d->imagesHash.value(currentIndex.second);
        GeoGroupState currentMarkerGroupState    = getImageState(currentIndex.second);

        if (GPSItemInfoSorter::fitsBetter(bestMarkerInfo,
                                          bestMarkerGroupState,
                                          currentMarkerInfo,
                                          currentMarkerGroupState,
                                          getGlobalGroupState(),
                                          GPSItemInfoSorter::SortOptions(sortKey)))
        {
            bestMarkerInfo       = currentMarkerInfo;
            bestMarkerGroupState = currentMarkerGroupState;
            bestMarkerTileIndex  = currentIndex.first;
        }
    }

    const QPair<TileIndex, int> returnedMarker(bestMarkerTileIndex, bestMarkerInfo.id);

    return QVariant::fromValue(returnedMarker);
}

/**
 * @brief This function retrieves the thumbnail for an index.
 *
 * @param index The marker's index.
 * @param size The size of the thumbnail.
 *
 * @return If the thumbnail has been loaded in the ThumbnailLoadThread instance, it is returned.
 * If not, a QPixmap is returned and ThumbnailLoadThread's signal named signalThumbnailLoaded is emitted when the thumbnail becomes available.
 */
QPixmap GPSMarkerTiler::pixmapFromRepresentativeIndex(const QVariant& index, const QSize& size)
{
    QPair<TileIndex, int> indexForPixmap = index.value<QPair<TileIndex, int> >();

    QPixmap thumbnail;
    ItemInfo info(indexForPixmap.second);
    d->thumbnailMap.insert(info.id(), index);

    if (d->thumbnailLoadThread->find(info.thumbnailIdentifier(), thumbnail, qMax(size.width() + 2, size.height() + 2)))
    {
        // digikam returns thumbnails with a border around them,
        // but geolocation interface expects them without a border

        return thumbnail.copy(1, 1, thumbnail.size().width() - 2, thumbnail.size().height() - 2);
    }
    else
    {
        return QPixmap();
    }
}

/**
 * @brief This function compares two marker indices.
 */
bool GPSMarkerTiler::indicesEqual(const QVariant& a, const QVariant& b) const
{
    QPair<TileIndex, int> firstIndex  = a.value<QPair<TileIndex, int> >();
    QPair<TileIndex, int> secondIndex = b.value<QPair<TileIndex, int> >();

    QList<int> aIndicesList           = firstIndex.first.toIntList();
    QList<int> bIndicesList           = secondIndex.first.toIntList();

    if ((firstIndex.second == secondIndex.second) && (aIndicesList == bIndicesList))
    {
        return true;
    }

    return false;
}

GeoGroupState GPSMarkerTiler::getTileGroupState(const TileIndex& tileIndex)
{
    const bool haveGlobalSelection = (d->mapGlobalGroupState & (FilteredPositiveMask | RegionSelectedMask));

    if (!haveGlobalSelection)
    {
        return SelectedNone;
    }

    /// @todo Store this state in the tiles!

    MyTile* const tile = static_cast<MyTile*>(getTile(tileIndex, true));
    GroupStateComputer tileStateComputer;

    for (int i = 0 ; i < tile->imagesId.count() ; ++i)
    {
        const GeoGroupState imageState = getImageState(tile->imagesId.at(i));

        tileStateComputer.addState(imageState);
    }

    return tileStateComputer.getState();
}

/**
 * @brief The marker data is returned from the database in batches. This function takes and unites the batches.
 */
void GPSMarkerTiler::slotMapImagesJobData(const QList<ItemListerRecord>& records)
{
    if (records.isEmpty())
    {
        return;
    }

    Private::InternalJobs* internalJob = nullptr;

    for (int i = 0 ; i < d->jobs.count() ; ++i)
    {
        if (sender() == d->jobs.at(i).jobThread)
        {
            /// @todo Is this really safe?

            internalJob = &d->jobs[i];
            break;
        }
    }

    if (!internalJob)
    {
        return;
    }

    for (const ItemListerRecord& record : std::as_const(records))
    {
        if (record.extraValues.count() < 2)
        {
            // skip info without coordinates

            continue;
        }

        GPSItemInfo entry;

        entry.id           = record.imageID;
        entry.rating       = record.rating;
        entry.dateTime     = record.creationDate;
        entry.coordinates.setLatLon(record.extraValues.first().toDouble(), record.extraValues.last().toDouble());

        internalJob->dataFromDatabase << entry;
    }
}

/**
 * @brief Now, all the marker data has been retrieved from the database. Here, the markers are sorted into tiles.
 */
void GPSMarkerTiler::slotMapImagesJobResult()
{
    int foundIndex = -1;

    for (int i = 0 ; i < d->jobs.count() ; ++i)
    {
        if (sender() == d->jobs.at(i).jobThread)
        {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex < 0)
    {
        // this should not happen, but ok...

        return;
    }

    if (d->jobs.at(foundIndex).jobThread->hasErrors())
    {
        const QString& err = d->jobs.at(foundIndex).jobThread->errorsList().first();

        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list images in selected area: "
                                       << err;

        // Pop-up a message about the error.

        DNotificationWrapper(QString(), err,
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    // get the results from the job:

    const QList<GPSItemInfo> returnedItemInfo = d->jobs.at(foundIndex).dataFromDatabase;

    /// @todo Currently, we ignore the wanted level and just add the images
/*
    const int wantedLevel = d->jobs.at(foundIndex).level;
*/
    // remove the finished job

    d->jobs[foundIndex].jobThread->cancel();
    d->jobs[foundIndex].jobThread = nullptr;
    d->jobs.removeAt(foundIndex);

    if (returnedItemInfo.isEmpty())
    {
        return;
    }

    // QElapsedTimer elapsedTimer;
    // elapsedTimer.start();

    for (int i = 0 ; i < returnedItemInfo.count() ; ++i)
    {
        const GPSItemInfo currentItemInfo = returnedItemInfo.at(i);

        if (!currentItemInfo.coordinates.hasCoordinates())
        {
            continue;
        }

        if (d->imagesHash.contains(currentItemInfo.id))
        {
            continue;
        }

        d->imagesHash.insert(currentItemInfo.id, currentItemInfo);

        const TileIndex markerTileIndex = TileIndex::fromCoordinates(currentItemInfo.coordinates, TileIndex::MaxLevel);
        addMarkerToTileAndChildren(currentItemInfo.id, markerTileIndex);
    }

    // qCDebug(DIGIKAM_GENERAL_LOG) << "added" << returnedItemInfo.count()
    //                              << "markers in" << elapsedTimer.nsecsElapsed() / 1e9 << "seconds";

    Q_EMIT signalTilesOrSelectionChanged();
}

/**
 * @brief Because of a call to pixmapFromRepresentativeIndex, some thumbnails are not yet loaded at the time of requesting.
 * When each thumbnail loads, this slot is called and emits a signal that announces the map that the thumbnail is available.
 */
void GPSMarkerTiler::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumbnail)
{
    QVariant index = d->thumbnailMap.value(loadingDescription.thumbnailIdentifier().id);
/*
    QPair<TileIndex, int> indexForPixmap =
    index.value<QPair<TileIndex, int> >();
*/
    Q_EMIT signalThumbnailAvailableForIndex(index, thumbnail.copy(1, 1, thumbnail.size().width() - 2, thumbnail.size().height() - 2));
}

/**
 * @brief Sets the map active/inactive
 * @param state New state of the map, true means active.
 */
void GPSMarkerTiler::setActive(const bool state)
{
    d->activeState = state;
}

AbstractMarkerTiler::Tile* GPSMarkerTiler::tileNew()
{
    return new MyTile();
}

/**
 * @brief Receives notifications from the database when images were changed and updates the tiler
 */
void GPSMarkerTiler::slotImageChange(const ImageChangeset& changeset)
{
    const DatabaseFields::Set changes = changeset.changes();

    if (
        !(
          (changes & DatabaseFields::LatitudeNumber)  ||
          (changes & DatabaseFields::LongitudeNumber) ||
          (changes & DatabaseFields::Altitude)
         )
       )
    {
        return;
    }

    const auto ids = changeset.ids();

    for (const qlonglong& id : ids)
    {
        const ItemInfo newItemInfo(id);

        if (!newItemInfo.hasCoordinates())
        {
            if (d->imagesHash.contains(id))
            {
                // the image has no coordinates any more
                // remove it from the tiles and the image list

                const GPSItemInfo oldInfo           = d->imagesHash.value(id);
                const GeoCoordinates oldCoordinates = oldInfo.coordinates;
                const TileIndex oldTileIndex        = TileIndex::fromCoordinates(oldCoordinates, TileIndex::MaxLevel);

                removeMarkerFromTileAndChildren(id, oldTileIndex);

                d->imagesHash.remove(id);
            }

            continue;
        }

        GeoCoordinates newCoordinates(newItemInfo.latitudeNumber(), newItemInfo.longitudeNumber());

        if (newItemInfo.hasAltitude())
        {
            newCoordinates.setAlt(newItemInfo.altitudeNumber());
        }

        if (d->imagesHash.contains(id))
        {
            // the image id is known, therefore the image has already been sorted into tiles.
            // We assume that the coordinates of the image have changed.

            const GPSItemInfo oldInfo           = d->imagesHash.value(id);
            const GeoCoordinates oldCoordinates = oldInfo.coordinates;
            const GPSItemInfo currentItemInfo   = GPSItemInfo::fromIdCoordinatesRatingDateTime(id,
                                                                                               newCoordinates,
                                                                                               newItemInfo.rating(),
                                                                                               newItemInfo.dateTime());

            d->imagesHash.insert(id, currentItemInfo);

            const TileIndex oldTileIndex        = TileIndex::fromCoordinates(oldCoordinates, TileIndex::MaxLevel);
            const TileIndex newTileIndex        = TileIndex::fromCoordinates(newCoordinates, TileIndex::MaxLevel);

            // remove from old position

            removeMarkerFromTileAndChildren(id, oldTileIndex);

            // add at new position

            addMarkerToTileAndChildren(id, newTileIndex);

        }
        else
        {
            // the image is new, add it to the existing tiles

            const GPSItemInfo currentItemInfo = GPSItemInfo::fromIdCoordinatesRatingDateTime(id,
                                                                                             newCoordinates,
                                                                                             newItemInfo.rating(),
                                                                                             newItemInfo.dateTime());

            d->imagesHash.insert(id, currentItemInfo);

            const TileIndex newMarkerTileIndex = TileIndex::fromCoordinates(currentItemInfo.coordinates, TileIndex::MaxLevel);

            addMarkerToTileAndChildren(id, newMarkerTileIndex);
        }
    }

    Q_EMIT signalTilesOrSelectionChanged();
}

/**
 * @brief Receives notifications from the album model about new items
 */
void GPSMarkerTiler::slotNewModelData(const QList<ItemInfo>& infoList)
{
    // We do not actually store the data from the model, we just want
    // to know that something was changed.

    /// @todo Also monitor removed, reset, etc. signals

    Q_UNUSED(infoList);

    Q_EMIT signalTilesOrSelectionChanged();
}

void GPSMarkerTiler::setRegionSelection(const GeoCoordinates::Pair& sel)
{
    d->currentRegionSelection = sel;

    if (sel.first.hasCoordinates())
    {
        d->mapGlobalGroupState |= RegionSelectedMask;
    }
    else
    {
        d->mapGlobalGroupState &= ~RegionSelectedMask;
    }

    Q_EMIT signalTilesOrSelectionChanged();
}

void GPSMarkerTiler::removeCurrentRegionSelection()
{
    d->currentRegionSelection.first.clear();

    d->mapGlobalGroupState &= ~RegionSelectedMask;

    Q_EMIT signalTilesOrSelectionChanged();
}

void GPSMarkerTiler::onIndicesClicked(const ClickInfo& clickInfo)
{
    /// @todo Also handle the representative index

    QList<qlonglong> clickedImagesId;

    for (const TileIndex& tileIndex : std::as_const(clickInfo.tileIndicesList))
    {
        clickedImagesId << getTileMarkerIds(tileIndex);
    }

    int repImageId = -1;

    if (clickInfo.representativeIndex.canConvert<QPair<TileIndex, int> >())
    {
        repImageId = clickInfo.representativeIndex.value<QPair<TileIndex, int> >().second;
    }

    if (clickInfo.currentMouseMode == MouseModeSelectThumbnail && d->selectionModel)
    {
        /**
         * @todo This does not work properly, because not all images in a tile
         * may be selectable because some of them are outside of the region selection
         */
        const bool doSelect = (clickInfo.groupSelectionState & SelectedMask) != SelectedAll;

        const QItemSelectionModel::SelectionFlags selectionFlags =
                    (doSelect ? QItemSelectionModel::Select
                              : QItemSelectionModel::Deselect) | QItemSelectionModel::Rows;

        for (int i = 0 ; i < clickedImagesId.count() ; ++i)
        {
            const QModelIndex currentIndex = d->imageFilterModel->indexForImageId(clickedImagesId.at(i));

            if (d->selectionModel->isSelected(currentIndex) != doSelect)
            {
                d->selectionModel->select(currentIndex, selectionFlags);
            }
        }

        if (repImageId >= 0)
        {
            const QModelIndex repImageIndex = d->imageFilterModel->indexForImageId(repImageId);

            if (repImageIndex.isValid())
            {
                d->selectionModel->setCurrentIndex(repImageIndex, selectionFlags);
            }
        }
    }
    else if (clickInfo.currentMouseMode == MouseModeFilter)
    {
        setPositiveFilterIsActive(true);

        Q_EMIT signalModelFilteredImages(clickedImagesId);
    }
}

QList<qlonglong> GPSMarkerTiler::getTileMarkerIds(const TileIndex& tileIndex)
{
    Q_ASSERT(tileIndex.level() <= TileIndex::MaxLevel);

    const MyTile* const myTile = static_cast<MyTile*>(getTile(tileIndex, true));

    if (!myTile)
    {
        return QList<qlonglong>();
    }

    return myTile->imagesId;
}

GeoGroupState GPSMarkerTiler::getGlobalGroupState()
{
    return d->mapGlobalGroupState;
}

GeoGroupState GPSMarkerTiler::getImageState(const qlonglong imageId)
{
    GeoGroupState imageState;

    // is the image inside the region selection?

    if (d->mapGlobalGroupState & RegionSelectedMask)
    {
        const QModelIndex imageAlbumModelIndex = d->imageAlbumModel->indexForImageId(imageId);

        if (imageAlbumModelIndex.isValid())
        {
            imageState |= RegionSelectedAll;
        }
        else
        {
            // not inside region selection, therefore
            // no other flags can apply

            return RegionSelectedNone;
        }
    }

    // is the image positively filtered?

    if (d->mapGlobalGroupState & FilteredPositiveMask)
    {
        const QModelIndex imageIndexInFilterModel = d->imageFilterModel->indexForImageId(imageId);

        if (imageIndexInFilterModel.isValid())
        {
            imageState |= FilteredPositiveAll;

            // is the image selected?

            if (d->selectionModel->hasSelection())
            {
                if (d->selectionModel->isSelected(imageIndexInFilterModel))
                {
                    imageState |= SelectedAll;
                }
            }
        }
        else
        {
            // the image is not positively filtered, therefore it can
            // not be selected

            return imageState;
        }
    }
    else
    {
        // is the image selected?

        if (d->selectionModel->hasSelection())
        {
            const QModelIndex imageIndexInFilterModel = d->imageFilterModel->indexForImageId(imageId);

            if (d->selectionModel->isSelected(imageIndexInFilterModel))
            {
                imageState |= SelectedAll;
            }
        }
    }

    return imageState;
}

void GPSMarkerTiler::setPositiveFilterIsActive(const bool state)
{
    if (state)
    {
        d->mapGlobalGroupState |= FilteredPositiveMask;
    }
    else
    {
        d->mapGlobalGroupState &= ~FilteredPositiveMask;
    }

    /// @todo Somehow, a delay is necessary before emitting this signal - probably the order in which the filtering
    /// is propagated to other parts of digikam is wrong or just takes too long

    QTimer::singleShot(100, this, SIGNAL(signalTilesOrSelectionChanged()));
/*
    Q_EMIT signalTilesOrSelectionChanged();
*/
}

void GPSMarkerTiler::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    /// @todo Buffer this information, update the tiles, etc.

    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    Q_EMIT signalTilesOrSelectionChanged();
}

void GPSMarkerTiler::removeMarkerFromTileAndChildren(const qlonglong imageId, const TileIndex& markerTileIndex)
{
    MyTile* currentParentTile = nullptr;
    MyTile* currentTile       = static_cast<MyTile*>(rootTile());

    for (int level = 0 ; level <= markerTileIndex.level() ; ++level)
    {
        currentTile->imagesId.removeOne(imageId);

        if (currentTile->imagesId.isEmpty())
        {
            if (currentTile == rootTile())
            {
                break;
            }

            // this tile can be deleted

            if (currentParentTile)
            {
                currentParentTile->deleteChild(currentTile);
            }

            break;
        }

        currentParentTile = currentTile;
        currentTile       = static_cast<MyTile*>(currentParentTile->getChild(markerTileIndex.at(level)));

        if (!currentTile)
        {
            break;
        }
    }
}

void GPSMarkerTiler::addMarkerToTileAndChildren(const qlonglong imageId, const TileIndex& markerTileIndex)
{
    MyTile* currentTile = static_cast<MyTile*>(rootTile());

    for (int level = 0 ; level <= markerTileIndex.level() ; ++level)
    {
        currentTile->imagesId.append(imageId);

        if (currentTile->childrenEmpty())
        {
            break;
        }

        MyTile* nextTile = static_cast<MyTile*>(currentTile->getChild(markerTileIndex.at(level)));

        if (!nextTile)
        {
            nextTile = static_cast<MyTile*>(tileNew());
            currentTile->addChild(markerTileIndex.at(level), nextTile);
        }

        currentTile = nextTile;
    }
}

} // namespace Digikam

#include "moc_gpsmarkertiler.cpp"
