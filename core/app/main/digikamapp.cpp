/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText:      2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText:      2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamapp_p.h"
#include "facepipelineedit.h"
#include "facebackgroundrecognition.h"
#include "systemsettings.h"

namespace Digikam
{

DigikamApp* DigikamApp::m_instance = nullptr;

DigikamApp::DigikamApp()
    : DXmlGuiWindow(nullptr),
      d            (new Private)
{
    setObjectName(QLatin1String("Digikam"));
    setConfigGroupName(ApplicationSettings::instance()->generalConfigGroupName());
    setFullScreenOptions(FS_ALBUMGUI);
    setXMLFile(QLatin1String("digikamui5.rc"));

#if (KXMLGUI_VERSION >= QT_VERSION_CHECK(5, 88, 0))

    setStateConfigGroup(configGroupName());

#endif

    m_instance         = this;
    d->config          = KSharedConfig::openConfig();
    KConfigGroup group = d->config->group(configGroupName());

#ifdef HAVE_DBUS

    new DigikamAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/Digikam"), this);
    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.digikam-") +
                                                  QString::number(QCoreApplication::instance()->applicationPid()));
#endif

    // collection scan

    if (!CollectionScanner::databaseInitialScanDone())
    {
        ScanController::instance()->completeCollectionScanDeferFiles();
    }

    if (
        ApplicationSettings::instance()->getShowSplashScreen() &&
        !qApp->isSessionRestored()
       )
    {
        d->splashScreen = new DSplashScreen();
        d->splashScreen->show();
    }

    // We need here QCoreApplication::processEvents() ?

    qApp->processEvents();

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Initializing..."));
    }

    // Ensure creation

    DNNModelManager::instance();
    AlbumManager::instance();
    LoadingCacheInterface::initialize();
    IccSettings::instance()->loadAllProfilesProperties();
    MetaEngineSettings::instance();
    DMetadataSettings::instance();
    ProgressManager::instance();
    ThumbnailLoadThread::setDisplayingWidget(this);
    DIO::instance();
    LocalizeSettings::instance();
    NetworkManager::instance();

    AIToolsPipeline::instance()->start();
    IdentityProvider::instance();
    FaceClassifier::instance();
    FaceRecognitionBackgroundController::instance();

#ifdef HAVE_GEOLOCATION

    connect(GeolocationSettings::instance(), &GeolocationSettings::signalSetupGeolocation,
            this, [this](int tab)
        {
            Setup::execGeolocation(this, tab);
        }
    );

    qCDebug(DIGIKAM_GENERAL_LOG) << GeolocationSettings::instance()->settings();

#endif

    connect(LocalizeSettings::instance(), &LocalizeSettings::signalOpenLocalizeSetup,
            this, [this]()
        {
            Setup::execLocalize(this);
        }
    );

    if (!ExifToolProcess::isCreated())
    {
        ExifToolThread* const exifToolThread = new ExifToolThread(qApp);
        exifToolThread->start();
    }

    // creation of the engine on first use - when drawing -
    // can take considerable time and cause a noticeable hang in the UI thread.

    QFontMetrics fm(font());
    fm.horizontalAdvance(QLatin1String("a"));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    connect(IccSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    d->cameraMenu      = new QMenu(this);
    d->usbMediaMenu    = new QMenu(this);
    d->cardReaderMenu  = new QMenu(this);
    d->quickImportMenu = new QMenu(this);

    d->cameraList = new CameraList(this, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                         QLatin1String("/cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType*)),
            this, SLOT(slotCameraAdded(CameraType*)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(QAction*)),
            this, SLOT(slotCameraRemoved(QAction*)));

    d->modelCollection = new DModelFactory;

    // This manager must be created after collection setup and before accelerators setup.

    d->tagsActionManager = new TagsActionMngr(this);

    // Load plugins

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Load Plugins..."));
    }

    DPluginLoader* const dpl = DPluginLoader::instance();
    dpl->init();

    // First create everything, then connect.
    // Otherwise some items may send signals and the slots can try
    // to access items which were not created yet.

    setupView();
    setupAccelerators();
    setupActions();
    setupStatusBar();

    initGui();

    setupViewConnections();
    slotColorManagementOptionsChanged();

    // Check ICC profiles repository availability

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Checking ICC repository..."));
    }

    d->validIccPath = SetupICC::iccRepositoryIsValid();

    // Clean up database if enabled in the settings

    if (
        ApplicationSettings::instance()->getCleanAtStart() &&
        CollectionScanner::databaseInitialScanDone()
       )
    {
        if (d->splashScreen)
        {
            d->splashScreen->setMessage(i18n("Clean up Database..."));
        }

        QEventLoop loop;

        DbCleaner* const tool = new DbCleaner(false, false);

        connect(tool, SIGNAL(signalComplete()),
                &loop, SLOT(quit()));

        tool->start();
        loop.exec();
    }

    // Build trash counters from all collections

    DIO::buildCollectionTrashCounters();

    // Read albums from database

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading albums..."));
    }

    if (CoreDbAccess().backend()->isOpen())
    {
        AlbumManager::instance()->startScan();
    }

    // Setting the initial menu options after all tools have been loaded

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    d->view->slotAlbumSelected(albumList);

    // preload additional windows

    preloadWindows();

    readFullScreenSettings(group);

