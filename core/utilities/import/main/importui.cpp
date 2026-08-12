/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Import tool interface
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importui_p.h"

namespace Digikam
{

ImportUI* ImportUI::m_instance = nullptr;

ImportUI::ImportUI(const QString& cameraTitle, const QString& model,
                   const QString& port, const QString& path, int startIndex)
    : DXmlGuiWindow(nullptr),
      d            (new Private)
{
    setConfigGroupName(QLatin1String("Camera Settings"));

    setXMLFile(QLatin1String("importui5.rc"));

#if (KXMLGUI_VERSION >= QT_VERSION_CHECK(5, 88, 0))

    setStateConfigGroup(configGroupName());

#endif

    setFullScreenOptions(FS_IMPORTUI);
    setWindowFlags(Qt::Window);


    m_instance     = this;

    // --------------------------------------------------------

    QString title  = CameraNameHelper::cameraName(cameraTitle);
    d->cameraTitle = (title.isEmpty()) ? cameraTitle : title;
    setCaption(d->cameraTitle);

    setupCameraController(model, port, path);
    setupUserArea();
    setInitialSorting();
    setupActions();
    setupStatusBar();
    setupAccelerators();

    // -- Make signals/slots connections ---------------------------------

    setupConnections();
    sidebarTabTitleStyleChanged();
    slotColorManagementOptionsChanged();

    // -- Read settings --------------------------------------------------

    readSettings();

    setAutoSaveSettings(configGroupName(), true);

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080

    // -------------------------------------------------------------------
/*
    d->historyUpdater = new CameraHistoryUpdater(this);

    connect(d->historyUpdater, SIGNAL(signalHistoryMap(CHUpdateItemMap)),
            this, SLOT(slotRefreshIconView(CHUpdateItemMap)));

    connect(d->historyUpdater, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));
*/
    // --------------------------------------------------------

    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));

    // --------------------------------------------------------

    d->renameCustomizer->setStartIndex(startIndex);
    d->view->setFocus();

    // -- Init icon view zoom factor --------------------------

    slotThumbSizeChanged(ImportSettings::instance()->getDefaultIconSize());
    slotZoomSliderChanged(ImportSettings::instance()->getDefaultIconSize());

    // try to connect in the end, this allows us not to block the UI to show up..

    QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
}

ImportUI::~ImportUI()
{
    saveSettings();
    disconnect(d->view, nullptr, this, nullptr);

    delete d->view;
    delete d->rightSideBar;
    delete d->controller;

    m_instance = nullptr;

    delete d;
}

ImportUI* ImportUI::instance()
{
    return m_instance;
}

void ImportUI::setupUserArea()
{
    DHBox* const widget = new DHBox(this);
    d->splitter         = new SidebarSplitter(widget);
    DVBox* const vbox   = new DVBox(d->splitter);
    d->view             = new ImportView(this, vbox);
    d->view->importFilterModel()->setCameraThumbsController(d->camThumbsCtrl);
    d->view->importFilterModel()->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->historyView      = new DHistoryView(vbox);
    d->rightSideBar     = new ImportItemPropertiesSideBarImport(widget, d->splitter, Qt::RightEdge, true);
    d->rightSideBar->setObjectName(QLatin1String("CameraGui Sidebar Right"));
    d->splitter->setFrameStyle(QFrame::NoFrame);
    d->splitter->setFrameShadow(QFrame::Plain);
    d->splitter->setFrameShape(QFrame::NoFrame);
    d->splitter->setOpaqueResize(false);
    d->splitter->setStretchFactor(d->splitter->indexOf(vbox), 10);      // set iconview default size to max.

    vbox->setStretchFactor(d->view, 10);
    vbox->setStretchFactor(d->historyView, 2);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(0);

    d->errorWidget = new DNotificationWidget(vbox);
    d->errorWidget->setMessageType(DNotificationWidget::Error);
    d->errorWidget->setCloseButtonVisible(false);
    d->errorWidget->hide();

    // -------------------------------------------------------------------------

    d->advBox = new DExpanderBox(d->rightSideBar);
    d->advBox->setObjectName(QLatin1String("Camera Settings Expander"));

    d->renameCustomizer = new RenameCustomizer(d->advBox, d->cameraTitle);
    d->renameCustomizer->setWhatsThis(i18nc("@info", "Set how digiKam will rename files as they are downloaded."));
    d->advBox->addItem(d->renameCustomizer, QIcon::fromTheme(QLatin1String("insert-image")), i18nc("@item", "File Renaming Options"),
                       QLatin1String("RenameCustomizer"), true);

    // -- Albums Auto-creation options -----------------------------------------

    d->albumCustomizer = new AlbumCustomizer(d->advBox);
    d->advBox->addItem(d->albumCustomizer, QIcon::fromTheme(QLatin1String("folder-new")), i18nc("@item", "Auto-creation of Albums"),
                       QLatin1String("AlbumBox"), false);

    // -- On the Fly options ---------------------------------------------------

    d->advancedSettings = new AdvancedSettings(d->advBox);
    d->advBox->addItem(d->advancedSettings, QIcon::fromTheme(QLatin1String("system-run")), i18nc("@item", "On the Fly Operations (JPEG only)"),
                       QLatin1String("OnFlyBox"), true);

#ifdef HAVE_JXL

    // -- DNG convert options --------------------------------------------------

    d->dngConvertSettings = new DNGConvertSettings(d->advBox);
    d->advBox->addItem(d->dngConvertSettings, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")), i18nc("@item", "DNG Convert Options"),
                       QLatin1String("DNGSettings"), false);

#endif

    // -- Scripting options ----------------------------------------------------

    d->scriptingSettings = new ScriptingSettings(d->advBox);
    d->advBox->addItem(d->scriptingSettings, QIcon::fromTheme(QLatin1String("utilities-terminal")), i18nc("@item", "Scripting"),
                       QLatin1String("ScriptingBox"), false);
    d->advBox->addStretch();

    d->rightSideBar->appendTab(d->advBox, QIcon::fromTheme(QLatin1String("configure")), i18nc("@title", "Settings"));
    d->rightSideBar->loadState();

    // -------------------------------------------------------------------------

    setCentralWidget(widget);
}

