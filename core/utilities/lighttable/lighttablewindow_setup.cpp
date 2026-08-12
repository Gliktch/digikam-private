/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam light table - Configure
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lighttablewindow_p.h"

namespace Digikam
{

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    KActionCollection* const ac =   actionCollection();

    d->backwardAction = buildStdAction(StdBackAction, this, SLOT(slotBackward()), this);
    ac->addAction(QLatin1String("lighttable_backward"), d->backwardAction);
    ac->setDefaultShortcuts(d->backwardAction, QList<QKeySequence>() << Qt::Key_PageUp << Qt::Key_Backspace);

    d->forwardAction = buildStdAction(StdForwardAction, this, SLOT(slotForward()), this);
    ac->addAction(QLatin1String("lighttable_forward"), d->forwardAction);
    ac->setDefaultShortcuts(d->forwardAction, QList<QKeySequence>() << Qt::Key_PageDown << Qt::Key_Space);
    d->forwardAction->setEnabled(false);

    d->firstAction = new QAction(QIcon::fromTheme(QLatin1String("go-first")), i18nc("@action: go to first item on list", "&First"), this);
    d->firstAction->setEnabled(false);
    connect(d->firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    ac->addAction(QLatin1String("lighttable_first"), d->firstAction);
    ac->setDefaultShortcuts(d->firstAction, QList<QKeySequence>() << (Qt::CTRL | Qt::Key_Home));

    d->lastAction = new QAction(QIcon::fromTheme(QLatin1String("go-last")), i18nc("@action: go to last item on list", "&Last"), this);
    d->lastAction->setEnabled(false);
    connect(d->lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    ac->addAction(QLatin1String("lighttable_last"), d->lastAction);
    ac->setDefaultShortcuts(d->lastAction, QList<QKeySequence>() << (Qt::CTRL | Qt::Key_End));

    d->setItemLeftAction = new QAction(QIcon::fromTheme(QLatin1String("arrow-left")), i18n("On left"), this);
    d->setItemLeftAction->setEnabled(false);
    d->setItemLeftAction->setWhatsThis(i18n("Show item on left panel"));
    connect(d->setItemLeftAction, SIGNAL(triggered()), this, SLOT(slotSetItemLeft()));
    ac->addAction(QLatin1String("lighttable_setitemleft"), d->setItemLeftAction);
    ac->setDefaultShortcut(d->setItemLeftAction, Qt::CTRL | Qt::Key_L);

    d->setItemRightAction = new QAction(QIcon::fromTheme(QLatin1String("arrow-right")), i18n("On right"), this);
    d->setItemRightAction->setEnabled(false);
    d->setItemRightAction->setWhatsThis(i18n("Show item on right panel"));
    connect(d->setItemRightAction, SIGNAL(triggered()), this, SLOT(slotSetItemRight()));
    ac->addAction(QLatin1String("lighttable_setitemright"), d->setItemRightAction);
    ac->setDefaultShortcut(d->setItemRightAction, Qt::CTRL | Qt::Key_R);

    d->editItemAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Edit"), this);
    d->editItemAction->setEnabled(false);
    connect(d->editItemAction, SIGNAL(triggered()), this, SLOT(slotEditItem()));
    ac->addAction(QLatin1String("lighttable_edititem"), d->editItemAction);
    ac->setDefaultShortcut(d->editItemAction, Qt::Key_F4);

    QAction* const openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")), i18n("Open With Default Application"), this);
    openWithAction->setWhatsThis(i18n("Open the item with default assigned application."));
    connect(openWithAction, SIGNAL(triggered()), this, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), openWithAction);
    ac->setDefaultShortcut(openWithAction, Qt::CTRL | Qt::Key_F4);

    d->removeItemAction = new QAction(QIcon::fromTheme(QLatin1String("list-remove")), i18n("Remove item from Light Table"), this);
    d->removeItemAction->setEnabled(false);
    connect(d->removeItemAction, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
    ac->addAction(QLatin1String("lighttable_removeitem"), d->removeItemAction);
    ac->setDefaultShortcut(d->removeItemAction, Qt::CTRL | Qt::Key_K);

    d->clearListAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Remove all items from Light Table"), this);
    d->clearListAction->setEnabled(false);
    connect(d->clearListAction, SIGNAL(triggered()), this, SLOT(slotClearItemsList()));
    ac->addAction(QLatin1String("lighttable_clearlist"), d->clearListAction);
    ac->setDefaultShortcut(d->clearListAction, Qt::CTRL | Qt::SHIFT | Qt::Key_K);
    d->fileDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("albumfolder-user-trash")),  // In red - bug 513774
                                      i18nc("Non-pluralized", "Move to Trash"), this);
    d->fileDeleteAction->setEnabled(false);
    connect(d->fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteItem()));
    ac->addAction(QLatin1String("lighttable_filedelete"), d->fileDeleteAction);
    ac->setDefaultShortcut(d->fileDeleteAction, Qt::Key_Delete);

    d->fileDeleteFinalAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete immediately"), this);
    d->fileDeleteFinalAction->setEnabled(false);
    connect(d->fileDeleteFinalAction, SIGNAL(triggered()), this, SLOT(slotDeleteFinalItem()));
    ac->addAction(QLatin1String("lighttable_filefinaldelete"), d->fileDeleteFinalAction);
    ac->setDefaultShortcut(d->fileDeleteFinalAction, Qt::SHIFT | Qt::Key_Delete);

    QAction* const closeAction = buildStdAction(StdCloseAction, this, SLOT(slotClose()), this);
    ac->addAction(QLatin1String("lighttable_close"), closeAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->syncPreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-split-left-right")), i18n("Synchronize"), this);
    d->syncPreviewAction->setEnabled(false);
    d->syncPreviewAction->setCheckable(true);
    d->syncPreviewAction->setWhatsThis(i18n("Synchronize preview from left and right panels"));
    connect(d->syncPreviewAction, SIGNAL(triggered()), this, SLOT(slotToggleSyncPreview()));
    ac->addAction(QLatin1String("lighttable_syncpreview"), d->syncPreviewAction);
    ac->setDefaultShortcut(d->syncPreviewAction, Qt::CTRL | Qt::SHIFT | Qt::Key_Y);

    d->navigateByPairAction = new QAction(QIcon::fromTheme(QLatin1String("system-run")), i18n("By Pair"), this);
    d->navigateByPairAction->setEnabled(false);
    d->navigateByPairAction->setCheckable(true);
    d->navigateByPairAction->setWhatsThis(i18n("Navigate by pairs with all items"));
    connect(d->navigateByPairAction, SIGNAL(triggered()), this, SLOT(slotToggleNavigateByPair()));
    ac->addAction(QLatin1String("lighttable_navigatebypair"), d->navigateByPairAction);
    ac->setDefaultShortcut(d->navigateByPairAction, Qt::CTRL | Qt::SHIFT | Qt::Key_P);

    d->clearOnCloseAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear On Close"), this);
    d->clearOnCloseAction->setEnabled(true);
    d->clearOnCloseAction->setCheckable(true);
    d->clearOnCloseAction->setToolTip(i18n("Clear light table when it is closed"));
    d->clearOnCloseAction->setWhatsThis(i18n("Remove all images from the light table when it is closed"));
    ac->addAction(QLatin1String("lighttable_clearonclose"), d->clearOnCloseAction);
    ac->setDefaultShortcut(d->clearOnCloseAction, Qt::CTRL | Qt::SHIFT | Qt::Key_C);

    d->showBarAction = d->barViewDock->getToggleAction(this);
    ac->addAction(QLatin1String("lighttable_showthumbbar"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL | Qt::Key_T);

    createFullScreenAction(QLatin1String("lighttable_fullscreen"));
    createSidebarActions();

    // Left Panel Zoom Actions

    d->leftZoomPlusAction  = buildStdAction(StdZoomInAction, d->previewView, SLOT(slotIncreaseLeftZoom()), this);
    d->leftZoomPlusAction->setEnabled(false);
    ac->addAction(QLatin1String("lighttable_zoomplus_left"), d->leftZoomPlusAction);

    d->leftZoomMinusAction  = buildStdAction(StdZoomOutAction, d->previewView, SLOT(slotDecreaseLeftZoom()), this);
    d->leftZoomMinusAction->setEnabled(false);
    ac->addAction(QLatin1String("lighttable_zoomminus_left"), d->leftZoomMinusAction);

    d->leftZoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->leftZoomTo100percents, SIGNAL(triggered()), d->previewView, SLOT(slotLeftZoomTo100()));
    ac->addAction(QLatin1String("lighttable_zoomto100percents_left"), d->leftZoomTo100percents);
    ac->setDefaultShortcut(d->leftZoomTo100percents, Qt::CTRL | Qt::Key_Period);

    d->leftZoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    connect(d->leftZoomFitToWindowAction, SIGNAL(triggered()), d->previewView, SLOT(slotLeftFitToWindow()));
    ac->addAction(QLatin1String("lighttable_zoomfit2window_left"), d->leftZoomFitToWindowAction);
    ac->setDefaultShortcut(d->leftZoomFitToWindowAction, Qt::CTRL | Qt::ALT | Qt::Key_E);

    // Right Panel Zoom Actions

    d->rightZoomPlusAction  = buildStdAction(StdZoomInAction, d->previewView, SLOT(slotIncreaseRightZoom()), this);
    d->rightZoomPlusAction->setEnabled(false);
    ac->addAction(QLatin1String("lighttable_zoomplus_right"), d->rightZoomPlusAction);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    ac->setDefaultShortcut(d->rightZoomPlusAction, QKeySequence(QKeyCombination(Qt::ShiftModifier, d->rightZoomPlusAction->shortcut()[0].key())));

#else

    ac->setDefaultShortcut(d->rightZoomPlusAction, Qt::Key(d->rightZoomPlusAction->shortcut()[0] ^ Qt::SHIFT) & d->rightZoomPlusAction->shortcut()[0]);

#endif

    d->rightZoomMinusAction  = buildStdAction(StdZoomOutAction, d->previewView, SLOT(slotDecreaseRightZoom()), this);
    d->rightZoomMinusAction->setEnabled(false);
    ac->addAction(QLatin1String("lighttable_zoomminus_right"), d->rightZoomMinusAction);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    ac->setDefaultShortcut(d->rightZoomMinusAction, QKeySequence(QKeyCombination(Qt::ShiftModifier, d->rightZoomMinusAction->shortcut()[0].key())));

#else

    ac->setDefaultShortcut(d->rightZoomMinusAction, Qt::Key(d->rightZoomMinusAction->shortcut()[0] ^ Qt::SHIFT) &  d->rightZoomMinusAction->shortcut()[0]);

#endif

    d->rightZoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->rightZoomTo100percents, SIGNAL(triggered()), d->previewView, SLOT(slotRightZoomTo100()));
    ac->addAction(QLatin1String("lighttable_zoomto100percents_right"), d->rightZoomTo100percents);
    ac->setDefaultShortcut(d->rightZoomTo100percents, Qt::CTRL | Qt::SHIFT | Qt::Key_Period);

    d->rightZoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    connect(d->rightZoomFitToWindowAction, SIGNAL(triggered()), d->previewView, SLOT(slotRightFitToWindow()));
    ac->addAction(QLatin1String("lighttable_zoomfit2window_right"), d->rightZoomFitToWindowAction);
    ac->setDefaultShortcut(d->rightZoomFitToWindowAction, Qt::CTRL | Qt::SHIFT | Qt::Key_E);

    // -----------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // -----------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // Standard 'Help' menu actions

    createHelpActions(QLatin1String("light_table"));

    // Provides a menu entry that allows showing/hiding the toolbar(s)

    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar

    createStandardStatusBarAction();

    // Standard 'Configure' menu actions

    createSettingsActions();

    // -- Keyboard-only actions ----------------------------------------------------

    d->addPageUpDownActions(this, this);

    QAction* const altBackwardAction = new QAction(i18n("Previous Image"), this);
    ac->addAction(QLatin1String("lighttable_backward_shift_space"), altBackwardAction);
    ac->setDefaultShortcut(altBackwardAction, Qt::SHIFT | Qt::Key_Space);
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.

    TagsActionMngr::defaultManager()->registerLabelsActions(ac);

    QAction* const editTitlesRight = new QAction(i18n("Edit Titles on the Right"), this);
    ac->addAction(QLatin1String("edit_titles_right"), editTitlesRight);
    ac->setDefaultShortcut(editTitlesRight, Qt::ALT | Qt::SHIFT | Qt::Key_T);
    connect(editTitlesRight, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateTitles()));

    QAction* const editCommentsRight = new QAction(i18n("Edit Comments on the Right"), this);
    ac->addAction(QLatin1String("edit_comments_right"), editCommentsRight);
    ac->setDefaultShortcut(editCommentsRight, Qt::ALT | Qt::SHIFT | Qt::Key_C);
    connect(editCommentsRight, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateComments()));

    QAction* const assignedTagsRight = new QAction(i18n("Show Assigned Tags on the Right"), this);
    ac->addAction(QLatin1String("assigned_tags_right"), assignedTagsRight);
    ac->setDefaultShortcut(assignedTagsRight, Qt::ALT | Qt::SHIFT | Qt::Key_A);
    connect(assignedTagsRight, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateAssignedTags()));

    QAction* const editTitlesLeft = new QAction(i18n("Edit Titles on the Left"), this);
    ac->addAction(QLatin1String("edit_titles_left"), editTitlesLeft);
    ac->setDefaultShortcut(editTitlesLeft, Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_T);
    connect(editTitlesLeft, SIGNAL(triggered()), this, SLOT(slotLeftSideBarActivateTitles()));

    QAction* const editCommentsLeft = new QAction(i18n("Edit Comments on the Left"), this);
    ac->addAction(QLatin1String("edit_comments_left"), editCommentsLeft);
    ac->setDefaultShortcut(editCommentsLeft, Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_C);
    connect(editCommentsLeft, SIGNAL(triggered()), this, SLOT(slotLeftSideBarActivateComments()));

    QAction* const assignedTagsLeft = new QAction(i18n("Show Assigned Tags on the Left"), this);
    ac->addAction(QLatin1String("assigned_tags_left"), assignedTagsLeft);
    ac->setDefaultShortcut(assignedTagsLeft, Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_A);
    connect(assignedTagsLeft, SIGNAL(triggered()), this, SLOT(slotLeftSideBarActivateAssignedTags()));

    // ---------------------------------------------------------------------------------

    createGUI(xmlFile());
    registerPluginsActions();

    cleanupActions();

    HostActionsMap actions;
    actions.insert(QLatin1String("FullScreen"),   actionCollection()->action(QLatin1String("lighttable_fullscreen")));
    actions.insert(QLatin1String("ShowMenu"),     actionCollection()->action(QLatin1String("options_show_menubar")));
    actions.insert(QLatin1String("ColorManaged"), actionCollection()->action(QLatin1String("color_managed_view")));
    d->previewView->setHostWindowActions(actions);
}

