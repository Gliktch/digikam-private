/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library - UI Setup
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

QAction* MapWidget::getControlAction(const QString& actionName)
{
    if      (actionName == QLatin1String("zoomin"))
    {
        return d->actionZoomIn;
    }
    else if (actionName == QLatin1String("zoomout"))
    {
        return d->actionZoomOut;
    }
    else if (actionName == QLatin1String("mousemode-regionselectionmode"))
    {
        return d->actionSetRegionSelectionMode;
    }
    else if (actionName == QLatin1String("mousemode-removecurrentregionselection"))
    {
        return d->actionRemoveCurrentRegionSelection;
    }
    else if (actionName == QLatin1String("mousemode-regionselectionfromiconmode"))
    {
        return d->actionSetRegionSelectionFromIconMode;
    }
    else if (actionName == QLatin1String("mousemode-removefilter"))
    {
        return d->actionRemoveFilter;
    }

    return nullptr;
}

QWidget* MapWidget::getControlWidget()
{
    if (!d->controlWidget)
    {
        d->controlWidget                           = new QWidget(this);
        QHBoxLayout* const controlWidgetHBoxLayout = new QHBoxLayout(d->controlWidget);
        controlWidgetHBoxLayout->setContentsMargins(QMargins());

        QPushButton* const configurationButton = new QPushButton(d->controlWidget);
        configurationButton->setIcon(QIcon::fromTheme(QLatin1String("globe")));
        controlWidgetHBoxLayout->addWidget(configurationButton);
        configurationButton->setToolTip(i18n("Map settings"));
        configurationButton->setMenu(d->configurationMenu);

        QToolButton* const zoomInButton = new QToolButton(d->controlWidget);
        controlWidgetHBoxLayout->addWidget(zoomInButton);
        zoomInButton->setDefaultAction(d->actionZoomIn);

        QToolButton* const zoomOutButton = new QToolButton(d->controlWidget);
        controlWidgetHBoxLayout->addWidget(zoomOutButton);
        zoomOutButton->setDefaultAction(d->actionZoomOut);

        QToolButton* const showThumbnailsButton = new QToolButton(d->controlWidget);
        controlWidgetHBoxLayout->addWidget(showThumbnailsButton);
        showThumbnailsButton->setDefaultAction(d->actionShowThumbnails);

        QFrame* const vline1 = new QFrame(d->controlWidget);
        vline1->setLineWidth(1);
        vline1->setMidLineWidth(0);
        vline1->setFrameShape(QFrame::VLine);
        vline1->setFrameShadow(QFrame::Sunken);
        vline1->setMinimumSize(2, 0);
        vline1->updateGeometry();
        controlWidgetHBoxLayout->addWidget(vline1);

        QToolButton* const increaseThumbnailSizeButton = new QToolButton(d->controlWidget);
        controlWidgetHBoxLayout->addWidget(increaseThumbnailSizeButton);
        increaseThumbnailSizeButton->setDefaultAction(d->actionIncreaseThumbnailSize);

        QToolButton* const decreaseThumbnailSizeButton = new QToolButton(d->controlWidget);
        controlWidgetHBoxLayout->addWidget(decreaseThumbnailSizeButton);
        decreaseThumbnailSizeButton->setDefaultAction(d->actionDecreaseThumbnailSize);

        /* --- --- --- */

        d->mouseModesHolder                           = new QWidget(d->controlWidget);
        QHBoxLayout* const mouseModesHolderHBoxLayout = new QHBoxLayout(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->setContentsMargins(QMargins());
        controlWidgetHBoxLayout->addWidget(d->mouseModesHolder);

        QFrame* const vline2 = new QFrame(d->mouseModesHolder);
        vline2->setLineWidth(1);
        vline2->setMidLineWidth(0);
        vline2->setFrameShape(QFrame::VLine);
        vline2->setFrameShadow(QFrame::Sunken);
        vline2->setMinimumSize(2, 0);
        vline2->updateGeometry();
        mouseModesHolderHBoxLayout->addWidget(vline2);

        d->setPanModeButton = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->setPanModeButton);
        d->setPanModeButton->setDefaultAction(d->actionSetPanMode);

        d->setSelectionModeButton = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->setSelectionModeButton);
        d->setSelectionModeButton->setDefaultAction(d->actionSetRegionSelectionMode);

        d->setRegionSelectionFromIconModeButton = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->setRegionSelectionFromIconModeButton);
        d->setRegionSelectionFromIconModeButton->setDefaultAction(d->actionSetRegionSelectionFromIconMode);

        d->removeCurrentSelectionButton = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->removeCurrentSelectionButton);
        d->removeCurrentSelectionButton->setDefaultAction(d->actionRemoveCurrentRegionSelection);

        d->setZoomModeButton = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->setZoomModeButton);
        d->setZoomModeButton->setDefaultAction(d->actionSetZoomIntoGroupMode);

        d->setFilterModeButton = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->setFilterModeButton);
        d->setFilterModeButton->setDefaultAction(d->actionSetFilterMode);

        d->removeFilterModeButton = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->removeFilterModeButton);
        d->removeFilterModeButton->setDefaultAction(d->actionRemoveFilter);

        d->setSelectThumbnailMode = new QToolButton(d->mouseModesHolder);
        mouseModesHolderHBoxLayout->addWidget(d->setSelectThumbnailMode);
        d->setSelectThumbnailMode->setDefaultAction(d->actionSetSelectThumbnailMode);

        d->buttonStickyMode = new QToolButton(d->controlWidget);
        controlWidgetHBoxLayout->addWidget(d->buttonStickyMode);
        d->buttonStickyMode->setDefaultAction(d->actionStickyMode);

        d->hBoxForAdditionalControlWidgetItems = new QWidget(d->controlWidget);
        QHBoxLayout *hBoxForAdditionalControlWidgetItemsHBoxLayout = new QHBoxLayout(d->hBoxForAdditionalControlWidgetItems);
        hBoxForAdditionalControlWidgetItemsHBoxLayout->setContentsMargins(QMargins());
        controlWidgetHBoxLayout->addWidget(d->hBoxForAdditionalControlWidgetItems);

        d->loadTracksFromAlbums = new QToolButton(d->controlWidget);
        hBoxForAdditionalControlWidgetItemsHBoxLayout->addWidget(d->loadTracksFromAlbums);
        d->loadTracksFromAlbums->setDefaultAction(d->actionLoadTracksFromAlbums);

        setVisibleMouseModes(s->visibleMouseModes);
        setVisibleExtraActions(d->visibleExtraActions);

        // add stretch after the controls:

        QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(d->controlWidget->layout());

        if (hBoxLayout)
        {
            hBoxLayout->addStretch();
        }
    }

    // make sure the menu exists, even if no backend has been set:

    rebuildConfigurationMenu();

    return d->controlWidget;
}

