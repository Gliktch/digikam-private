/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2001-2003 by Joseph Wenninger <jowenn at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sidebar_p.h"

namespace Digikam
{

Sidebar::Sidebar(QWidget* const parent, SidebarSplitter* const sp, Qt::Edge side, bool minimizedDefault)
    : DMultiTabBar     (side, parent),
      StateSavingObject(this),
      d                (new Private)
{
    d->splitter         = sp;
    d->minimizedDefault = minimizedDefault;
    d->stack            = new QStackedWidget(d->splitter);
    d->dragSwitchTimer  = new QTimer(this);

    connect(d->dragSwitchTimer, SIGNAL(timeout()),
            this, SLOT(slotDragSwitchTimer()));

    d->splitter->d->sidebars << this;

    setStyle(DMultiTabBar::ActiveIconText);
}

Sidebar::~Sidebar()
{
    saveState();

    if (d->splitter)
    {
        d->splitter->d->sidebars.removeAll(this);
    }

    delete d;
}

SidebarSplitter* Sidebar::splitter() const
{
    return d->splitter;
}

void Sidebar::doLoadState()
{
    KConfigGroup group = getConfigGroup();
    int tab            = group.readEntry(entryName(d->optionActiveTabEntry),   0);
    bool minimized     = group.readEntry(entryName(d->optionMinimizedEntry),   d->minimizedDefault);
    d->restoreSize     = group.readEntry(entryName(d->optionRestoreSizeEntry), -1);

    // validate

    if ((tab >= d->tabs) || (tab < 0))
    {
        tab = 0;
    }

    if (minimized)
    {
        d->activeTab = tab;
        setTab(d->activeTab, false);
        d->stack->setCurrentIndex(d->activeTab);
        shrink();

        Q_EMIT signalChangedTab(d->stack->currentWidget());

        return;
    }

    d->activeTab = -1;

    QTimer::singleShot(0, this, [this, tab]()->void
        {
            slotClicked(tab);
        }
    );
}

void Sidebar::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->optionActiveTabEntry),   d->activeTab);
    group.writeEntry(entryName(d->optionMinimizedEntry),   d->minimized);
    group.writeEntry(entryName(d->optionRestoreSizeEntry), d->minimized ? d->restoreSize : -1);
}

void Sidebar::backup()
{
    // backup preview state of sidebar view (shrink or not)

    d->isMinimized = d->minimized;

    // In all case, shrink sidebar view

    shrink();

    DMultiTabBar::hide();
}

void Sidebar::backup(const QList<QWidget*>& thirdWidgetsToBackup, QList<int>* const sizes)
{
    sizes->clear();

    for (QWidget* const widget : std::as_const(thirdWidgetsToBackup))
    {
        *sizes << d->splitter->size(widget);
    }

    backup();
}

void Sidebar::restore()
{
    DMultiTabBar::show();

    // restore preview state of sidebar view, stored in backup()

    if (!d->isMinimized)
    {
        expand();
    }
}

void Sidebar::restore(const QList<QWidget*>& thirdWidgetsToRestore, const QList<int>& sizes)
{
    restore();

    if (thirdWidgetsToRestore.size() == sizes.size())
    {
        for (int i = 0 ; i < thirdWidgetsToRestore.size() ; ++i)
        {
            d->splitter->setSize(thirdWidgetsToRestore.at(i), sizes.at(i));
        }
    }
}

void Sidebar::appendTab(QWidget* const w, const QIcon& pic, const QString& title)
{
    // Store state (but not on initialization)

    if (isVisible())
    {
        d->appendedTabsStateCache[w] = SidebarState(d->stack->currentWidget(), d->splitter->size(this));
    }

    // Add tab

    w->setParent(d->stack);
    DMultiTabBar::appendTab(pic, d->tabs, title);
    d->stack->insertWidget(d->tabs, w);

    tab(d->tabs)->setAcceptDrops(true);
    tab(d->tabs)->installEventFilter(this);

    connect(tab(d->tabs), SIGNAL(signalClicked(int)),
            this, SLOT(slotClicked(int)));

    d->tabs++;
}

void Sidebar::deleteTab(QWidget* const w)
{
    int tab = d->stack->indexOf(w);

    if (tab < 0)
    {
        return;
    }

    bool removingActiveTab = (tab == d->activeTab);

    if (removingActiveTab)
    {
        d->activeTab = -1;
    }

    d->stack->removeWidget(d->stack->widget(tab));

    // delete widget

    removeTab(tab);
    d->tabs--;

    // restore or reset active tab and width

    if (!d->minimized)
    {
        // restore to state before adding tab
        // using a hash is simple, but does not handle well multiple add/removal operations at a time

        SidebarState state = d->appendedTabsStateCache.take(w);

        if (state.activeWidget)
        {
            int atab = d->stack->indexOf(state.activeWidget);

            if (atab != -1)
            {
                switchTabAndStackToTab(atab);

                Q_EMIT signalChangedTab(d->stack->currentWidget());

                if (state.size == 0)
                {
                    d->minimized = true;
                    setTab(d->activeTab, false);
                }

                d->splitter->setSize(this, state.size);
            }
        }
        else
        {
            if (removingActiveTab)
            {
                slotClicked(d->tabs - 1);
            }

            d->splitter->setSize(this, -1);
        }
    }
    else
    {
        d->restoreSize = -1;
    }
}

