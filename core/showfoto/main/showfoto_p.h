/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "showfoto.h"

// Qt includes

#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QPointer>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStandardPaths>
#include <QStyle>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMimeDatabase>
#include <QMessageBox>
#include <QApplication>
#include <QMimeType>
#include <QSplitter>
#include <QAction>
#include <QUrl>
#include <QScopedPointer>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kxmlgui_version.h>

#ifdef HAVE_KICONTHEMES
#   include <kiconthemes_version.h>
#endif

// Local includes

#include "showfotoiteminfo.h"
#include "showfotothumbnailbar.h"
#include "dsplashscreen.h"
#include "itempropertiessidebar.h"
#include "showfotodragdrophandler.h"
#include "thumbnailloadthread.h"
#include "drawdecoder.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "canvas.h"
#include "editorcore.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "dfileoperations.h"
#include "iccsettingscontainer.h"
#include "imagedialog.h"
#include "iofilesettings.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "metaenginesettings.h"
#include "savingcontext.h"
#include "showfotosetup.h"
#include "showfotosetupmisc.h"
#include "setupicc.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailsize.h"
#include "dnotificationwrapper.h"
#include "showfotodelegate.h"
#include "showfotothumbnailmodel.h"
#include "showfotocategorizedview.h"
#include "showfotofolderviewsidebar.h"
#include "showfotofolderviewlist.h"
#include "showfotostackviewsidebar.h"
#include "showfotostackviewlist.h"
#include "showfotosettings.h"
#include "showfotoinfoiface.h"
#include "showfotofiltermodel.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dpluginloader.h"
#include "exiftoolthread.h"
#include "exiftoolparser.h"
#include "sidebarwidget.h"
#include "localizesettings.h"
#include "networkmanager.h"

#ifdef HAVE_GEOLOCATION
#   include "geolocationsettings.h"
#endif

namespace ShowFoto
{

class Q_DECL_HIDDEN Showfoto::Private
{
public:

    Private() = default;

public:

    bool                              validIccPath              = true;

    int                               itemsNb                   = 0;

    QSplitter*                        vSplitter                 = nullptr;
    QAction*                          fileOpenAction            = nullptr;
    QUrl                              currentLoadedUrl;
    QUrl                              lastOpenedDirectory;
    QAction*                          openFilesInFolderAction   = nullptr;
    QAction*                          mediaServerAction         = nullptr;
    QAction*                          first                     = nullptr;

    ShowfotoItemInfoList              infoList;
    ShowfotoThumbnailModel*           model                     = nullptr;
    ShowfotoDragDropHandler*          dDHandler                 = nullptr;
    ShowfotoFilterModel*              filterModel               = nullptr;
    Digikam::ThumbnailLoadThread*     thumbLoadThread           = nullptr;
    ShowfotoThumbnailBar*             thumbBar                  = nullptr;
    Digikam::ThumbBarDock*            thumbBarDock              = nullptr;
    ShowfotoNormalDelegate*           normalDelegate            = nullptr;
    Digikam::Sidebar*                 leftSideBar               = nullptr;
    Digikam::ItemPropertiesSideBar*   rightSideBar              = nullptr;
    Digikam::DSplashScreen*           splash                    = nullptr;
    ShowfotoSettings*                 settings                  = nullptr;
    ShowfotoFolderViewSideBar*        folderView                = nullptr;
    ShowfotoStackViewSideBar*         stackView                 = nullptr;
};

} // namespace ShowFoto