#ifdef HAVE_KFILEMETADATA

    // Create BalooWrap object, because it need to register a listener
    // to update digiKam data when changes in Baloo occur

    BalooWrap* const baloo = BalooWrap::instance();
    Q_UNUSED(baloo);

#endif // HAVE_KFILEMETADATA

    LoadSaveThread::setInfoProvider(new DatabaseLoadSaveFileInfoProvider);

    setAutoSaveSettings(configGroupName(), true);

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080

    setupSelectToolsAction();
}

DigikamApp::~DigikamApp()
{
    d->terminating = true;

    ProgressManager::instance()->slotAbortAll();

    AIToolsPipeline::instance()->cancel();

/*
    while (!aiToolsPipeline->hasFinished())
    {
        QCoreApplication::processEvents();
    }
*/

    FacePipelineEdit::instance()->cancel();

    FaceRecognitionBackgroundController::instance()->stop();
    FaceRecognitionBackgroundController::instance()->waitForDone();

    IdentityProvider::instance()->cancel();

    FaceClassifier::instance()->cancel();

    ItemAttributesWatch::shutDown();

    // Close and delete image editor instance.

    if (ImageWindow::imageWindowCreated())
    {
        ImageWindow::imageWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        ImageWindow::imageWindow()->close();
        qApp->processEvents();
    }

    // Close and delete light table instance.

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        LightTableWindow::lightTableWindow()->close();
        qApp->processEvents();
    }

    // Close and delete survey instance.

    if (SurveyWindow::surveyWindowCreated())
    {
        SurveyWindow::surveyWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        SurveyWindow::surveyWindow()->close();
        qApp->processEvents();
    }

    // Close and delete Batch Queue Manager instance.

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        QueueMgrWindow::queueManagerWindow()->close();
        qApp->processEvents();
    }

    // Close and delete Tags Manager instance.

    if (TagsManager::isCreated())
    {
        TagsManager::instance()->close();
        delete TagsManager::internalPtr;
    }

    if (MetadataHubMngr::isCreated())
    {
        delete MetadataHubMngr::internalPtr;
    }

#ifdef HAVE_KFILEMETADATA

    if (BalooWrap::isCreated())
    {
        delete BalooWrap::internalPtr;
    }

#endif

    delete d->view;
    d->view = nullptr;

    DPluginLoader::instance()->cleanUp();

    ApplicationSettings::instance()->setRecurseAlbums(d->recurseAlbumsAction->isChecked());
    ApplicationSettings::instance()->setRecurseTags(d->recurseTagsAction->isChecked());
    ApplicationSettings::instance()->setShowThumbbar(d->showBarAction->isChecked());
    ApplicationSettings::instance()->saveSettings();

    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();
    ItemAttributesWatch::cleanUp();
    AlbumThumbnailLoader::instance()->cleanUp();
    ThumbnailLoadThread::cleanUp();
    LoadingCacheInterface::cleanUp();
    DIO::cleanUp();

    // close database server

    DatabaseServerStarter::instance()->stopServerManagerProcess();

    delete d->modelCollection;

    m_instance = nullptr;

    delete d;
}

DigikamApp* DigikamApp::instance()
{
    return m_instance;
}