void ImportUI::setupActions()
{
    d->cameraActions            = new QActionGroup(this);
    KActionCollection* const ac = actionCollection();

    // -- File menu ----------------------------------------------------

    d->cameraCancelAction = new QAction(QIcon::fromTheme(QLatin1String("process-stop")), i18nc("@action Cancel process", "Cancel"), this);
    connect(d->cameraCancelAction, SIGNAL(triggered()), this, SLOT(slotCancelButton()));
    ac->addAction(QLatin1String("importui_cancelprocess"), d->cameraCancelAction);
    d->cameraCancelAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->cameraInfoAction = new QAction(QIcon::fromTheme(QLatin1String("camera-photo")), i18nc("@action Information about camera", "Information"), this);
    connect(d->cameraInfoAction, SIGNAL(triggered()), this, SLOT(slotInformation()));
    ac->addAction(QLatin1String("importui_info"), d->cameraInfoAction);
    d->cameraActions->addAction(d->cameraInfoAction);

    // -----------------------------------------------------------------

    d->cameraCaptureAction = new QAction(QIcon::fromTheme(QLatin1String("webcamreceive")), i18nc("@action Capture photo from camera", "Capture"), this);
    connect(d->cameraCaptureAction, SIGNAL(triggered()), this, SLOT(slotCapture()));
    ac->addAction(QLatin1String("importui_capture"), d->cameraCaptureAction);
    d->cameraActions->addAction(d->cameraCaptureAction);

    // -----------------------------------------------------------------

    QAction* const closeAction = buildStdAction(StdCloseAction, this, SLOT(slotClose()), this);
    ac->addAction(QLatin1String("importui_close"), closeAction);

    // -- Edit menu ----------------------------------------------------

    d->selectAllAction = new QAction(i18nc("@action:inmenu", "Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    ac->addAction(QLatin1String("importui_selectall"), d->selectAllAction);
    ac->setDefaultShortcut(d->selectAllAction, Qt::CTRL | Qt::Key_A);
    d->cameraActions->addAction(d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new QAction(i18nc("@action:inmenu", "Select None"), this);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    ac->addAction(QLatin1String("importui_selectnone"), d->selectNoneAction);
    ac->setDefaultShortcut(d->selectNoneAction, Qt::CTRL | Qt::SHIFT | Qt::Key_A);
    d->cameraActions->addAction(d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new QAction(i18nc("@action:inmenu", "Invert Selection"), this);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    ac->addAction(QLatin1String("importui_selectinvert"), d->selectInvertAction);
    ac->setDefaultShortcut(d->selectInvertAction, Qt::CTRL | Qt::Key_Asterisk);
    d->cameraActions->addAction(d->selectInvertAction);

    // -----------------------------------------------------------

    d->selectNewItemsAction = new QAction(QIcon::fromTheme(QLatin1String("folder-favorites")), i18nc("@action:inmenu", "Select New Items"), this);
    connect(d->selectNewItemsAction, SIGNAL(triggered()), this, SLOT(slotSelectNew()));
    ac->addAction(QLatin1String("importui_selectnewitems"), d->selectNewItemsAction);
    d->cameraActions->addAction(d->selectNewItemsAction);

    // -----------------------------------------------------------

    d->selectLockedItemsAction = new QAction(QIcon::fromTheme(QLatin1String("object-locked")), i18nc("@action:inmenu", "Select Locked Items"), this);
    connect(d->selectLockedItemsAction, SIGNAL(triggered()), this, SLOT(slotSelectLocked()));
    ac->addAction(QLatin1String("importui_selectlockeditems"), d->selectLockedItemsAction);
    ac->setDefaultShortcut(d->selectLockedItemsAction, Qt::CTRL | Qt::Key_L);
    d->cameraActions->addAction(d->selectLockedItemsAction);

    // --- Download actions ----------------------------------------------------

    d->downloadAction = new QMenu(i18nc("@title:menu", "Download"), this);
    d->downloadAction->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    ac->addAction(QLatin1String("importui_imagedownload"), d->downloadAction->menuAction());
    d->cameraActions->addAction(d->downloadAction->menuAction());

    d->downloadNewAction = new QAction(QIcon::fromTheme(QLatin1String("folder-favorites")), i18nc("@action", "Download New"), this);
    connect(d->downloadNewAction, SIGNAL(triggered()), this, SLOT(slotDownloadNew()));
    ac->addAction(QLatin1String("importui_imagedownloadnew"), d->downloadNewAction);
    ac->setDefaultShortcut(d->downloadNewAction, Qt::CTRL | Qt::Key_N);
    d->downloadAction->addAction(d->downloadNewAction);
    d->cameraActions->addAction(d->downloadNewAction);

    connect(d->downloadAction->menuAction(), SIGNAL(triggered()),
            d->downloadNewAction, SLOT(trigger()));

    d->downloadSelectedAction = new QAction(QIcon::fromTheme(QLatin1String("document-save")), i18nc("@action", "Download Selected"), this);
    connect(d->downloadSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadSelected()));
    ac->addAction(QLatin1String("importui_imagedownloadselected"), d->downloadSelectedAction);
    d->downloadSelectedAction->setEnabled(false);
    d->downloadAction->addAction(d->downloadSelectedAction);
    d->cameraActions->addAction(d->downloadSelectedAction);

    d->downloadAllAction = new QAction(QIcon::fromTheme(QLatin1String("document-save")), i18nc("@action", "Download All"), this);
    connect(d->downloadAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAll()));
    ac->addAction(QLatin1String("importui_imagedownloadall"), d->downloadAllAction);
    d->downloadAction->addAction(d->downloadAllAction);
    d->cameraActions->addAction(d->downloadAllAction);

    // -------------------------------------------------------------------------

    d->downloadDelNewAction = new QAction(i18nc("@action", "Download && Delete New"), this);
    connect(d->downloadDelNewAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteNew()));
    ac->addAction(QLatin1String("importui_imagedownloaddeletenew"), d->downloadDelNewAction);
    ac->setDefaultShortcut(d->downloadDelNewAction, Qt::CTRL | Qt::SHIFT | Qt::Key_N);
    d->cameraActions->addAction(d->downloadDelNewAction);

    // -----------------------------------------------------------------

    d->downloadDelSelectedAction = new QAction(i18nc("@action", "Download && Delete Selected"), this);
    connect(d->downloadDelSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteSelected()));
    ac->addAction(QLatin1String("importui_imagedownloaddeleteselected"), d->downloadDelSelectedAction);
    d->downloadDelSelectedAction->setEnabled(false);
    d->cameraActions->addAction(d->downloadDelSelectedAction);

    // -------------------------------------------------------------------------

    d->downloadDelAllAction = new QAction(i18nc("@action", "Download && Delete All"), this);
    connect(d->downloadDelAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteAll()));
    ac->addAction(QLatin1String("importui_imagedownloaddeleteall"), d->downloadDelAllAction);
    d->cameraActions->addAction(d->downloadDelAllAction);

    // -------------------------------------------------------------------------

    d->uploadAction = new QAction(QIcon::fromTheme(QLatin1String("media-flash-sd-mmc")), i18nc("@action", "Upload..."), this);
    connect(d->uploadAction, SIGNAL(triggered()), this, SLOT(slotUpload()));
    ac->addAction(QLatin1String("importui_imageupload"), d->uploadAction);
    ac->setDefaultShortcut(d->uploadAction, Qt::CTRL | Qt::Key_U);
    d->cameraActions->addAction(d->uploadAction);

    // -------------------------------------------------------------------------

    d->lockAction = new QAction(QIcon::fromTheme(QLatin1String("object-locked")), i18nc("@action", "Toggle Lock"), this);
    connect(d->lockAction, SIGNAL(triggered()), this, SLOT(slotToggleLock()));
    ac->addAction(QLatin1String("importui_imagelock"), d->lockAction);
    ac->setDefaultShortcut(d->lockAction, Qt::CTRL | Qt::Key_G);
    d->cameraActions->addAction(d->lockAction);

    // -------------------------------------------------------------------------

    d->markAsDownloadedAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-ok-apply")), i18nc("@action", "Mark as downloaded"), this);
    connect(d->markAsDownloadedAction, SIGNAL(triggered()), this, SLOT(slotMarkAsDownloaded()));
    ac->addAction(QLatin1String("importui_imagemarkasdownloaded"), d->markAsDownloadedAction);
    d->cameraActions->addAction(d->markAsDownloadedAction);

    // --- Delete actions ------------------------------------------------------

    d->deleteAction = new QMenu(i18nc("@title:menu", "Delete"), this);
    d->deleteAction->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    ac->addAction(QLatin1String("importui_delete"), d->deleteAction->menuAction());
    d->cameraActions->addAction(d->deleteAction->menuAction());

    d->deleteSelectedAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18nc("@action:inmenu delete item", "Delete Selected"), this);
    connect(d->deleteSelectedAction, SIGNAL(triggered()), this, SLOT(slotDeleteSelected()));
    ac->addAction(QLatin1String("importui_imagedeleteselected"), d->deleteSelectedAction);
    ac->setDefaultShortcut(d->deleteSelectedAction, Qt::Key_Delete);
    d->deleteSelectedAction->setEnabled(false);
    d->deleteAction->addAction(d->deleteSelectedAction);
    d->cameraActions->addAction(d->deleteSelectedAction);

    d->deleteAllAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18nc("@action:inmenu delete item", "Delete All"), this);
    connect(d->deleteAllAction, SIGNAL(triggered()), this, SLOT(slotDeleteAll()));
    ac->addAction(QLatin1String("importui_imagedeleteall"), d->deleteAllAction);
    d->deleteAction->addAction(d->deleteAllAction);
    d->cameraActions->addAction(d->deleteAllAction);

    d->deleteNewAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18nc("@action:inmenu delete item", "Delete New"), this);
    connect(d->deleteNewAction, SIGNAL(triggered()), this, SLOT(slotDeleteNew()));
    ac->addAction(QLatin1String("importui_imagedeletenew"), d->deleteNewAction);
    d->deleteAction->addAction(d->deleteNewAction);
    d->cameraActions->addAction(d->deleteNewAction);

    // --- Icon view, items preview, and map actions ------------------------------------------------------

    d->imageViewSelectionAction = new KSelectAction(QIcon::fromTheme(QLatin1String("view-preview")), i18nc("@title:group", "Views"), this);
    ac->addAction(QLatin1String("importui_view_selection"), d->imageViewSelectionAction);

    d->iconViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-icons")),
                                    i18nc("@action Go to thumbnails (icon) view", "Thumbnails"), this);
    d->iconViewAction->setCheckable(true);
    ac->addAction(QLatin1String("importui_icon_view"), d->iconViewAction);
    connect(d->iconViewAction, SIGNAL(triggered()), d->view, SLOT(slotIconView()));
    d->imageViewSelectionAction->addAction(d->iconViewAction);

    d->camItemPreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                                i18nc("@action View the selected image", "Preview Item"), this);
    d->camItemPreviewAction->setCheckable(true);
    ac->addAction(QLatin1String("importui_item_view"), d->camItemPreviewAction);
    ac->setDefaultShortcut(d->camItemPreviewAction, Qt::Key_F3);
    connect(d->camItemPreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    d->imageViewSelectionAction->addAction(d->camItemPreviewAction);

#ifdef HAVE_GEOLOCATION

    d->mapViewAction = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                   i18nc("@action Switch to map view", "Map"), this);
    d->mapViewAction->setCheckable(true);
    ac->addAction(QLatin1String("importui_map_view"), d->mapViewAction);
    connect(d->mapViewAction, SIGNAL(triggered()), d->view, SLOT(slotMapWidgetView()));
    d->imageViewSelectionAction->addAction(d->mapViewAction);

