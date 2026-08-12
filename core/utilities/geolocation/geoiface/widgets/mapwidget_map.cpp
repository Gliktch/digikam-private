/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library - Map Management
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mapwidget_p.h"

namespace Digikam
{

QStringList MapWidget::availableBackends() const
{
    QStringList result;

    for (const MapBackend* const backend : std::as_const(d->loadedBackends))
    {
        result.append(backend->backendName());
    }

    return result;
}

QList<MapBackend*> MapWidget::backends() const
{
    return d->loadedBackends;
}

bool MapWidget::setBackend(const QString& backendName)
{
    if (backendName == d->currentBackendName)
    {
        return true;
    }

    saveBackendToCache();

    // switch to the placeholder widget:

    setShowPlaceholderWidget(true);
    removeMapWidgetFromFrame();

    // disconnect signals from old backend:

    if (d->currentBackend)
    {
        d->currentBackend->setActive(false);

        disconnect(d->currentBackend, SIGNAL(signalBackendReadyChanged(QString)),
                   this, SLOT(slotBackendReadyChanged(QString)));

        disconnect(d->currentBackend, SIGNAL(signalZoomChanged(QString)),
                   this, SLOT(slotBackendZoomChanged(QString)));

        disconnect(d->currentBackend, SIGNAL(signalClustersMoved(QIntList,QPair<int,QModelIndex>)),
                   this, SLOT(slotClustersMoved(QIntList,QPair<int,QModelIndex>)));

        disconnect(d->currentBackend, SIGNAL(signalClustersClicked(QIntList)),
                   this, SLOT(slotClustersClicked(QIntList)));

        disconnect(this, SIGNAL(signalUngroupedModelChanged(int)),
                   d->currentBackend, SLOT(slotUngroupedModelChanged(int)));

        if (s->markerModel)
        {
            disconnect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(QVariant,QPixmap)),
                       d->currentBackend, SLOT(slotThumbnailAvailableForIndex(QVariant,QPixmap)));
        }

        disconnect(d->currentBackend, SIGNAL(signalSelectionHasBeenMade(Digikam::GeoCoordinates::Pair)),
                   this, SLOT(slotNewSelectionFromMap(Digikam::GeoCoordinates::Pair)));
    }

    for (MapBackend* const backend : std::as_const(d->loadedBackends))
    {
        if (backend->backendName() == backendName)
        {   // cppcheck-suppress useStlAlgorithm
            qCDebug(DIGIKAM_GEOIFACE_LOG) << QString::fromLatin1("setting backend %1").arg(backendName);

            d->currentBackend     = backend;
            d->currentBackendName = backendName;

            connect(d->currentBackend, &MapBackend::signalBackendReadyChanged,
                    this, &MapWidget::slotBackendReadyChanged);

            connect(d->currentBackend, &MapBackend::signalZoomChanged,
                    this, &MapWidget::slotBackendZoomChanged);

            connect(d->currentBackend, &MapBackend::signalClustersMoved,
                    this, &MapWidget::slotClustersMoved);

            connect(d->currentBackend, &MapBackend::signalClustersClicked,
                    this, &MapWidget::slotClustersClicked);

            /**
             * @todo This connection is queued because otherwise QAbstractItemModel::itemSelected does not
             *       reflect the true state. Maybe monitor another signal instead?
             */
            connect(this, SIGNAL(signalUngroupedModelChanged(int)),
                    d->currentBackend, SLOT(slotUngroupedModelChanged(int)), Qt::QueuedConnection);

            if (s->markerModel)
            {
                connect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(QVariant,QPixmap)),
                        d->currentBackend, SLOT(slotThumbnailAvailableForIndex(QVariant,QPixmap)));
            }

            connect(d->currentBackend, &MapBackend::signalSelectionHasBeenMade,
                    this, &MapWidget::slotNewSelectionFromMap);

            if (s->activeState)
            {
                setMapWidgetInFrame(d->currentBackend->mapWidget());

                // call this slot manually in case the backend was ready right away:

                if (d->currentBackend->isReady())
                {
                    slotBackendReadyChanged(d->currentBackendName);
                }
                else
                {
                    rebuildConfigurationMenu();
                }
            }

            d->currentBackend->setActive(s->activeState);
            GeolocationSettings::instance()->registerWidget(this);

            return true;
        }
    }

    return false;
}