ItemIconView* DigikamApp::view() const
{
    return d->view;
}

void DigikamApp::show()
{
    // Remove Splashscreen.

    if (d->splashScreen)
    {
        d->splashScreen->finish(this);
    }

    // Display application window.

    KMainWindow::show();

    // Report errors from ICC repository path.

    if (!d->validIccPath)
    {
        QString message = i18n("<p>The ICC profiles folder seems to be invalid.</p>"
                               "<p>If you want to try setting it again, choose \"Yes\" here, otherwise "
                               "choose \"No\", and the \"Color Management\" feature "
                               "will be disabled until you solve this issue.</p>");

        if (QMessageBox::warning(this, qApp->applicationName(), message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (!setupICC())
            {
                d->config->group(QLatin1String("Color Management")).writeEntry(QLatin1String("EnableCM"), false);
                d->config->sync();
            }
        }
        else
        {
            d->config->group(QLatin1String("Color Management")).writeEntry(QLatin1String("EnableCM"), false);
            d->config->sync();
        }
    }

    // Init album icon view zoom factor.

    const ApplicationSettings* const settings = ApplicationSettings::instance();

    slotThumbSizeChanged(settings->getDefaultIconSize());
    slotZoomSliderChanged(settings->getDefaultIconSize());
    d->autoShowZoomToolTip = true;

    // Enable finished the collection scan as deferred process

    if (
        settings->getScanAtStart()                  ||
        !CollectionScanner::databaseInitialScanDone()
       )
    {
        NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScanDeferredFiles);

        if (settings->getDetectFacesInNewImages())
        {
            connect(tool, SIGNAL(signalComplete()),
                    this, SLOT(slotDetectFaces()));
        }

        QTimer::singleShot(1000, tool, SLOT(start()));
    }

    if (d->splashScreen)
    {
        delete d->splashScreen;
        d->splashScreen = nullptr;
    }

    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));

    if (!parser->exifToolAvailable())
    {
        QString msg = i18n("ExifTool is not available in your digiKam installation or "
                           "is not working correctly. Go to the digiKam settings and "
                           "check the availability of the ExifTool binary file in "
                           "the metadata section.\nError message: %1",
                           parser->currentErrorString());

        Q_EMIT signalNotificationError(msg, DNotificationWidget::Warning);
    }

    QTimer::singleShot(1000, this, SLOT(slotCheckFaceTrainingVersion()));
}

void DigikamApp::restoreSession()
{
    // TODO: show and restore ImageEditor, Lighttable, and Batch Queue Manager main windows

    if (qApp->isSessionRestored())
    {
        int n = 1;

        while (KMainWindow::canBeRestored(n))
        {
            const QString className = KMainWindow::classNameOfToplevel(n);

            if (className == QLatin1String(Digikam::DigikamApp::staticMetaObject.className()))
            {
                restore(n, false);
                break;
            }

            ++n;
        }
    }
}

void DigikamApp::closeEvent(QCloseEvent* e)
{
    // may show a progress dialog to finish actions

    FileActionMngr::instance()->requestShutDown();

    // may show a progress dialog to apply pending metadata

    if (MetadataHubMngr::isCreated())
    {
        MetadataHubMngr::instance()->requestShutDown();
    }

    DXmlGuiWindow::closeEvent(e);
}

bool DigikamApp::queryClose()
{
    bool ret = true;

    if (ImageWindow::imageWindowCreated())
    {
        ret &= ImageWindow::imageWindow()->queryClose();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        ret &= QueueMgrWindow::queueManagerWindow()->queryClose();
    }

    return ret;
}

void DigikamApp::enableZoomPlusAction(bool val)
{
    d->zoomPlusAction->setEnabled(val);
}

void DigikamApp::enableZoomMinusAction(bool val)
{
    d->zoomMinusAction->setEnabled(val);
}

void DigikamApp::enableAlbumBackwardHistory(bool enable)
{
    d->backwardActionMenu->setEnabled(enable);
}

void DigikamApp::enableAlbumForwardHistory(bool enable)
{
    d->forwardActionMenu->setEnabled(enable);
}

void DigikamApp::slotAboutToShowBackwardMenu()
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->backwardActionMenu->popupMenu()->clear();

#else

    d->backwardActionMenu->menu()->clear();