#endif // HAVE_GEOLOCATION

    /// @todo Add table view stuff here

    // -- Item Sorting ------------------------------------------------------------

    d->itemSortAction                    = new KSelectAction(i18nc("@title:menu", "&Sort Items"), this);
    d->itemSortAction->setWhatsThis(i18nc("@info:whatsthis", "The value by which the items are sorted in the thumbnail view"));
    ac->addAction(QLatin1String("item_sort"), d->itemSortAction);

    // map to CamItemSortSettings enum

    QAction* const sortByNameAction       = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By Name"));
    QAction* const sortByPathAction       = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By Path"));
    QAction* const sortByDateAction       = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By Date"));
    QAction* const sortByFileSizeAction   = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By Size"));
    QAction* const sortByFormatAction     = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By Format"));
    d->itemSortAction->menu()->addSeparator();
    QAction* const sortByRatingAction     = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By Rating"));
    QAction* const sortByPickLabelAction  = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By PickLabel"));
    QAction* const sortByColorLabelAction = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By ColorLabel"));
    d->itemSortAction->menu()->addSeparator();
    QAction* const sortByDownloadAction   = d->itemSortAction->addAction(i18nc("@item:inmenu Sort by", "By Download State"));

    connect(sortByNameAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByFileName); });

    connect(sortByPathAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByFilePath); });

    connect(sortByDateAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByCreationDate); });

    connect(sortByFileSizeAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByFileSize); });

    connect(sortByFormatAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByFormat); });

    connect(sortByRatingAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByRating); });

    connect(sortByPickLabelAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByPickLabel); });

    connect(sortByColorLabelAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByColorLabel); });


    connect(sortByDownloadAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesBy((int)CamItemSortSettings::SortByDownloadState); });

    d->itemSortAction->setCurrentItem(ImportSettings::instance()->getImageSortBy());

    // -- Item Sort Order ------------------------------------------------------------

    d->itemSortOrderAction                    = new KSelectAction(i18nc("@action", "Item Sorting &Order"), this);
    d->itemSortOrderAction->setWhatsThis(i18nc("@info:whatsthis", "Defines whether items are sorted in ascending or descending manner."));
    ac->addAction(QLatin1String("item_sort_order"), d->itemSortOrderAction);

    QAction* const sortAscendingAction  = d->itemSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")),
                                                                            i18nc("@item:inmenu Sorting Order", "Ascending"));
    QAction* const sortDescendingAction = d->itemSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")),
                                                                            i18nc("@item:inmenu Sorting Order", "Descending"));

    connect(sortAscendingAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesOrder((int)CamItemSortSettings::AscendingOrder); });

    connect(sortDescendingAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesOrder((int)CamItemSortSettings::DescendingOrder); });

    d->itemSortOrderAction->setCurrentItem(ImportSettings::instance()->getImageSortOrder());

    // -- Item Grouping ------------------------------------------------------------

    d->itemsGroupAction                  = new KSelectAction(i18nc("@title:menu", "&Group Items"), this);
    d->itemsGroupAction->setWhatsThis(i18nc("@info:whatsthis", "The categories in which the items in the thumbnail view are displayed"));
    ac->addAction(QLatin1String("item_group"), d->itemsGroupAction);

    // map to CamItemSortSettings enum

    QAction* const noCategoriesAction  = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "Flat List"));
    QAction* const groupByFolderAction = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Folder"));
    QAction* const groupByFormatAction = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Format"));
    QAction* const groupByDateAction   = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Date"));

    connect(noCategoriesAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)CamItemSortSettings::NoCategories); });

    connect(groupByFolderAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)CamItemSortSettings::CategoryByFolder); });

    connect(groupByFormatAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)CamItemSortSettings::CategoryByFormat); });

    connect(groupByDateAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)CamItemSortSettings::CategoryByDate); });

    d->itemsGroupAction->setCurrentItem(ImportSettings::instance()->getImageSeparationMode());

    // -- Standard 'View' menu actions ---------------------------------------------

    d->increaseThumbsAction = buildStdAction(StdZoomInAction, d->view, SLOT(slotZoomIn()), this);
    d->increaseThumbsAction->setEnabled(false);
    ac->addAction(QLatin1String("importui_zoomplus"), d->increaseThumbsAction);

    d->decreaseThumbsAction = buildStdAction(StdZoomOutAction, d->view, SLOT(slotZoomOut()), this);
    d->decreaseThumbsAction->setEnabled(false);
    ac->addAction(QLatin1String("importui_zoomminus"), d->decreaseThumbsAction);

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18nc("@action:inmenu", "Fit to &Window"), this);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    ac->addAction(QLatin1String("import_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, Qt::CTRL | Qt::ALT | Qt::Key_E);

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18nc("@action:inmenu", "Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    ac->addAction(QLatin1String("import_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, Qt::CTRL | Qt::Key_Period);

    // ------------------------------------------------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18nc("@action", "Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // ------------------------------------------------------------------------------------------------

    createFullScreenAction(QLatin1String("importui_fullscreen"));
    createSidebarActions();

    d->showLogAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18nc("@option:check", "Show History"), this);
    d->showLogAction->setCheckable(true);
    connect(d->showLogAction, SIGNAL(triggered()), this, SLOT(slotShowLog()));
    ac->addAction(QLatin1String("importui_showlog"), d->showLogAction);
    ac->setDefaultShortcut(d->showLogAction, Qt::CTRL | Qt::Key_H);

    d->showBarAction = new QAction(QIcon::fromTheme(QLatin1String("view-choose")), i18nc("@option:check", "Show Thumbbar"), this);
    d->showBarAction->setCheckable(true);
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    ac->addAction(QLatin1String("showthumbs"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL | Qt::Key_T);
    d->showBarAction->setEnabled(false);

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // Standard 'Help' menu actions
    createHelpActions(QLatin1String("camera_import"));

    // Provides a menu entry that allows showing/hiding the toolbar(s)

    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar

    createStandardStatusBarAction();

    // Standard 'Configure' menu actions

    createSettingsActions();

    // -- Keyboard-only actions added to <MainWindow> ----------------------------------

    QAction* const altBackwardAction = new QAction(i18nc("@action", "Previous Image"), this);
    ac->addAction(QLatin1String("importui_backward_shift_space"), altBackwardAction);
    ac->setDefaultShortcut(altBackwardAction, Qt::SHIFT | Qt::Key_Space);
    connect(altBackwardAction, SIGNAL(triggered()), d->view, SLOT(slotPrevItem()));

    // ---------------------------------------------------------------------------------

    d->connectAction = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18nc("@action Connection failed, try again?", "Retry"), this);
    connect(d->connectAction, SIGNAL(triggered()), d->controller, SLOT(slotConnect()));

    createGUI(xmlFile());
    cleanupActions();
}

void ImportUI::updateActions()
{
    const CamItemInfoList& list = d->view->selectedCamItemInfos();
    bool hasSelection           = !list.isEmpty();

    d->downloadDelSelectedAction->setEnabled(hasSelection && d->controller->cameraDeleteSupport());
    d->deleteSelectedAction->setEnabled(hasSelection && d->controller->cameraDeleteSupport());
    d->camItemPreviewAction->setEnabled(hasSelection && cameraUseUMSDriver());
    d->downloadSelectedAction->setEnabled(hasSelection);
    d->lockAction->setEnabled(hasSelection);

    if (hasSelection)
    {
        // only enable "Mark as downloaded" if at least one
        // selected image has not been downloaded

        bool haveNotDownloadedItem = false;

        for (const CamItemInfo& info : std::as_const(list))
        {
            haveNotDownloadedItem = !(info.downloaded == CamItemInfo::DownloadedYes);

            if (haveNotDownloadedItem)
            {
                break;
            }
        }

        d->markAsDownloadedAction->setEnabled(haveNotDownloadedItem);
    }
    else
    {
        d->markAsDownloadedAction->setEnabled(false);
    }
}

void ImportUI::setupConnections()
{
    // TODO: Needs testing.

    connect(d->advancedSettings, SIGNAL(signalDownloadNameChanged()),
            this, SLOT(slotUpdateRenamePreview()));

#ifdef HAVE_JXL

    connect(d->dngConvertSettings, SIGNAL(signalDownloadNameChanged()),
            this, SLOT(slotUpdateRenamePreview()));

#endif

    connect(d->historyView, SIGNAL(signalEntryClicked(QVariant)),
            this, SLOT(slotHistoryEntryClicked(QVariant)));

    connect(IccSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalImageSelected(CamItemInfoList,CamItemInfoList)),
            this, SLOT(slotImageSelected(CamItemInfoList,CamItemInfoList)));

    connect(d->view, SIGNAL(signalSwitchedToPreview()),
            this, SLOT(slotSwitchedToPreview()));

    connect(d->view, SIGNAL(signalSwitchedToIconView()),
            this, SLOT(slotSwitchedToIconView()));

    connect(d->view, SIGNAL(signalSwitchedToMapView()),
            this, SLOT(slotSwitchedToMapView()));

    connect(d->view, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotNewSelection(bool)));

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));

    connect(d->view, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->view, SLOT(setZoomFactor(double)));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->zoomBar, SLOT(slotUpdateTrackerPos()));

    // -------------------------------------------------------------------------

    connect(CollectionManager::instance(), SIGNAL(locationStatusChanged(CollectionLocation,int)),
            this, SLOT(slotCollectionLocationStatusChanged(CollectionLocation,int)));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    connect(d->renameCustomizer, SIGNAL(signalChanged()),
            this, SLOT(slotUpdateRenamePreview()));
}

void ImportUI::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->statusProgressBar->setNotificationTitle(d->cameraTitle, QIcon::fromTheme(QLatin1String("camera-photo")));
    statusBar()->addWidget(d->statusProgressBar, 100);

    //------------------------------------------------------------------------------

    d->cameraFreeSpace = new FreeSpaceWidget(statusBar(), 100);

    if (cameraUseGPhotoDriver())
    {
        d->cameraFreeSpace->setMode(FreeSpaceWidget::GPhotoCamera);
        connect(d->controller, SIGNAL(signalFreeSpace(qint64,qint64)),
                this, SLOT(slotCameraFreeSpaceInfo(qint64,qint64)));
    }
    else
    {
        d->cameraFreeSpace->setMode(FreeSpaceWidget::UMSCamera);
        d->cameraFreeSpace->setPath(d->controller->cameraPath());
    }

    statusBar()->addWidget(d->cameraFreeSpace, 1);

    //------------------------------------------------------------------------------

    d->albumLibraryFreeSpace = new FreeSpaceWidget(statusBar(), 100);
    d->albumLibraryFreeSpace->setMode(FreeSpaceWidget::AlbumLibrary);
    statusBar()->addWidget(d->albumLibraryFreeSpace, 1);
    refreshCollectionFreeSpace();

    //------------------------------------------------------------------------------

    // TODO: Replace it with FilterStatusBar after advanced filtering is implemented.

    d->filterComboBox = new ImportFilterComboBox(statusBar());
    setFilter(d->filterComboBox->currentFilter());
    statusBar()->addWidget(d->filterComboBox, 1);

    connect(d->filterComboBox, SIGNAL(signalFilterChanged(Filter*)),
            this, SLOT(setFilter(Filter*)));

    //------------------------------------------------------------------------------

    d->zoomBar = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->increaseThumbsAction);
    d->zoomBar->setZoomMinusAction(d->decreaseThumbsAction);
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    statusBar()->addPermanentWidget(d->zoomBar, 1);
}

void ImportUI::setupCameraController(const QString& model, const QString& port, const QString& path)
{
    d->controller = new CameraController(this, d->cameraTitle, model, port, path);

    connect(d->controller, SIGNAL(signalConnected(bool)),
            this, SLOT(slotConnected(bool)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalLogMsg(QString,DHistoryView::EntryType,QString,QString)),
            this, SLOT(slotLogMsg(QString,DHistoryView::EntryType,QString,QString)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalCameraInformation(QString,QString,QString)),
            this, SLOT(slotCameraInformation(QString,QString,QString)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalFolderList(QStringList)),
            this, SLOT(slotFolderList(QStringList)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalDownloaded(QString,QString,QString,int)),
            this, SLOT(slotDownloaded(QString,QString,QString,int)),
            Qt::BlockingQueuedConnection);

    connect(d->controller, SIGNAL(signalDeleted(QString,QString,bool)),
            this, SLOT(slotDeleted(QString,QString,bool)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalLocked(QString,QString,bool)),
            this, SLOT(slotLocked(QString,QString,bool)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalMetadata(QString,QString,MetaEngineData)),
            this, SLOT(slotMetadata(QString,QString,MetaEngineData)),
            Qt::QueuedConnection);

    connect(d->controller, SIGNAL(signalUploaded(CamItemInfo)),
            this, SLOT(slotUploaded(CamItemInfo)),
            Qt::QueuedConnection);

    d->controller->start();

    // Setup Thumbnails controller -------------------------------------------------------

    d->camThumbsCtrl = new CameraThumbsCtrl(d->controller, this);
}

CameraThumbsCtrl* ImportUI::getCameraThumbsCtrl() const
{
    return d->camThumbsCtrl;
}