void MapWidget::applyCacheToBackend()
{
    if ((!currentBackendReady()) || (!s->activeState))
    {
        return;
    }

    /// @todo Only do this if the zoom was changed!

    qCDebug(DIGIKAM_GEOIFACE_LOG) << d->cacheZoom;

    setZoom(d->cacheZoom);
    setCenter(d->cacheCenterCoordinate);
    d->currentBackend->mouseModeChanged();
    d->currentBackend->regionSelectionChanged();
}

void MapWidget::saveBackendToCache()
{
    if (!currentBackendReady())
    {
        return;
    }

    d->cacheCenterCoordinate = getCenter();
    d->cacheZoom             = getZoom();
}

GeoCoordinates MapWidget::getCenter() const
{
    if (!currentBackendReady())
    {
        return d->cacheCenterCoordinate;
    }

    return d->currentBackend->getCenter();
}

void MapWidget::setCenter(const GeoCoordinates& coordinate)
{
    d->cacheCenterCoordinate = coordinate;

    if (!currentBackendReady())
    {
        return;
    }

    d->currentBackend->setCenter(coordinate);
}

void MapWidget::slotBackendReadyChanged(const QString& backendName)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << QString::fromLatin1("backend %1 is ready!").arg(backendName);

    if (backendName != d->currentBackendName)
    {
        return;
    }

    if (!currentBackendReady())
    {
        return;
    }

    applyCacheToBackend();

    setShowPlaceholderWidget(false);

    if (!d->thumbnailsHaveBeenLoaded)
    {
        d->thumbnailTimer      = new QTimer(this);
        d->thumbnailTimerCount = 0;

        connect(d->thumbnailTimer, &QTimer::timeout,
                this, &MapWidget::stopThumbnailTimer);

        d->thumbnailTimer->start(2000);
    }

    updateMarkers();
    markClustersAsDirty();
    rebuildConfigurationMenu();

    if (
        dynamic_cast<BackendMarble*>(sender()) ||
        (dynamic_cast<QActionGroup*>(sender()) &&
        (backendName == QLatin1String("marble")))
       )
    {
        QTimer::singleShot(2000, this, SLOT(slotApplySettings()));
    }
}

void MapWidget::setZoom(const QString& newZoom)
{
    d->cacheZoom = newZoom;

    if (currentBackendReady())
    {
        d->currentBackend->setZoom(d->cacheZoom);
    }
}

QString MapWidget::getZoom()
{
    if (currentBackendReady())
    {
        d->cacheZoom = d->currentBackend->getZoom();
    }

    return d->cacheZoom;
}

void MapWidget::adjustBoundariesToGroupedMarkers(const bool useSaneZoomLevel)
{
    if ((!s->activeState) || (!s->markerModel) || (!currentBackendReady()))
    {
        return;
    }

#ifdef HAVE_GEOLOCATION

    Marble::GeoDataLineString tileString;

    /// @todo not sure that this is the best way to find the bounding box of all items

    for (AbstractMarkerTiler::NonEmptyIterator tileIterator(s->markerModel, TileIndex::MaxLevel);
         !tileIterator.atEnd();
         tileIterator.nextIndex())
    {
        const TileIndex tileIndex = tileIterator.currentIndex();

        for (int corner = 1 ; corner <= 4 ; ++corner)
        {
            const GeoCoordinates currentTileCoordinate = tileIndex.toCoordinates(TileIndex::CornerPosition(corner));

            const Marble::GeoDataCoordinates tileCoordinate(currentTileCoordinate.lon(),
                                                            currentTileCoordinate.lat(),
                                                            0,
                                                            Marble::GeoDataCoordinates::Degree);

            tileString.append(tileCoordinate);
        }
    }

    const Marble::GeoDataLatLonBox latLonBox = Marble::GeoDataLatLonBox::fromLineStringBase(tileString);

    /// @todo use a sane zoom level

    d->currentBackend->centerOn(latLonBox, useSaneZoomLevel);

#else

    Q_UNUSED(useSaneZoomLevel);

#endif
}

