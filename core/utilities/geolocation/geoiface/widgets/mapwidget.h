/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QList>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"
#include "geoifacetypes.h"
#include "geocoordinates.h"
#include "geogroupstate.h"

class QAction;
class QDragEnterEvent;
class QDropEvent;
class QMenu;

class KConfigGroup;

namespace Digikam
{

class GeoIfaceSharedData;
class GeoModelHelper;
class GeoDragDropHandler;
class AbstractMarkerTiler;
class TrackManager;
class MapBackend;

/**
 * @class MapWidget
 * @brief The central map view class of geolocation interface
 *
 * The MapWidget class is the central widget of geolocation interface. It provides a widget which can display maps using
 * either the Marble or Google Maps backend. Using a model, items can be displayed on the map. For
 * models containing only a small number of items, the items can be shown directly, but for models with
 * a larger number of items, the items can also be grouped. Currently, any number of ungrouped models
 * can be shown, but only one grouped model. Item selection models can also be used along with the models,
 * to interact with the selection states of the items on the map. In order to use a model with geolocation interface, however,
 * a model helper has to be implemented, which extracts data from the model that is not provided by the Qt part
 * of a model's API.
 *
 * Now, a brief introduction on how to get geolocation interface working is provided:
 * @li First, an instance of @c MapWidget has to be created.
 * @li Next, @c GeoModelHelper has to be subclassed and at least the pure virtual functions have to be implemented.
 * @li To show the model's data ungrouped, the model helper has to be added to @c MapWidget instance using addUngroupedModel.
 * @li To show the model's data grouped, an instance of @c AbstractMarkerTiler has to be created and the model helper has to be
 *     set to it using setMarkerGeoModelHelper. The @c AbstractMarkerTiler has then to be given to MapWidget using setGroupedModel. If
 *     the items to be displayed do not reside in a model, a subclass of @c AbstractMarkerTiler can be created which returns
 *     just the number of items in a particular area, and picks representative items for thumbnails.
 * @li To handle dropping of items from the host applications UI onto the map, @c DragDropHandler has to be subclassed
 *     as well and added to the model using setDragDropHandler.
 * @li Finally, setActive() has to be called to tell the widget that it should start displaying things.
 */

class DIGIKAM_EXPORT MapWidget : public QWidget
{
    Q_OBJECT

public:

    explicit MapWidget(QWidget* const parent = nullptr);
    ~MapWidget() override;

// --------------------------------------------------

/// @name Settings Management related functions
//@{

public:

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

protected Q_SLOTS:

    void slotItemDisplaySettingsChanged();

private Q_SLOTS:

    void slotApplySettings();
//@}

// --------------------------------------------------

/// @name Map related functions
//@{

public:

    QStringList availableBackends() const;
    bool setBackend(const QString& backendName);

    QList<MapBackend*> backends() const;

    GeoCoordinates getCenter() const;
    void setCenter(const GeoCoordinates& coordinate);

    void setZoom(const QString& newZoom);
    QString getZoom();

    /**
     * @brief Adjusts the visible map area such that all grouped markers are visible.
     *
     * Note that a call to this function currently has no effect if the widget has been
     * set inactive via setActive() or the backend is not yet ready.
     *
     * @param useSaneZoomLevel Stop zooming at a sane level, if markers are too close together.
     */
    void adjustBoundariesToGroupedMarkers(const bool useSaneZoomLevel = true);
    void refreshMap();

    void setRegionSelection(const GeoCoordinates::Pair& region);
    GeoCoordinates::Pair getRegionSelection();
    void clearRegionSelection();

    void updateMarkers();
    void updateClusters();
    void markClustersAsDirty();

    QPixmap getDecoratedPixmapForCluster(const int clusterId, const GeoGroupState* const selectedStateOverride,
                                         const int* const countOverride, QPoint* const centerPoint);
    QVariant getClusterRepresentativeMarker(const int clusterIndex, const int sortKey);

protected:

    bool currentBackendReady() const;
    void applyCacheToBackend();
    void saveBackendToCache();
    void setShowPlaceholderWidget(const bool state);

    /**
     * @brief Set @p widgetForFrame as the widget in the frame, but does not show it.
     */
    void setMapWidgetInFrame(QWidget* const widgetForFrame);

    void removeMapWidgetFromFrame();

public Q_SLOTS:

    void slotClustersNeedUpdating();

protected Q_SLOTS:

    void slotBackendReadyChanged(const QString& backendName);
    void slotChangeBackend(QAction* action);
    void slotBackendZoomChanged(const QString& newZoom);
    void slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void slotClustersClicked(const QIntList& clusterIndices);

