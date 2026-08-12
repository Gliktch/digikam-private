/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "welcomepageview_p.h"

namespace Digikam
{

TitleEffect::TitleEffect(QWidget* const parent)
    : QGraphicsDropShadowEffect(parent)
{
    setColor(Qt::white);
    setBlurRadius(35);
    setOffset(0, 0);
}

// ---

GradientWidget::GradientWidget(QWidget* const parent)
    : QWidget (parent)
{
    setContentsMargins(0, 0, 0, 0);
}

void GradientWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0,    QColor(qRgb(0x01, 0x08, 0x0F)));
    gradient.setColorAt(0.55, QColor(qRgb(0x22, 0x3c, 0x54)));
    painter.fillRect(rect(), gradient);
}

// ---

InvertedGradientWidget::InvertedGradientWidget(QWidget* const parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
}

void InvertedGradientWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0,    QColor(qRgb(0x22, 0x3c, 0x54)));
    gradient.setColorAt(0.55, QColor(qRgb(0x01, 0x08, 0x0F)));
    painter.fillRect(rect(), gradient);
}

// ---

ResizableBackgroundWidget::ResizableBackgroundWidget(QWidget* const parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
}

void ResizableBackgroundWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void ResizableBackgroundWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(qRgb(0x22, 0x3c, 0x54)));

    QPixmap scaledPixmap = m_backgroundPixmap.scaledToWidth(width(), Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaledPixmap);
}

} // namespace Digikam