void MapWidget::refreshMap()
{
    slotRequestLazyReclustering();
}

void MapWidget::setRegionSelection(const GeoCoordinates::Pair& region)
{
    s->selectionRectangle = region;

    d->currentBackend->regionSelectionChanged();

    slotUpdateActionsEnabled();
}

void MapWidget::clearRegionSelection()
{
    s->selectionRectangle.first.clear();

    d->currentBackend->regionSelectionChanged();

    slotUpdateActionsEnabled();
}

void MapWidget::slotNewSelectionFromMap(const Digikam::GeoCoordinates::Pair& sel)
{
    /// @todo Should the backend update s on its own?

    s->selectionRectangle = sel;

    slotUpdateActionsEnabled();

    Q_EMIT signalRegionSelectionChanged();
}

void MapWidget::slotRemoveCurrentRegionSelection()
{
    clearRegionSelection();
    d->currentBackend->regionSelectionChanged();

    slotUpdateActionsEnabled();

    Q_EMIT signalRegionSelectionChanged();
}

GeoCoordinates::Pair MapWidget::getRegionSelection()
{
    return s->selectionRectangle;
}

void MapWidget::updateMarkers()
{
    if (!currentBackendReady())
    {
        return;
    }

    // tell the backend to update the markers

    d->currentBackend->updateMarkers();
}

void MapWidget::updateClusters()
{
    /// @todo Find a better way to tell the TileGrouper about the backend

    s->tileGrouper->setCurrentBackend(d->currentBackend);
    s->tileGrouper->updateClusters();
}

void MapWidget::markClustersAsDirty()
{
    s->tileGrouper->setClustersDirty();
}