void ImportUI::setupAccelerators()
{
    KActionCollection* const ac = actionCollection();

    QAction* const escapeAction = new QAction(i18nc("@action", "Exit Preview Mode"), this);
    ac->addAction(QLatin1String("exit_preview_mode"), escapeAction);
    ac->setDefaultShortcut(escapeAction, Qt::Key_Escape);
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    QAction* const nextImageAction = new QAction(i18nc("@action","Next Image"), this);
    nextImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    ac->addAction(QLatin1String("next_image"), nextImageAction);
    ac->setDefaultShortcut(nextImageAction, Qt::Key_Space);
    connect(nextImageAction, SIGNAL(triggered()), d->view, SLOT(slotNextItem()));

    QAction* const previousImageAction = new QAction(i18nc("@action", "Previous Image"), this);
    previousImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    ac->addAction(QLatin1String("previous_image"), previousImageAction);
    ac->setDefaultShortcuts(previousImageAction, QList<QKeySequence>() << Qt::Key_Backspace << (Qt::SHIFT | Qt::Key_Space));
    connect(previousImageAction, SIGNAL(triggered()), d->view, SLOT(slotPrevItem()));

    QAction* const firstImageAction = new QAction(i18nc("@action Go to first image", "First Image"), this);
    ac->addAction(QLatin1String("first_image"), firstImageAction);
    ac->setDefaultShortcut(firstImageAction, Qt::Key_Home);
    connect(firstImageAction, SIGNAL(triggered()), d->view, SLOT(slotFirstItem()));

    QAction* const lastImageAction = new QAction(i18nc("@action Go to last image", "Last Image"), this);
    ac->addAction(QLatin1String("last_image"), lastImageAction);
    ac->setDefaultShortcut(lastImageAction, Qt::Key_End);
    connect(lastImageAction, SIGNAL(triggered()), d->view, SLOT(slotLastItem()));
}

void ImportUI::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    readFullScreenSettings(group);

    d->showBarAction->setChecked(ImportSettings::instance()->getShowThumbbar());
    d->showLogAction->setChecked(group.readEntry(QLatin1String("ShowLog"), false));
    d->albumCustomizer->readSettings(group);
    d->advancedSettings->readSettings(group);

#ifdef HAVE_JXL

    d->dngConvertSettings->readSettings(group);

#endif

    d->scriptingSettings->readSettings(group);

    d->advBox->readSettings(group);

    d->splitter->restoreState(group);

    slotShowLog();
}

void ImportUI::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    ImportSettings::instance()->setShowThumbbar(d->showBarAction->isChecked());
    ImportSettings::instance()->saveSettings();
    group.writeEntry(QLatin1String("ShowLog"), d->showLogAction->isChecked());
    d->albumCustomizer->saveSettings(group);
    d->advancedSettings->saveSettings(group);

#ifdef HAVE_JXL

    d->dngConvertSettings->saveSettings(group);

#endif

    d->scriptingSettings->saveSettings(group);

    d->advBox->writeSettings(group);

    d->rightSideBar->saveState();
    d->splitter->saveState(group);
    d->filterComboBox->saveSettings();

    config->sync();
}

bool ImportUI::isBusy() const
{
    return d->busy;
}

bool ImportUI::isClosed() const
{
    return d->closed;
}

QString ImportUI::cameraTitle() const
{
    return d->cameraTitle;
}

DownloadSettings ImportUI::downloadSettings() const
{
    DownloadSettings settings = d->advancedSettings->settings();

#ifdef HAVE_JXL

    d->dngConvertSettings->settings(&settings);

#endif

    d->scriptingSettings->settings(&settings);

    return settings;
}

void ImportUI::setInitialSorting()
{
    d->view->slotSeparateImages(ImportSettings::instance()->getImageSeparationMode());
    d->view->slotSortImagesBy(ImportSettings::instance()->getImageSortBy());
    d->view->slotSortImagesOrder(ImportSettings::instance()->getImageSortOrder());
}

void ImportUI::slotCancelButton()
{
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                             i18nc("@info:status", "Canceling current operation, please wait..."));
    d->controller->slotCancel();

    d->currentlyDeleting.clear();

    postProcessAfterDownload();

    refreshFreeSpace();
}

void ImportUI::refreshFreeSpace()
{
    if (cameraUseGPhotoDriver())
    {
        d->controller->getFreeSpace();
    }
    else
    {
        d->cameraFreeSpace->refresh();
    }
}

void ImportUI::closeEvent(QCloseEvent* e)
{
    if (dialogClosed())
    {
        DXmlGuiWindow::closeEvent(e);
    }
    else
    {
        e->ignore();
    }
}

void ImportUI::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)

    Q_EMIT signalWindowHasMoved();
}

bool ImportUI::dialogClosed()
{
    if (d->closed)
    {
        return true;
    }

    if (isBusy())
    {
        if (QMessageBox::question(this, qApp->applicationName(),
                                  i18nc("@info", "Do you want to close the dialog "
                                        "and cancel the current operation?"),
                                  QMessageBox::Yes | QMessageBox::No
                                 ) == QMessageBox::No)
        {
            return false;
        }
    }

    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                             i18nc("@info:status", "Disconnecting from camera, please wait..."));

    if (d->waitAutoRotate)
    {
        hide();
        d->waitAutoRotate = false;

        return false;
    }

    if (isBusy())
    {
        d->controller->slotCancel();

        // will be read in slotBusy later and finishDialog
        // will be called only when everything is finished

        d->closed = true;
    }
    else
    {
        d->closed = true;
        finishDialog();
    }

    return true;
}

void ImportUI::finishDialog()
{
    // Look if an item have been downloaded to computer during camera GUI session.
    // If yes, update the starting number value used to rename camera items from camera list.

    if (d->view->downloadedCamItemInfos() > 0)
    {
        CameraList* const clist = CameraList::defaultList();

        if (clist)
        {
            clist->changeCameraStartIndex(d->cameraTitle, d->renameCustomizer->startIndex());
        }
    }

    deleteLater();

    if (!d->lastDestURL.isEmpty())
    {
        Q_EMIT signalLastDestination(d->lastDestURL);
    }

    saveSettings();
}

void ImportUI::slotBusy(bool val)
{
    if (!val)   // Camera is available for actions.
    {
        if (!d->busy)
        {
            return;
        }

        d->busy = false;
        d->view->setEnabled(true);
        d->advBox->setEnabled(true);
        d->cameraActions->setEnabled(true);
        d->cameraCancelAction->setEnabled(false);

        // selection-dependent update of lockAction, markAsDownloadedAction,
        // downloadSelectedAction, downloadDelSelectedAction, deleteSelectedAction

        updateActions();

        m_animLogo->stop();
        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode, d->statusBarText     +
                                                                              QLatin1String(" - ") +
                                                                              i18nc("@info:status busy state", "Ready"));

        // like WDestructiveClose, but after camera controller operation has safely finished

        if (d->closed)
        {
            finishDialog();
        }
    }
    else    // Camera is busy.
    {
        if (d->busy)
        {
            return;
        }

        if (!m_animLogo->running())
        {
            m_animLogo->start();
        }

        d->busy = true;
        d->cameraActions->setEnabled(false);
    }
}

void ImportUI::slotZoomSliderChanged(int size)
{
    d->view->setThumbSize(size);
}

