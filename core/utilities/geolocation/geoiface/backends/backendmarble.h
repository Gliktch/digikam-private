/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-08
 * Description : Marble-backend for geolocation interface
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

/// @cond false
namespace Marble
{
    class GeoPainter;
}
/// @endcond

namespace Digikam
{

class DIGIKAM_EXPORT BackendMarble : public MapBackend
{
    Q_OBJECT

public:

    explicit BackendMarble(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData,
                           QObject* const parent = nullptr);
    ~BackendMarble()                                                          override;

    QString backendName()                                               const override;
    QString backendHumanName()                                          const override;
    QWidget* mapWidget()                                                      override;
    void releaseWidget(GeoIfaceInternalWidgetInfo* const info)                override;
    void mapWidgetDocked(const bool state)                                    override;

    GeoCoordinates getCenter()                                          const override;
    void setCenter(const GeoCoordinates& coordinate)                          override;

    bool isReady()                                                      const override;

    void zoomIn()                                                             override;
    void zoomOut()                                                            override;

    void saveSettingsToGroup(KConfigGroup* const group)                       override;
    void readSettingsFromGroup(const KConfigGroup* const group)               override;

    void addActionsToConfigurationMenu(QMenu* const configurationMenu)        override;

    void updateMarkers()                                                      override;
    void updateClusters()                                                     override;

    QString getMapTheme()                                               const;
    void setMapTheme(const QString& newMapTheme);

    QString getProjection()                                             const;
    void setProjection(const QString& newProjection);

    bool screenCoordinates(const GeoCoordinates& coordinates,
                           QPoint* const point)                               override;
    bool geoCoordinates(const QPoint& point,
                        GeoCoordinates* const coordinates)              const override;
    QSize mapSize()                                                     const override;

    void setZoom(const QString& newZoom)                                      override;
    QString getZoom()                                                   const override;

    int getMarkerModelLevel()                                                 override;
    GeoCoordinates::PairList getNormalizedBounds()                            override;
/*
    virtual void updateDragDropMarker(const QPoint& pos,
                                      const GeoIfaceDragData* const dragData);
    virtual void updateDragDropMarkerPosition(const QPoint& pos);
*/
    void updateActionAvailability()                                           override;

    void marbleCustomPaint(Marble::GeoPainter* painter);
    void setShowCompass(const bool state);
    void setShowScaleBar(const bool state);
    void setShowNavigation(const bool state);
    void setShowOverviewMap(const bool state);

    void regionSelectionChanged()                                             override;
    void mouseModeChanged()                                                   override;

    void centerOn(const Marble::GeoDataLatLonBox& box,
                  const bool useSaneZoomLevel)                                override;
    void setActive(const bool state)                                          override;

    void reload()                                                             override;

public Q_SLOTS:

    void slotClustersNeedUpdating()                                           override;
    void slotThumbnailAvailableForIndex(const QVariant& index,
                                        const QPixmap& pixmap)                override;
    void slotUngroupedModelChanged(const int index);
    void slotTrackManagerChanged()                                            override;

protected:

    bool eventFilter(QObject* object, QEvent* event)                          override;
    void createActions();
    bool findSnapPoint(const QPoint& actualPoint,
                       QPoint* const snapPoint,
                       GeoCoordinates* const snapCoordinates,
                       QPair<int, QModelIndex>* const snapTargetIndex);
    void GeoPainter_drawPixmapAtCoordinates(Marble::GeoPainter* const painter,
                                            const QPixmap& pixmap,
                                            const GeoCoordinates& coordinates,
                                            const QPoint& basePoint);
    void drawSearchRectangle(Marble::GeoPainter* const painter,
                             const GeoCoordinates::Pair& searchRectangle,
                             const bool isOldRectangle);
    void applyCacheToWidget();

    static void deleteInfoFunction(GeoIfaceInternalWidgetInfo* const info);

protected Q_SLOTS:

    void slotMapThemeActionTriggered(QAction* action);
    void slotProjectionActionTriggered(QAction* action);
    void slotFloatSettingsTriggered(QAction* action);
    void slotMarbleZoomChanged();
    void slotTracksChanged(const QList<TrackManager::TrackChanges>& trackChanges);
    void slotScheduleUpdate();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