#endif

    QStringList titles;
    d->view->getBackwardHistory(titles);

    for (int i = 0 ; i < titles.size() ; ++i)
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QAction* const action = d->backwardActionMenu->popupMenu()->addAction(titles.at(i));

#else

        QAction* const action = d->backwardActionMenu->menu()->addAction(titles.at(i));

#endif

        int id                = i + 1;

        connect(action, &QAction::triggered,
                this, [this, id]() { d->view->slotAlbumHistoryBack(id); });
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->forwardActionMenu->popupMenu()->clear();

#else

    d->forwardActionMenu->menu()->clear();

#endif

    QStringList titles;
    d->view->getForwardHistory(titles);

    for (int i = 0 ; i < titles.size() ; ++i)
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QAction* const action = d->forwardActionMenu->popupMenu()->addAction(titles.at(i));

#else

        QAction* const action = d->forwardActionMenu->menu()->addAction(titles.at(i));

#endif

        int id                = i + 1;

        connect(action, &QAction::triggered,
                this, [this, id]() { d->view->slotAlbumHistoryForward(id); });
    }
}

void DigikamApp::slotAlbumSelected(Album* album)
{
    if (album && !album->isTrashAlbum())
    {
        const PAlbum* const palbum = dynamic_cast<PAlbum*>(album);
        bool writeMetadata         = MetaEngineSettings::instance()->isMetadataWriteEnabled();

        if (album->type() != Album::PHYSICAL || !palbum)
        {
            // Rules if not Physical album.

            d->deleteAction->setEnabled(false);
            d->renameAction->setEnabled(false);
            d->propsEditAction->setEnabled(false);
            d->openInFileManagerAction->setEnabled(false);
            d->newAction->setEnabled(false);
            d->writeAlbumMetadataAction->setEnabled(writeMetadata);
            d->readAlbumMetadataAction->setEnabled(true);

            d->pasteItemsAction->setEnabled(!album->isRoot());

            d->selectInvertAction->setEnabled(!album->isRoot());
            d->selectNoneAction->setEnabled(!album->isRoot());
            d->selectAllAction->setEnabled(!album->isRoot());

            // Special case if Tag album.

            bool enabled = (
                            (album->type() == Album::TAG)                       &&
                            !album->isRoot()                                    &&
                            (album->id() != FaceTags::unconfirmedPersonTagId()) &&
                            (album->id() != FaceTags::unknownPersonTagId())
                           );

            d->newTagAction->setEnabled(enabled);
            d->deleteTagAction->setEnabled(enabled);
            d->editTagAction->setEnabled(enabled);
        }
        else
        {
            // Rules if Physical album.

            // We have either the abstract root album,
            // the album root album for collection base dirs, or normal albums.

            bool isRoot          = palbum->isRoot();
            bool isAlbumRoot     = palbum->isAlbumRoot();
            bool isNormalAlbum   = !isRoot && !isAlbumRoot;

            d->deleteAction->setEnabled(isNormalAlbum);
            d->renameAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->propsEditAction->setEnabled(isNormalAlbum);
            d->openInFileManagerAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->newAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->writeAlbumMetadataAction->setEnabled((isNormalAlbum || isAlbumRoot) && writeMetadata);
            d->readAlbumMetadataAction->setEnabled(isNormalAlbum || isAlbumRoot);

            d->pasteItemsAction->setEnabled(isNormalAlbum || isAlbumRoot);

            d->selectInvertAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->selectNoneAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->selectAllAction->setEnabled(isNormalAlbum || isAlbumRoot);
        }
    }
    else
    {
        // Rules if album is trash or no current album.

        d->deleteAction->setEnabled(false);
        d->renameAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInFileManagerAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->writeAlbumMetadataAction->setEnabled(false);
        d->readAlbumMetadataAction->setEnabled(false);

        d->pasteItemsAction->setEnabled(false);
        d->copyItemsAction->setEnabled(false);
        d->cutItemsAction->setEnabled(false);

        d->selectInvertAction->setEnabled(false);
        d->selectNoneAction->setEnabled(false);
        d->selectAllAction->setEnabled(false);

        d->newTagAction->setEnabled(false);
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
    }
}

