/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam Survey tool
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "surveywindow.h"

// Qt includes

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QSplitter>
#include <QTimer>

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
#include <kxmlgui_version.h>

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "itemthumbnailbar.h"
#include "itempropertiessidebardb.h"
#include "statusprogressbar.h"
#include "dzoombar.h"
#include "surveystack.h"
#include "thumbbardock.h"
#include "drawdecoder.h"
#include "componentsinfodlg.h"
#include "digikamapp.h"
#include "thememanager.h"
#include "dimg.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "metaenginesettings.h"
#include "applicationsettings.h"
#include "loadingcacheinterface.h"
#include "iccsettings.h"
#include "imagewindow.h"
#include "itempreviewview.h"
#include "itemdescedittab.h"
#include "setup.h"
#include "syncjob.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "coredbchangesets.h"
#include "collectionscanner.h"
#include "scancontroller.h"
#include "tagsactionmngr.h"
#include "fileactionmngr.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "dexpanderbox.h"
#include "dbinfoiface.h"

#ifdef HAVE_MEDIAPLAYER
#   include "itempreviewvideo.h"
#endif // HAVE_MEDIAPLAYER

namespace Digikam
{

class DAdjustableLabel;

class Q_DECL_HIDDEN SurveyWindow::Private
{

public:

    Private() = default;

    void addPageUpDownActions(const SurveyWindow* const q, QWidget* const w)
    {
        defineShortcut(w, Qt::Key_Down,  q, SLOT(slotForward()));
        defineShortcut(w, Qt::Key_Right, q, SLOT(slotForward()));
        defineShortcut(w, Qt::Key_Up,    q, SLOT(slotBackward()));
        defineShortcut(w, Qt::Key_Left,  q, SLOT(slotBackward()));
    }

public:

    QAction*                  editItemAction                = nullptr;
    QAction*                  fileDeleteAction              = nullptr;
    QAction*                  fileDeleteFinalAction         = nullptr;
    QAction*                  zoomPlusAction                = nullptr;
    QAction*                  zoomMinusAction               = nullptr;
    QAction*                  zoomTo100percents             = nullptr;
    QAction*                  zoomFitToWindowAction         = nullptr;

    QAction*                  forwardAction                 = nullptr;
    QAction*                  backwardAction                = nullptr;
    QAction*                  firstAction                   = nullptr;
    QAction*                  lastAction                    = nullptr;

    QAction*                  showBarAction                 = nullptr;
    QAction*                  viewCMViewAction              = nullptr;


    QMainWindow*              dockArea                      = nullptr;

    StatusProgressBar*        fileName                      = nullptr;     ///< File name loaded in preview and show on status bar.

    SidebarSplitter*          hSplitter                     = nullptr;

    SurveyStack*              stack                         = nullptr;     ///< Central view with thumbbar and preview stack.

    DZoomBar*                 zoomBar                       = nullptr;     ///< Zoom bar to control the preview and show on status bar.

    ItemPropertiesSideBarDB*  sideBar                       = nullptr;     ///< Side bar to display properties of the preview and show on the right side.
};

} // namespace Digikam
