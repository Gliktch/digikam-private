/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-31-01
 * Description : main digiKam interface implementation
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikamapp.h"

// Qt includes

#include <QEventLoop>
#include <QMap>
#include <QPointer>
#include <QScopedPointer>
#include <QString>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QApplication>
#include <QStringList>
#include <QDomDocument>
#include <QStandardPaths>
#include <QKeySequence>
#include <QMenuBar>
#include <QTimer>
#include <QIcon>
#include <QMessageBox>
#include <QStatusBar>
#include <QDir>
#include <QMetaType>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <kselectaction.h>
#include <ksharedconfig.h>
#include <kxmlgui_version.h>

#ifdef HAVE_KICONTHEMES
#   include <kiconthemes_version.h>
#endif

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "cameralist.h"
#include "cameratype.h"
#include "cameranamehelper.h"
#include "dsplashscreen.h"
#include "dzoombar.h"
#include "itemiconview.h"
#include "metadatastatusbar.h"
#include "itempropertiestab.h"
#include "importui.h"
#include "setup.h"
#include "actioncategorizedview.h"
#include "drawdecoder.h"
#include "dlayoutbox.h"
#include "album.h"
#include "coredb.h"
#include "albummodel.h"
#include "albumselectdialog.h"
#include "albumthumbnailloader.h"
#include "dbinfoiface.h"
#include "categorizeditemmodel.h"
#include "collectionscanner.h"
#include "collectionmanager.h"
#include "componentsinfodlg.h"
#include "coredbthumbinfoprovider.h"
#include "dio.h"
#include "dlogoaction.h"
#include "fileactionmngr.h"
#include "filterstatusbar.h"
#include "iccsettings.h"
#include "itemattributeswatch.h"
#include "iteminfo.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "surveywindow.h"
#include "queuemgrwindow.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "loadsavethread.h"
#include "metaengine_rotation.h"
#include "scancontroller.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "thememanager.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "dmetadata.h"
#include "tagscache.h"
#include "tagsactionmngr.h"
#include "databaseserverstarter.h"
#include "metaenginesettings.h"
#include "statusprogressbar.h"
#include "statusbarprogresswidget.h"
#include "dbmigrationdlg.h"
#include "progressmanager.h"
#include "progressview.h"
#include "maintenancedlg.h"
#include "maintenancemngr.h"
#include "newitemsfinder.h"
#include "dbcleaner.h"
#include "tagsmanager.h"
#include "itemsortsettings.h"
#include "metadatahubmngr.h"
#include "dfiledialog.h"
#include "dpluginloader.h"
#include "exiftoolparser.h"
#include "exiftoolthread.h"
#include "localizesettings.h"
#include "networkmanager.h"
#include "dnotificationwidget.h"
#include "dnnmodelmanager.h"
#include "faceclassifier.h"
#include "facesengine.h"
#include "identityprovider.h"
#include "aitoolspipeline.h"
#include "systemsettings.h"
#include "facetrainingupgradedlg.h"
#include "peoplesidebarwidget.h"

#ifdef HAVE_GEOLOCATION
#   include "geolocationsettings.h"
#endif

#ifdef HAVE_DBUS
#   include "digikamadaptor.h"
#endif

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

class KToolBarPopupAction;

namespace Digikam
{

class SearchTextBar;
class FilterStatusBar;
class TagsActionMngr;
class DAdjustableLabel;

class Q_DECL_HIDDEN ProgressEntry
{
public:

    ProgressEntry() = default;

public:

