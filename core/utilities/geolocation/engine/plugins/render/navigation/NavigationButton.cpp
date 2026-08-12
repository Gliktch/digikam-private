/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "NavigationButton.h"

// Qt includes

#include <QPainter>
#include <QMouseEvent>

namespace Marble
{

NavigationButton::NavigationButton(QWidget* parent)
    : QAbstractButton(parent),
      m_iconMode(QIcon::Normal)
{
    // nothing to do
}

void NavigationButton::mousePressEvent(QMouseEvent* mouseEvent)
{
    if (isEnabled())
    {
        if (mouseEvent->button() == Qt::LeftButton)
        {
            m_iconMode = QIcon::Selected;
        }
    }

    Q_EMIT repaintNeeded();
}

void NavigationButton::mouseReleaseEvent(QMouseEvent*)
{
    if (isEnabled())
    {
        m_iconMode = QIcon::Active;
        Q_EMIT clicked();
    }

    Q_EMIT repaintNeeded();
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

void NavigationButton::enterEvent(QEnterEvent*)

#else

void NavigationButton::enterEvent(QEvent*)

#endif

{
    if (isEnabled())
    {
        m_iconMode = QIcon::Active;
    }

    Q_EMIT repaintNeeded();
}

void NavigationButton::leaveEvent(QEvent*)
{
    if (isEnabled())
    {
        m_iconMode = QIcon::Normal;
    }

    Q_EMIT repaintNeeded();
}

void NavigationButton::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::EnabledChange)
    {
        m_iconMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
    }

    Q_EMIT repaintNeeded();
}

void NavigationButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, icon().pixmap(iconSize(), m_iconMode));
}

} // namespace Marble

#include "moc_NavigationButton.cpp"