void MapWidget::addWidgetToControlWidget(QWidget* const newWidget)
{
    // make sure the control widget exists

    if (!d->controlWidget)
    {
        getControlWidget();
    }

    QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(d->hBoxForAdditionalControlWidgetItems->layout());

    if (hBoxLayout)
    {
        hBoxLayout->addWidget(newWidget);
    }
}

void MapWidget::setActive(const bool state)
{
    const bool oldState = s->activeState;
    s->activeState      = state;

    if (d->currentBackend)
    {
        d->currentBackend->setActive(state);
    }

    if (s->markerModel)
    {
        s->markerModel->setActive(state);
    }

    if (state)
    {
        // do we have a map widget shown?

        if ((d->stackedLayout->count() == 1) && d->currentBackend)
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
    }

    if (state && !oldState && s->tileGrouper->getClustersDirty())
    {
        slotRequestLazyReclustering();
    }
}

bool MapWidget::getActiveState()
{
    return s->activeState;
}

void MapWidget::setVisibleMouseModes(const GeoMouseModes mouseModes)
{
    s->visibleMouseModes = mouseModes;

    if (d->mouseModesHolder)
    {
        d->mouseModesHolder->setVisible(s->visibleMouseModes);
        d->setSelectionModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeRegionSelection));
        d->removeCurrentSelectionButton->setVisible(s->visibleMouseModes.testFlag(MouseModeRegionSelection));
        d->setPanModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModePan));
        d->setZoomModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeZoomIntoGroup));
        d->setRegionSelectionFromIconModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeRegionSelectionFromIcon));
        d->setFilterModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeFilter));
        d->removeFilterModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeFilter));
        d->setSelectThumbnailMode->setVisible(s->visibleMouseModes.testFlag(MouseModeSelectThumbnail));
    }
}

