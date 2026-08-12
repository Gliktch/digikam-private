/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI - Splitter implementation.
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

const QString SidebarSplitter::DEFAULT_CONFIG_KEY = QLatin1String("SplitterState");

SidebarSplitter::SidebarSplitter(QWidget* const parent)
    : QSplitter(parent),
      d        (new Private)
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved(int,int)));
}

SidebarSplitter::SidebarSplitter(Qt::Orientation orientation, QWidget* const parent)
    : QSplitter(orientation, parent),
      d        (new Private)
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved(int,int)));
}

SidebarSplitter::~SidebarSplitter()
{
    // retreat cautiously from sidebars that live longer

    for (Sidebar* const sidebar : std::as_const(d->sidebars))
    {
        sidebar->d->splitter = nullptr;
    }

    delete d;
}

void SidebarSplitter::restoreState(KConfigGroup& group)
{
    restoreState(group, DEFAULT_CONFIG_KEY);
}

void SidebarSplitter::restoreState(KConfigGroup& group, const QString& key)
{
    if (group.hasKey(key))
    {
        QByteArray state;
        state = group.readEntry(key, state);

        if (!state.isEmpty())
        {
            QSplitter::restoreState(QByteArray::fromBase64(state));
        }
    }
}

void SidebarSplitter::saveState(KConfigGroup& group)
{
    saveState(group, DEFAULT_CONFIG_KEY);
}

void SidebarSplitter::saveState(KConfigGroup& group, const QString& key)
{
    group.writeEntry(key, QSplitter::saveState().toBase64());
}

int SidebarSplitter::size(Sidebar* const bar) const
{
    return size(bar->d->stack);
}

int SidebarSplitter::size(QWidget* const widget) const
{
    int index = indexOf(widget);

    if (index == -1)
    {
        return -1;
    }

    return sizes().at(index);
}

void SidebarSplitter::setSize(Sidebar* const bar, int size)
{
    setSize(bar->d->stack, size);
}

void SidebarSplitter::setSize(QWidget* const widget, int size)
{
    int index = indexOf(widget);

    if (index == -1)
    {
        return;
    }

    // special case: Use minimum size hint

    if (size == -1)
    {
        if (orientation() == Qt::Horizontal)
        {
            size = widget->minimumSizeHint().width();
        }

        if (orientation() == Qt::Vertical)
        {
            size = widget->minimumSizeHint().height();
        }
    }

    QList<int> sizeList = sizes();
    sizeList[index]     = size;
    setSizes(sizeList);
}

void SidebarSplitter::slotSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);

    // When the user moves the splitter so that size is 0 (collapsed),
    // it may be difficult to restore the sidebar as clicking the buttons
    // has no effect (only hides/shows the splitter handle)
    // So we want to transform this size-0-sidebar
    // to a sidebar that is shrunk (d->stack hidden)
    // and can be restored by clicking a tab bar button

    // We need to look at the widget between index-1 and index
    // and the one between index and index+1

    QList<int> sizeList = sizes();

    // Is there a sidebar with size 0 before index ?

    if ((index > 0) && (sizeList.at(index-1) == 0))
    {
        const QWidget* const w = widget(index-1);

        for (Sidebar* const sidebar : std::as_const(d->sidebars))
        {
            if (w == sidebar->d->stack)
            {    // cppcheck-suppress useStlAlgorithm
                if (!sidebar->d->minimized)
                {
                    sidebar->setTab(sidebar->d->activeTab, false);
                    sidebar->shrink();
                }

                break;
            }
        }
    }

    // Is there a sidebar with size 0 after index ?

    if (sizeList.at(index) == 0)
    {
        const QWidget* const w = widget(index);

        for (Sidebar* const sidebar : std::as_const(d->sidebars))
        {
            if (w == sidebar->d->stack)
            {   // cppcheck-suppress useStlAlgorithm
                if (!sidebar->d->minimized)
                {
                    sidebar->setTab(sidebar->d->activeTab, false);
                    sidebar->shrink();
                }

                break;
            }
        }
    }
}

} // namespace Digikam