QPixmap MapWidget::getDecoratedPixmapForCluster(const int clusterId,
                                                const GeoGroupState* const selectedStateOverride,
                                                const int* const countOverride,
                                                QPoint* const centerPoint)
{
    GeoIfaceCluster& cluster = s->clusterList[clusterId];
    int markerCount          = cluster.markerCount;
    GeoGroupState groupState = cluster.groupState;

    if (selectedStateOverride)
    {
        groupState  = *selectedStateOverride;
        markerCount = *countOverride;
    }

    const GeoGroupState selectedState = groupState & SelectedMask;

    // first determine all the color and style values

    QColor       fillColor;
    QColor       strokeColor;
    Qt::PenStyle strokeStyle;
    QColor       labelColor;
    QString      labelText;
    getColorInfos(clusterId, &fillColor, &strokeColor,
                  &strokeStyle, &labelText, &labelColor,
                  &selectedState,
                  &markerCount);

    // determine whether we should use a pixmap or a placeholder

    if (!s->showThumbnails)
    {
        /// @todo Handle positive filtering and region selection!

        QString pixmapName = fillColor.name().mid(1);

        if (selectedState == SelectedAll)
        {
            pixmapName += QLatin1String("-selected");
        }

        if (selectedState == SelectedSome)
        {
            pixmapName += QLatin1String("-someselected");
        }

        const QPixmap& markerPixmap = GeoIfaceGlobalObject::instance()->getMarkerPixmap(pixmapName);

        // update the display information stored in the cluster:

        cluster.pixmapType          = GeoIfaceCluster::PixmapMarker;
        cluster.pixmapOffset        = QPoint(markerPixmap.width()/2, markerPixmap.height()-1);
        cluster.pixmapSize          = markerPixmap.size();

        if (centerPoint)
        {
            *centerPoint = cluster.pixmapOffset;
        }

        return markerPixmap;
    }

    /// @todo This check is strange, there can be no clusters without a markerModel?

    bool displayThumbnail = (s->markerModel != nullptr);

    if (displayThumbnail)
    {
        if (markerCount == 1)
        {
            displayThumbnail = s->previewSingleItems;
        }
        else
        {
            displayThumbnail = s->previewGroupedItems;
        }
    }

    if (displayThumbnail)
    {
        const QVariant representativeMarker = getClusterRepresentativeMarker(clusterId, s->sortKey);
        const int undecoratedThumbnailSize  = getUndecoratedThumbnailSize();
        QPixmap clusterPixmap               = s->markerModel->pixmapFromRepresentativeIndex(representativeMarker,
                                                                                            QSize(undecoratedThumbnailSize,
                                                                                                  undecoratedThumbnailSize));

        if (!clusterPixmap.isNull())
        {
            QPixmap resultPixmap(clusterPixmap.size() + QSize(2, 2));

            // we may draw with partially transparent pixmaps later, so make sure we have a defined
            // background color

            resultPixmap.fill(QColor::fromRgb(0xff, 0xff, 0xff));
            QPainter painter(&resultPixmap);
/*
             painter.setRenderHint(QPainter::Antialiasing);
*/
            const int borderWidth = (groupState&SelectedSome) ? 2 : 1;
            QPen borderPen;
            borderPen.setWidth(borderWidth);
            borderPen.setJoinStyle(Qt::MiterJoin);

            GeoGroupState globalState = s->markerModel->getGlobalGroupState();

            /// @todo What about partially in the region or positively filtered?

            const bool clusterIsNotInRegionSelection  = (globalState & RegionSelectedMask) &&
                                                        ((groupState & RegionSelectedMask) == RegionSelectedNone);
            const bool clusterIsNotPositivelyFiltered = (globalState & FilteredPositiveMask) &&
                                                        ((groupState & FilteredPositiveMask) == FilteredPositiveNone);

            const bool shouldGrayOut                  = clusterIsNotInRegionSelection || clusterIsNotPositivelyFiltered;
            const bool shouldCrossOut                 = clusterIsNotInRegionSelection;

            if (shouldGrayOut)
            {
                /// @todo Cache the alphaPixmap!

                QPixmap alphaPixmap(clusterPixmap.size());
                alphaPixmap.fill(QColor::fromRgb(0x80, 0x80, 0x80));
/*
                NOTE : old Qt4 code ported to Qt5 due to deprecated QPixmap::setAlphaChannel()
                clusterPixmap.setAlphaChannel(alphaPixmap);
*/
                QPainter p(&clusterPixmap);
                p.setOpacity(0.2);
                p.drawPixmap(0, 0, alphaPixmap);
                p.end();
            }

            painter.drawPixmap(QPoint(1, 1), clusterPixmap);

            if (shouldGrayOut || shouldCrossOut)
            {
                // draw a red cross above the pixmap

                QPen crossPen(Qt::red);

                if (!shouldCrossOut)
                {
                    /// @todo Maybe we should also do a cross for not positively filtered images?

                    crossPen.setColor(Qt::blue);
                }

                crossPen.setWidth(2);
                painter.setPen(crossPen);

                const int width  = resultPixmap.size().width();
                const int height = resultPixmap.size().height();
                painter.drawLine(0, 0, width - 1, height - 1);
                painter.drawLine(width - 1, 0, 0, height - 1);
            }

            if (strokeStyle != Qt::SolidLine)
            {
                // paint a white border around the image

                borderPen.setColor(Qt::white);
                painter.setPen(borderPen);
                painter.drawRect(borderWidth - 1, borderWidth - 1,
                                 resultPixmap.size().width() - borderWidth,
                                 resultPixmap.size().height() - borderWidth);
            }

            // now draw the selection border

            borderPen.setColor(strokeColor);
            borderPen.setStyle(strokeStyle);
            painter.setPen(borderPen);
            painter.drawRect(borderWidth - 1, borderWidth - 1,
                             resultPixmap.size().width() - borderWidth,
                             resultPixmap.size().height() - borderWidth);

            if (s->showNumbersOnItems)
            {
                QPen labelPen(labelColor);

                // note: the pen has to be set, otherwise the bounding rect is 0 x 0!!!

                painter.setPen(labelPen);
                const QRect textRect(0, 0, resultPixmap.width(), resultPixmap.height());
                QRect textBoundingRect = painter.boundingRect(textRect,
                                                              Qt::AlignHCenter | Qt::AlignVCenter,
                                                              labelText);
                textBoundingRect.adjust(-1, -1, 1, 1);

                // fill the bounding rect:

                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor::fromRgb(0xff, 0xff, 0xff, 0x80));
                painter.drawRect(textBoundingRect);

                // draw the text:

                painter.setPen(labelPen);
                painter.setBrush(Qt::NoBrush);
                painter.drawText(textRect, Qt::AlignHCenter|Qt::AlignVCenter, labelText);
            }

            // update the display information stored in the cluster:

            cluster.pixmapType   = GeoIfaceCluster::PixmapImage;
            cluster.pixmapOffset = QPoint(resultPixmap.width() / 2, resultPixmap.height() / 2);
            cluster.pixmapSize   = resultPixmap.size();

            if (centerPoint)
            {
                *centerPoint = cluster.pixmapOffset;
            }

            return resultPixmap;
        }
    }

    // we do not have a thumbnail, draw the circle instead:

    const int circleRadius = s->thumbnailSize / 2;
    QPen circlePen;
    circlePen.setColor(strokeColor);
    circlePen.setStyle(strokeStyle);
    circlePen.setWidth(2);
    QBrush circleBrush(fillColor);
    QPen labelPen;
    labelPen.setColor(labelColor);
    const QRect circleRect(0, 0, 2 * circleRadius, 2 * circleRadius);

    const int pixmapDiameter = 2 * (circleRadius + 1);
    QPixmap circlePixmap(pixmapDiameter, pixmapDiameter);

    /// @todo cache this somehow

    circlePixmap.fill(QColor(0, 0, 0, 0));

    QPainter circlePainter(&circlePixmap);
    circlePainter.setPen(circlePen);
    circlePainter.setBrush(circleBrush);
    circlePainter.drawEllipse(circleRect);

    circlePainter.setPen(labelPen);
    circlePainter.setBrush(Qt::NoBrush);
    circlePainter.drawText(circleRect, Qt::AlignHCenter|Qt::AlignVCenter, labelText);

    // update the display information stored in the cluster:

    cluster.pixmapType   = GeoIfaceCluster::PixmapCircle;
    cluster.pixmapOffset = QPoint(circlePixmap.width() / 2, circlePixmap.height() / 2);
    cluster.pixmapSize   = circlePixmap.size();

    if (centerPoint)
    {
        *centerPoint = QPoint(circlePixmap.width() / 2, circlePixmap.height() / 2);
    }

    return circlePixmap;
}

