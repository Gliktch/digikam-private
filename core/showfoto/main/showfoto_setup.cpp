/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - Internal setup
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfoto_p.h"

namespace ShowFoto
{

void Showfoto::setupActions()
{
    Digikam::ThemeManager::instance()->setThemeMenuAction(new QMenu(i18n("&Themes"), this));
    setupStandardActions();

    // Extra 'File' menu actions ---------------------------------------------

    d->fileOpenAction = buildStdAction(StdOpenAction, this, SLOT(slotOpenFile()), this);
    actionCollection()->addAction(QLatin1String("showfoto_open_file"), d->fileOpenAction);

    // ---------

    d->openFilesInFolderAction = new QAction(QIcon::fromTheme(QLatin1String("folder-pictures")), i18n("Open folder"), this);
    actionCollection()->setDefaultShortcut(d->openFilesInFolderAction, Qt::CTRL | Qt::SHIFT | Qt::Key_O);

    connect(d->openFilesInFolderAction, &QAction::triggered,
            this, &Showfoto::slotOpenFolder);

    actionCollection()->addAction(QLatin1String("showfoto_open_folder"), d->openFilesInFolderAction);

    // ---------

    QAction* const quit = buildStdAction(StdQuitAction, this, SLOT(close()), this);
    actionCollection()->addAction(QLatin1String("showfoto_quit"), quit);

    // -- Standard 'Help' menu actions ---------------------------------------------

    createHelpActions(QLatin1String("showfoto_editor"), false);
}

void Showfoto::setupConnections()
{
    setupStandardConnections();

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->thumbBar, SIGNAL(showfotoItemInfoActivated(ShowfotoItemInfo)),
            this, SLOT(slotShowfotoItemInfoActivated(ShowfotoItemInfo)));

    connect(d->stackView, SIGNAL(signalShowfotoItemInfoActivated(ShowfotoItemInfo)),
            this, SLOT(slotShowfotoItemInfoActivated(ShowfotoItemInfo)));

    connect(d->folderView, SIGNAL(signalLoadContentsFromPath(QString)),
            this, SLOT(slotOpenFolderFromPath(QString)));

    connect(d->folderView, SIGNAL(signalLoadContentsFromFiles(QStringList,QString)),
            this, SLOT(slotOpenFilesfromPath(QStringList,QString)));

    connect(d->folderView, SIGNAL(signalAppendContentsFromFiles(QStringList,QString)),
            this, SLOT(slotAppendFilesfromPath(QStringList,QString)));

    connect(d->stackView, SIGNAL(signalLoadContentsFromFiles(QStringList,QString)),
            this, SLOT(slotOpenFilesfromPath(QStringList,QString)));

    connect(d->stackView, SIGNAL(signalClearItemsList()),
            this, SLOT(slotClearThumbBar()));

    connect(d->stackView, SIGNAL(signalRemoveItemInfos(QList<ShowfotoItemInfo>)),
            this, SLOT(slotRemoveItemInfos(QList<ShowfotoItemInfo>)));

    connect(this, SIGNAL(signalSelectionChanged(QRect)),
            d->rightSideBar, SLOT(slotImageSelectionChanged(QRect)));

    connect(this, &Showfoto::signalOpenFolder,
            this, &Showfoto::slotOpenFolder);

    connect(this, &Showfoto::signalOpenFile,
            this, &Showfoto::slotOpenFile);

    connect(this, SIGNAL(signalInfoList(ShowfotoItemInfoList)),
            d->model, SLOT(reAddShowfotoItemInfos(ShowfotoItemInfoList)));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            d->model, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    connect(d->rightSideBar, SIGNAL(signalSetupMetadataFilters(int)),
            this, SLOT(slotSetupMetadataFilters(int)));

    connect(d->rightSideBar, SIGNAL(signalSetupExifTool()),
            this, SLOT(slotSetupExifTool()));

    connect(d->folderView, SIGNAL(signalSetup()),
            this, SLOT(slotSetup()));

