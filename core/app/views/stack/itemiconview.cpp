/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemiconview_p.h"

namespace Digikam
{

ItemIconView::ItemIconView(QWidget* const parent, DModelFactory* const modelCollection)
    : DHBox(parent),
      d    (new Private)
{
    d->parent                   = static_cast<DigikamApp*>(parent);
    d->modelCollection          = modelCollection;
    d->albumManager             = AlbumManager::instance();

    d->albumModificationHelper  = new AlbumModificationHelper(this, this);
    d->tagModificationHelper    = new TagModificationHelper(this, this);
    d->searchModificationHelper = new SearchModificationHelper(this, this);

    const int spacing           = layoutSpacing();

    d->splitter    = new SidebarSplitter;
    d->splitter->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    d->splitter->setOpaqueResize(false);

    d->leftSideBar = new Sidebar(this, d->splitter, Qt::LeftEdge);
    d->leftSideBar->setObjectName(QLatin1String("Digikam Left Sidebar"));
    d->leftSideBar->setContentsMargins(0, 0, spacing, 0);

    d->splitter->setParent(this);

    // The dock area where the thumbnail bar is allowed to go.

    d->dockArea    = new QMainWindow(this, Qt::Widget);
    d->dockArea->setContentsMargins(QMargins());
    d->splitter->addWidget(d->dockArea);
    d->splitter->setStretchFactor(d->splitter->indexOf(d->dockArea), 10);

    DVBox* const vbox = new DVBox(d->dockArea);
    d->errorWidget    = new DNotificationWidget(vbox);
    d->errorWidget->setCloseButtonVisible(true);
    d->errorWidget->setWordWrap(true);
    d->errorWidget->hide();

    connect(d->leftSideBar, SIGNAL(signalChangedTab(QWidget*)),
            d->errorWidget, SLOT(animatedHide()));

    d->stackedView = new StackedView(vbox);

    d->dockArea->setCentralWidget(vbox);
    d->stackedView->setDockArea(d->dockArea);

    d->iconView  = d->stackedView->imageIconView();

#ifdef HAVE_GEOLOCATION

    d->mapView   = d->stackedView->mapWidgetView();

#endif // HAVE_GEOLOCATION

    d->tableView = d->stackedView->tableView();
    d->trashView = d->stackedView->trashView();

    d->utilities = d->iconView->utilities();

    d->addPageUpDownActions(this, d->stackedView->imagePreviewView());
    d->addPageUpDownActions(this, d->stackedView->thumbBar());

#ifdef HAVE_MEDIAPLAYER

    d->addPageUpDownActions(this, d->stackedView->mediaPlayerView());

#endif // HAVE_MEDIAPLAYER

    d->rightSideBar        = new ItemPropertiesSideBarDB(this, d->splitter, Qt::RightEdge, true);
    d->rightSideBar->setObjectName(QLatin1String("Digikam Right Sidebar"));
    d->rightSideBar->setItemFilterModel(d->iconView->itemFilterModel());
    d->rightSideBar->setShowAllPropertiesMode(true);

    connect(d->stackedView, SIGNAL(signalOpenGeolocationMap()),
            d->rightSideBar, SLOT(slotOpenGeolocationMap()));

    // album folder view

    d->albumFolderSideBar  = new AlbumFolderViewSideBarWidget(d->leftSideBar,
                                                              d->modelCollection->getAlbumModel(),
                                                              d->albumModificationHelper);
    d->leftSideBarWidgets << d->albumFolderSideBar;

    connect(d->albumFolderSideBar, SIGNAL(signalFindDuplicates(QList<PAlbum*>)),
            this, SLOT(slotNewDuplicatesSearch(QList<PAlbum*>)));

    // Tags sidebar tab contents.

    d->tagViewSideBar      = new TagViewSideBarWidget(d->leftSideBar, d->modelCollection->getTagModel());
    d->leftSideBarWidgets << d->tagViewSideBar;

    connect(d->tagViewSideBar, SIGNAL(signalFindDuplicates(QList<TAlbum*>)),
            this, SLOT(slotNewDuplicatesSearch(QList<TAlbum*>)));

    // Labels sidebar

    d->labelsSideBar       = new LabelsSideBarWidget(d->leftSideBar);
    d->leftSideBarWidgets << d->labelsSideBar;
    d->labelsSearchHandler = new AlbumLabelsSearchHandler(d->labelsSideBar->labelsTree());

    // date view

    d->dateViewSideBar     = new DateFolderViewSideBarWidget(d->leftSideBar,
                                                             d->modelCollection->getDateAlbumModel(),
                                                             d->iconView->itemAlbumFilterModel());
    d->leftSideBarWidgets << d->dateViewSideBar;

    // timeline side bar

    d->timelineSideBar     = new TimelineSideBarWidget(d->leftSideBar,
                                                       d->modelCollection->getSearchModel(),
                                                       d->searchModificationHelper);
    d->leftSideBarWidgets << d->timelineSideBar;

    // Search sidebar tab contents.

    d->searchSideBar       = new SearchSideBarWidget(d->leftSideBar,
                                                     d->modelCollection->getSearchModel(),
                                                     d->searchModificationHelper);
    d->leftSideBarWidgets << d->searchSideBar;

    // Fuzzy search

    d->fuzzySearchSideBar  = new FuzzySearchSideBarWidget(d->leftSideBar,
                                                          d->modelCollection->getSearchModel(),
                                                          d->searchModificationHelper);
    d->leftSideBarWidgets << d->fuzzySearchSideBar;

    connect(d->fuzzySearchSideBar,SIGNAL(signalActive(bool)),
            this, SIGNAL(signalFuzzySidebarActive(bool)));

    connect(d->fuzzySearchSideBar, SIGNAL(signalNotificationError(QString,int)),
            this, SLOT(slotNotificationError(QString,int)));

#ifdef HAVE_GEOLOCATION

    d->gpsSearchSideBar    = new GPSSearchSideBarWidget(d->leftSideBar,
                                                        d->modelCollection->getSearchModel(),
                                                        d->searchModificationHelper,
                                                        d->iconView->itemFilterModel(),
                                                        d->iconView->getSelectionModel());

    d->leftSideBarWidgets << d->gpsSearchSideBar;

#endif // HAVE_GEOLOCATION

    // People Sidebar

    d->peopleSideBar       = new PeopleSideBarWidget(d->leftSideBar,
                                                     d->modelCollection->getTagFaceModel(),
                                                     d->searchModificationHelper);

    connect(d->peopleSideBar, SIGNAL(signalFindDuplicates(QList<TAlbum*>)),
            this, SLOT(slotNewDuplicatesSearch(QList<TAlbum*>)));

    connect(d->peopleSideBar, SIGNAL(signalNotificationError(QString,int)),
            this, SLOT(slotNotificationError(QString,int)));

    d->leftSideBarWidgets << d->peopleSideBar;

    for (SidebarWidget* const leftWidget : std::as_const(d->leftSideBarWidgets))
    {
        d->leftSideBar->appendTab(leftWidget, leftWidget->getIcon(), leftWidget->getCaption());

        connect(leftWidget, SIGNAL(requestActiveTab(SidebarWidget*)),
                this, SLOT(slotLeftSideBarActivate(SidebarWidget*)));
    }

    // add only page up and down to work correctly with QCompleter

    defineShortcut(d->rightSideBar->imageDescEditTab(), Qt::Key_PageDown, this, SLOT(slotNextItem()));
    defineShortcut(d->rightSideBar->imageDescEditTab(), Qt::Key_PageUp,   this, SLOT(slotPrevItem()));

    // Tags Filter sidebar tab contents.

    d->filterWidget   = new FilterSideBarWidget(d->rightSideBar, d->modelCollection->getTagFilterModel());
    d->rightSideBar->appendTab(d->filterWidget, QIcon::fromTheme(QLatin1String("view-filter")),
                               i18nc("Filters as in Search type Filters", "Filters"));

    // Versions sidebar overlays
    d->rightSideBar->getFiltersHistoryTab()->addOpenAlbumAction(d->iconView->itemModel());
    d->rightSideBar->getFiltersHistoryTab()->addShowHideOverlay();

    d->selectionTimer = new QTimer(this);
    d->selectionTimer->setSingleShot(true);
    d->selectionTimer->setInterval(75);

    d->thumbSizeTimer = new QTimer(this);
    d->thumbSizeTimer->setSingleShot(true);
    d->thumbSizeTimer->setInterval(300);

    d->msgNotifyTimer = new QTimer(this);
    d->msgNotifyTimer->setSingleShot(true);
    d->msgNotifyTimer->setInterval(250);

    d->albumHistory = new AlbumHistory();

    slotSidebarTabTitleStyleChanged();
    setupConnections();

    connect(d->rightSideBar->imageDescEditTab()->getNewTagEdit(), SIGNAL(taggingActionFinished()),
            this, SLOT(slotFocusAndNextImage()));

    connect(d->rightSideBar, SIGNAL(signalSetupMetadataFilters(int)),
            this, SLOT(slotSetupMetadataFilters(int)));

    connect(d->rightSideBar, SIGNAL(signalSetupExifTool()),
            this, SLOT(slotSetupExifTool()));

    connect(d->rightSideBar, SIGNAL(signalRightSideBarBusy(bool)),
            this, SLOT(slotLeftSideBarEnabled(bool)));

    connect(d->iconView, SIGNAL(signalSeparationModeChanged(int)),
            this, SIGNAL(signalSeparationModeChanged(int)));

    connect(d->stackedView->imagePreviewView(), SIGNAL(signalStartedLoading()),
            this, SIGNAL(signalStartedLoading()));

    connect(d->stackedView->imagePreviewView(), SIGNAL(signalLoadingProgress(float)),
            this, SIGNAL(signalLoadingProgress(float)));

    connect(d->stackedView->imagePreviewView(), SIGNAL(signalLoadingComplete()),
            this, SIGNAL(signalLoadingComplete()));
}

ItemIconView::~ItemIconView()
{
    saveViewState();

    delete d->labelsSearchHandler;
    delete d->albumHistory;
    delete d;
}

void ItemIconView::applySettings()
{
    for (SidebarWidget* const sidebarWidget : std::as_const(d->leftSideBarWidgets))
    {
        sidebarWidget->applySettings();
    }

    d->iconView->itemFilterModel()->setVersionItemFilterSettings(
        VersionItemFilterSettings(
            ApplicationSettings::instance()->getVersionManagerSettings()));

    refreshView();
}

void ItemIconView::setupConnections()
{
    // -- DigikamApp connections ----------------------------------

    connect(d->parent, SIGNAL(signalEscapePressed()),
            this, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->parent, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->parent, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->parent, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(d->parent, SIGNAL(signalCutAlbumItemsSelection()),
            d->iconView, SLOT(cut()));

    connect(d->parent, SIGNAL(signalCopyAlbumItemsSelection()),
            d->iconView, SLOT(copy()));

    connect(d->parent, SIGNAL(signalPasteAlbumItemsSelection()),
            this, SLOT(slotImagePaste()));

    connect(d->parent, SIGNAL(signalNotificationError(QString,int)),
            this, SLOT(slotNotificationError(QString,int)));

    // -- AlbumManager connections --------------------------------

    connect(d->albumManager, SIGNAL(signalAlbumCurrentChanged(QList<Album*>)),
            this, SLOT(slotAlbumSelected(QList<Album*>)));

    connect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->albumManager, SIGNAL(signalEmptyTrash()),
            d->trashView, SIGNAL(signalEmptytrash()));

    // -- IconView Connections -------------------------------------

    connect(d->iconView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(layoutChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->itemModel(), SIGNAL(allRefreshingFinished()),
            d->msgNotifyTimer, SLOT(start()));

    connect(d->iconView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(previewRequested(ItemInfo)),
            this, SLOT(slotTogglePreviewMode(ItemInfo)));

    connect(d->iconView, SIGNAL(zoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->iconView, SIGNAL(zoomInStep()),
            this, SLOT(slotZoomIn()));

    connect(d->iconView, SIGNAL(signalShowContextMenu(QContextMenuEvent*,QList<QAction*>)),
            this, SLOT(slotShowContextMenu(QContextMenuEvent*,QList<QAction*>)));

    connect(d->iconView, SIGNAL(signalShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)),
            this, SLOT(slotShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)));

    connect(d->iconView, SIGNAL(signalShowGroupContextMenu(QContextMenuEvent*,QList<ItemInfo>,ItemFilterModel*)),
            this, SLOT(slotShowGroupContextMenu(QContextMenuEvent*,QList<ItemInfo>,ItemFilterModel*)));

    // -- TableView Connections -----------------------------------

    connect(d->tableView, SIGNAL(signalPreviewRequested(ItemInfo)),
            this, SLOT(slotTogglePreviewMode(ItemInfo)));

    connect(d->tableView, SIGNAL(signalZoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->tableView, SIGNAL(signalZoomInStep()),
            this, SLOT(slotZoomIn()));

    connect(d->tableView, SIGNAL(signalShowContextMenu(QContextMenuEvent*,QList<QAction*>)),
            this, SLOT(slotShowContextMenu(QContextMenuEvent*,QList<QAction*>)));

    connect(d->tableView, SIGNAL(signalShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)),
            this, SLOT(slotShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)));

