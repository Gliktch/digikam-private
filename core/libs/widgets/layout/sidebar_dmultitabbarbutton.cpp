/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI - Multi-tab bar frame implementation.
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

DMultiTabBarButton::DMultiTabBarButton(const QIcon& pic, const QString& text,
                                       int id, QWidget* const parent)
    : QPushButton(pic, text, parent),
      m_id       (id)
{
    // --- NOTE: use dynamic binding as slotClicked() is a virtual method which can be re-implemented in derived classes.

    connect(this, &QPushButton::clicked,
            this, &DMultiTabBarButton::slotClicked);

    // we cannot see the focus, so don't take focus. #45557
    // If keyboard navigation is wanted, then only the bar should take focus,
    // and arrows could change the focused button; but generally, tabbars don't take focus anyway.

    setFocusPolicy(Qt::NoFocus);

    // See RB #128005

    setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

DMultiTabBarButton::~DMultiTabBarButton()
{
}

void DMultiTabBarButton::setText(const QString& text)
{
    QPushButton::setText(text);
}

void DMultiTabBarButton::slotClicked()
{
    updateGeometry();

    Q_EMIT signalClicked(m_id);
}

int DMultiTabBarButton::id() const
{
    return m_id;
}

void DMultiTabBarButton::hideEvent(QHideEvent* e)
{
    QPushButton::hideEvent(e);
    DMultiTabBar* const tb = dynamic_cast<DMultiTabBar*>(parentWidget());

    if (tb)
    {
        tb->updateSeparator();
    }
}

void DMultiTabBarButton::showEvent(QShowEvent* e)
{
    QPushButton::showEvent(e);
    DMultiTabBar* const tb = dynamic_cast<DMultiTabBar*>(parentWidget());

    if (tb)
    {
        tb->updateSeparator();
    }
}

void DMultiTabBarButton::paintEvent(QPaintEvent*)
{
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.icon     = icon();
    opt.iconSize = iconSize();

    // removes the QStyleOptionButton::HasMenu ButtonFeature

    opt.features = QStyleOptionButton::Flat;
    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);
}

} // namespace Digikam