    /**
     * @brief Helper function to buffer reclustering
     */
    void slotLazyReclusteringRequestCallBack();

    /**
     * @brief Request reclustering, repeated calls should generate only one actual update of the clusters
     */
    void slotRequestLazyReclustering();

    void slotRemoveCurrentRegionSelection();
    void slotNewSelectionFromMap(const Digikam::GeoCoordinates::Pair& sel);

Q_SIGNALS:

    void signalRegionSelectionChanged();
//@}

// --------------------------------------------------

/// @name Data Management
//@{

public:

    void addUngroupedModel(GeoModelHelper* const modelHelper);
    void removeUngroupedModel(GeoModelHelper* const modelHelper);
    void setGroupedModel(AbstractMarkerTiler* const markerModel);
    void setDragDropHandler(GeoDragDropHandler* const dragDropHandler);
    void setTrackManager(TrackManager* const trackManager);

protected Q_SLOTS:

    void slotUngroupedModelChanged();

Q_SIGNALS:

    void signalLoadTracksFromAlbums();
    void signalUngroupedModelChanged(const int index);
//@}

// --------------------------------------------------

/// @name UI setup
//@{

public:

    QAction* getControlAction(const QString& actionName);

    /**
     * @brief Returns the control widget instance.
     */
    QWidget* getControlWidget();

    void addWidgetToControlWidget(QWidget* const newWidget);
    void setSortOptionsMenu(QMenu* const sortMenu);
    void setMouseMode(const GeoMouseModes mouseMode);
    void setAvailableMouseModes(const GeoMouseModes mouseModes);
    void setVisibleMouseModes(const GeoMouseModes mouseModes);
    void setAllowModifications(const bool state);
    void setActive(const bool state);
    bool getActiveState();
    bool getStickyModeState() const;
    void setStickyModeState(const bool state);
    void setVisibleExtraActions(const GeoExtraActions actions);
    void setEnabledExtraActions(const GeoExtraActions actions);

public Q_SLOTS:

    void slotUpdateActionsEnabled();
    void slotStickyModeChanged();

protected Q_SLOTS:

    void slotMouseModeChanged(QAction* triggeredAction);

protected:

    void rebuildConfigurationMenu();
    void createActions();
    void createActionsForBackendSelection();

    void dropEvent(QDropEvent* event)           override;
    void dragMoveEvent(QDragMoveEvent* event)   override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

Q_SIGNALS:

    void signalMouseModeChanged(const Digikam::GeoMouseModes& currentMouseMode);
//@}

// --------------------------------------------------

/// @name Appearance
//@{

public:

    void setSortKey(const int sortKey);
    void setThumnailSize(const int newThumbnailSize);
    void setThumbnailGroupingRadius(const int newGroupingRadius);
    void setMarkerGroupingRadius(const int newGroupingRadius);
    int  getThumbnailSize()                                                                 const;
    int  getUndecoratedThumbnailSize()                                                      const;
    void setShowThumbnails(const bool state);

    QString convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const;

    /**
     * @brief Return color and style information for rendering the cluster
     * @param clusterIndex Index of the cluster
     * @param fillColor Color used to fill the circle
     * @param strokeColor Color used for the stroke around the circle
     * @param strokeStyle Style used to draw the stroke around the circle
     * @param labelText Text for the label
     * @param labelColor Color for the label text
     * @param overrideSelection Get the colors for a different selection state
     * @param overrideCount Get the colors for a different amount of markers
     */
    void getColorInfos(const int clusterIndex, QColor* fillColor, QColor* strokeColor,
                       Qt::PenStyle* strokeStyle, QString* labelText, QColor* labelColor,
                       const GeoGroupState* const overrideSelection = nullptr,
                       const int* const overrideCount = nullptr)                            const;

    void getColorInfos(const GeoGroupState groupState,
                       const int nMarkers,
                       QColor* fillColor, QColor* strokeColor,
                       Qt::PenStyle* strokeStyle, QString* labelText, QColor* labelColor)   const;

public Q_SLOTS:

    void slotZoomIn();
    void slotZoomOut();
    void slotDecreaseThumbnailSize();
    void slotIncreaseThumbnailSize();
    void stopThumbnailTimer();

protected Q_SLOTS:

    void slotShowThumbnailsChanged();

Q_SIGNALS:

    void signalRemoveCurrentFilter();
    void signalStickyModeChanged();
//@}

// --------------------------------------------------

private:

    const QExplicitlySharedDataPointer<GeoIfaceSharedData> s;

    class Private;
    Private* const                                         d = nullptr;

    Q_DISABLE_COPY(MapWidget)
};

} // namespace Digikam