    // TableView::signalItemsChanged is emitted when something changes in the model that
    // ItemIconView should care about, not only the selection.

    connect(d->tableView, SIGNAL(signalItemsChanged()),
            this, SLOT(slotImageSelected()));

    // -- Trash View Connections ----------------------------------

    connect(d->trashView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotImageSelected()));

    // -- Sidebar Connections -------------------------------------

    connect(d->leftSideBar, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotLeftSidebarChangedTab(QWidget*)));

    connect(d->rightSideBar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->rightSideBar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->rightSideBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->rightSideBar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

#ifdef HAVE_GEOLOCATION

    connect(d->gpsSearchSideBar, SIGNAL(signalMapSoloItems(QList<qlonglong>,QString)),
            d->iconView->itemFilterModel(), SLOT(setIdWhitelist(QList<qlonglong>,QString)));

#endif // HAVE_GEOLOCATION

    // -- Filter Bars Connections ---------------------------------

    ItemAlbumFilterModel* const model = d->iconView->itemAlbumFilterModel();

    connect(d->filterWidget,
            SIGNAL(signalTagFilterChanged(QList<int>,QList<int>,ItemFilterSettings::MatchingCondition,bool,QList<int>,QList<int>)),
            d->iconView->itemFilterModel(), SLOT(setTagFilter(QList<int>,QList<int>,ItemFilterSettings::MatchingCondition,bool,QList<int>,QList<int>)));

    connect(d->filterWidget, SIGNAL(signalRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)),
            model, SLOT(setRatingFilter(int,ItemFilterSettings::RatingCondition,bool)));

    connect(d->filterWidget, SIGNAL(signalSearchTextFilterChanged(SearchTextFilterSettings)),
            model, SLOT(setTextFilter(SearchTextFilterSettings)));

    connect(model, SIGNAL(filterMatchesForText(bool)),
            d->filterWidget, SLOT(slotFilterMatchesForText(bool)));

    connect(d->filterWidget, SIGNAL(signalMimeTypeFilterChanged(int)),
            model, SLOT(setMimeTypeFilter(int)));

    connect(d->filterWidget, SIGNAL(signalGeolocationFilterChanged(ItemFilterSettings::GeolocationCondition)),
            model, SLOT(setGeolocationFilter(ItemFilterSettings::GeolocationCondition)));

    // -- Preview image widget Connections ------------------------

    connect(d->stackedView, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->stackedView, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->stackedView, SIGNAL(signalDeleteItem()),
            this, SLOT(slotImageDelete()));

    connect(d->stackedView, SIGNAL(signalViewModeChanged()),
            this, SLOT(slotViewModeChanged()));

    connect(d->stackedView, SIGNAL(signalEscapePreview()),
            this, SLOT(slotEscapePreview()));

    connect(d->stackedView, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->stackedView, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotImageAddToExistingQueue(int)));

    connect(d->stackedView, SIGNAL(signalGotoAlbumAndItem(ItemInfo)),
            this, SLOT(slotGotoAlbumAndItem(ItemInfo)));

    connect(d->stackedView, SIGNAL(signalGotoDateAndItem(ItemInfo)),
            this, SLOT(slotGotoDateAndItem(ItemInfo)));

    connect(d->stackedView, SIGNAL(signalGotoTagAndItem(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    connect(d->stackedView, SIGNAL(signalPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    // -- FileActionMngr progress ---------------

    connect(FileActionMngr::instance(), SIGNAL(signalImageChangeFailed(QString,QStringList)),
            this, SLOT(slotImageChangeFailed(QString,QStringList)));

    // -- timers ---------------

    connect(d->selectionTimer, SIGNAL(timeout()),
            this, SLOT(slotDispatchImageSelected()));

    connect(d->thumbSizeTimer, SIGNAL(timeout()),
            this, SLOT(slotThumbSizeEffect()));

    connect(d->msgNotifyTimer, SIGNAL(timeout()),
            this, SLOT(slotEmptyMessageTimer()));

    // -- Album Settings ----------------

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));

    // -- Album History -----------------

    connect(this, SIGNAL(signalAlbumSelected(Album*)),
            d->albumHistory, SLOT(slotAlbumSelected()));

    connect(this, SIGNAL(signalImageSelected(ItemInfoList,ItemInfoList)),
            d->albumHistory, SLOT(slotImageSelected(ItemInfoList)));

    connect(d->iconView, SIGNAL(currentInfoChanged(ItemInfo)),
            d->albumHistory, SLOT(slotCurrentChange(ItemInfo)));

    connect(d->iconView->itemModel(), SIGNAL(imageInfosAdded(QList<ItemInfo>)),
            d->albumHistory, SLOT(slotAlbumCurrentChanged()));

    connect(d->albumHistory, SIGNAL(signalSetCurrent(qlonglong)),
            this, SLOT(slotSetCurrentWhenAvailable(qlonglong)));

    connect(d->albumHistory, SIGNAL(signalSetSelectedInfos(QList<ItemInfo>)),
            d->iconView, SLOT(setSelectedItemInfos(QList<ItemInfo>)));

    connect(d->albumManager, SIGNAL(signalAlbumDeleted(Album*)),
            d->albumHistory, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            d->albumHistory, SLOT(slotAlbumsCleared()));

    // -- Image versions ----------------

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(imageSelected(ItemInfo)),
            d->iconView, SLOT(hintAt(ItemInfo)));

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(actionTriggered(ItemInfo)),
            this, SLOT(slotGotoAlbumAndItem(ItemInfo)));

    // -- Survey view -------------------

    const SurveyWindow* const svw = SurveyWindow::surveyWindow();

    connect(svw, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(svw, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(svw, SIGNAL(signalAssignRating(int,bool)),
            this, SLOT(slotAssignRating(int,bool)));
}

void ItemIconView::loadViewState()
{
    for (SidebarWidget* const widget : std::as_const(d->leftSideBarWidgets))
    {
        widget->loadState();
    }

    d->filterWidget->loadState();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("MainWindow"));

    // Restore the splitter

    d->splitter->restoreState(group);

    // Restore the thumbnail bar dock.

    QByteArray thumbbarState;
    thumbbarState     = group.readEntry(QLatin1String("ThumbbarState"), thumbbarState);
    d->dockArea->restoreState(QByteArray::fromBase64(thumbbarState));

    d->initialAlbumID = group.readEntry(QLatin1String("InitialAlbumID"), 0);

#ifdef HAVE_GEOLOCATION

    d->mapView->loadState();

#endif // HAVE_GEOLOCATION

    d->tableView->loadState();
    d->leftSideBar->loadState();
    d->rightSideBar->loadState();
}

void ItemIconView::saveViewState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("MainWindow"));

    for (SidebarWidget* const widget : std::as_const(d->leftSideBarWidgets))
    {
        widget->saveState();
    }

    d->filterWidget->saveState();

    // Save the splitter states.

    d->splitter->saveState(group);

    // Save the position and size of the thumbnail bar. The thumbnail bar dock
    // needs to be closed explicitly, because when it is floating and visible
    // (when the user is in image preview mode) when the layout is saved, it
    // also reappears when restoring the view, while it should always be hidden.

    d->stackedView->thumbBarDock()->close();
    group.writeEntry(QLatin1String("ThumbbarState"), d->dockArea->saveState().toBase64());

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    const Album* album      = nullptr;

    if (!albumList.isEmpty())
    {
        album = albumList.first();
    }

    if (album)
    {
        group.writeEntry(QLatin1String("InitialAlbumID"), album->globalID());
    }
    else
    {
        group.writeEntry(QLatin1String("InitialAlbumID"), 0);
    }

#ifdef HAVE_GEOLOCATION

    d->mapView->saveState();

#endif // HAVE_GEOLOCATION

    d->tableView->saveState();
    d->rightSideBar->saveState();
}

} // namespace Digikam

#include "moc_itemiconview.cpp"