void Sidebar::slotClicked(int tab)
{
    if ((tab >= d->tabs) || (tab < 0))
    {
        return;
    }

    if (tab == d->activeTab)
    {
        d->stack->isHidden() ? expand()
                             : shrink();
    }
    else
    {
        switchTabAndStackToTab(tab);

        if (d->minimized)
        {
            expand();
        }

        Q_EMIT signalChangedTab(d->stack->currentWidget());
    }
}

void Sidebar::setActiveTab(QWidget* const w)
{
    int tab = d->stack->indexOf(w);

    if (tab < 0)
    {
        return;
    }

    switchTabAndStackToTab(tab);

    if (d->minimized)
    {
        expand();
    }

    Q_EMIT signalChangedTab(d->stack->currentWidget());
}

void Sidebar::activePreviousTab()
{
    int tab = d->stack->indexOf(d->stack->currentWidget());

    if (tab == 0)
    {
        tab = d->tabs-1;
    }
    else
    {
        tab--;
    }

    setActiveTab(d->stack->widget(tab));
}

void Sidebar::activeNextTab()
{
    int tab = d->stack->indexOf(d->stack->currentWidget());

    if (tab == d->tabs-1)
    {
        tab = 0;
    }
    else
    {
        tab++;
    }

    setActiveTab(d->stack->widget(tab));
}

void Sidebar::switchTabAndStackToTab(int tab)
{
    if (d->activeTab >= 0)
    {
        setTab(d->activeTab, false);
    }

    d->activeTab = tab;
    setTab(d->activeTab, true);
    d->stack->setCurrentIndex(d->activeTab);
}

QWidget* Sidebar::getActiveTab() const
{
    if (d->splitter)
    {
        return d->stack->currentWidget();
    }
    else
    {
        return nullptr;
    }
}

void Sidebar::shrink()
{
    d->minimized = true;

    // store the size that we had. We may later need it when we restore to visible.

    int currentSize = d->splitter->size(this);

    if (currentSize)
    {
        d->restoreSize = currentSize;
    }

    d->stack->hide();

    Q_EMIT signalViewChanged();
}

void Sidebar::expand()
{
    d->minimized = false;
    d->stack->show();

    QTimer::singleShot(0, this, SLOT(slotExpandTimer()));
}

void Sidebar::slotExpandTimer()
{
    // Do not expand to size 0 (only splitter handle visible)
    // but either to previous size, or the minimum size hint

    if (d->splitter->size(this) == 0)
    {
        setTab(d->activeTab, true);
        d->splitter->setSize(this, d->restoreSize ? d->restoreSize : -1);
    }

    Q_EMIT signalViewChanged();
}

bool Sidebar::isExpanded() const
{
    return !d->minimized;
}

bool Sidebar::eventFilter(QObject* obj, QEvent* ev)
{
    for (int i = 0 ; i < d->tabs ; ++i)
    {
        if (obj == tab(i))
        {
            if      (ev->type() == QEvent::DragEnter)
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                QEnterEvent* const e = static_cast<QEnterEvent*>(ev);

#else

                QDragEnterEvent* const e = static_cast<QDragEnterEvent*>(ev);

#endif

                enterEvent(e);
                e->accept();

                return false;
            }
            else if (ev->type() == QEvent::DragMove)
            {
                if (!d->dragSwitchTimer->isActive())
                {
                    d->dragSwitchTimer->setSingleShot(true);
                    d->dragSwitchTimer->start(800);
                    d->dragSwitchId = i;
                }

                return false;
            }
            else if (ev->type() == QEvent::DragLeave)
            {
                d->dragSwitchTimer->stop();
                QDragLeaveEvent* const e = static_cast<QDragLeaveEvent*>(ev);
                leaveEvent(e);

                return false;
            }
            else if (ev->type() == QEvent::Drop)
            {
                d->dragSwitchTimer->stop();
                QDropEvent* const e = static_cast<QDropEvent*>(ev);
                leaveEvent(e);

                return false;
            }
            else
            {
                return false;
            }
        }
    }

    // Else, pass the event on to the parent class

    return DMultiTabBar::eventFilter(obj, ev);
}

void Sidebar::slotDragSwitchTimer()
{
    slotClicked(d->dragSwitchId);
}

void Sidebar::slotSplitterBtnClicked()
{
    slotClicked(d->activeTab);
}

} // namespace Digikam

#include "moc_sidebar.cpp"