void LightTableWindow::setupStatusBar()
{
    d->leftZoomBar = new DZoomBar(statusBar());
    d->leftZoomBar->setZoomToFitAction(d->leftZoomFitToWindowAction);
    d->leftZoomBar->setZoomTo100Action(d->leftZoomTo100percents);
    d->leftZoomBar->setZoomPlusAction(d->leftZoomPlusAction);
    d->leftZoomBar->setZoomMinusAction(d->leftZoomMinusAction);
    d->leftZoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->leftZoomBar->setEnabled(false);
    statusBar()->addWidget(d->leftZoomBar, 1);

    d->leftFileName = new StatusProgressBar(statusBar());
    d->leftFileName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(d->leftFileName, 10);

    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(d->statusProgressBar, 10);

    d->rightFileName = new StatusProgressBar(statusBar());
    d->rightFileName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusBar()->addWidget(d->rightFileName, 10);

    d->rightZoomBar = new DZoomBar(statusBar());
    d->rightZoomBar->setZoomToFitAction(d->rightZoomFitToWindowAction);
    d->rightZoomBar->setZoomTo100Action(d->rightZoomTo100percents);
    d->rightZoomBar->setZoomPlusAction(d->rightZoomPlusAction);
    d->rightZoomBar->setZoomMinusAction(d->rightZoomMinusAction);
    d->rightZoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->rightZoomBar->setEnabled(false);
    statusBar()->addWidget(d->rightZoomBar, 1);
}