void ImportUI::slotZoomChanged(double zoom)
{
    double zmin = d->view->zoomMin();
    double zmax = d->view->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);

    if (!fullScreenIsActive())
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void ImportUI::slotThumbSizeChanged(int size)
{
    d->zoomBar->setThumbsSize(size);

    if (!fullScreenIsActive())
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void ImportUI::slotConnected(bool val)
{
    if (!val)
    {
        d->errorWidget->setText(i18nc("@info", "Failed to connect to the camera. "
                                               "Please make sure it is connected "
                                               "properly and turned on."));

        d->errorWidget->clearAllActions();
        d->errorWidget->addAction(d->connectAction);
        d->errorWidget->addAction(d->showPreferencesAction);
        d->errorWidget->animatedShow();
    }
    else
    {
        // disable unsupported actions

        d->uploadAction->setEnabled(d->controller->cameraUploadSupport());

        d->cameraCaptureAction->setEnabled(d->controller->cameraCaptureImageSupport());

        d->errorWidget->hide();
        refreshFreeSpace();

        // FIXME: ugly c&p from slotFolderList

        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(d->configGroupName);
        bool useMetadata          = group.readEntry(d->configUseFileMetadata, false);
        d->controller->listRootFolder(useMetadata);
    }
}

void ImportUI::slotFolderList(const QStringList& folderList)
{
    if (d->closed)
    {
        return;
    }

    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(0);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    bool useMetadata          = group.readEntry(d->configUseFileMetadata, false);

    // when getting a list of subfolders, request their contents and also their subfolders

    for (QStringList::const_iterator it = folderList.constBegin() ;
         it != folderList.constEnd() ; ++it)
    {
        d->controller->listFiles(*it, useMetadata);
        d->controller->listFolders(*it);
    }
}

void ImportUI::setFilter(Filter* filter)
{
    d->view->importFilterModel()->setFilter(filter);
}

void ImportUI::slotCapture()
{
    if (d->busy)
    {
        return;
    }

    CaptureDlg* const captureDlg = new CaptureDlg(this, d->controller, d->cameraTitle);
    captureDlg->show();
}

void ImportUI::slotInformation()
{
    if (d->busy)
    {
        return;
    }

    d->controller->getCameraInformation();
}

void ImportUI::slotCameraInformation(const QString& summary, const QString& manual, const QString& about)
{
    CameraInfoDialog* const infoDlg = new CameraInfoDialog(this, summary, manual, about);
    infoDlg->show();
}

void ImportUI::slotUpload()
{
    if (d->busy)
    {
        return;
    }

    QList<QUrl> urls = ImageDialog::getImageURLs(this,
                                                 QUrl::fromLocalFile(CollectionManager::instance()->oneAlbumRootPath()),
                                                 i18nc("@title:window", "Select Image to Upload"));

    if (!urls.isEmpty())
    {
        slotUploadItems(urls);
    }
}

void ImportUI::slotUploadItems(const QList<QUrl>& urls)
{
    if (d->busy)
    {
        return;
    }

    if (urls.isEmpty())
    {
        return;
    }

    if (d->cameraFreeSpace->isValid())
    {
        // Check if space require to upload new items in camera is enough.

        qint64 totalBytesSize = 0;

        for (QList<QUrl>::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
        {
            QFileInfo fi((*it).toLocalFile());
            totalBytesSize += fi.size();
        }

        if (totalBytesSize >= d->cameraFreeSpace->bytesAvail())
        {
            QMessageBox::critical(this, qApp->applicationName(),
                                  i18nc("@info", "There is not enough free space on the Camera Medium "
                                        "to upload pictures.\n\n"
                                        "Space require: %1\n"
                                        "Available free space: %2",
                                        ItemPropertiesTab::humanReadableBytesCount(totalBytesSize),
                                        ItemPropertiesTab::humanReadableBytesCount(d->cameraFreeSpace->bytesAvail())));
            return;
        }
    }

    QMap<QString, int> map           = countItemsByFolders();
    QPointer<CameraFolderDialog> dlg = new CameraFolderDialog(this, map, d->controller->cameraTitle(),
                                                              d->controller->cameraPath());

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return;
    }

    // since we access members here, check if the pointer is still valid

    if (!dlg)
    {
        return;
    }

    QString cameraFolder = dlg->selectedFolderPath();

    for (QList<QUrl>::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        QFileInfo fi((*it).toLocalFile());

        if (!fi.exists())
        {
            continue;
        }

        if (fi.isDir())
        {
            continue;
        }

        QString ext  = QLatin1Char('.') + fi.completeSuffix();
        QString name = fi.fileName();
        name.truncate(fi.fileName().length() - ext.length());

        bool ok;

        CamItemInfo uploadInfo;
        uploadInfo.folder = cameraFolder;
        uploadInfo.name   = name + ext;

        while (d->view->hasImage(uploadInfo))
        {
            QString msg(i18nc("@info", "Camera Folder \"%1\" already contains the item \"%2\".\n"
                              "Please enter a new filename (without extension):",
                              QDir::toNativeSeparators(cameraFolder), fi.fileName()));
            uploadInfo.name = QInputDialog::getText(this,
                                                    i18nc("@title:window", "File Already Exists"),
                                                    msg,
                                                    QLineEdit::Normal,
                                                    name,
                                                    &ok) + ext;

            if (!ok)
            {
                return;
            }
        }

        d->controller->upload(fi, uploadInfo.name, cameraFolder);
    }

    delete dlg;
}

void ImportUI::slotUploaded(const CamItemInfo& /*itemInfo*/)
{
    if (d->closed)
    {
        return;
    }

    refreshFreeSpace();
}

void ImportUI::slotDownloadNew()
{
    slotSelectNew();
    QTimer::singleShot(0, this, SLOT(slotDownloadSelected()));
}

void ImportUI::slotDownloadAndDeleteNew()
{
    slotSelectNew();
    QTimer::singleShot(0, this, SLOT(slotDownloadAndDeleteSelected()));
}

void ImportUI::slotDownloadSelected()
{
    slotDownload(true, false);
}

void ImportUI::slotDownloadAndDeleteSelected()
{
    slotDownload(true, true);
}

void ImportUI::slotDownloadAll()
{
    slotDownload(false, false);
}

void ImportUI::slotDownloadAndDeleteAll()
{
    slotDownload(false, true);
}

void ImportUI::slotDownload(bool onlySelected, bool deleteAfter, Album* album)
{
    if (
        d->albumCustomizer->autoAlbumDateEnabled()                                    &&
        (d->albumCustomizer->folderDateFormat() == AlbumCustomizer::CustomDateFormat) &&
        !d->albumCustomizer->customDateFormatIsValid()
       )
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info", "Your custom target album date format is not valid. Please check your settings..."));
        return;
    }

    // See bug #143934: force to select all items to prevent problem.

    if (!onlySelected)
    {
        d->view->slotSelectAll();
    }

    // Update the download names.

    slotNewSelection(d->view->selectedUrls().count() > 0);

    // -- Get the destination album from digiKam library ---------------

    PAlbum* pAlbum = nullptr;

    if (!album)
    {
        AlbumManager* const man   = AlbumManager::instance();

        // Check if default target album option is enabled.

        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(d->configGroupName);
        bool useDefaultTarget     = group.readEntry(d->configUseDefaultTargetAlbum, false);

        if (useDefaultTarget)
        {
            PAlbum* const pa = man->findPAlbum(group.readEntry(d->configDefaultTargetAlbumId, 0));

            if (pa)
            {
                CollectionLocation cl = CollectionManager::instance()->locationForAlbumRootId(pa->albumRootId());

                if (!cl.isAvailable() || cl.isNull())
                {
                    QMessageBox::information(this, qApp->applicationName(),
                                             i18nc("@info", "Collection which host your default target album set to process "
                                                   "download from camera device is not available. Please select another one from "
                                                   "camera configuration dialog."));
                    return;
                }
            }
            else
            {
                QMessageBox::information(this, qApp->applicationName(),
                                         i18nc("@info", "Your default target album set to process download "
                                               "from camera device is not available. Please select another one from "
                                               "camera configuration dialog."));
                return;
            }

            pAlbum = pa;
        }
        else
        {
            AlbumList list = man->currentAlbums();
            int albumId    = 0;

            if (!list.isEmpty())
            {
                albumId = group.readEntry(d->configLastTargetAlbum, list.first()->globalID());
            }

            album = man->findAlbum(albumId);

            if (album && (album->type() != Album::PHYSICAL))
            {
                album = nullptr;
            }

            QString header(i18nc("@info", "Please select the destination album from the digiKam library to "
                                 "import the camera pictures into."));

            album  = AlbumSelectDialog::selectAlbum(this, dynamic_cast<PAlbum*>(album), header);
            pAlbum = dynamic_cast<PAlbum*>(album);

            if (!pAlbum)
            {
                qCDebug(DIGIKAM_IMPORTUI_LOG) << "Destination Album is null";
                return;
            }

            group.writeEntry(d->configLastTargetAlbum, album->globalID());
        }
    }
    else
    {
        pAlbum = dynamic_cast<PAlbum*>(album);
    }

    if (!pAlbum)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Destination Album is null";
        return;
    }

    CollectionLocation cl = CollectionManager::instance()->locationForAlbumRootId(pAlbum->albumRootId());

    if (!cl.isAvailable() || cl.isNull())
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info", "Collection selected as the destination album to download "
                                       "from the camera device is not available. Please select another "
                                       "destination album."));
        return;
    }

    // -- Check disk space ------------------------
    // See bug #139519: Always check free space available before to
    // download items selection from camera.

    if (!checkDiskSpace(pAlbum))
    {
        return;
    }

    // -- Prepare and download camera items ------------------------
    // Since we show camera items in reverse order, downloading need to be done also in reverse order.

    downloadCameraItems(pAlbum, onlySelected, deleteAfter);
}

void ImportUI::slotDownloaded(const QString& folder, const QString& file, const QString& temp, int status)
{
    if (status == CamItemInfo::DownloadedYes)
    {
        QDateTime creationDate;
        QFileInfo tempInfo(temp);
        QScopedPointer<DMetadata> metadata(new DMetadata);
        QUrl downloadUrl = QUrl::fromLocalFile(tempInfo.path());

        if (metadata->load(temp, true))
        {
            creationDate = metadata->getItemDateTime();

            if (creationDate.isValid())
            {
                creationDate.setTime(creationDate.time().addMSecs(metadata->getMSecsInfo()));
            }
        }

        if (!creationDate.isValid())
        {
            QDateTime ctime = asDateTimeUTC(tempInfo.birthTime());
            QDateTime mtime = asDateTimeUTC(tempInfo.lastModified());

            if      (ctime.isValid() && mtime.isValid())
            {
                creationDate = qMin(ctime, mtime);
            }
            else if (mtime.isValid())
            {
                creationDate = mtime;
            }
            else if (ctime.isValid())
            {
                creationDate = ctime;
            }
        }

        if (!createSubAlbums(downloadUrl, tempInfo.suffix(), creationDate))
        {
            downloadUrl = QUrl::fromLocalFile(tempInfo.path());
        }

        QString dest = downloadUrl.toLocalFile() + QLatin1Char('/') + tempInfo.fileName();

        if (DMetadata::hasSidecar(temp))
        {
            QString sctemp = DMetadata::sidecarPath(temp);
            QString scdest = DMetadata::sidecarPath(dest);

            if (!DFileOperations::renameFile(sctemp, scdest))
            {
                slotLogMsg(xi18n("Failed to move sidecar file for <filename>%1</filename>", tempInfo.fileName()),
                                 DHistoryView::ErrorEntry);
            }
        }

        if (!DFileOperations::renameFile(temp, dest))
        {
            slotLogMsg(xi18n("Failed to move file for <filename>%1</filename>", tempInfo.fileName()),
                             DHistoryView::ErrorEntry);
        }

        d->downloadedItemList << dest;
        d->downloadedDateHash.insert(dest, creationDate);
        d->downloadedInfoHash.insert(dest, qMakePair(folder, file));

        if (!d->foldersToScan.contains(downloadUrl.toLocalFile()))
        {
            d->foldersToScan << downloadUrl.toLocalFile();
        }
    }

    if (
        (status == CamItemInfo::DownloadedYes) ||
        (status == CamItemInfo::DownloadFailed)
       )
    {
        int curr = d->statusProgressBar->progressValue();
        d->statusProgressBar->setProgressValue(curr + 1);
    }

    CamItemInfo& info = d->view->camItemInfoRef(folder, file);

    if (!info.isNull())
    {
        setDownloaded(info, status);

        bool previewItems = ImportSettings::instance()->getPreviewItemsWhileDownload();

        if (status == CamItemInfo::DownloadStarted && previewItems)
        {
            Q_EMIT signalPreviewRequested(info, true);
        }

        if (d->rightSideBar->url() == info.url())
        {
            updateRightSideBar(info);
        }

        if (info.downloaded == CamItemInfo::DownloadedYes)
        {
            CoreDbDownloadHistory::setDownloaded(QString::fromUtf8(d->controller->cameraMD5ID()),
                                                 info.name,
                                                 info.size,
                                                 info.ctime);
        }
    }

    // Download all items is complete ?

    if (d->statusProgressBar->progressValue() == d->statusProgressBar->progressTotalSteps())
    {
        if (d->deleteAfter)
        {
            // No need passive pop-up here, because we will ask to confirm items deletion with dialog.

            QTimer::singleShot(0, this, SLOT(slotDeleteAfterDownload()));
        }

        postProcessAfterDownload();
    }
}

void ImportUI::slotMarkAsDownloaded()
{
    const auto infs = d->view->selectedCamItemInfos();

    for (const CamItemInfo& info : infs)
    {
        setDownloaded(d->view->camItemInfoRef(info.folder, info.name), CamItemInfo::DownloadedYes);

        CoreDbDownloadHistory::setDownloaded(QString::fromUtf8(d->controller->cameraMD5ID()),
                                             info.name,
                                             info.size,
                                             info.ctime);
    }
}