QVariant MapWidget::getClusterRepresentativeMarker(const int clusterIndex, const int sortKey)
{
    if (!s->markerModel)
    {
        return QVariant();
    }

    const GeoIfaceCluster cluster          = s->clusterList.at(clusterIndex);
    QMap<int, QVariant>::const_iterator it = cluster.representativeMarkers.find(sortKey);

    if (it != cluster.representativeMarkers.end())
    {
        return *it;
    }

    QList<QVariant> repIndices;

    for (int i = 0 ; i < cluster.tileIndicesList.count() ; ++i)
    {
        repIndices << s->markerModel->getTileRepresentativeMarker(cluster.tileIndicesList.at(i), sortKey);
    }

    const QVariant clusterRepresentative                        = s->markerModel->bestRepresentativeIndexFromList(repIndices, sortKey);
    s->clusterList[clusterIndex].representativeMarkers[sortKey] = clusterRepresentative;

    return clusterRepresentative;
}

bool MapWidget::currentBackendReady() const
{
    if (!d->currentBackend)
    {
        return false;
    }

    return d->currentBackend->isReady();
}

void MapWidget::setShowPlaceholderWidget(const bool state)
{
    if (state)
    {
        d->stackedLayout->setCurrentIndex(0);
    }
    else
    {
        if (d->stackedLayout->count() > 1)
        {
            d->stackedLayout->setCurrentIndex(1);
        }
    }
}

void MapWidget::setMapWidgetInFrame(QWidget* const widgetForFrame)
{
    if (d->stackedLayout->count() > 1)
    {
        // widget 0 is the status widget, widget 1 is the map widget

        if (d->stackedLayout->widget(1) == widgetForFrame)
        {
            return;
        }

        // there is some other widget at the target position.
        // remove it and add our widget instead

        d->stackedLayout->removeWidget(d->stackedLayout->widget(1));
    }

    d->stackedLayout->addWidget(widgetForFrame);
}

