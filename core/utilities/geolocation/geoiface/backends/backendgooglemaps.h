/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Google-Maps-backend for geolocation interface
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014      by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "mapbackend.h"
#include "trackmanager.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT BackendGoogleMaps : public MapBackend
{
    Q_OBJECT

public:

    explicit BackendGoogleMaps(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData,
                               QObject* const parent = nullptr);
    ~BackendGoogleMaps()                                                                  override;

    QString backendName()                                                           const override;
    QString backendHumanName()                                                      const override;
    QWidget* mapWidget()                                                                  override;
    void releaseWidget(GeoIfaceInternalWidgetInfo* const info)                            override;
    void mapWidgetDocked(const bool state)                                                override;

    GeoCoordinates getCenter()                                                      const override;
    void setCenter(const GeoCoordinates& coordinate)                                      override;

    bool isReady()                                                                  const override;

    void zoomIn()                                                                         override;
    void zoomOut()                                                                        override;

    void saveSettingsToGroup(KConfigGroup* const group)                                   override;
    void readSettingsFromGroup(const KConfigGroup* const group)                           override;

    void addActionsToConfigurationMenu(QMenu* const configurationMenu)                    override;

    void updateMarkers()                                                                  override;
    void updateClusters()                                                                 override;

    bool screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point)        override;
    bool geoCoordinates(const QPoint& point, GeoCoordinates* const coordinates)     const override;
    QSize mapSize()                                                                 const override;

    void setZoom(const QString& newZoom)                                                  override;
    QString getZoom()                                                               const override;

    int getMarkerModelLevel()                                                             override;
    GeoCoordinates::PairList getNormalizedBounds()                                        override;
/*
    virtual void updateDragDropMarker(const QPoint& pos,
                                      const GeoIfaceDragData* const dragData);
    virtual void updateDragDropMarkerPosition(const QPoint& pos);
*/
    void updateActionAvailability()                                                       override;

    QString getMapType()                                                            const;
    void setMapType(const QString& newMapType);
    void setShowMapTypeControl(const bool state);
    void setShowScaleControl(const bool state);
    void setShowNavigationControl(const bool state);

    void regionSelectionChanged()                                                         override;
    void mouseModeChanged()                                                               override;

    void setActive(const bool state)                                                      override;

#ifdef HAVE_GEOLOCATION

    void centerOn(const Marble::GeoDataLatLonBox& latLonBox, const bool useSaneZoomLevel) override;

#endif

    void reload()                                                                         override;

public Q_SLOTS:

    void slotClustersNeedUpdating()                                                       override;
    void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap)     override;
    void slotUngroupedModelChanged(const int mindex);

protected:

    bool eventFilter(QObject* object, QEvent* event)                                      override;
    void createActions();
    void setClusterPixmap(const int clusterId,
                          const QPoint& centerPoint,
                          const QPixmap& clusterPixmap);
    void setMarkerPixmap(const int modelId,
                         const int markerId,
                         const QPoint& centerPoint,
                         const QPixmap& markerPixmap);
    void setMarkerPixmap(const int modelId,
                         const int markerId,
                         const QPoint& centerPoint,
                         const QSize& iconSize,
                         const QUrl& iconUrl);
    void storeTrackChanges(const TrackManager::TrackChanges trackChanges);

private Q_SLOTS:

    void slotHTMLInitialized();
    void slotSetCenterTimer();
    void slotMessageEvent(const QString& message);
    void slotMapTypeActionTriggered(QAction* action);
    void slotHTMLEvents(const QStringList& eventStrings);
    void slotFloatSettingsTriggered(QAction* action);
    void slotSelectionHasBeenMade(const Digikam::GeoCoordinates::Pair& searchCoordinates);
    void slotTrackManagerChanged()                                                        override;
    void slotTracksChanged(const QList<TrackManager::TrackChanges>& trackChanges);
    void slotTrackVisibilityChanged(const bool newState);

private:

    void updateZoomMinMaxCache();
    static void deleteInfoFunction(GeoIfaceInternalWidgetInfo* const info);
    void addPointsToTrack(const quint64 trackId,
                          TrackManager::TrackPoint::List const& track,
                          const int firstPoint,
                          const int nPoints);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
