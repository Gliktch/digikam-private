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

void RegionFrameItem::setFlags(Flags flags)
{
    if (d->flags == flags)
    {
        return;
    }

    d->flags = flags;
    update();
    setAcceptHoverEvents(d->flags & GeometryEditable);
    d->resizeHandleVisibility->controller()->setShallBeShown(d->flags & ShowResizeHandles);

    // ensure cursor is reset

    CropHandle handle = d->handleAt(QCursor::pos());
    d->updateCursor(handle, false/* buttonDown*/);
}

void RegionFrameItem::changeFlags(Flags flags, bool addOrRemove)
{
    if (addOrRemove)
    {
        setFlags(d->flags | flags);
    }
    else
    {
        setFlags(d->flags & ~flags);
    }
}

RegionFrameItem::Flags RegionFrameItem::flags() const
{
    return d->flags;
}

void RegionFrameItem::setFixedRatio(double ratio)
{
    d->fixedRatio = ratio;
}

void RegionFrameItem::setRectInSceneCoordinatesAdjusted(const QRectF& rect)
{
    setRectInSceneCoordinates(d->keepRectInsideImage(rect, false));
}

void RegionFrameItem::setViewportRect(const QRectF& rect)
{
    d->viewportRect = rect;
    d->updateHudWidgetPosition();
}

} // namespace Digikam