void DigikamApp::slotImageSelected(const ItemInfoList& selection, const ItemInfoList& listAll)
{
    qint64 listAllFileSize               = 0;
    qint64 selectionFileSize             = 0;

    int numOfImagesInModel               = 0;
    int numImagesWithGrouped             = listAll.count();
    int numImagesWithoutGrouped          = d->view->allUrls(false).count();

    ItemInfoList selectionWithoutGrouped = d->view->selectedInfoList(true, false);

    for (const ItemInfo& info : std::as_const(selection))
    {
        // cppcheck-suppress useStlAlgorithm
        selectionFileSize += info.fileSize();
    }

    for (const ItemInfo& info : std::as_const(listAll))
    {
        // cppcheck-suppress useStlAlgorithm
        listAllFileSize += info.fileSize();
    }

    const Album* const album = d->view->currentAlbum();

    if (album && (album->type() == Album::PHYSICAL))
    {
        numOfImagesInModel = d->view->itemCount();
    }

    QString statusBarSelectionText;
    QString statusBarSelectionToolTip;

    switch (selection.count())
    {
        case 0:
        {
            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = i18np("No item selected (%1 item)",
                                               "No item selected (%1 items)",
                                               numImagesWithoutGrouped);
                break;
            }

            statusBarSelectionText    = i18np("No item selected (%1 [%2] item)",
                                              "No item selected (%1 [%2] items)",
                                              numImagesWithoutGrouped,
                                              numImagesWithGrouped);

            statusBarSelectionToolTip = i18np("No item selected (%1 item. With grouped items: %2)",
                                              "No item selected (%1 items. With grouped items: %2)",
                                              numImagesWithoutGrouped,
                                              numImagesWithGrouped);
            break;
        }

        default:
        {
            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = i18np("1/%2 item selected (%3/%4)",
                                               "%1/%2 items selected (%3/%4)",
                                               selection.count(),
                                               numImagesWithoutGrouped,
                                               ItemPropertiesTab::humanReadableBytesCount(selectionFileSize),
                                               ItemPropertiesTab::humanReadableBytesCount(listAllFileSize));
                break;
            }

            if (selectionWithoutGrouped.count() > 1)
            {
                if (selection.count() == selectionWithoutGrouped.count())
                {
                    statusBarSelectionText    = i18np("1/%2 [%3] item selected (%4/%5)",
                                                      "%1/%2 [%3] items selected (%4/%5)",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      numImagesWithGrouped,
                                                      ItemPropertiesTab::humanReadableBytesCount(selectionFileSize),
                                                      ItemPropertiesTab::humanReadableBytesCount(listAllFileSize));

                    statusBarSelectionToolTip = i18np("1/%2 item selected. Total with grouped items: %3",
                                                      "%1/%2 items selected. Total with grouped items: %3",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      numImagesWithGrouped);
                }
                else
                {
                    statusBarSelectionText    = i18np("1/%2 [%3/%4] item selected (%5/%6)",
                                                      "%1/%2 [%3/%4] items selected (%5/%6)",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      selection.count(),
                                                      numImagesWithGrouped,
                                                      ItemPropertiesTab::humanReadableBytesCount(selectionFileSize),
                                                      ItemPropertiesTab::humanReadableBytesCount(listAllFileSize));

                    statusBarSelectionToolTip = i18np("1/%2 item selected. With grouped items: %3/%4",
                                                      "%1/%2 items selected. With grouped items: %3/%4",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      selection.count(),
                                                      numImagesWithGrouped);
                }

                break;
            }

#if __GNUC__ >= 7   // krazy:exclude=cpp

            // no break; is completely intentional, arriving here is equivalent to case 1:
            [[fallthrough]];

#endif
        }

        case 1:
        {
            if (!selectionWithoutGrouped.isEmpty())
            {
                slotSetCheckedExifOrientationAction(selectionWithoutGrouped.first());
            }

            int index = listAll.indexOf(selection.first()) + 1;

            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = selection.first().fileUrl().fileName()
                                            + i18n(" (%1 of %2)",
                                                   index, numImagesWithoutGrouped);
            }
            else
            {
                int indexWithoutGrouped = 0;

                if (!selectionWithoutGrouped.isEmpty())
                {
                    indexWithoutGrouped = d->view->allInfo(false).indexOf(selectionWithoutGrouped.first()) + 1;
                }

                statusBarSelectionText
                        = selection.first().fileUrl().fileName()
                          + i18n(" (%1 of %2 [%3] )", indexWithoutGrouped,
                                 numImagesWithoutGrouped, numImagesWithGrouped);
                statusBarSelectionToolTip
                        = selection.first().fileUrl().fileName()
                          + i18n(" (%1 of %2. Total with grouped items: %3)", indexWithoutGrouped,
                                 numImagesWithoutGrouped, numImagesWithGrouped);
            }

            break;
        }
    }

    if (numImagesWithGrouped < numOfImagesInModel)
    {
        statusBarSelectionText += QLatin1String(" - ");
        statusBarSelectionText += i18np("%1 item hidden", "%1 items hidden",
                                        numOfImagesInModel - numImagesWithGrouped);
    }

    d->statusLabel->setText(statusBarSelectionText);
    d->statusLabel->setToolTip(statusBarSelectionToolTip);
}