    connect(d->dDHandler, SIGNAL(signalDroppedUrls(QList<QUrl>,bool,QUrl)),
            this, SLOT(slotDroppedUrls(QList<QUrl>,bool,QUrl)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

void Showfoto::setupUserArea()
{
    QWidget* const widget            = new QWidget(this);
    QHBoxLayout* const hlay          = new QHBoxLayout(widget);
    m_splitter                       = new Digikam::SidebarSplitter(widget);

    const int spacing                = layoutSpacing();

    d->leftSideBar                   = new Digikam::Sidebar(widget, m_splitter, Qt::LeftEdge);
    d->leftSideBar->setObjectName(QLatin1String("ShowFoto Sidebar Left"));
    d->leftSideBar->setContentsMargins(0, 0, spacing, 0);

    d->folderView                    = new ShowfotoFolderViewSideBar(this);
    d->leftSideBar->appendTab(d->folderView, d->folderView->getIcon(), d->folderView->getCaption());

    d->stackView                     = new ShowfotoStackViewSideBar(this);
    d->leftSideBar->appendTab(d->stackView, d->stackView->getIcon(), d->stackView->getCaption());

    KMainWindow* const viewContainer = new KMainWindow(widget, Qt::Widget);

#if (KXMLGUI_VERSION >= QT_VERSION_CHECK(5, 88, 0))

    viewContainer->setStateConfigGroup(QLatin1String("ImageViewer Thumbbar"));

#endif

    m_splitter->addWidget(viewContainer);
    m_stackView                      = new Digikam::EditorStackView(viewContainer);
    m_canvas                         = new Digikam::Canvas(m_stackView);
    viewContainer->setCentralWidget(m_stackView);

    m_splitter->setFrameStyle(QFrame::NoFrame);
    m_splitter->setFrameShape(QFrame::NoFrame);
    m_splitter->setFrameShadow(QFrame::Plain);
    m_splitter->setStretchFactor(m_splitter->indexOf(viewContainer), 10);      // set Canvas default size to max.
    m_splitter->setOpaqueResize(false);

    m_canvas->makeDefaultEditingCanvas();
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(Digikam::EditorStackView::CanvasMode);

    d->rightSideBar                  = new Digikam::ItemPropertiesSideBar(widget, m_splitter, Qt::RightEdge);
    d->rightSideBar->setObjectName(QLatin1String("ShowFoto Sidebar Right"));

    hlay->addWidget(d->leftSideBar);
    hlay->addWidget(m_splitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);

    // Code to check for the now depreciated HorizontalThumbar directive. It
    // is found, it is honored and deleted. The state will from than on be saved
    // by viewContainers built-in mechanism.

    d->thumbBarDock                  = new Digikam::ThumbBarDock(viewContainer, Qt::Tool);
    d->thumbBarDock->setObjectName(QLatin1String("editor_thumbbar"));
    d->thumbBarDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    d->thumbBar                      = new ShowfotoThumbnailBar(d->thumbBarDock);

    d->thumbBarDock->setWidget(d->thumbBar);

    viewContainer->addDockWidget(Qt::TopDockWidgetArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);

    d->model                         = new ShowfotoThumbnailModel(d->thumbBar);
    d->model->setThumbnailLoadThread(d->thumbLoadThread);
    d->model->setSendRemovalSignals(true);

    d->dDHandler                     = new ShowfotoDragDropHandler(d->model);
    d->model->setDragDropHandler(d->dDHandler);

    d->filterModel                   = new ShowfotoFilterModel(d->thumbBar);
    d->filterModel->setSourceShowfotoModel(d->model);
    d->filterModel->setCategorizationMode(ShowfotoItemSortSettings::NoCategories);
    d->filterModel->sort(0);

    d->thumbBar->setModels(d->model, d->filterModel);
    d->thumbBar->setSelectionMode(QAbstractItemView::SingleSelection);

    viewContainer->setAutoSaveSettings(QLatin1String("ImageViewer Thumbbar"), true);

    d->thumbBar->installOverlays();
    d->stackView->setThumbbar(d->thumbBar);

    setCentralWidget(widget);
}

void Showfoto::slotContextMenu()
{
    if (m_contextMenu)
    {
        m_contextMenu->addSeparator();
        addServicesMenu();
        m_contextMenu->exec(QCursor::pos());
    }
}

void Showfoto::addServicesMenu()
{
    addServicesMenuForUrl(d->thumbBar->currentUrl());
}

void Showfoto::toggleNavigation(int index)
{
    if (!m_actionEnabledState)
    {
        return;
    }

    if ((d->itemsNb == 0) || (d->itemsNb == 1))
    {
        m_backwardAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
    else
    {
        m_backwardAction->setEnabled(true);
        m_forwardAction->setEnabled(true);
        m_firstAction->setEnabled(true);
        m_lastAction->setEnabled(true);
    }

    if (index == 1)
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (index == d->itemsNb)
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
}

void Showfoto::toggleActions(bool val)
{
    toggleStandardActions(val);
}

} // namespace ShowFoto
