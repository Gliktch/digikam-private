/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library - Private container
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "mapwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QItemSelectionModel>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QStackedLayout>
#include <QTimer>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QAction>
#include <QActionGroup>
#include <QFrame>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Marble includes

#ifdef HAVE_GEOLOCATION

#   include "GeoDataLineString.h"
#   include "GeoDataLatLonBox.h"
#   include "MarbleGlobal.h"

#endif

// Local includes

#include "geoifacecommon.h"
#include "geodragdrophandler.h"
#include "geomodelhelper.h"
#include "trackmanager.h"
#include "placeholderwidget.h"
#include "tilegrouper.h"
#include "digikam_debug.h"
#include "abstractmarkertiler.h"
#include "backendgooglemaps.h"
#include "geolocationsettings.h"

#ifdef HAVE_GEOLOCATION

#   include "backendmarble.h"

#endif

namespace Digikam
{

class Q_DECL_HIDDEN MapWidget::Private
{
public:

    Private() = default;

    QList<MapBackend*>      loadedBackends;
    MapBackend*             currentBackend                          = nullptr;
    QString                 currentBackendName;
    QStackedLayout*         stackedLayout                           = nullptr;

    /// @note these values are cached in case the backend is not ready:

    GeoCoordinates          cacheCenterCoordinate                   = GeoCoordinates(52.0, 6.0);
    QString                 cacheZoom                               = QLatin1String("marble:900");

    /// actions for controlling the widget

    QMenu*                  configurationMenu                       = nullptr;
    QActionGroup*           actionGroupBackendSelection             = nullptr;
    QAction*                actionZoomIn                            = nullptr;
    QAction*                actionZoomOut                           = nullptr;
    QAction*                actionShowThumbnails                    = nullptr;
    QWidget*                mouseModesHolder                        = nullptr;
    QPointer<QWidget>       controlWidget;
    QAction*                actionPreviewSingleItems                = nullptr;
    QAction*                actionPreviewGroupedItems               = nullptr;
    QAction*                actionShowNumbersOnItems                = nullptr;

    bool                    lazyReclusteringRequested               = false;

    GeoDragDropHandler*     dragDropHandler                         = nullptr;

    QMenu*                  sortMenu                                = nullptr;
    QAction*                actionIncreaseThumbnailSize             = nullptr;
    QAction*                actionDecreaseThumbnailSize             = nullptr;
    QWidget*                hBoxForAdditionalControlWidgetItems     = nullptr;

    QActionGroup*           mouseModeActionGroup                    = nullptr;
    QAction*                actionRemoveCurrentRegionSelection      = nullptr;
    QAction*                actionSetRegionSelectionMode            = nullptr;
    QAction*                actionSetPanMode                        = nullptr;
    QAction*                actionSetZoomIntoGroupMode              = nullptr;
    QAction*                actionSetRegionSelectionFromIconMode    = nullptr;
    QAction*                actionSetFilterMode                     = nullptr;
    QAction*                actionRemoveFilter                      = nullptr;
    QAction*                actionSetSelectThumbnailMode            = nullptr;
    QAction*                actionLoadTracksFromAlbums              = nullptr;
    QToolButton*            setPanModeButton                        = nullptr;
    QToolButton*            setSelectionModeButton                  = nullptr;
    QToolButton*            removeCurrentSelectionButton            = nullptr;
    QToolButton*            setZoomModeButton                       = nullptr;
    QToolButton*            setRegionSelectionFromIconModeButton    = nullptr;
    QToolButton*            setFilterModeButton                     = nullptr;
    QToolButton*            removeFilterModeButton                  = nullptr;
    QToolButton*            setSelectThumbnailMode                  = nullptr;
    QToolButton*            loadTracksFromAlbums                    = nullptr;

    QTimer*                 thumbnailTimer                          = nullptr;
    int                     thumbnailTimerCount                     = 0;
    bool                    thumbnailsHaveBeenLoaded                = false;

    GeoExtraActions         availableExtraActions;
    GeoExtraActions         visibleExtraActions;
    QAction*                actionStickyMode                        = nullptr;
    QToolButton*            buttonStickyMode                        = nullptr;

    /// @note to be sorted later

    PlaceholderWidget*      placeholderWidget                       = nullptr;
};

} // namespace Digikam