void DigikamApp::slotTrashSelectionChanged(const QString& text)
{
    d->statusLabel->setText(text);
    d->statusLabel->setToolTip(text);
}

void DigikamApp::slotSelectionChanged(int selectionCount)
{
    // The preview can either be activated when only one image is selected,
    // or if multiple images are selected, but one image is the 'current image'.

    bool hasAtLeastCurrent = (selectionCount == 1) || ((selectionCount > 0) && d->view->hasCurrentItem());
    bool writeMetadata     = MetaEngineSettings::instance()->isMetadataWriteEnabled();

    d->imagePreviewAction->setEnabled(hasAtLeastCurrent);
    d->imageViewAction->setEnabled(hasAtLeastCurrent);

    // show/shide face actions depending on the settings

    d->imageScanForFacesAction->setEnabled(selectionCount > 0);
    d->imageScanForFacesAction->setVisible(SystemSettings(qApp->applicationName()).enableFaceEngine);
    d->imageRecognizeFacesAction->setEnabled(selectionCount > 0);
    d->imageRecognizeFacesAction->setVisible(SystemSettings(qApp->applicationName()).enableFaceEngine);

    d->imageRemoveAllFacesAction->setEnabled(selectionCount > 0);
    d->imageFindSimilarAction->setEnabled(selectionCount == 1);
    d->imageRenameAction->setEnabled(selectionCount > 0);
    d->imageLightTableAction->setEnabled(selectionCount > 0);
    d->imageAddLightTableAction->setEnabled(selectionCount > 0);
    d->imageAddCurrentQueueAction->setEnabled((selectionCount > 0) && !QueueMgrWindow::queueManagerWindow()->isBusy());
    d->imageAddNewQueueAction->setEnabled((selectionCount > 0) && !QueueMgrWindow::queueManagerWindow()->isBusy());
    d->imageWriteMetadataAction->setEnabled((selectionCount > 0) && writeMetadata);
    d->imageReadMetadataAction->setEnabled(selectionCount > 0);
    d->imageDeleteAction->setEnabled(selectionCount > 0);
    d->imageRotateActionMenu->setEnabled(selectionCount > 0);
    d->imageFlipActionMenu->setEnabled(selectionCount > 0);
    d->imageExifOrientationActionMenu->setEnabled(selectionCount > 0);
    d->moveSelectionToAlbumAction->setEnabled(selectionCount > 0);
    d->copySelectionToAction->setEnabled(selectionCount > 0);
    d->cutItemsAction->setEnabled(selectionCount > 0);
    d->copyItemsAction->setEnabled(selectionCount > 0);
    d->openWithAction->setEnabled(selectionCount > 0);
    d->imageAutoExifActionMenu->setEnabled(selectionCount > 0);
    const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::GenericMetadata, this);

    for (DPluginAction* const ac : acs)
    {
        ac->setEnabled(selectionCount > 0);
    }

    if (selectionCount > 0)
    {
        d->imageWriteMetadataAction->setText(i18np("Write Metadata to File",
                                                   "Write Metadata to Selected Files", selectionCount));
        d->imageReadMetadataAction->setText(i18np("Reread Metadata From File",
                                                  "Reread Metadata From Selected Files", selectionCount));

        slotResetExifOrientationActions();
    }
}

void DigikamApp::slotExit()
{
    close();
}

