/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "queuemgrwindow.h"

// Qt includes

#include <QLabel>
#include <QString>
#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <QApplication>

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

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "actiondata.h"
#include "album.h"
#include "batchtoolsfactory.h"
#include "actionthread.h"
#include "queuepool.h"
#include "workflowmanager.h"
#include "queuelist.h"
#include "queuesettings.h"
#include "queuesettingsview.h"
#include "assignedlist.h"
#include "toolsettingsview.h"
#include "toolsview.h"
#include "componentsinfodlg.h"
#include "digikamapp.h"
#include "thememanager.h"
#include "dimg.h"
#include "dlogoaction.h"
#include "albummanager.h"
#include "imagewindow.h"
#include "thumbnailsize.h"
#include "sidebar.h"
#include "dnotificationwrapper.h"
#include "statusprogressbar.h"

namespace Digikam
{

class BatchToolsFactory;

class Q_DECL_HIDDEN QueueMgrWindow::Private
{

public:

    Private() = default;

    bool                     busy                           = false;
    bool                     processingAllQueues            = false;

    int                      currentQueueToProcess          = 0;

    QLabel*                  statusLabel                    = nullptr;

    QAction*                 clearQueueAction               = nullptr;
    QAction*                 removeItemsSelAction           = nullptr;
    QAction*                 removeItemsDoneAction          = nullptr;
    QAction*                 moveUpToolAction               = nullptr;
    QAction*                 moveDownToolAction             = nullptr;
    QAction*                 removeToolAction               = nullptr;
    QAction*                 clearToolsAction               = nullptr;

    QAction*                 runAction                      = nullptr;
    QAction*                 runAllAction                   = nullptr;
    QAction*                 stopAction                     = nullptr;
    QAction*                 removeQueueAction              = nullptr;
    QAction*                 newQueueAction                 = nullptr;
    QAction*                 saveQueueAction                = nullptr;
    QAction*                 donateMoneyAction              = nullptr;
    QAction*                 contributeAction               = nullptr;
    QAction*                 rawCameraListAction            = nullptr;

    SidebarSplitter*         topSplitter                    = nullptr;
    SidebarSplitter*         bottomSplitter                 = nullptr;
    SidebarSplitter*         verticalSplitter               = nullptr;

    BatchToolsFactory*       batchToolsMgr                  = nullptr;

    StatusProgressBar*       statusProgressBar              = nullptr;

    ActionThread*            thread                         = nullptr;

    ToolsView*               toolsView                      = nullptr;
    ToolSettingsView*        toolSettings                   = nullptr;
    AssignedListView*        assignedList                   = nullptr;
    QueuePool*               queuePool                      = nullptr;
    QueueSettingsView*       queueSettingsView              = nullptr;

    const QString            TOP_SPLITTER_CONFIG_KEY        = QLatin1String("BqmTopSplitter");
    const QString            BOTTOM_SPLITTER_CONFIG_KEY     = QLatin1String("BqmBottomSplitter");
    const QString            VERTICAL_SPLITTER_CONFIG_KEY   = QLatin1String("BqmVerticalSplitter");
};

} // namespace Digikam
