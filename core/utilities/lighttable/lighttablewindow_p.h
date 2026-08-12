/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "lighttablewindow.h"

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

#include "digikam_globals.h"
#include "itempropertiessidebardb.h"
#include "statusprogressbar.h"
#include "dzoombar.h"
#include "lighttableview.h"
#include "lighttablethumbbar.h"
#include "thumbbardock.h"
#include "drawdecoder.h"
#include "digikam_debug.h"
#include "componentsinfodlg.h"
#include "digikamapp.h"
#include "thememanager.h"
#include "dimg.h"
#include "dio.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "metaenginesettings.h"
#include "applicationsettings.h"
#include "albummanager.h"
#include "loadingcacheinterface.h"
#include "deletedialog.h"
#include "iccsettings.h"
#include "imagewindow.h"
#include "itemdescedittab.h"
#include "setup.h"
#include "syncjob.h"
#include "lighttablepreview.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "coredbchangesets.h"
#include "collectionscanner.h"
#include "scancontroller.h"
#include "tagsactionmngr.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "dexpanderbox.h"
#include "dbinfoiface.h"

namespace Digikam
{

class DAdjustableLabel;

class Q_DECL_HIDDEN LightTableWindow::Private
{

public:

    Private() = default;

    void addPageUpDownActions(const LightTableWindow* const q, QWidget* const w)
    {
        defineShortcut(w, Qt::Key_Down,  q, SLOT(slotForward()));
        defineShortcut(w, Qt::Key_Right, q, SLOT(slotForward()));
        defineShortcut(w, Qt::Key_Up,    q, SLOT(slotBackward()));
        defineShortcut(w, Qt::Key_Left,  q, SLOT(slotBackward()));
    }

public:

    bool                      autoLoadOnRightPanel          = true;
    bool                      autoSyncPreview               = true;
    bool                      fromLeftPreview               = true;

    QAction*                  setItemLeftAction             = nullptr;
    QAction*                  setItemRightAction            = nullptr;
    QAction*                  clearListAction               = nullptr;
    QAction*                  editItemAction                = nullptr;
    QAction*                  removeItemAction              = nullptr;
    QAction*                  fileDeleteAction              = nullptr;
    QAction*                  fileDeleteFinalAction         = nullptr;
    QAction*                  leftZoomPlusAction            = nullptr;
    QAction*                  leftZoomMinusAction           = nullptr;
    QAction*                  leftZoomTo100percents         = nullptr;
    QAction*                  leftZoomFitToWindowAction     = nullptr;
    QAction*                  rightZoomPlusAction           = nullptr;
    QAction*                  rightZoomMinusAction          = nullptr;
    QAction*                  rightZoomTo100percents        = nullptr;
    QAction*                  rightZoomFitToWindowAction    = nullptr;

    QAction*                  forwardAction                 = nullptr;
    QAction*                  backwardAction                = nullptr;
    QAction*                  firstAction                   = nullptr;
    QAction*                  lastAction                    = nullptr;

    QAction*                  showBarAction                 = nullptr;
    QAction*                  viewCMViewAction              = nullptr;
    QAction*                  syncPreviewAction             = nullptr;
    QAction*                  navigateByPairAction          = nullptr;
    QAction*                  clearOnCloseAction            = nullptr;

    QMainWindow*              dockArea                      = nullptr;

    StatusProgressBar*        leftFileName                  = nullptr;     ///< File name loaded in left preview and show on status bar.
    StatusProgressBar*        rightFileName                 = nullptr;     ///< File name loaded in right preview and show on status bar.

    SidebarSplitter*          hSplitter                     = nullptr;
    ThumbBarDock*             barViewDock                   = nullptr;

    LightTableThumbBar*       thumbView                     = nullptr;     ///< Top level view to host thumbar.

    LightTableView*           previewView                   = nullptr;     ///< Central view including left and right panels.

    DZoomBar*                 leftZoomBar                   = nullptr;     ///< Zoom bar to control the left preview and show on status bar.
    DZoomBar*                 rightZoomBar                  = nullptr;     ///< Zoom bar to control the right preview and show on status bar.

    StatusProgressBar*        statusProgressBar             = nullptr;

    ItemPropertiesSideBarDB*  leftSideBar                   = nullptr;     ///< Side bar to display properties of the left preview and show on the left side..
    ItemPropertiesSideBarDB*  rightSideBar                  = nullptr;     ///< Side bar to display properties of the right preview and show on the right side..
};

} // namespace Digikam
