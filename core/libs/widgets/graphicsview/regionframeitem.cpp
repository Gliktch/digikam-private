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

RegionFrameItem::RegionFrameItem(QGraphicsItem* const item)
    : DImgChildItem(item),
      d            (new Private(this))
{
    d->resizeHandleVisibility = new AnimatedVisibility(this);
    d->resizeHandleVisibility->controller()->setShallBeShown(false);

    connect(d->resizeHandleVisibility, SIGNAL(visibleChanged()),
            this, SLOT(slotUpdate()));

    connect(d->resizeHandleVisibility, SIGNAL(opacityChanged()),
            this, SLOT(slotUpdate()));

    d->hudTimer = new QTimer(this);
    d->hudTimer->setInterval(d->HUD_TIMER_ANIMATION_INTERVAL);

    connect(scene(), SIGNAL(sceneRectChanged(QRectF)),
            this, SLOT(slotSizeChanged()));

    connect(this, SIGNAL(sizeChanged()),
            this, SLOT(slotSizeChanged()));

    connect(this, SIGNAL(positionChanged()),
            this, SLOT(slotPosChanged()));

    connect(d->hudTimer, SIGNAL(timeout()),
            this, SLOT(moveHudWidget()));

    setFlags(GeometryEditable);

    d->updateHudWidgetPosition();

    // ---

    d->marchingAntsTimer = new QTimer(this);
    d->marchingAntsTimer->setInterval(400); // Interval in ms for the animation

    connect(d->marchingAntsTimer, &QTimer::timeout,
            this, [this]()
        {
            d->marchingAntsOffset = (d->marchingAntsOffset + 1) % 8; // 8 step of a complete cycle.
            update();
        }
    );

    d->marchingAntsTimer->start();
}

RegionFrameItem::~RegionFrameItem()
{
    if (d->hudWidget)
    {
        // See bug #359196: hide or close the QGraphicsWidget before delete it. Possible Qt bug?

        d->hudWidget->hide();
        delete d->hudWidget;
    }

    delete d->marchingAntsTimer;
    delete d;
}

void RegionFrameItem::slotSizeChanged()
{
    d->updateHudWidgetPosition();
}

void RegionFrameItem::slotPosChanged()
{
    d->updateHudWidgetPosition();
}

void RegionFrameItem::slotUpdate()
{
    update();
}

} // namespace Digikam

#include "moc_regionframeitem.cpp"