void ImportUI::slotToggleLock()
{
    const CamItemInfoList& list = d->view->selectedCamItemInfos();
    int count                   = list.count();

    if (count > 0)
    {
        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(count);
        d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);
    }

    for (const CamItemInfo& info : std::as_const(list))
    {
        QString folder = info.folder;
        QString file   = info.name;
        int writePerm  = info.writePermissions;
        bool lock      = true;

        // If item is currently locked, unlock it.

        if (writePerm == 0)
        {
            lock = false;
        }

        d->controller->lockFile(folder, file, lock);
    }
}

void ImportUI::slotLocked(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        CamItemInfo& info = d->view->camItemInfoRef(folder, file);

        if (!info.isNull())
        {
            toggleLock(info);

            if (d->rightSideBar->url() == info.url())
            {
                updateRightSideBar(info);
            }
        }
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr + 1);
}

void ImportUI::slotUpdateRenamePreview()
{
    const CamItemInfoList& list = d->view->selectedCamItemInfos();

    if (list.isEmpty())
    {
       d->renameCustomizer->setPreviewText(QString());

       return;
    }

    const CamItemInfo& info   = list.constFirst();
    DownloadSettings settings = downloadSettings();

    if (info.isNull())
    {
        d->renameCustomizer->setPreviewText(QString());

        return;
    }

    QString newName = info.name;

    if      (d->renameCustomizer->useDefault())
    {
        newName = d->renameCustomizer->newName(info.name);
    }
    else if (d->renameCustomizer->isEnabled())
    {
        newName = d->renameCustomizer->newName(info.url().toLocalFile());
    }

    if (settings.convertJpeg && info.mime == QLatin1String("image/jpeg"))
    {
        QFileInfo     fi(newName);
        QString ext = fi.suffix();

        if (!ext.isEmpty())
        {
            if      (ext[0].isUpper() && ext[ext.length()-1].isUpper())
            {
                ext = settings.losslessFormat.toUpper();
            }
            else if (ext[0].isUpper())
            {
                ext    = settings.losslessFormat.toLower();
                ext[0] = ext[0].toUpper();
            }
            else
            {
                ext = settings.losslessFormat.toLower();
            }

            newName = fi.completeBaseName() + QLatin1Char('.') + ext;
        }
        else
        {
            newName = newName + QLatin1Char('.') + settings.losslessFormat.toLower();
        }
    }

#ifdef HAVE_JXL

    else if (settings.convertDng && info.mime == QLatin1String("image/x-raw"))
    {
        QFileInfo     fi(newName);
        QString ext = fi.suffix();

        if (!ext.isEmpty())
        {
            if      (ext[0].isUpper() && (ext[ext.length()-1].isUpper() || ext[ext.length()-1].isDigit()))
            {
                ext = QLatin1String("DNG");
            }
            else if (ext[0].isUpper())
            {
                ext = QLatin1String("Dng");
            }
            else
            {
                ext = QLatin1String("dng");
            }

            newName = fi.completeBaseName() + QLatin1Char('.') + ext;
        }
        else
        {
            newName = newName + QLatin1Char('.') + QLatin1String("dng");
        }
    }

#endif

    CamItemInfo& refInfo = d->view->camItemInfoRef(info.folder, info.name);

    if (!refInfo.isNull())
    {
        d->renameCustomizer->setPreviewText(newName);
        refInfo.downloadName = newName;
    }
}

// FIXME: the new pictures are marked by CameraHistoryUpdater which is not working yet.

void ImportUI::slotSelectNew()
{
    CamItemInfoList toBeSelected;
    const auto infs = d->view->allItems();

    for (const CamItemInfo& info: infs)
    {
        if (info.downloaded == CamItemInfo::DownloadedNo)
        {
            toBeSelected << info;
        }
    }

    d->view->setSelectedCamItemInfos(toBeSelected);
}

void ImportUI::slotSelectLocked()
{
    CamItemInfoList toBeSelected;
    const auto infs = d->view->allItems();

    for (const CamItemInfo& info : infs)
    {
        if (info.writePermissions == 0)
        {
            toBeSelected << info;
        }
    }

    d->view->setSelectedCamItemInfos(toBeSelected);
}

void ImportUI::toggleLock(CamItemInfo& info)
{
    if (!info.isNull())
    {
        if (info.writePermissions == 0)
        {
            info.writePermissions = 1;
        }
        else
        {
            info.writePermissions = 0;
        }
    }
}

// TODO is this really necessary? why not just use the folders from listfolders call?

QMap<QString, int> ImportUI::countItemsByFolders() const
{
    QString                      path;
    QMap<QString, int>           map;
    QMap<QString, int>::iterator it;
    const auto infs = d->view->allItems();

    for (const CamItemInfo& info : infs)
    {
        path = info.folder;

        if (!path.isEmpty() && path.endsWith(QLatin1Char('/')))
        {
            path.truncate(path.length() - 1);
        }

        it = map.find(path);

        if (it == map.end())
        {
            map.insert(path, 1);
        }
        else
        {
            it.value() ++;
        }
    }

    return map;
}

void ImportUI::setDownloaded(CamItemInfo& itemInfo, int status)
{
    itemInfo.downloaded = status;
    d->progressValue    = 0;

    if (itemInfo.downloaded == CamItemInfo::DownloadStarted)
    {
        d->progressTimer->start(500);
    }
    else
    {
        d->progressTimer->stop();
    }
}

void ImportUI::slotProgressTimerDone()
{
    d->progressTimer->start(300);
}

void ImportUI::itemsSelectionSizeInfo(qint64& fSizeBytes, qint64& dSizeBytes)
{
    fSizeBytes = 0;  // Files size
    dSizeBytes = 0;  // Estimated space requires to download and process files.

    const QList<QUrl>& selected = d->view->selectedUrls();
    DownloadSettings settings   = downloadSettings();
    const auto infs = d->view->allItems();

    for (const CamItemInfo& info : infs)
    {
        if (selected.contains(info.url()))
        {
            qint64 size = info.size;

            if (size < 0) // -1 if size is not provided by camera
            {
                continue;
            }

            fSizeBytes += size;

            if (info.mime == QLatin1String("image/jpeg"))
            {
                if      (settings.convertJpeg)
                {
                    // Estimated size is around 5 x original size when JPEG=>PNG.

                    dSizeBytes += size * 5;
                }
                else if (settings.autoRotate)
                {
                    // We need a double size to perform rotation.

                    dSizeBytes += size * 2;
                }
                else
                {
                    // Real file size is added.

                    dSizeBytes += size;
                }
            }

#ifdef HAVE_JXL

            else if (settings.convertDng && info.mime == QLatin1String("image/x-raw"))
            {
                // Estimated size is around 2 x original size when RAW=>DNG.

                dSizeBytes += size * 2;
            }

#endif

            else
            {
                dSizeBytes += size;
            }

        }
    }
}

void ImportUI::checkItem4Deletion(const CamItemInfo& info, QStringList& folders, QStringList& files,
                                  CamItemInfoList& deleteList, CamItemInfoList& lockedList)
{
    if (info.writePermissions != 0)  // Item not locked ?
    {
        QString folder = info.folder;
        QString file   = info.name;
        folders.append(folder);
        files.append(file);
        deleteList.append(info);
    }
    else
    {
        lockedList.append(info);
    }
}

void ImportUI::deleteItems(bool onlySelected, bool onlyDownloaded)
{
    QStringList     folders;
    QStringList     files;
    CamItemInfoList deleteList;
    CamItemInfoList lockedList;
    const CamItemInfoList& list = onlySelected ? d->view->selectedCamItemInfos()
                                               : d->view->allItems();

    for (const CamItemInfo& info : std::as_const(list))
    {
        if (onlyDownloaded)
        {
            if (info.downloaded == CamItemInfo::DownloadedYes)
            {
                checkItem4Deletion(info, folders, files, deleteList, lockedList);
            }
        }
        else
        {
            checkItem4Deletion(info, folders, files, deleteList, lockedList);
        }
    }

    // If we want to delete some locked files, just give a feedback to user.

    if (!lockedList.isEmpty())
    {
        QString infoMsg(i18nc("@info", "The items listed below are locked by camera (read-only). "
                              "These items will not be deleted. If you really want to delete these items, "
                              "please unlock them and try again."));
        CameraMessageBox::informationList(d->camThumbsCtrl, this, i18nc("@title", "Information"), infoMsg, lockedList);
    }

    if (folders.isEmpty())
    {
        return;
    }

    QString warnMsg(i18ncp("@info", "About to delete this image.\n"
                           "Deleted file is unrecoverable.\n"
                           "Are you sure?",
                           "About to delete these %1 images.\n"
                           "Deleted files are unrecoverable.\n"
                           "Are you sure?",
                           deleteList.count()));

    if (CameraMessageBox::warningContinueCancelList(d->camThumbsCtrl,
                                                    this,
                                                    i18nc("@title: confirm delete items dialog", "Warning"),
                                                    warnMsg,
                                                    deleteList,
                                                    QLatin1String("DontAskAgainToDeleteItemsFromCamera"))
        ==  QMessageBox::Yes)
    {
        QStringList::const_iterator itFolder = folders.constBegin();
        QStringList::const_iterator itFile   = files.constBegin();

        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(deleteList.count());
        d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);

        // enable cancel action.

        d->cameraCancelAction->setEnabled(true);

        for ( ; itFolder != folders.constEnd() ; ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);

            // the currentlyDeleting list is used to prevent loading items which
            // will immanently be deleted into the sidebar and wasting time

            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

bool ImportUI::checkDiskSpace(PAlbum *pAlbum)
{
    if (!pAlbum)
    {
        return false;
    }

    qint64 fSizeBytes = 0;
    qint64 dSizeBytes = 0;
    itemsSelectionSizeInfo(fSizeBytes, dSizeBytes);
    QString albumRootPath = pAlbum->albumRootPath();
    qint64 bytesAvail     = d->albumLibraryFreeSpace->bytesAvail(albumRootPath);

    if ((bytesAvail != -1) && (dSizeBytes >= bytesAvail))
    {
        int result = QMessageBox::warning(this, i18nc("@title:window", "Insufficient Disk Space"),
                                          i18nc("@info", "There is not enough free space on the disk of the album you selected "
                                                "to download and process the selected pictures from the camera.\n\n"
                                                "Estimated space required: %1\n"
                                                "Available free space: %2\n\n"
                                                "Try Anyway?",
                                                ItemPropertiesTab::humanReadableBytesCount(dSizeBytes),
                                                ItemPropertiesTab::humanReadableBytesCount(bytesAvail)),
                                          QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::No)
        {
            return false;
        }
    }

    return true;
}

bool ImportUI::downloadCameraItems(PAlbum* pAlbum, bool onlySelected, bool deleteAfter)
{
    const CamItemInfoList& list = onlySelected ? d->view->selectedCamItemInfos()
                                               : d->view->allItems();
    DownloadSettings settings   = downloadSettings();
    QUrl url                    = pAlbum->fileUrl();
    DownloadSettingsList allItems;

    // -- Download camera items -------------------------------

    for (const CamItemInfo& info : std::as_const(list))
    {
        settings.folder     = info.folder;
        settings.file       = info.name;
        settings.mime       = info.mime;
        settings.pickLabel  = info.pickLabel;
        settings.colorLabel = info.colorLabel;
        settings.rating     = info.rating;
        settings.dest       = url.toLocalFile();

        allItems.append(settings);
    }

    if (allItems.isEmpty())
    {
        return false;
    }

    d->lastDestURL = url;
    d->statusProgressBar->setNotify(true);
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(allItems.count());
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);

    // enable cancel action.

    d->cameraCancelAction->setEnabled(true);

    // disable settings tab here instead of slotBusy:
    // Only needs to be disabled while downloading

    d->advBox->setEnabled(false);
    d->view->setEnabled(false);

    d->deleteAfter = deleteAfter;

    d->downloadedItemList.clear();
    d->downloadedDateHash.clear();
    d->downloadedInfoHash.clear();
    d->foldersToScan.clear();

    d->controller->download(allItems);

    return true;
}