void MapWidget::removeMapWidgetFromFrame()
{
    if (d->stackedLayout->count() > 1)
    {
        d->stackedLayout->removeWidget(d->stackedLayout->widget(1));
    }

    d->stackedLayout->setCurrentIndex(0);
}

void MapWidget::slotClustersNeedUpdating()
{
    if (currentBackendReady())
    {
        d->currentBackend->slotClustersNeedUpdating();
    }
}

void MapWidget::slotChangeBackend(QAction* action)
{
    GEOIFACE_ASSERT(action != nullptr);

    if (!action)
    {
        return;
    }

    const QString newBackendName = action->data().toString();
    setBackend(newBackendName);
}

void MapWidget::slotBackendZoomChanged(const QString& newZoom)
{
    d->cacheZoom = newZoom;
}

void MapWidget::slotClustersMoved(const QIntList& clusterIndices,
                                  const QPair<int, QModelIndex>& snapTarget)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << clusterIndices;

    /// @todo We actually expect only one clusterindex

    int             clusterIndex      = clusterIndices.first();
    GeoCoordinates  targetCoordinates = s->clusterList.at(clusterIndex).coordinates;
    TileIndex::List movedTileIndices;

    if (s->clusterList.at(clusterIndex).groupState == SelectedNone)
    {
        // a not-selected marker was moved. update all of its items:

        const GeoIfaceCluster& cluster = s->clusterList.at(clusterIndex);

        for (int i = 0 ; i < cluster.tileIndicesList.count() ; ++i)
        {
            const TileIndex tileIndex = cluster.tileIndicesList.at(i);
            movedTileIndices << tileIndex;
        }
    }
    else
    {
        // selected items were moved. The model helper should know which tiles are selected,
        // therefore we give him an empty list
    }

    s->markerModel->onIndicesMoved(movedTileIndices, targetCoordinates, snapTarget.second);

    /**
     * @todo Clusters are marked as dirty by slotClustersNeedUpdating
     * which is called while we update the model
     */
}

/**
 * @todo Clicking on several clusters at once is not actually possible
 */
void MapWidget::slotClustersClicked(const QIntList& clusterIndices)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << clusterIndices;

    if (
        (s->currentMouseMode == MouseModeZoomIntoGroup) ||
        (s->currentMouseMode == MouseModeRegionSelectionFromIcon)
       )
    {

#ifdef HAVE_GEOLOCATION

        int maxTileLevel = 0;

        Marble::GeoDataLineString tileString;

        for (int i = 0 ; i < clusterIndices.count() ; ++i)
        {
            const int clusterIndex               = clusterIndices.at(i);
            const GeoIfaceCluster currentCluster = s->clusterList.at(clusterIndex);

            for (int j = 0 ; j < currentCluster.tileIndicesList.count() ; ++j)
            {
                const TileIndex& currentTileIndex = currentCluster.tileIndicesList.at(j);

                for (int corner = 1 ; corner <= 4 ; ++corner)
                {
                    GeoCoordinates currentTileCoordinate;

                    if      (corner == 1)
                    {
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerNW);
                    }
                    else if (corner == 2)
                    {
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerSW);
                    }
                    else if (corner == 3)
                    {
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerNE);
                    }
                    else // corner == 4
                    {
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerSE);
                    }

                    const Marble::GeoDataCoordinates tileCoordinate(currentTileCoordinate.lon(),
                                                                    currentTileCoordinate.lat(),
                                                                    0,
                                                                    Marble::GeoDataCoordinates::Degree);

                    if (maxTileLevel < currentTileIndex.level())
                    {
                        maxTileLevel = currentTileIndex.level();
                    }

                    tileString.append(tileCoordinate);
                }
            }
        }

        Marble::GeoDataLatLonBox latLonBox = Marble::GeoDataLatLonBox::fromLineStringBase(tileString);

        /// @todo Review this section