void MapWidget::setAvailableMouseModes(const GeoMouseModes mouseModes)
{
    s->availableMouseModes = mouseModes;
}

bool MapWidget::getStickyModeState() const
{
    return d->actionStickyMode->isChecked();
}

void MapWidget::setStickyModeState(const bool state)
{
    d->actionStickyMode->setChecked(state);

    slotUpdateActionsEnabled();
}

void MapWidget::setVisibleExtraActions(const GeoExtraActions actions)
{
    d->visibleExtraActions = actions;

    if (d->buttonStickyMode)
    {
        d->buttonStickyMode->setVisible(actions.testFlag(ExtraActionSticky));
    }

    if (d->loadTracksFromAlbums)
    {
        d->loadTracksFromAlbums->setVisible(actions.testFlag(ExtraLoadTracks));
    }

    slotUpdateActionsEnabled();
}

void MapWidget::setEnabledExtraActions(const GeoExtraActions actions)
{
    d->availableExtraActions = actions;

    slotUpdateActionsEnabled();
}

void MapWidget::slotStickyModeChanged()
{
    slotUpdateActionsEnabled();

    Q_EMIT signalStickyModeChanged();
}

void MapWidget::setAllowModifications(const bool state)
{
    s->modificationsAllowed = state;

    slotUpdateActionsEnabled();
    slotRequestLazyReclustering();
}

void MapWidget::slotMouseModeChanged(QAction* triggeredAction)
{
    // determine the new mouse mode:

    const QVariant triggeredActionData = triggeredAction->data();
    const GeoMouseModes newMouseMode   = triggeredActionData.value<Digikam::GeoMouseModes>();

    if (newMouseMode == s->currentMouseMode)
    {
        return;
    }

    // store the new mouse mode:

    s->currentMouseMode = newMouseMode;

    if (d->currentBackend)
    {
        d->currentBackend->mouseModeChanged();
    }

    Q_EMIT signalMouseModeChanged(s->currentMouseMode);

    /// @todo Update action availability?
}

void MapWidget::setMouseMode(const GeoMouseModes mouseMode)
{
    s->currentMouseMode = mouseMode;

    if (currentBackendReady())
    {
        d->currentBackend->mouseModeChanged();
    }

    slotUpdateActionsEnabled();
}

void MapWidget::setSortOptionsMenu(QMenu* const sortMenu)
{
    d->sortMenu = sortMenu;

    rebuildConfigurationMenu();
}

void MapWidget::slotUpdateActionsEnabled()
{
    if (!s->activeState)
    {
        // this widget is not active, no need to update the action availability

        return;
    }

    d->actionDecreaseThumbnailSize->setEnabled((s->showThumbnails)&&(s->thumbnailSize>GeoIfaceMinThumbnailSize));

    /// @todo Define an upper limit for the thumbnail size!

    d->actionIncreaseThumbnailSize->setEnabled(s->showThumbnails);

    d->actionSetRegionSelectionMode->setEnabled(s->availableMouseModes.testFlag(MouseModeRegionSelection));

    d->actionSetPanMode->setEnabled(s->availableMouseModes.testFlag(MouseModePan));
    d->actionSetZoomIntoGroupMode->setEnabled(s->availableMouseModes.testFlag(MouseModeZoomIntoGroup));
    d->actionSetRegionSelectionFromIconMode->setEnabled(s->availableMouseModes.testFlag(MouseModeRegionSelectionFromIcon));
    d->actionSetFilterMode->setEnabled(s->availableMouseModes.testFlag(MouseModeFilter));
    d->actionSetSelectThumbnailMode->setEnabled(s->availableMouseModes.testFlag(MouseModeSelectThumbnail));

    // the 'Remove X' actions are only available if the corresponding X is actually there:

    bool clearRegionSelectionAvailable = s->availableMouseModes.testFlag(MouseModeRegionSelection);

    if (clearRegionSelectionAvailable && s->markerModel)
    {
        clearRegionSelectionAvailable = s->markerModel->getGlobalGroupState() & RegionSelectedMask;
    }

    d->actionRemoveCurrentRegionSelection->setEnabled(clearRegionSelectionAvailable);
    bool clearFilterAvailable = s->availableMouseModes.testFlag(MouseModeRegionSelectionFromIcon);

    if (clearFilterAvailable && s->markerModel)
    {
        clearFilterAvailable = s->markerModel->getGlobalGroupState() & FilteredPositiveMask;
    }

    d->actionRemoveFilter->setEnabled(clearFilterAvailable);

    d->actionStickyMode->setEnabled(d->availableExtraActions.testFlag(ExtraActionSticky));

    /// @todo Only set the icons if they have to be changed!

    d->actionStickyMode->setIcon(QIcon::fromTheme(QLatin1String(d->actionStickyMode->isChecked()
                                                                ? "document-encrypted"
                                                                : "document-decrypt")));
    d->actionShowThumbnails->setIcon(d->actionShowThumbnails->isChecked()
                                     ? QIcon::fromTheme(QLatin1String("folder-pictures"))
                                     : GeoIfaceGlobalObject::instance()->getMarkerPixmap(QLatin1String("marker-icon-16x16")));

    // make sure the action for the current mouse mode is checked

    const QList<QAction*> mouseModeActions = d->mouseModeActionGroup->actions();

    for (QAction* const action : std::as_const(mouseModeActions))
    {
        if (action->data().value<GeoMouseModes>() == s->currentMouseMode)
        {   // cppcheck-suppress useStlAlgorithm
            action->setChecked(true);

            break;
        }
    }
}

