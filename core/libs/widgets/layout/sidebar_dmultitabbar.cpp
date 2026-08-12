/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI - Multi-tab bar implementation.
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

DMultiTabBar::DMultiTabBar(Qt::Edge pos, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    if ((pos == Qt::LeftEdge) || (pos == Qt::RightEdge))
    {
        d->layout = new QVBoxLayout(this);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    else
    {
        d->layout = new QHBoxLayout(this);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    d->layout->setContentsMargins(QMargins());
    d->layout->setSpacing(0);

    d->internal = new DMultiTabBarFrame(this, pos);
    setPosition(pos);
    setStyle(ActiveIconText);
    d->layout->insertWidget(0, d->internal);
    d->layout->insertWidget(0, d->btnTabSep = new QFrame(this));
    d->btnTabSep->setFixedHeight(4);
    d->btnTabSep->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    d->btnTabSep->setLineWidth(2);
    d->btnTabSep->hide();

    updateGeometry();
}

DMultiTabBar::~DMultiTabBar()
{
    qDeleteAll(d->buttons);
    d->buttons.clear();
    delete d;
}

void DMultiTabBar::appendButton(const QIcon& pic, int id, QMenu* const popup, const QString&)
{
    DMultiTabBarButton* const btn = new DMultiTabBarButton(pic, QString(), id, this);

    // a button with a QMenu can have another size. Make sure the button has always the same size.

    btn->setFixedWidth(btn->height());
    btn->setMenu(popup);
    d->buttons.append(btn);
    d->layout->insertWidget(0, btn);
    btn->show();
    d->btnTabSep->show();
}

void DMultiTabBar::updateSeparator()
{
    bool hideSep = true;
    QListIterator<DMultiTabBarButton*> it(d->buttons);

    while (it.hasNext())
    {
        if (it.next()->isVisibleTo(this))
        {
            hideSep = false;
            break;
        }
    }

    if (hideSep)
    {
        d->btnTabSep->hide();
    }
    else
    {
       d->btnTabSep->show();
    }
}

void DMultiTabBar::appendTab(const QIcon& pic, int id, const QString& text)
{
    d->internal->appendTab(pic, id, text);
}

DMultiTabBarButton* DMultiTabBar::button(int id) const
{
    QListIterator<DMultiTabBarButton*> it(d->buttons);

    while (it.hasNext())
    {
        DMultiTabBarButton* const btn = it.next();

        if (btn->id() == id)
        {
            return btn;
        }
    }

    return nullptr;
}

DMultiTabBarTab* DMultiTabBar::tab(int id) const
{
    return d->internal->tab(id);
}

void DMultiTabBar::removeButton(int id)
{
    for (int pos = 0 ; pos < d->buttons.count() ; ++pos)
    {
        if (d->buttons.at(pos)->id() == id)
        {
            d->buttons.takeAt(pos)->deleteLater();
            break;
        }
    }

    if (d->buttons.count() == 0)
    {
        d->btnTabSep->hide();
    }
}

void DMultiTabBar::removeTab(int id)
{
    d->internal->removeTab(id);
}

void DMultiTabBar::setTab(int id,bool state)
{
    DMultiTabBarTab* const ttab = tab(id);

    if (ttab)
    {
        ttab->setState(state);
    }
}

bool DMultiTabBar::isTabRaised(int id) const
{
    DMultiTabBarTab* const ttab = tab(id);

    if (ttab)
    {
        return ttab->isChecked();
    }

    return false;
}

void DMultiTabBar::setStyle(TextStyle style)
{
    d->internal->setStyle(style);
}

DMultiTabBar::TextStyle DMultiTabBar::tabStyle() const
{
    return d->internal->d->style;
}

void DMultiTabBar::setPosition(Qt::Edge pos)
{
    d->position = pos;
    d->internal->setPosition(pos);
}

Qt::Edge DMultiTabBar::position() const
{
    return d->position;
}

void DMultiTabBar::fontChange(const QFont&)
{
    updateGeometry();
}

} // namespace Digikam
