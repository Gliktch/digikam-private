/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI - Private headers.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2001-2003 by Joseph Wenninger <jowenn at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "sidebar.h"

// Qt includes

#include <QDragEnterEvent>
#include <QEvent>
#include <QPixmap>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QHash>
#include <QScrollArea>
#include <QFrame>
#include <QActionEvent>
#include <QLayout>
#include <QPainter>
#include <QFontMetrics>
#include <QStyle>
#include <QStyleOptionButton>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DMultiTabBarFrame::Private
{
public:

    Private() = default;

public:

    QBoxLayout*             mainLayout  = nullptr;
    QList<DMultiTabBarTab*> tabs;
    Qt::Edge                position    = Qt::LeftEdge;
    DMultiTabBar::TextStyle style       = DMultiTabBar::AllIconsText;
};

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DMultiTabBarTab::Private
{
public:

    Private() = default;

public:

    Qt::Edge                position    = Qt::LeftEdge;
    DMultiTabBar::TextStyle style       = DMultiTabBar::AllIconsText;
};

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DMultiTabBar::Private
{
public:

    Private() = default;

public:

    DMultiTabBarFrame*         internal     = nullptr;
    QBoxLayout*                layout       = nullptr;
    QFrame*                    btnTabSep    = nullptr;
    QList<DMultiTabBarButton*> buttons;
    Qt::Edge                   position     = Qt::LeftEdge;
};

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN SidebarState
{
public:

    SidebarState()  = default;

    SidebarState(QWidget* const w, int size)
        : activeWidget(w),
          size        (size)
    {
    }

public:

    QWidget* activeWidget   = nullptr;
    int      size           = 0;
};

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN Sidebar::Private
{

public:

    Private() = default;

public:

    bool                          minimizedDefault          = false;
    bool                          minimized                 = false;

    /**
     * Backup of shrink status before backup(), restored by restore()
     * @note when sidebar is hidden, only icon bar is affected. If sidebar view is
     * visible, this one must be shrink and restored accordingly.
     */
    bool                          isMinimized               = false;

    int                           tabs                      = 0;
    int                           activeTab                 = -1;
    int                           dragSwitchId              = -1;
    int                           restoreSize               = 0;

    QStackedWidget*               stack                     = nullptr;
    SidebarSplitter*              splitter                  = nullptr;
    QTimer*                       dragSwitchTimer           = nullptr;

    QHash<QWidget*, SidebarState> appendedTabsStateCache;

    const QString                 optionActiveTabEntry      = QLatin1String("ActiveTab");
    const QString                 optionMinimizedEntry      = QLatin1String("Minimized");
    const QString                 optionRestoreSizeEntry    = QLatin1String("RestoreSize");
};

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN SidebarSplitter::Private
{
public:

    Private() = default;

public:

    QList<Sidebar*> sidebars;
};

} // namespace Digikam