    QString message;
    float   progress  = 0.0;
    bool    canCancel = false;
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN DigikamApp::Private
{
public:

    Private() = default;

public:

    bool                                autoShowZoomToolTip                     = false;
    bool                                validIccPath                            = true;
    bool                                terminating                             = false;

    QMenu*                              cameraMenu                              = nullptr;
    QMenu*                              usbMediaMenu                            = nullptr;
    QMenu*                              cardReaderMenu                          = nullptr;
    QMenu*                              quickImportMenu                         = nullptr;
    QHash<QString, QDateTime>           cameraAppearanceTimes;

    KSharedConfig::Ptr                  config;

    //@{
    /// Album Actions

    QAction*                            newAction                               = nullptr;
    QAction*                            moveSelectionToAlbumAction              = nullptr;
    QAction*                            copySelectionToAction                   = nullptr;
    QAction*                            deleteAction                            = nullptr;
    QAction*                            renameAction                            = nullptr;
    QAction*                            imageDeletePermanentlyAction            = nullptr;
    QAction*                            imageDeletePermanentlyDirectlyAction    = nullptr;
    QAction*                            imageTrashDirectlyAction                = nullptr;
    KToolBarPopupAction*                backwardActionMenu                      = nullptr;
    KToolBarPopupAction*                forwardActionMenu                       = nullptr;

    QAction*                            addImagesAction                         = nullptr;
    QAction*                            propsEditAction                         = nullptr;
    QAction*                            addFoldersAction                        = nullptr;
    QAction*                            openInFileManagerAction                 = nullptr;
    QAction*                            refreshAction                           = nullptr;
    QAction*                            writeAlbumMetadataAction                = nullptr;
    QAction*                            readAlbumMetadataAction                 = nullptr;

    //@}

    //@{
    /// Tag Actions

    QAction*                            browseTagsAction                        = nullptr;
    QAction*                            openTagMngrAction                       = nullptr;
    QAction*                            newTagAction                            = nullptr;
    QAction*                            deleteTagAction                         = nullptr;
    QAction*                            editTagAction                           = nullptr;
    QAction*                            assignTagAction                         = nullptr;

    //@}

    //@{
    /// Item Actions

    KSelectAction*                      imageViewSelectionAction                = nullptr;
    QAction*                            imagePreviewAction                      = nullptr;

#ifdef HAVE_GEOLOCATION

    QAction*                            imageMapViewAction                      = nullptr;

#endif // HAVE_GEOLOCATION

    QAction*                            imageTableViewAction                    = nullptr;
    QAction*                            imageIconViewAction                     = nullptr;
    QAction*                            imageLightTableAction                   = nullptr;
    QAction*                            imageAddLightTableAction                = nullptr;
    QAction*                            imageAddCurrentQueueAction              = nullptr;
    QAction*                            imageAddNewQueueAction                  = nullptr;
    QAction*                            imageViewAction                         = nullptr;
    QAction*                            imageWriteMetadataAction                = nullptr;
    QAction*                            imageReadMetadataAction                 = nullptr;
    QAction*                            imageScanForFacesAction                 = nullptr;
    QAction*                            imageRecognizeFacesAction               = nullptr;
    QAction*                            imageRemoveAllFacesAction               = nullptr;
    QAction*                            imageFindSimilarAction                  = nullptr;
    QAction*                            imageSetExifOrientation1Action          = nullptr;
    QAction*                            imageSetExifOrientation2Action          = nullptr;
    QAction*                            imageSetExifOrientation3Action          = nullptr;
    QAction*                            imageSetExifOrientation4Action          = nullptr;
    QAction*                            imageSetExifOrientation5Action          = nullptr;
    QAction*                            imageSetExifOrientation6Action          = nullptr;
    QAction*                            imageSetExifOrientation7Action          = nullptr;
    QAction*                            imageSetExifOrientation8Action          = nullptr;
    QAction*                            imageRenameAction                       = nullptr;
    QMenu*                              imageRotateActionMenu                   = nullptr;
    QMenu*                              imageFlipActionMenu                     = nullptr;
    QAction*                            imageAutoExifActionMenu                 = nullptr;
    QAction*                            imageDeleteAction                       = nullptr;
    QMenu*                              imageExifOrientationActionMenu          = nullptr;
    QAction*                            openWithAction                          = nullptr;
    QAction*                            ieAction                                = nullptr;          // Image Editor
    QAction*                            ltAction                                = nullptr;          // Light Table
    QAction*                            svAction                                = nullptr;          // Survey

    //@}

    //@{
    /// Edit Actions

    QAction*                            cutItemsAction                          = nullptr;
    QAction*                            copyItemsAction                         = nullptr;
    QAction*                            pasteItemsAction                        = nullptr;
    QAction*                            selectAllAction                         = nullptr;
    QAction*                            selectNoneAction                        = nullptr;
    QAction*                            selectInvertAction                      = nullptr;

    //@}

    //@{
    /// View Actions

    QAction*                            zoomPlusAction                          = nullptr;
    QAction*                            zoomMinusAction                         = nullptr;
    QAction*                            zoomFitToWindowAction                   = nullptr;
    QAction*                            zoomTo100percents                       = nullptr;
    KSelectAction*                      imageSortAction                         = nullptr;
    KSelectAction*                      imageSortOrderAction                    = nullptr;
    KSelectAction*                      imageSeparationAction                   = nullptr;
    KSelectAction*                      imageSeparationSortOrderAction          = nullptr;
    KSelectAction*                      albumSortAction                         = nullptr;
    QAction*                            allGroupsOpenAction                     = nullptr;
    QAction*                            recurseAlbumsAction                     = nullptr;
    QAction*                            recurseTagsAction                       = nullptr;
    QAction*                            showBarAction                           = nullptr;
    QAction*                            viewCMViewAction                        = nullptr;

    //@}

    //@{
    /// Tools Actions

    QAction*                            bqmAction                               = nullptr;
    QAction*                            maintenanceAction                       = nullptr;
    QAction*                            scanNewItemsAction                      = nullptr;
    QAction*                            qualityAction                           = nullptr;
    QAction*                            advSearchAction                         = nullptr;

    //@}

    //@{
    /// Application Actions

    QAction*                            addCameraSeparatorAction                = nullptr;
    QAction*                            quitAction                              = nullptr;
    QAction*                            tipAction                               = nullptr;

    QActionGroup*                       manualCameraActionGroup                 = nullptr;
    QActionGroup*                       solidCameraActionGroup                  = nullptr;
    QActionGroup*                       solidUsmActionGroup                     = nullptr;
    QActionGroup*                       exifOrientationActionGroup              = nullptr;

    //@}

    QMap<QString, QPointer<ImportUI> >  cameraUIMap;

    QEventLoop*                         eventLoop                               = nullptr;
    QString                             solidErrorMessage;

    MetadataStatusBar*                  metadataStatusBar                       = nullptr;
    FilterStatusBar*                    filterStatusBar                         = nullptr;
    DSplashScreen*                      splashScreen                            = nullptr;
    ItemIconView*                       view                                    = nullptr;
    CameraList*                         cameraList                              = nullptr;
    TagsActionMngr*                     tagsActionManager                       = nullptr;
    DZoomBar*                           zoomBar                                 = nullptr;
    StatusProgressBar*                  statusLabel                             = nullptr;

    DModelFactory*                      modelCollection                         = nullptr;
};

} // namespace Digikam