void MapWidget::rebuildConfigurationMenu()
{
    d->configurationMenu->clear();
    const QList<QAction*> backendSelectionActions = d->actionGroupBackendSelection->actions();

    for (int i = 0 ; i < backendSelectionActions.count() ; ++i)
    {
        QAction* const backendAction = backendSelectionActions.at(i);

        if (backendAction->data().toString() == d->currentBackendName)
        {
            backendAction->setChecked(true);
        }

        d->configurationMenu->addAction(backendAction);
    }

    if (currentBackendReady())
    {
        d->currentBackend->addActionsToConfigurationMenu(d->configurationMenu);
    }

    if (s->showThumbnails)
    {
        d->configurationMenu->addSeparator();

        if (d->sortMenu)
        {
            d->configurationMenu->addMenu(d->sortMenu);
        }

        d->configurationMenu->addAction(d->actionPreviewSingleItems);
        d->configurationMenu->addAction(d->actionPreviewGroupedItems);
        d->configurationMenu->addAction(d->actionShowNumbersOnItems);
    }

    slotUpdateActionsEnabled();
}

void MapWidget::createActions()
{
    d->actionZoomIn  = new QAction(this);
    d->actionZoomIn->setIcon(QIcon::fromTheme(QLatin1String("zoom-in")));
    d->actionZoomIn->setToolTip(i18n("Zoom in"));

    connect(d->actionZoomIn, &QAction::triggered,
            this, &MapWidget::slotZoomIn);

    d->actionZoomOut = new QAction(this);
    d->actionZoomOut->setIcon(QIcon::fromTheme(QLatin1String("zoom-out")));
    d->actionZoomOut->setToolTip(i18n("Zoom out"));

    connect(d->actionZoomOut, &QAction::triggered,
            this, &MapWidget::slotZoomOut);

    d->actionShowThumbnails = new QAction(this);
    d->actionShowThumbnails->setToolTip(i18n("Switch between markers and thumbnails."));
    d->actionShowThumbnails->setCheckable(true);
    d->actionShowThumbnails->setChecked(true);

    connect(d->actionShowThumbnails, &QAction::triggered,
            this, &MapWidget::slotShowThumbnailsChanged);

    // create backend selection entries:

    d->actionGroupBackendSelection = new QActionGroup(this);
    d->actionGroupBackendSelection->setExclusive(true);

    connect(d->actionGroupBackendSelection, &QActionGroup::triggered,
            this, &MapWidget::slotChangeBackend);

    createActionsForBackendSelection();

    d->configurationMenu         = new QMenu(this);
    d->actionPreviewSingleItems  = new QAction(i18n("Preview single items"), this);
    d->actionPreviewSingleItems->setCheckable(true);
    d->actionPreviewSingleItems->setChecked(true);
    d->actionPreviewGroupedItems = new QAction(i18n("Preview grouped items"), this);
    d->actionPreviewGroupedItems->setCheckable(true);
    d->actionPreviewGroupedItems->setChecked(true);
    d->actionShowNumbersOnItems  = new QAction(i18n("Show numbers"), this);
    d->actionShowNumbersOnItems->setCheckable(true);
    d->actionShowNumbersOnItems->setChecked(true);

    d->actionIncreaseThumbnailSize = new QAction(this);
    d->actionIncreaseThumbnailSize->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->actionIncreaseThumbnailSize->setToolTip(i18n("Increase the thumbnail size on the map"));
    d->actionDecreaseThumbnailSize = new QAction(this);
    d->actionDecreaseThumbnailSize->setIcon(QIcon::fromTheme(QLatin1String("spinbox-decrease")));
    d->actionDecreaseThumbnailSize->setToolTip(i18n("Decrease the thumbnail size on the map"));

    d->actionRemoveCurrentRegionSelection = new QAction(this);
/*
    d->actionRemoveCurrentRegionSelection->setEnabled(false);
*/
    d->actionRemoveCurrentRegionSelection->setIcon(QIcon::fromTheme(QLatin1String("edit-clear")));
    d->actionRemoveCurrentRegionSelection->setToolTip(i18n("Remove the current region selection"));

    d->mouseModeActionGroup = new QActionGroup(this);
    d->mouseModeActionGroup->setExclusive(true);

    d->actionSetRegionSelectionMode = new QAction(d->mouseModeActionGroup);
    d->actionSetRegionSelectionMode->setCheckable(true);
    d->actionSetRegionSelectionMode->setIcon(QIcon::fromTheme(QLatin1String("select-rectangular")));
    d->actionSetRegionSelectionMode->setToolTip(i18n("Select images by drawing a rectangle"));
    d->actionSetRegionSelectionMode->setData(QVariant::fromValue<Digikam::GeoMouseModes>(MouseModeRegionSelection));

    d->actionSetPanMode = new QAction(d->mouseModeActionGroup);
    d->actionSetPanMode->setCheckable(true);
    d->actionSetPanMode->setToolTip(i18n("Pan mode"));
    d->actionSetPanMode->setIcon(QIcon::fromTheme(QLatin1String("transform-move")));
    d->actionSetPanMode->setChecked(true);
    d->actionSetPanMode->setData(QVariant::fromValue<Digikam::GeoMouseModes>(MouseModePan));

    d->actionSetZoomIntoGroupMode = new QAction(d->mouseModeActionGroup);
    d->actionSetZoomIntoGroupMode->setCheckable(true);
    d->actionSetZoomIntoGroupMode->setToolTip(i18n("Zoom into a group"));
    d->actionSetZoomIntoGroupMode->setIcon(QIcon::fromTheme(QLatin1String("zoom-fit-best")));
    d->actionSetZoomIntoGroupMode->setData(QVariant::fromValue<Digikam::GeoMouseModes>(MouseModeZoomIntoGroup));

    d->actionSetRegionSelectionFromIconMode = new QAction(d->mouseModeActionGroup);
    d->actionSetRegionSelectionFromIconMode->setCheckable(true);
    d->actionSetRegionSelectionFromIconMode->setToolTip(i18n("Create a region selection from a thumbnail"));
    d->actionSetRegionSelectionFromIconMode->setIcon(QIcon::fromTheme(QLatin1String("edit-node")));
    d->actionSetRegionSelectionFromIconMode->setData(QVariant::fromValue<Digikam::GeoMouseModes>(MouseModeRegionSelectionFromIcon));

    d->actionSetFilterMode = new QAction(d->mouseModeActionGroup);
    d->actionSetFilterMode->setCheckable(true);
    d->actionSetFilterMode->setToolTip(i18n("Filter images"));
    d->actionSetFilterMode->setIcon(QIcon::fromTheme(QLatin1String("view-filter")));
    d->actionSetFilterMode->setData(QVariant::fromValue<Digikam::GeoMouseModes>(MouseModeFilter));

    d->actionRemoveFilter = new QAction(this);
    d->actionRemoveFilter->setToolTip(i18n("Remove the current filter"));
    d->actionRemoveFilter->setIcon(QIcon::fromTheme(QLatin1String("window-close")));

    d->actionSetSelectThumbnailMode = new QAction(d->mouseModeActionGroup);
    d->actionSetSelectThumbnailMode->setCheckable(true);
    d->actionSetSelectThumbnailMode->setToolTip(i18n("Select images"));
    d->actionSetSelectThumbnailMode->setIcon(QIcon::fromTheme(QLatin1String("edit-select")));
    d->actionSetSelectThumbnailMode->setData(QVariant::fromValue<Digikam::GeoMouseModes>(MouseModeSelectThumbnail));

    d->actionStickyMode = new QAction(this);
    d->actionStickyMode->setCheckable(true);
    d->actionStickyMode->setToolTip(i18n("Lock the map position"));

    d->actionLoadTracksFromAlbums = new QAction(this);
    d->actionLoadTracksFromAlbums->setToolTip(i18n("Load tracks from albums"));
    d->actionLoadTracksFromAlbums->setIcon(QIcon::fromTheme(QLatin1String("draw-polyline")));

    connect(d->actionStickyMode, &QAction::triggered,
            this, &MapWidget::slotStickyModeChanged);

    connect(d->actionIncreaseThumbnailSize, &QAction::triggered,
            this, &MapWidget::slotIncreaseThumbnailSize);

    connect(d->actionDecreaseThumbnailSize, &QAction::triggered,
            this, &MapWidget::slotDecreaseThumbnailSize);

    connect(d->actionPreviewSingleItems, &QAction::changed,
            this, &MapWidget::slotItemDisplaySettingsChanged);

    connect(d->actionPreviewGroupedItems, &QAction::changed,
            this, &MapWidget::slotItemDisplaySettingsChanged);

    connect(d->actionShowNumbersOnItems, &QAction::changed,
            this, &MapWidget::slotItemDisplaySettingsChanged);

    connect(d->mouseModeActionGroup, &QActionGroup::triggered,
            this, &MapWidget::slotMouseModeChanged);

    connect(d->actionRemoveFilter, &QAction::triggered,
            this, &MapWidget::signalRemoveCurrentFilter);

    connect(d->actionRemoveCurrentRegionSelection, &QAction::triggered,
            this, &MapWidget::slotRemoveCurrentRegionSelection);

    connect(d->actionLoadTracksFromAlbums, &QAction::triggered,
            this, &MapWidget::signalLoadTracksFromAlbums);
}