/*
        if (maxTileLevel != 0)
        {
            //increase the selection boundaries with 0.1 degrees because some thumbnails aren't caught by selection

            latLonBox.setWest((latLonBox.west(Marble::GeoDataCoordinates::Degree) - (0.1 / maxTileLevel)),   Marble::GeoDataCoordinates::Degree);
            latLonBox.setNorth((latLonBox.north(Marble::GeoDataCoordinates::Degree) + (0.1 / maxTileLevel)), Marble::GeoDataCoordinates::Degree);
            latLonBox.setEast((latLonBox.east(Marble::GeoDataCoordinates::Degree) + (0.1 / maxTileLevel)),   Marble::GeoDataCoordinates::Degree);
            latLonBox.setSouth((latLonBox.south(Marble::GeoDataCoordinates::Degree) - (0.1 / maxTileLevel)), Marble::GeoDataCoordinates::Degree);
        }
        else
        {
*/
            latLonBox.setWest((latLonBox.west(Marble::GeoDataCoordinates::Degree) - 0.0001),   Marble::GeoDataCoordinates::Degree);
            latLonBox.setNorth((latLonBox.north(Marble::GeoDataCoordinates::Degree) + 0.0001), Marble::GeoDataCoordinates::Degree);
            latLonBox.setEast((latLonBox.east(Marble::GeoDataCoordinates::Degree) + 0.0001),   Marble::GeoDataCoordinates::Degree);
            latLonBox.setSouth((latLonBox.south(Marble::GeoDataCoordinates::Degree) - 0.0001), Marble::GeoDataCoordinates::Degree);
/*
        }
*/

        if (s->currentMouseMode == MouseModeZoomIntoGroup)
        {
            /// @todo Very small latLonBoxes can crash Marble

            d->currentBackend->centerOn(latLonBox);
        }
        else
        {
            const GeoCoordinates::Pair newSelection(
                    GeoCoordinates(latLonBox.north(Marble::GeoDataCoordinates::Degree),
                                latLonBox.west(Marble::GeoDataCoordinates::Degree)),
                    GeoCoordinates(latLonBox.south(Marble::GeoDataCoordinates::Degree),
                                latLonBox.east(Marble::GeoDataCoordinates::Degree))
                );

            s->selectionRectangle = newSelection;
            d->currentBackend->regionSelectionChanged();

            Q_EMIT signalRegionSelectionChanged();
        }

#endif

    }
    else if (
             (
              (s->currentMouseMode == MouseModeFilter) &&
              s->selectionRectangle.first.hasCoordinates()
             ) ||
             (s->currentMouseMode == MouseModeSelectThumbnail)
            )
    {
        // update the selection and filtering state of the clusters

        for (int i = 0 ; i < clusterIndices.count() ; ++i)
        {
            const int clusterIndex               = clusterIndices.at(i);
            const GeoIfaceCluster currentCluster = s->clusterList.at(clusterIndex);
            const TileIndex::List tileIndices    = currentCluster.tileIndicesList;

            /// @todo Isn't this cached in the cluster?

            const QVariant representativeIndex   = getClusterRepresentativeMarker(clusterIndex, s->sortKey);

            AbstractMarkerTiler::ClickInfo clickInfo;
            clickInfo.tileIndicesList            = tileIndices;
            clickInfo.representativeIndex        = representativeIndex;
            clickInfo.groupSelectionState        = currentCluster.groupState;
            clickInfo.currentMouseMode           = s->currentMouseMode;
            s->markerModel->onIndicesClicked(clickInfo);
        }
    }
}

void MapWidget::slotLazyReclusteringRequestCallBack()
{
    if (!d->lazyReclusteringRequested)
    {
        return;
    }

    d->lazyReclusteringRequested = false;
    slotClustersNeedUpdating();
}

void MapWidget::slotRequestLazyReclustering()
{
    if (d->lazyReclusteringRequested)
    {
        return;
    }

    s->tileGrouper->setClustersDirty();

    if (s->activeState)
    {
        d->lazyReclusteringRequested = true;

        QTimer::singleShot(0, this, SLOT(slotLazyReclusteringRequestCallBack()));
    }
}

} // namespace Digikam
