/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "importui.h"

// Qt includes

#include <QCheckBox>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeySequence>
#include <QInputDialog>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>
#include <QActionGroup>
#include <QLocale>
#include <QDateTime>
#include <QString>
#include <QUrl>

// KDE includes

#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kselectaction.h>
#include <kxmlgui_version.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "drawdecoder.h"
#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dfileselector.h"
#include "cameramessagebox.h"
#include "advancedrenamemanager.h"
#include "album.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "albumselectdialog.h"
#include "cameracontroller.h"
#include "camerafolderdialog.h"
#include "camerainfodialog.h"
#include "cameralist.h"
#include "cameranamehelper.h"
#include "cameratype.h"
#include "capturedlg.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "componentsinfodlg.h"
#include "dlogoaction.h"
#include "coredbdownloadhistory.h"
#include "dzoombar.h"
#include "fileactionmngr.h"
#include "freespacewidget.h"
#include "iccsettings.h"
#include "importitempropertiessidebar.h"
#include "importsettings.h"
#include "importview.h"
#include "imagedialog.h"
#include "dnotificationwrapper.h"
#include "newitemsfinder.h"
#include "parsesettings.h"
#include "renamecustomizer.h"
#include "scancontroller.h"
#include "setup.h"
#include "sidebar.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailsize.h"
#include "importthumbnailmodel.h"
#include "itempropertiestab.h"
#include "camerathumbsctrl.h"
#include "importfiltercombobox.h"
#include "albumcustomizer.h"
#include "advancedsettings.h"
#include "scriptingsettings.h"
#include "filterstatusbar.h"
#include "dnotificationwidget.h"
#include "dfileoperations.h"
#include "setupcamera.h"

#ifdef HAVE_JXL
#   include "dngconvertsettings.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN ImportUI::Private
{
public:

    Private() = default;

    bool                                     waitAutoRotate                 = false;
    bool                                     deleteAfter                    = false;
    bool                                     busy                           = false;
    bool                                     closed                         = false;

    QString                                  cameraTitle;
    QString                                  statusBarText;

    QHash<QString, QPair<QString, QString> > downloadedInfoHash;
    QHash<QString, QDateTime>                downloadedDateHash;
    QStringList                              downloadedItemList;
    QStringList                              currentlyDeleting;
    QStringList                              foldersToScan;

    QMenu*                                   downloadMenu                   = nullptr;
    QMenu*                                   deleteMenu                     = nullptr;
    QMenu*                                   imageMenu                      = nullptr;

    QAction*                                 cameraCancelAction             = nullptr;
    QAction*                                 cameraCaptureAction            = nullptr;
    QAction*                                 cameraInfoAction               = nullptr;
    QAction*                                 increaseThumbsAction           = nullptr;
    QAction*                                 decreaseThumbsAction           = nullptr;
    QAction*                                 zoomFitToWindowAction          = nullptr;
    QAction*                                 zoomTo100percents              = nullptr;
    QMenu*                                   deleteAction                   = nullptr;
    QAction*                                 deleteNewAction                = nullptr;
    QAction*                                 deleteAllAction                = nullptr;
    QAction*                                 deleteSelectedAction           = nullptr;
    QMenu*                                   downloadAction                 = nullptr;
    QAction*                                 downloadNewAction              = nullptr;
    QAction*                                 downloadAllAction              = nullptr;
    QAction*                                 downloadSelectedAction         = nullptr;
    QAction*                                 downloadDelNewAction           = nullptr;
    QAction*                                 downloadDelAllAction           = nullptr;
    QAction*                                 downloadDelSelectedAction      = nullptr;
    QAction*                                 lockAction                     = nullptr;
    QAction*                                 selectAllAction                = nullptr;
    QAction*                                 selectInvertAction             = nullptr;
    QAction*                                 selectLockedItemsAction        = nullptr;
    QAction*                                 selectNewItemsAction           = nullptr;
    QAction*                                 selectNoneAction               = nullptr;
    QAction*                                 uploadAction                   = nullptr;
    QAction*                                 markAsDownloadedAction         = nullptr;
    QAction*                                 resumeAction                   = nullptr;
    QAction*                                 pauseAction                    = nullptr;
    QAction*                                 connectAction                  = nullptr;
    KSelectAction*                           itemSortAction                 = nullptr;
    KSelectAction*                           itemSortOrderAction            = nullptr;
    KSelectAction*                           itemsGroupAction               = nullptr;
    QAction*                                 showPreferencesAction          = nullptr;
    QAction*                                 showLogAction                  = nullptr;
    QAction*                                 showBarAction                  = nullptr;
    KSelectAction*                           imageViewSelectionAction       = nullptr;
    QAction*                                 iconViewAction                 = nullptr;
    QAction*                                 camItemPreviewAction           = nullptr;

#ifdef HAVE_GEOLOCATION

    QAction*                                 mapViewAction                  = nullptr;

#endif // HAVE_GEOLOCATION

    QAction*                                 viewCMViewAction               = nullptr;

    QActionGroup*                            cameraActions                  = nullptr;

    QUrl                                     lastDestURL;

    DExpanderBox*                            advBox                         = nullptr;

    SidebarSplitter*                         splitter                       = nullptr;

    CameraThumbsCtrl*                        camThumbsCtrl                  = nullptr;
    CameraController*                        controller                     = nullptr;
/*
    CameraHistoryUpdater*                    historyUpdater                 = nullptr;
*/
    ImportView*                              view                           = nullptr;

    RenameCustomizer*                        renameCustomizer               = nullptr;
    AlbumCustomizer*                         albumCustomizer                = nullptr;
    AdvancedSettings*                        advancedSettings               = nullptr;

#ifdef HAVE_JXL

    DNGConvertSettings*                      dngConvertSettings             = nullptr;

#endif

    ScriptingSettings*                       scriptingSettings              = nullptr;

    FilterStatusBar*                         filterStatusBar                = nullptr;
    ImportItemPropertiesSideBarImport*       rightSideBar                   = nullptr;

    DZoomBar*                                zoomBar                        = nullptr;
    StatusProgressBar*                       statusProgressBar              = nullptr;

    FreeSpaceWidget*                         albumLibraryFreeSpace          = nullptr;
    FreeSpaceWidget*                         cameraFreeSpace                = nullptr;

    QTimer*                                  progressTimer                  = nullptr;

    float                                    progressValue                  = 0.0F;

    DHistoryView*                            historyView                    = nullptr;
    ImportFilterComboBox*                    filterComboBox                 = nullptr;
/*
    CHUpdateItemMap                          map;
*/
    DNotificationWidget*                     errorWidget                    = nullptr;

    const QString                            configGroupName                = QLatin1String("Camera Settings");
    const QString                            configUseFileMetadata          = QLatin1String("UseFileMetadata");
    const QString                            configUseDefaultTargetAlbum    = QLatin1String("UseDefaultTargetAlbum");
    const QString                            configLastTargetAlbum          = QLatin1String("LastTargetAlbum");
    const QString                            configDefaultTargetAlbumId     = QLatin1String("DefaultTargetAlbumId");
    const QString                            configFileSaveConflictRule     = QLatin1String("FileSaveConflictRule");
    const QString                            importFiltersConfigGroupName   = QLatin1String("Import Filters");
};

} // namespace Digikam