void MapWidget::createActionsForBackendSelection()
{
    // delete the existing actions:

    qDeleteAll(d->actionGroupBackendSelection->actions());

    // create actions for all backends:

    for (int i = 0 ; i < d->loadedBackends.size() ; ++i)
    {
        const QString backendName    = d->loadedBackends.at(i)->backendName();
        QAction* const backendAction = new QAction(d->actionGroupBackendSelection);
        backendAction->setData(backendName);
        backendAction->setText(d->loadedBackends.at(i)->backendHumanName());
        backendAction->setCheckable(true);
    }
}

void MapWidget::dropEvent(QDropEvent* event)
{
    // remove the drag marker:
/*
    d->currentBackend->updateDragDropMarker(QPoint(), 0);
*/
    if (!d->dragDropHandler)
    {
        event->ignore();
        return;
    }

    GeoCoordinates dropCoordinates;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (!d->currentBackend->geoCoordinates(event->position().toPoint(), &dropCoordinates))

#else

    if (!d->currentBackend->geoCoordinates(event->pos(), &dropCoordinates))

#endif

    {
        return;
    }

    // the drag and drop handler handled the drop if it returned true here

    if (d->dragDropHandler->dropEvent(event, dropCoordinates))
    {
        event->acceptProposedAction();
    }
}

void MapWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);

    // remove the marker
/*
    d->currentBackend->updateDragDropMarker(QPoint(), 0);
*/
}

void MapWidget::dragEnterEvent(QDragEnterEvent* event)
{
    /// @todo ignore drops if no marker tiler or model can accept them

    if (!d->dragDropHandler)
    {
        event->ignore();
        return;
    }

    if (d->dragDropHandler->accepts(event) == Qt::IgnoreAction)
    {
        event->ignore();
        return;
    }

    /// @todo need data about the dragged object: #markers, selected, icon, ...

    event->accept();
}

void MapWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (!d->dragDropHandler)
    {
        event->ignore();
        return;
    }

    event->accept();
}

} // namespace Digikam
