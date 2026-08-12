/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-09
 * Description : tag region frame
 *
 * SPDX-FileCopyrightText: 2007      by Aurelien Gateau <agateau at kde dot org>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "regionframeitem_p.h"

namespace Digikam
{

void RegionFrameItem::setHudWidget(QWidget* const widget, Qt::WindowFlags wFlags)
{
    QGraphicsProxyWidget* const proxy = new QGraphicsProxyWidget(nullptr, wFlags);

    /*
     * This is utterly undocumented magic. If you add a normal widget directly,
     * with transparent parts (round corners), you will have ugly color in the corners.
     * If you set WA_TranslucentBackground on the widget directly, a lot of the
     * painting and stylesheets is broken. Like this, with an extra container, it seems to work.
     */

    QWidget* const container  = new QWidget;
    container->setAttribute(Qt::WA_TranslucentBackground);
    QHBoxLayout* const layout = new QHBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    layout->addWidget(widget);
    container->setLayout(layout);
    proxy->setWidget(container);

    // Reset fixed sizes wrongly copied by setWidget onto the QGraphicsWidget

    proxy->setMinimumSize(QSizeF());
    proxy->setMaximumSize(QSizeF());

    setHudWidget(proxy);
}

void RegionFrameItem::setHudWidget(QGraphicsWidget* const hudWidget)
{
    if (d->hudWidget == hudWidget)
    {
        return;
    }

    if (d->hudWidget)
    {
        d->hudWidget->hide();
        delete d->hudWidget;
    }

    d->hudWidget = hudWidget;

    if (d->hudWidget)
    {
        d->hudWidget->setParentItem(this);
        d->hudWidget->installEventFilter(this);
        d->updateHudWidgetPosition();
    }
}

QGraphicsWidget* RegionFrameItem::hudWidget() const
{
    return d->hudWidget;
}

void RegionFrameItem::setHudWidgetVisible(bool visible)
{
    if (d->hudWidget)
    {
        d->hudWidget->setVisible(visible);
    }
}

void RegionFrameItem::hudSizeChanged()
{
    d->updateHudWidgetPosition();
}

void RegionFrameItem::moveHudWidget()
{
    const QPointF delta   = d->hudEndPos - d->hudWidget->pos();
    const double distance = sqrt(pow(delta.x(), 2) + pow(delta.y(), 2));
    QPointF pos;

    if (distance > double(d->HUD_TIMER_MAX_PIXELS_PER_UPDATE))
    {
        pos = d->hudWidget->pos() + delta * double(d->HUD_TIMER_MAX_PIXELS_PER_UPDATE) / distance;
    }
    else
    {
        pos = d->hudEndPos;
        d->hudTimer->stop();
    }

    d->hudWidget->setPos(pos);
}

} // namespace Digikam