bool ImportUI::createSubAlbums(QUrl& downloadUrl, const QString& suffix, const QDateTime& dateTime)
{
    bool success = true;

    if (d->albumCustomizer->autoAlbumDateEnabled())
    {
        success &= createDateBasedSubAlbum(downloadUrl, dateTime);
    }

    if (d->albumCustomizer->autoAlbumExtEnabled())
    {
        success &= createExtBasedSubAlbum(downloadUrl, suffix, dateTime.date());
    }

    return success;
}

bool ImportUI::createSubAlbum(QUrl& downloadUrl, const QString& subalbum, const QDate& date)
{
    QString errMsg;

    if (!createAutoAlbum(downloadUrl, subalbum, date, errMsg))
    {
        slotLogMsg(errMsg, DHistoryView::ErrorEntry);

        return false;
    }

    downloadUrl = downloadUrl.adjusted(QUrl::StripTrailingSlash);
    downloadUrl.setPath(downloadUrl.path() + QLatin1Char('/') + subalbum);

    return true;
}

bool ImportUI::createDateBasedSubAlbum(QUrl& downloadUrl, const QDateTime& dateTime)
{
    QString dirName;

    switch (d->albumCustomizer->folderDateFormat())
    {
        case AlbumCustomizer::TextDateFormat:
        {
            dirName = dateTime.date().toString(Qt::TextDate);
            break;
        }

        case AlbumCustomizer::LocalDateFormat:
        {
            dirName = QLocale().toString(dateTime, QLocale::ShortFormat);
            break;
        }

        case AlbumCustomizer::IsoDateFormat:
        {
            dirName = dateTime.date().toString(Qt::ISODate);
            break;
        }

        default:        // Custom
        {
            dirName = dateTime.date().toString(d->albumCustomizer->customDateFormat());
            break;
        }
    }

    return createSubAlbum(downloadUrl, dirName, dateTime.date());
}

bool ImportUI::createExtBasedSubAlbum(QUrl& downloadUrl, const QString& suffix, const QDate& date)
{
    // We use the target file suffix to compute sub-albums name to take a care about
    // conversion on the fly option.

    QString subAlbum = suffix.toUpper();

    if      (
             (subAlbum == QLatin1String("JPEG")) ||
             (subAlbum == QLatin1String("JPE"))
            )
    {
        subAlbum = QLatin1String("JPG");
    }
    else if (subAlbum == QLatin1String("TIFF"))
    {
        subAlbum = QLatin1String("TIF");
    }
    else if (
             (subAlbum == QLatin1String("MPEG")) ||
             (subAlbum == QLatin1String("MPE"))  ||
             (subAlbum == QLatin1String("MTS"))
            )
    {
        subAlbum = QLatin1String("MPG");
    }

    return createSubAlbum(downloadUrl, subAlbum, date);
}

void ImportUI::slotDeleteAfterDownload()
{
    deleteItems(true, true);
}

void ImportUI::slotDeleteSelected()
{
    deleteItems(true, false);
}

void ImportUI::slotDeleteNew()
{
    slotSelectNew();
    QTimer::singleShot(0, this, SLOT(slotDeleteSelected()));
}

void ImportUI::slotDeleteAll()
{
    deleteItems(false, false);
}

void ImportUI::slotDeleted(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        // do this after removeItem.

        d->currentlyDeleting.removeAll(folder + file);
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr + 1);
    refreshFreeSpace();
}

void ImportUI::slotMetadata(const QString& folder, const QString& file, const MetaEngineData& data)
{
    CamItemInfo info = d->view->camItemInfo(folder, file);

    if (!info.isNull())
    {
        QScopedPointer<DMetadata> meta(new DMetadata);
        meta.data()->setData(data);
        d->rightSideBar->itemChanged(info, *meta);
    }
}

void ImportUI::slotNewSelection(bool hasSelection)
{
    updateActions();

    const CamItemInfoList& list = d->view->selectedCamItemInfos();

    if (hasSelection && !list.isEmpty())
    {
        QList<ParseSettings> renameFiles;
        const CamItemInfo& info = list.constFirst();

        ParseSettings parseSettings;

        parseSettings.fileUrl      = info.url();
        parseSettings.creationTime = info.ctime;
        renameFiles.append(parseSettings);

        d->renameCustomizer->renameManager()->reset();
        d->renameCustomizer->renameManager()->addFiles(renameFiles);
        d->renameCustomizer->renameManager()->parseFiles();

        slotUpdateRenamePreview();
    }
    else
    {
        d->renameCustomizer->renameManager()->reset();
        d->renameCustomizer->setPreviewText(QString());
    }

    qint64 fSizeBytes = 0;
    qint64 dSizeBytes = 0;
    itemsSelectionSizeInfo(fSizeBytes, dSizeBytes);
    d->albumLibraryFreeSpace->setEstimatedDSizeBytes(dSizeBytes);
}

void ImportUI::slotImageSelected(const CamItemInfoList& selection, const CamItemInfoList& listAll)
{
    if (d->cameraCancelAction->isEnabled())
    {
        return;
    }

    int num_images = listAll.count();

    switch (selection.count())
    {
        case 0:
        {
            d->statusBarText = i18ncp("@info:status",
                                      "No item selected (%1 item)",
                                      "No item selected (%1 items)",
                                      num_images);

            d->rightSideBar->slotNoCurrentItem();
            break;
        }

        case 1:
        {
            // if selected item is in the list of item which will be deleted, set no current item

            if (!d->currentlyDeleting.contains(selection.first().folder + selection.first().name))
            {
                updateRightSideBar(selection.first());

                int index = listAll.indexOf(selection.first()) + 1;

                d->statusBarText = i18nc("@info:status Filename of first selected item of number of items",
                                         "\"%1\" (%2 of %3)",
                                         selection.first().url().fileName(), index, num_images);
            }
            else
            {
                d->rightSideBar->slotNoCurrentItem();
                d->statusBarText = i18ncp("@info:status",
                                          "No item selected (%1 item)",
                                          "No item selected (%1 items)",
                                          num_images);
            }

            break;
        }

        default:
        {
            d->statusBarText = i18ncp("@info:status", "%2/%1 item selected",
                                      "%2/%1 items selected",
                                      num_images, selection.count());
            break;
        }
    }

    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode, d->statusBarText);
}

void ImportUI::updateRightSideBar(const CamItemInfo& info)
{
    d->rightSideBar->itemChanged(info, DMetadata());

    if (!d->busy)
    {
        d->controller->getMetadata(info.folder, info.name);
    }
}

QString ImportUI::identifyCategoryforMime(const QString& mime)
{
    return mime.split(QLatin1Char('/')).at(0);
}

