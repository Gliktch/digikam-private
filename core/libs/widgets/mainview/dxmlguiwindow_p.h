/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window - Private container.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "dxmlguiwindow.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QString>
#include <QList>
#include <QMap>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QEvent>
#include <QHoverEvent>
#include <QApplication>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QScreen>
#include <QMenu>
#include <QUrl>
#include <QDomDocument>
#include <QUrlQuery>
#include <QIcon>
#include <QPushButton>
#include <QDir>
#include <QFileInfo>
#include <QResource>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDialogButtonBox>

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
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>
#include <ksharedconfig.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kxmlguifactory.h>
#include <kconfigwidgets_version.h>

#ifdef HAVE_KNOTIFYCONFIG
#   include <knotifyconfigwidget.h>
#endif

#if KCONFIGWIDGETS_VERSION > QT_VERSION_CHECK(5, 80, 0)
#   include <khamburgermenu.h>
#endif

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "daboutdata.h"
#include "dpluginloader.h"
#include "solidhardwaredlg.h"
#include "webbrowserdlg.h"

namespace Digikam
{

class Q_DECL_HIDDEN DXmlGuiWindow::Private
{
public:

    Private() = default;

public:

    /**
     * Settings taken from managed window configuration to handle toolbar visibility in full-screen mode
     */
    bool                     fullScreenHideToolBars     = false;

    /**
     * Settings taken from managed window configuration to handle thumbbar visibility in full-screen mode
     */
    bool                     fullScreenHideThumbBar     = true;

    /**
     * Settings taken from managed window configuration to handle toolbar visibility in full-screen mode
     */
    bool                     fullScreenHideSideBars     = false;

    /**
     * Settings taken from managed window configuration to handle statusbar visibility in full-screen mode
     */
    bool                     fullScreenHideStatusBar    = false;

    /**
     * Full-Screen options. See FullScreenOptions enum and setFullScreenOptions() for details.
     */
    int                      fsOptions                  = FS_NONE;

    /**
     * Action plug in managed window to switch fullscreen state
     */
    KToggleFullScreenAction* fullScreenAction           = nullptr;

    /**
     * Used under MacOS only. See bug #414117
     */
    QWidget*                 fullScreenParent           = nullptr;

    /**
     * Show only if toolbar is hidden
     */
    QToolButton*             fullScreenBtn              = nullptr;

    /**
     * Used by slotToggleFullScreen() to manage state of full-screen button on managed window
     */
    bool                     dirtyMainToolBar           = false;

    /**
     * Store previous visibility of toolbars before ful-screen mode.
     */
    QMap<KToolBar*, bool>    toolbarsVisibility;

    /**
     * Store previous visibility of thumbbar before ful-screen mode.
     */
    bool                     thumbbarVisibility         = true;

    /**
     * Store previous visibility of menubar before ful-screen mode.
     */
    bool                     menubarVisibility          = true;

    /**
     * Store previous visibility of statusbar before ful-screen mode.
     */
    bool                     statusbarVisibility        = true;

    // Common Help actions
    QAction*                 dbStatAction               = nullptr;
    QAction*                 libsInfoAction             = nullptr;
    QAction*                 showMenuBarAction          = nullptr;
    QAction*                 showStatusBarAction        = nullptr;
    DAboutData*              about                      = nullptr;

#if KCONFIGWIDGETS_VERSION > QT_VERSION_CHECK(5, 80, 0)

    KHamburgerMenu*          hamburgerMenu              = nullptr;

#endif

    DLogoAction*             anim                       = nullptr;

    QString                  configGroupName;

    QString                  handbookSection;
};

} // namespace Digikam