void LightTableWindow::setupConnections()
{
    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(IccSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    // Thumbs bar connections ---------------------------------------

    connect(d->thumbView, SIGNAL(signalSetItemOnLeftPanel(ItemInfo)),
            this, SLOT(slotSetItemOnLeftPanel(ItemInfo)));

    connect(d->thumbView, SIGNAL(signalSetItemOnRightPanel(ItemInfo)),
            this, SLOT(slotSetItemOnRightPanel(ItemInfo)));

    connect(d->thumbView, SIGNAL(signalRemoveItem(ItemInfo)),
            this, SLOT(slotRemoveItem(ItemInfo)));

    connect(d->thumbView, SIGNAL(signalEditItem(ItemInfo)),
            this, SLOT(slotEditItem(ItemInfo)));

    connect(d->thumbView, SIGNAL(signalClearAll()),
            this, SLOT(slotClearItemsList()));

    connect(d->thumbView, SIGNAL(signalDroppedItems(QList<ItemInfo>)),
            this, SLOT(slotThumbbarDroppedItems(QList<ItemInfo>)));

    connect(d->thumbView, SIGNAL(currentInfoChanged(ItemInfo)),
            this, SLOT(slotItemSelected(ItemInfo)));

    connect(d->thumbView, SIGNAL(signalContentChanged()),
            this, SLOT(slotRefreshStatusBar()));

    // Zoom bars connections -----------------------------------------

    connect(d->leftZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotLeftZoomSliderChanged(int)));

    connect(d->leftZoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->previewView, SLOT(setLeftZoomFactor(double)));

    connect(d->rightZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotRightZoomSliderChanged(int)));

    connect(d->rightZoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->previewView, SLOT(setRightZoomFactor(double)));

    // View connections ---------------------------------------------

    connect(d->previewView, SIGNAL(signalLeftPreviewSelected(bool)),
            this, SLOT(slotLeftPreviewSelected(bool)));

    connect(d->previewView, SIGNAL(signalRightPreviewSelected(bool)),
            this, SLOT(slotRightPreviewSelected(bool)));

    connect(d->previewView, SIGNAL(signalLeftPopupTagsView()),
            d->leftSideBar, SLOT(slotPopupTagsView()));

    connect(d->previewView, SIGNAL(signalRightPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    connect(d->previewView, SIGNAL(signalLeftZoomFactorChanged(double)),
            this, SLOT(slotLeftZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalRightZoomFactorChanged(double)),
            this, SLOT(slotRightZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalEditItem(ItemInfo)),
            this, SLOT(slotEditItem(ItemInfo)));

    connect(d->previewView, SIGNAL(signalDeleteItem(ItemInfo)),
            this, SLOT(slotDeleteItem(ItemInfo)));

    connect(d->previewView, SIGNAL(signalLeftSlideShowCurrent()),
            this, SLOT(slotLeftSlideShowManualFromCurrent()));

    connect(d->previewView, SIGNAL(signalRightSlideShowCurrent()),
            this, SLOT(slotRightSlideShowManualFromCurrent()));

    connect(d->previewView, SIGNAL(signalLeftDroppedItems(ItemInfoList)),
            this, SLOT(slotLeftDroppedItems(ItemInfoList)));

    connect(d->previewView, SIGNAL(signalRightDroppedItems(ItemInfoList)),
            this, SLOT(slotRightDroppedItems(ItemInfoList)));

    connect(d->previewView, SIGNAL(signalToggleOnSyncPreview(bool)),
            this, SLOT(slotToggleOnSyncPreview(bool)));

    connect(d->previewView, SIGNAL(signalLeftPreviewLoaded(bool)),
            this, SLOT(slotLeftPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalRightPreviewLoaded(bool)),
            this, SLOT(slotRightPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalLeftPanelLeftButtonClicked()),
            this, SLOT(slotLeftPanelLeftButtonClicked()));

    connect(d->previewView, SIGNAL(signalRightPanelLeftButtonClicked()),
            this, SLOT(slotRightPanelLeftButtonClicked()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->leftZoomBar, SLOT(slotUpdateTrackerPos()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->rightZoomBar, SLOT(slotUpdateTrackerPos()));

    // ---

    connect(d->previewView, &LightTableView::signalLeftPreviewStartedLoading,
            this, [this]()
       {
            d->leftFileName->setProgressBarMode(StatusProgressBar::ProgressBarMode, i18nc("@label", "Loading:"));
       }
    );

    connect(d->previewView, &LightTableView::signalLeftPreviewLoadingProgress,
            this, [this](float progress)
       {
            d->leftFileName->setProgressValue((int)(progress * 100.0));
       }
    );

    connect(d->previewView, &LightTableView::signalLeftPreviewLoadingComplete,
            this, [this]()
       {
            d->leftFileName->setProgressBarMode(StatusProgressBar::TextMode, d->leftFileName->text());
       }
    );

    // ---

    connect(d->previewView, &LightTableView::signalRightPreviewStartedLoading,
            this, [this]()
       {
            d->rightFileName->setProgressBarMode(StatusProgressBar::ProgressBarMode, i18nc("@label", "Loading:"));
       }
    );

    connect(d->previewView, &LightTableView::signalRightPreviewLoadingProgress,
            this, [this](float progress)
       {
            d->rightFileName->setProgressValue((int)(progress * 100.0));
       }
    );

    connect(d->previewView, &LightTableView::signalRightPreviewLoadingComplete,
            this, [this]()
       {
            d->rightFileName->setProgressBarMode(StatusProgressBar::TextMode, d->rightFileName->text());
       }
    );

    // -- FileWatch connections ------------------------------

    LoadingCacheInterface::connectToSignalFileChanged(this, SLOT(slotFileChanged(QString)));
}

void LightTableWindow::setupUserArea()
{
    QWidget* const mainW    = new QWidget(this);
    d->hSplitter            = new SidebarSplitter(Qt::Horizontal, mainW);
    QHBoxLayout* const hlay = new QHBoxLayout(mainW);

    // The left sidebar

    d->leftSideBar          = new ItemPropertiesSideBarDB(mainW, d->hSplitter, Qt::LeftEdge, true);

    // The central preview is wrapped in a KMainWindow so that the thumbnail
    // bar can float around it.

    d->dockArea             = new QMainWindow(mainW, Qt::Widget);

    d->hSplitter->addWidget(d->dockArea);
    d->hSplitter->setStretchFactor(d->hSplitter->indexOf(d->dockArea), 10);
    d->previewView          = new LightTableView(d->dockArea);
    d->dockArea->setCentralWidget(d->previewView);

    // The right sidebar.

    d->rightSideBar         = new ItemPropertiesSideBarDB(mainW, d->hSplitter, Qt::RightEdge, true);

    hlay->addWidget(d->leftSideBar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setSpacing(0);
    hlay->setContentsMargins(QMargins());
    hlay->setStretchFactor(d->hSplitter, 10);

    d->hSplitter->setFrameStyle(QFrame::NoFrame);
    d->hSplitter->setFrameShadow(QFrame::Plain);
    d->hSplitter->setFrameShape(QFrame::NoFrame);
    d->hSplitter->setOpaqueResize(false);
    d->hSplitter->setStretchFactor(1, 10);      // set previewview+thumbbar container default size to max.

    // The thumb bar is placed in a detachable/dockable widget.

    d->barViewDock = new ThumbBarDock(d->dockArea, Qt::Tool);
    d->barViewDock->setObjectName(QLatin1String("lighttable_thumbbar"));

    d->thumbView   = new LightTableThumbBar(d->barViewDock);

    d->barViewDock->setWidget(d->thumbView);
    d->dockArea->addDockWidget(Qt::TopDockWidgetArea, d->barViewDock);
    d->barViewDock->setFloating(false);

    connect(d->barViewDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbView, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    d->barViewDock->reInitialize();

    d->leftSideBar->setItemFilterModel(d->thumbView->itemFilterModel());
    d->rightSideBar->setItemFilterModel(d->thumbView->itemFilterModel());

    setCentralWidget(mainW);
}

} // namespace Digikam