void DigikamApp::slotDBStat()
{
    showDigikamDatabaseStat();
}

void DigikamApp::slotRecurseAlbums(bool checked)
{
    d->view->setRecurseAlbums(checked);
}

void DigikamApp::slotRecurseTags(bool checked)
{
    d->view->setRecurseTags(checked);
}

void DigikamApp::slotAllGroupsOpen(bool checked)
{
    d->view->setAllGroupsOpen(checked);
}

void DigikamApp::slotZoomSliderChanged(int size)
{
    d->view->setThumbSize(size);
}

void DigikamApp::slotThumbSizeChanged(int size)
{
    d->zoomBar->setThumbsSize(size);

    if (!fullScreenIsActive() && d->autoShowZoomToolTip)
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void DigikamApp::slotZoomChanged(double zoom)
{
    double zmin = d->view->zoomMin();
    double zmax = d->view->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);

    if (!fullScreenIsActive() && d->autoShowZoomToolTip)
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void DigikamApp::slotToggleShowBar()
{
    d->view->toggleShowBar(d->showBarAction->isChecked());
}

void DigikamApp::moveEvent(QMoveEvent*)
{
    Q_EMIT signalWindowHasMoved();
}

void DigikamApp::slotTransformAction()
{
    if      (sender()->objectName() == QLatin1String("rotate_auto"))
    {
        d->view->imageTransform(MetaEngineRotation::RotateAuto);
    }
    else if (sender()->objectName() == QLatin1String("rotate_ccw"))
    {
        d->view->imageTransform(MetaEngineRotation::Rotate270);
    }
    else if (sender()->objectName() == QLatin1String("rotate_cw"))
    {
        d->view->imageTransform(MetaEngineRotation::Rotate90);
    }
    else if (sender()->objectName() == QLatin1String("flip_horizontal"))
    {
        d->view->imageTransform(MetaEngineRotation::FlipHorizontal);
    }
    else if (sender()->objectName() == QLatin1String("flip_vertical"))
    {
        d->view->imageTransform(MetaEngineRotation::FlipVertical);
    }
    else if (sender()->objectName() == QLatin1String("image_transform_exif"))
    {
        // special value for FileActionMngr

        d->view->imageTransform(MetaEngineRotation::NoTransformation);
    }
}

void DigikamApp::slotResetExifOrientationActions()
{
    d->imageSetExifOrientation1Action->setChecked(false);
    d->imageSetExifOrientation2Action->setChecked(false);
    d->imageSetExifOrientation3Action->setChecked(false);
    d->imageSetExifOrientation4Action->setChecked(false);
    d->imageSetExifOrientation5Action->setChecked(false);
    d->imageSetExifOrientation6Action->setChecked(false);
    d->imageSetExifOrientation7Action->setChecked(false);
    d->imageSetExifOrientation8Action->setChecked(false);
}

void DigikamApp::slotSetCheckedExifOrientationAction(const ItemInfo& info)
{
/*
    QScopedPointer<DMetadata> meta(new DMetadata(info.fileUrl().toLocalFile()));
    int orientation = (meta->isEmpty()) ? 0 : meta->getItemOrientation();
*/
    int orientation = info.orientation();

    switch (orientation)
    {
        case 1:
        {
            d->imageSetExifOrientation1Action->setChecked(true);
            break;
        }

        case 2:
        {
            d->imageSetExifOrientation2Action->setChecked(true);
            break;
        }

        case 3:
        {
            d->imageSetExifOrientation3Action->setChecked(true);
            break;
        }

        case 4:
        {
            d->imageSetExifOrientation4Action->setChecked(true);
            break;
        }

        case 5:
        {
            d->imageSetExifOrientation5Action->setChecked(true);
            break;
        }

        case 6:
        {
            d->imageSetExifOrientation6Action->setChecked(true);
            break;
        }

        case 7:
        {
            d->imageSetExifOrientation7Action->setChecked(true);
            break;
        }

        case 8:
        {
            d->imageSetExifOrientation8Action->setChecked(true);
            break;
        }

        default:
        {
            slotResetExifOrientationActions();
            break;
        }
    }
}

void DigikamApp::showSideBars(bool visible)
{
    visible ? d->view->showSideBars()
            : d->view->hideSideBars();
}

void DigikamApp::slotToggleLeftSideBar()
{
    d->view->toggleLeftSidebar();
}

void DigikamApp::slotToggleRightSideBar()
{
    d->view->toggleRightSidebar();
}

void DigikamApp::slotPreviousLeftSideBarTab()
{
    d->view->previousLeftSideBarTab();
}

void DigikamApp::slotNextLeftSideBarTab()
{
    d->view->nextLeftSideBarTab();
}

void DigikamApp::slotNextRightSideBarTab()
{
    d->view->nextRightSideBarTab();
}

void DigikamApp::slotPreviousRightSideBarTab()
{
    d->view->previousRightSideBarTab();
}

void DigikamApp::showThumbBar(bool visible)
{
    view()->toggleShowBar(visible);
}

bool DigikamApp::thumbbarVisibility() const
{
    return d->showBarAction->isChecked();
}

void DigikamApp::slotSwitchedToPreview()
{
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->imagePreviewAction->setChecked(true);
    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToIconView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageIconViewAction->setChecked(true);
    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToMapView()
{
    // TODO: Link to map view's zoom actions

    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);

#ifdef HAVE_GEOLOCATION

    d->imageMapViewAction->setChecked(true);

#endif // HAVE_GEOLOCATION

    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToTableView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageTableViewAction->setChecked(true);
    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToTrashView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    customizedTrashView(false);
    toggleShowBar();
}

void DigikamApp::customizedFullScreenMode(bool set)
{
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    showStatusBarAction()->setEnabled(!set);

    set ? d->showBarAction->setEnabled(false)
        : toggleShowBar();

    d->view->toggleFullScreen(set);
}

void DigikamApp::customizedTrashView(bool set)
{
    d->imageTableViewAction->setEnabled(set);
    d->imageIconViewAction->setEnabled(set);

#ifdef HAVE_GEOLOCATION

    d->imageMapViewAction->setEnabled(set);

#endif

    d->imagePreviewAction->setEnabled(set);
    d->bqmAction->setEnabled(set);
    d->ltAction->setEnabled(set);
    d->svAction->setEnabled(set);
    d->ieAction->setEnabled(set);

    d->imageSeparationSortOrderAction->setEnabled(set);
    d->imageSeparationAction->setEnabled(set);
    d->imageSortOrderAction->setEnabled(set);
    d->imageSortAction->setEnabled(set);
    d->albumSortAction->setEnabled(set);

    d->zoomFitToWindowAction->setEnabled(set);
    d->recurseAlbumsAction->setEnabled(set);
    d->recurseTagsAction->setEnabled(set);
    d->refreshAction->setEnabled(set);
    const auto acs = DPluginLoader::instance()->pluginsActions(DPluginAction::GenericView, this);

    for (DPluginAction* const ac : acs)
    {
        ac->setEnabled(set);
    }
}

void DigikamApp::toggleShowBar()
{
    switch (d->view->viewMode())
    {
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
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

void DigikamApp::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void DigikamApp::slotOnlineVersionCheck()
{
    Setup::onlineVersionCheck();
}

void DigikamApp::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
}

void DigikamApp::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    ThumbnailLoadThread::setDisplayingWidget(this);

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

DInfoInterface* DigikamApp::infoIface(DPluginAction* const ac)
{
    OperationType aset = UnspecifiedOps;

    switch (ac->actionCategory())
    {
        case DPluginAction::GenericExport:
        case DPluginAction::GenericImport:
        {
            aset = ImportExportOps;
            break;
        }

        case DPluginAction::GenericMetadata:
        {
            aset = MetadataOps;
            break;
        }

        case DPluginAction::GenericTool:
        {
            aset = ToolsOps;
            break;
        }

        case DPluginAction::GenericView:
        {
            aset = SlideshowOps;
            break;
        }

        default:
        {
            break;
        }
    }

    DBInfoIface* const iface = new DBInfoIface(this, QList<QUrl>(), aset);

    connect(iface, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));

    if (aset == SlideshowOps)
    {
        connect(iface, SIGNAL(signalLastItemUrl(QUrl)),
                d->view, SLOT(slotSetCurrentUrlWhenAvailable(QUrl)));
    }

    return iface;
}

} // namespace Digikam

#include "moc_digikamapp.cpp"
