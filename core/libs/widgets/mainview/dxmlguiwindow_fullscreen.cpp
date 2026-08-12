/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window - Full-screen methods.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dxmlguiwindow_p.h"

namespace Digikam
{

void DXmlGuiWindow::setFullScreenOptions(int options)
{
    d->fsOptions = options;
}

void DXmlGuiWindow::createFullScreenAction(const QString& name)
{
    d->fullScreenAction = KStandardAction::fullScreen(nullptr, nullptr, this, this);
    actionCollection()->addAction(name, d->fullScreenAction);
    d->fullScreenBtn    = new QToolButton(this);
    d->fullScreenBtn->setDefaultAction(d->fullScreenAction);
    d->fullScreenBtn->hide();

    connect(d->fullScreenAction, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleFullScreen(bool)));
}

void DXmlGuiWindow::readFullScreenSettings(const KConfigGroup& group)
{
    if (d->fsOptions & FS_TOOLBARS)
    {
        d->fullScreenHideToolBars  = group.readEntry(configFullScreenHideToolBarsEntry(),  false);
    }

    if (d->fsOptions & FS_THUMBBAR)
    {
        d->fullScreenHideThumbBar  = group.readEntry(configFullScreenHideThumbBarEntry(),  true);
    }

    if (d->fsOptions & FS_SIDEBARS)
    {
        d->fullScreenHideSideBars  = group.readEntry(configFullScreenHideSideBarsEntry(),  false);
    }

    if (d->fsOptions & FS_STATUSBAR)
    {
        d->fullScreenHideStatusBar = group.readEntry(configFullScreenHideStatusBarEntry(), false);
    }
}

void DXmlGuiWindow::slotToggleFullScreen(bool set)
{

#ifdef Q_OS_MACOS

    // Work around Qt bug under macOS. See bug #414117

    if (set)
    {
        d->fullScreenParent = parentWidget();
        setParent(nullptr);
    }

#endif

    KToggleFullScreenAction::setFullScreen(this, set);

#ifdef Q_OS_MACOS

    if (!set)
    {
        setParent(d->fullScreenParent);
    }

#endif

    customizedFullScreenMode(set);

    if (!set)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "TURN OFF fullscreen";

        // restore menubar

        if (d->menubarVisibility)
        {
            menuBar()->setVisible(true);
        }

        // restore statusbar

        if ((d->fsOptions & FS_STATUSBAR) && d->fullScreenHideStatusBar)
        {
            statusBar()->setVisible(d->statusbarVisibility);
        }

        // restore sidebars

        if ((d->fsOptions & FS_SIDEBARS) && d->fullScreenHideSideBars)
        {
            showSideBars(true);
        }

        // restore thumbbar

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
        {
            showThumbBar(d->thumbbarVisibility);
        }

        // restore toolbars and manage full-screen button

        showToolBars(true);
        d->fullScreenBtn->hide();

        if (d->dirtyMainToolBar)
        {
            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                mainbar->removeAction(d->fullScreenAction);
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "TURN ON fullscreen";

        // hide menubar

#ifdef Q_OS_WIN

        d->menubarVisibility = d->showMenuBarAction->isChecked();

#else

        d->menubarVisibility = menuBar()->isVisible();

#endif

        menuBar()->setVisible(false);

        // hide statusbar

#ifdef Q_OS_WIN

        d->statusbarVisibility = d->showStatusBarAction->isChecked();

#else

        d->statusbarVisibility = statusBar()->isVisible();

#endif

        if ((d->fsOptions & FS_STATUSBAR) && d->fullScreenHideStatusBar)
        {
            statusBar()->setVisible(false);
        }

        // hide sidebars

        if ((d->fsOptions & FS_SIDEBARS) && d->fullScreenHideSideBars)
        {
            showSideBars(false);
        }

        // hide thumbbar

        d->thumbbarVisibility = thumbbarVisibility();

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
        {
            showThumbBar(false);
        }

        // hide toolbars and manage full-screen button

        if ((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars)
        {
            showToolBars(false);
        }
        else
        {
            showToolBars(true);

            // add fullscreen action if necessary in toolbar

            KToolBar* const mainbar = mainToolBar();

            if (mainbar && !mainbar->actions().contains(d->fullScreenAction))
            {
                if (mainbar->actions().isEmpty())
                {
                    mainbar->addAction(d->fullScreenAction);
                }
                else
                {
                    mainbar->insertAction(mainbar->actions().constFirst(), d->fullScreenAction);
                }

                d->dirtyMainToolBar = true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.

                d->dirtyMainToolBar = false;
            }
        }
    }
}

bool DXmlGuiWindow::fullScreenIsActive() const
{
    if (d->fullScreenAction)
    {
        return d->fullScreenAction->isChecked();
    }

    qCDebug(DIGIKAM_WIDGETS_LOG) << "FullScreenAction is not initialized";

    return false;
}

void DXmlGuiWindow::customizedFullScreenMode(bool set)
{
    Q_UNUSED(set);
}

void DXmlGuiWindow::checkFullScreenBeforeClosing()
{
    if (fullScreenIsActive())
    {
        slotToggleFullScreen(false);
    }

    qApp->processEvents();

    if (!testAttribute(Qt::WA_DeleteOnClose))
    {
        setVisible(false);
    }
}

} // namespace Digikam
