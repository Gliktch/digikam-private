/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam Survey tool - Configure
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "surveywindow_p.h"

namespace Digikam
{

void SurveyWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    KActionCollection* const ac =   actionCollection();

    d->backwardAction = buildStdAction(StdBackAction, this, SLOT(slotBackward()), this);
    ac->addAction(QLatin1String("survey_backward"), d->backwardAction);
    ac->setDefaultShortcuts(d->backwardAction, QList<QKeySequence>() << Qt::Key_PageUp << Qt::Key_Backspace);

    d->forwardAction = buildStdAction(StdForwardAction, this, SLOT(slotForward()), this);
    ac->addAction(QLatin1String("survey_forward"), d->forwardAction);
    ac->setDefaultShortcuts(d->forwardAction, QList<QKeySequence>() << Qt::Key_PageDown << Qt::Key_Space);
    d->forwardAction->setEnabled(false);

    d->firstAction = new QAction(QIcon::fromTheme(QLatin1String("go-first")), i18nc("@action: go to first item on list", "&First"), this);
    d->firstAction->setEnabled(false);
    connect(d->firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    ac->addAction(QLatin1String("survey_first"), d->firstAction);
    ac->setDefaultShortcuts(d->firstAction, QList<QKeySequence>() << (Qt::CTRL | Qt::Key_Home));

    d->lastAction = new QAction(QIcon::fromTheme(QLatin1String("go-last")), i18nc("@action: go to last item on list", "&Last"), this);
    d->lastAction->setEnabled(false);
    connect(d->lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    ac->addAction(QLatin1String("survey_last"), d->lastAction);
    ac->setDefaultShortcuts(d->lastAction, QList<QKeySequence>() << (Qt::CTRL | Qt::Key_End));

    d->editItemAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Edit"), this);
    d->editItemAction->setEnabled(false);
    connect(d->editItemAction, SIGNAL(triggered()), this, SLOT(slotEditItem()));
    ac->addAction(QLatin1String("survey_edititem"), d->editItemAction);
    ac->setDefaultShortcut(d->editItemAction, Qt::Key_F4);

    QAction* const openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")), i18n("Open With Default Application"), this);
    openWithAction->setWhatsThis(i18n("Open the item with default assigned application."));
    connect(openWithAction, SIGNAL(triggered()), this, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), openWithAction);
    ac->setDefaultShortcut(openWithAction, Qt::CTRL | Qt::Key_F4);

    d->fileDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("albumfolder-user-trash")),  // In red - bug 513774
                                      i18nc("Non-pluralized", "Move to Trash"), this);
    d->fileDeleteAction->setEnabled(false);
    connect(d->fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteItem()));
    ac->addAction(QLatin1String("survey_filedelete"), d->fileDeleteAction);
    ac->setDefaultShortcut(d->fileDeleteAction, Qt::Key_Delete);

    d->fileDeleteFinalAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete immediately"), this);
    d->fileDeleteFinalAction->setEnabled(false);
    connect(d->fileDeleteFinalAction, SIGNAL(triggered()), this, SLOT(slotDeleteFinalItem()));
    ac->addAction(QLatin1String("survey_filefinaldelete"), d->fileDeleteFinalAction);
    ac->setDefaultShortcut(d->fileDeleteFinalAction, Qt::SHIFT | Qt::Key_Delete);

    QAction* const closeAction = buildStdAction(StdCloseAction, this, SLOT(slotClose()), this);
    ac->addAction(QLatin1String("survey_close"), closeAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->showBarAction = d->stack->thumbBarDock()->getToggleAction(this);
    ac->addAction(QLatin1String("survey_showthumbbar"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL | Qt::Key_T);

    createFullScreenAction(QLatin1String("survey_fullscreen"));
    createSidebarActions();

    // Panel Zoom Actions

    d->zoomPlusAction  = buildStdAction(StdZoomInAction, d->stack, SLOT(increaseZoom()), this);
    d->zoomPlusAction->setEnabled(false);
    ac->addAction(QLatin1String("survey_zoomplus"), d->zoomPlusAction);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    ac->setDefaultShortcut(d->zoomPlusAction, QKeySequence(QKeyCombination(Qt::ShiftModifier, d->zoomPlusAction->shortcut()[0].key())));

#else

    ac->setDefaultShortcut(d->zoomPlusAction, Qt::Key(d->zoomPlusAction->shortcut()[0] ^ Qt::SHIFT) & d->zoomPlusAction->shortcut()[0]);

#endif

    d->zoomMinusAction  = buildStdAction(StdZoomOutAction, d->stack, SLOT(decreaseZoom()), this);
    d->zoomMinusAction->setEnabled(false);
    ac->addAction(QLatin1String("survey_zoomminus"), d->zoomMinusAction);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    ac->setDefaultShortcut(d->zoomMinusAction, QKeySequence(QKeyCombination(Qt::ShiftModifier, d->zoomMinusAction->shortcut()[0].key())));

#else

    ac->setDefaultShortcut(d->zoomMinusAction, Qt::Key(d->zoomMinusAction->shortcut()[0] ^ Qt::SHIFT) &  d->zoomMinusAction->shortcut()[0]);

#endif

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->stack, SLOT(zoomTo100Percents()));
    ac->addAction(QLatin1String("survey_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, Qt::CTRL | Qt::SHIFT | Qt::Key_Period);

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->stack, SLOT(fitToWindow()));
    ac->addAction(QLatin1String("survey_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, Qt::CTRL | Qt::SHIFT | Qt::Key_E);

    // -----------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // -----------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // Standard 'Help' menu actions

    createHelpActions(QLatin1String("survey_tool"));

    // Provides a menu entry that allows showing/hiding the toolbar(s)

    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar

    createStandardStatusBarAction();

    // Standard 'Configure' menu actions

    createSettingsActions();

    // -- Keyboard-only actions ----------------------------------------------------

    d->addPageUpDownActions(this, this);

    QAction* const altBackwardAction = new QAction(i18n("Previous Image"), this);
    ac->addAction(QLatin1String("survey_backward_shift_space"), altBackwardAction);
    ac->setDefaultShortcut(altBackwardAction, Qt::SHIFT | Qt::Key_Space);
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.

    TagsActionMngr::defaultManager()->registerLabelsActions(ac);

    QAction* const editTitles = new QAction(i18n("Edit Titles"), this);
    ac->addAction(QLatin1String("edit_titles"), editTitles);
    ac->setDefaultShortcut(editTitles, Qt::ALT | Qt::SHIFT | Qt::Key_T);
    connect(editTitles, SIGNAL(triggered()), this, SLOT(slotSideBarActivateTitles()));

    QAction* const editComments = new QAction(i18n("Edit Comments"), this);
    ac->addAction(QLatin1String("edit_comments"), editComments);
    ac->setDefaultShortcut(editComments, Qt::ALT | Qt::SHIFT | Qt::Key_C);
    connect(editComments, SIGNAL(triggered()), this, SLOT(slotSideBarActivateComments()));

    QAction* const assignedTags = new QAction(i18n("Show Assigned Tags"), this);
    ac->addAction(QLatin1String("assigned_tags"), assignedTags);
    ac->setDefaultShortcut(assignedTags, Qt::ALT | Qt::SHIFT | Qt::Key_A);
    connect(assignedTags, SIGNAL(triggered()), this, SLOT(slotSideBarActivateAssignedTags()));

    // ---------------------------------------------------------------------------------

    createGUI(xmlFile());
    registerPluginsActions();

    cleanupActions();

    HostActionsMap actions;
    actions.insert(QLatin1String("FullScreen"),   actionCollection()->action(QLatin1String("survey_fullscreen")));
    actions.insert(QLatin1String("ShowMenu"),     actionCollection()->action(QLatin1String("options_show_menubar")));
    actions.insert(QLatin1String("ColorManaged"), actionCollection()->action(QLatin1String("color_managed_view")));

    d->stack->imagePreviewView()->setHostWindowActions(actions);

#ifdef HAVE_MEDIAPLAYER

    d->stack->mediaPlayerView()->setHostWindowActions(actions);

#endif // HAVE_MEDIAPLAYER

}

void SurveyWindow::setupStatusBar()
{
    d->fileName = new StatusProgressBar(statusBar());
    d->fileName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(d->fileName, 10);

    d->zoomBar = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->zoomPlusAction);
    d->zoomBar->setZoomMinusAction(d->zoomMinusAction);
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->zoomBar->setEnabled(true);
    statusBar()->addWidget(d->zoomBar, 1);
}

void SurveyWindow::setupConnections()
{
    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(IccSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    // Zoom bars connections -----------------------------------------

    connect(d->zoomBar, &DZoomBar::signalZoomSliderChanged,
            this, [this](int size)
        {
            double z = DZoomBar::zoomFromSize(size, d->stack->zoomMin(), d->stack->zoomMax());
            d->stack->setZoomFactor(z);
        }
    );

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->stack, SLOT(setZoomFactor(double)));

    // View connections ---------------------------------------------

    connect(d->stack->thumbBar(), SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelected()));

    connect(d->stack->thumbBar(), SIGNAL(signalOpenGeolocationMap()),
            d->sideBar, SLOT(slotOpenGeolocationMap()));

    connect(d->stack, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->stack, SIGNAL(signalPopupTagsView()),
            d->sideBar, SLOT(slotPopupTagsView()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->zoomBar, SLOT(slotUpdateTrackerPos()));

    // ---

    connect(d->stack->imagePreviewView(), &ItemPreviewView::signalStartedLoading,
            this, [this]()
       {
            d->fileName->setProgressBarMode(StatusProgressBar::ProgressBarMode, i18nc("@label", "Loading:"));
       }
    );

    connect(d->stack->imagePreviewView(), &ItemPreviewView::signalLoadingProgress,
            this, [this](float progress)
       {
            d->fileName->setProgressValue((int)(progress * 100.0));
       }
    );

    connect(d->stack->imagePreviewView(), &ItemPreviewView::signalLoadingComplete,
            this, [this]()
       {
            d->fileName->setProgressBarMode(StatusProgressBar::TextMode, d->fileName->text());
       }
    );
}

void SurveyWindow::setupUserArea(DigikamItemView* const iconView)
{
    QWidget* const mainW    = new QWidget(this);
    d->hSplitter            = new SidebarSplitter(Qt::Horizontal, mainW);
    QHBoxLayout* const hlay = new QHBoxLayout(mainW);

    // The central preview is wrapped in a KMainWindow so that the thumbnail
    // bar can float around it.

    d->dockArea             = new QMainWindow(mainW, Qt::Widget);

    d->hSplitter->addWidget(d->dockArea);
    d->hSplitter->setStretchFactor(d->hSplitter->indexOf(d->dockArea), 10);

    d->stack                = new SurveyStack(iconView, this);
    d->stack->setDockArea(d->dockArea);
    d->dockArea->setCentralWidget(d->stack);

    // The right sidebar.

    d->sideBar              = new ItemPropertiesSideBarDB(mainW, d->hSplitter, Qt::RightEdge, true);

    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->sideBar);
    hlay->setSpacing(0);
    hlay->setContentsMargins(QMargins());
    hlay->setStretchFactor(d->hSplitter, 10);

    d->hSplitter->setFrameStyle(QFrame::NoFrame);
    d->hSplitter->setFrameShadow(QFrame::Plain);
    d->hSplitter->setFrameShape(QFrame::NoFrame);
    d->hSplitter->setOpaqueResize(false);
    d->hSplitter->setStretchFactor(1, 10);      // set previewview+thumbbar container default size to max.

    d->sideBar->setItemFilterModel(d->stack->thumbBar()->itemFilterModel());

    setCentralWidget(mainW);
}

} // namespace Digikam