void ImportUI::postProcessAfterDownload()
{
    if (d->downloadedItemList.isEmpty())
    {
        return;
    }

    KSharedConfig::Ptr config              = KSharedConfig::openConfig();
    KConfigGroup group                     = config->group(d->configGroupName);
    SetupCamera::ConflictRule conflictRule = (SetupCamera::ConflictRule)group.readEntry(d->configFileSaveConflictRule,
                                                                                        (int)SetupCamera::DIFFNAME);

    // Is auto-rotate option checked?

    bool autoRotate = downloadSettings().autoRotate;
    QList<ParseSettings> renameFiles;
    QStringList renamedItemsList;

    for (const QString& srcFile : std::as_const(d->downloadedItemList))
    {
        ParseSettings parseSettings;

        parseSettings.fileUrl      = QUrl::fromLocalFile(srcFile);
        parseSettings.creationTime = d->downloadedDateHash.value(srcFile);
        renameFiles.append(parseSettings);
    }

    d->renameCustomizer->renameManager()->reset();
    d->renameCustomizer->renameManager()->setCutFileName(39);
    d->renameCustomizer->renameManager()->addFiles(renameFiles);
    d->renameCustomizer->renameManager()->parseFiles();

    for (const QString& srcFile : std::as_const(d->downloadedItemList))
    {
        QFileInfo srcInfo(srcFile);

        QString newName;
        QString orgName = srcInfo.fileName().mid(39);

        if (d->renameCustomizer->useDefault())
        {
            newName = d->renameCustomizer->newName(orgName);
        }
        else
        {
            newName = d->renameCustomizer->newName(srcFile);
        }

        if (newName == srcInfo.fileName())
        {
            newName = orgName;
        }

        QString dstFile = srcInfo.path() + QLatin1Char('/') + newName;
        QFileInfo dstInfo(dstFile);

        if      (dstInfo.exists() && (conflictRule == SetupCamera::SKIPFILE))
        {
            QFile::remove(srcFile);

            slotLogMsg(xi18n("Skipped file <filename>%1</filename>", dstInfo.fileName()),
                       DHistoryView::WarningEntry);

            QPair<QString, QString> fPair = d->downloadedInfoHash.value(srcFile);
            CamItemInfo& info             = d->view->camItemInfoRef(fPair.first,
                                                                    fPair.second);

            if (!info.isNull())
            {
                setDownloaded(info, CamItemInfo::DownloadedNo);
            }

            continue;
        }
        else if (conflictRule != SetupCamera::OVERWRITE)
        {
            bool newurl = false;
            dstFile     = DFileOperations::getUniqueFileUrl(QUrl::fromLocalFile(dstFile), &newurl).toLocalFile();
            dstInfo     = QFileInfo(dstFile);

            if (newurl)
            {
                slotLogMsg(xi18n("Rename file to <filename>%1</filename>", dstInfo.fileName()),
                                 DHistoryView::WarningEntry);
            }
        }

        // move the file to the destination file

        if (DMetadata::hasSidecar(srcFile))
        {
            QString sctemp = DMetadata::sidecarPath(srcFile);
            QString scdest = DMetadata::sidecarPath(dstFile);

            qCDebug(DIGIKAM_IMPORTUI_LOG) << "File" << sctemp << " has a sidecar, renaming it to " << scdest;

            // remove scdest file if it exist

            if ((sctemp != scdest) && QFile::exists(sctemp) && QFile::exists(scdest))
            {
                QFile::remove(scdest);
            }

            if (!DFileOperations::renameFile(sctemp, scdest))
            {
                slotLogMsg(xi18n("Failed to save sidecar file for <filename>%1</filename>", dstInfo.fileName()),
                           DHistoryView::ErrorEntry);
            }
        }

        // remove dest file if it exist

        if ((srcFile != dstFile) && QFile::exists(srcFile) && QFile::exists(dstFile))
        {
            QFile::remove(dstFile);
        }

        if (!DFileOperations::renameFile(srcFile, dstFile))
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Renaming " << srcFile << " to " << dstFile << " failed";

            // rename failed. delete the temp file

            QFile::remove(srcFile);

            slotLogMsg(xi18n("Failed to download <filename>%1</filename>", dstInfo.fileName()),
                       DHistoryView::ErrorEntry);
        }
        else
        {
            renamedItemsList << dstFile;
        }
    }

    NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScheduleCollectionScan, d->foldersToScan);
    tool->start();

    d->foldersToScan.clear();
    d->downloadedItemList.clear();
    d->downloadedDateHash.clear();
    d->downloadedInfoHash.clear();

    // Pop-up a notification to inform user when all is done,
    // and inform if auto-rotation will take place.

    if (autoRotate)
    {
        d->waitAutoRotate = true;

        connect(tool, &NewItemsFinder::signalComplete,
                this, [this, renamedItemsList]()
            {
                ItemInfoList infoList;

                for (const QString& dstFile : std::as_const(renamedItemsList))
                {
                    ItemInfo itemInfo = ItemInfo::fromLocalFile(dstFile);

                    if (itemInfo.format() == QLatin1String("JPG"))
                    {
                        infoList << itemInfo;
                    }
                }

                if (!infoList.isEmpty())
                {
                    FileActionMngr::instance()->transform(infoList, MetaEngineRotation::NoTransformation);
                }

                if (d->waitAutoRotate)
                {
                    d->waitAutoRotate = false;
                }
                else
                {
                    close();
                }
            }
        );

        DNotificationWrapper(QLatin1String("cameradownloaded"),
                             i18nc("@info Popup notification",
                                   "Images download finished, you can now detach "
                                   "your camera while the images are auto-rotated"),
                             this, windowTitle());
    }
    else
    {
        DNotificationWrapper(QLatin1String("cameradownloaded"),
                             i18nc("@info Popup notification",
                                   "Images download finished"),
                             this, windowTitle());
    }
}

bool ImportUI::createAutoAlbum(const QUrl& parentURL, const QString& sub,
                               const QDate& date, QString& errMsg) const
{
    QUrl url(parentURL);
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + sub);

    // first stat to see if the album exists

    QFileInfo info(url.toLocalFile());

    if (info.exists())
    {
        // now check if its really a directory

        if (info.isDir())
        {
            return true;
        }
        else
        {
            errMsg = i18nc("@info", "A file with the same name (\"%1\") already exists in folder \"%2\".",
                           sub, QDir::toNativeSeparators(parentURL.toLocalFile()));
            return false;
        }
    }

    // looks like the directory does not exist, try to create it.
    // First we make sure that the parent exists.

    PAlbum* parent = AlbumManager::instance()->findPAlbum(parentURL);

    if (!parent)
    {
        errMsg = i18nc("@info", "Failed to find Album for path \"%1\".",
                       QDir::toNativeSeparators(parentURL.toLocalFile()));
        return false;
    }

    // Create the album, with any parent albums required for the structure

    QUrl albumUrl(parentURL);
    const auto dirs = sub.split(QLatin1Char('/'), Qt::SkipEmptyParts);

    for (const QString& folder : dirs)
    {
        albumUrl      = albumUrl.adjusted(QUrl::StripTrailingSlash);
        albumUrl.setPath(albumUrl.path() + QLatin1Char('/') + folder);

        PAlbum* album = AlbumManager::instance()->findPAlbum(albumUrl);

        if (!album)
        {
            album = AlbumManager::instance()->createPAlbum(parent, folder, QString(), date, QString(), errMsg);

            if (!album)
            {
                return false;
            }
        }

        parent = album;
    }

    return true;
}

void ImportUI::slotSetup()
{
    Setup::execDialog(this);
}

void ImportUI::slotCameraFreeSpaceInfo(qint64 bytesSize, qint64 bytesAvail)
{
    d->cameraFreeSpace->addInformation(bytesSize, bytesSize - bytesAvail, bytesAvail, QString());
}

bool ImportUI::cameraDeleteSupport() const
{
    return d->controller->cameraDeleteSupport();
}

bool ImportUI::cameraUploadSupport() const
{
    return d->controller->cameraUploadSupport();
}

bool ImportUI::cameraMkDirSupport() const
{
    return d->controller->cameraMkDirSupport();
}

bool ImportUI::cameraDelDirSupport() const
{
    return d->controller->cameraDelDirSupport();
}

bool ImportUI::cameraUseUMSDriver() const
{
    return d->controller->cameraDriverType() == DKCamera::UMSDriver;
}

bool ImportUI::cameraUseGPhotoDriver() const
{
    return d->controller->cameraDriverType() == DKCamera::GPhotoDriver;
}

void ImportUI::enableZoomPlusAction(bool val)
{
    d->increaseThumbsAction->setEnabled(val);
}

void ImportUI::enableZoomMinusAction(bool val)
{
    d->decreaseThumbsAction->setEnabled(val);
}

void ImportUI::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void ImportUI::slotDBStat()
{
    showDigikamDatabaseStat();
}

void ImportUI::slotOnlineVersionCheck()
{
    Setup::onlineVersionCheck();
}

void ImportUI::refreshCollectionFreeSpace()
{
    d->albumLibraryFreeSpace->setPaths(CollectionManager::instance()->allAvailableAlbumRootPaths());
}

void ImportUI::slotCollectionLocationStatusChanged(const CollectionLocation&, int)
{
    refreshCollectionFreeSpace();
}

void ImportUI::slotToggleShowBar()
{
    // Method called in constructor: use dynamic binding

    this->showThumbBar(d->showBarAction->isChecked());
}

void ImportUI::slotLogMsg(const QString& msg, DHistoryView::EntryType type,
                          const QString& folder, const QString& file)
{
    d->statusProgressBar->setProgressText(msg);
    QStringList meta;
    meta << folder << file;
    d->historyView->addEntry(msg, type, QVariant(meta));
}

void ImportUI::slotShowLog()
{
    d->showLogAction->isChecked() ? d->historyView->show() : d->historyView->hide();
}

void ImportUI::slotHistoryEntryClicked(const QVariant& metadata)
{
    QStringList meta = metadata.toStringList();
    QString folder   = meta.at(0);
    QString file     = meta.at(1);
    d->view->scrollTo(folder, file);
}

void ImportUI::showSideBars(bool visible)
{
    visible ? d->rightSideBar->restore()
            : d->rightSideBar->backup();
}

void ImportUI::slotToggleRightSideBar()
{
    d->rightSideBar->isExpanded() ? d->rightSideBar->shrink()
                                  : d->rightSideBar->expand();
}

void ImportUI::slotPreviousRightSideBarTab()
{
    d->rightSideBar->activePreviousTab();
}

void ImportUI::slotNextRightSideBarTab()
{
    d->rightSideBar->activeNextTab();
}

void ImportUI::showThumbBar(bool visible)
{
    d->view->toggleShowBar(visible);
}

bool ImportUI::thumbbarVisibility() const
{
    return d->showBarAction->isChecked();
}

void ImportUI::slotSwitchedToPreview()
{
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->imageViewSelectionAction->setCurrentAction(d->camItemPreviewAction);
    toogleShowBar();
}

void ImportUI::slotSwitchedToIconView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageViewSelectionAction->setCurrentAction(d->iconViewAction);
    toogleShowBar();
}

void ImportUI::slotSwitchedToMapView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);

#ifdef HAVE_GEOLOCATION

    d->imageViewSelectionAction->setCurrentAction(d->mapViewAction);

#endif

    toogleShowBar();
}

void ImportUI::customizedFullScreenMode(bool set)
{
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    showStatusBarAction()->setEnabled(!set);
    set ? d->showBarAction->setEnabled(false)
        : toogleShowBar();

    d->view->toggleFullScreen(set);
}

void ImportUI::toogleShowBar()
{
    switch (d->view->viewMode())
    {
        case ImportStackedView::PreviewImageMode:
        case ImportStackedView::MediaPlayerMode:
        {
            d->showBarAction->setEnabled(true);
            break;
        }

        default:
        {
            d->showBarAction->setEnabled(false);
            break;
        }
    }
}

void ImportUI::slotSetupChanged()
{
    d->filterComboBox->updateFilter();
    setFilter(d->filterComboBox->currentFilter());
    d->view->importFilterModel()->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());

    // Load full-screen options

    KConfigGroup group = KSharedConfig::openConfig()->group(ApplicationSettings::instance()->generalConfigGroupName());
    readFullScreenSettings(group);

    d->view->applySettings();
    sidebarTabTitleStyleChanged();
}

void ImportUI::sidebarTabTitleStyleChanged()
{
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->applySettings();
}

void ImportUI::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
    d->camThumbsCtrl->clearCache();
}

void ImportUI::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

} // namespace Digikam

#include "moc_importui.cpp"
