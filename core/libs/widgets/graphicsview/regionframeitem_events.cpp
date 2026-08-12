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

void RegionFrameItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    const QColor borderColor = QColor::fromHsvF(0, 0, 1.0, 0.66 + 0.34 * d->hoverAnimationOpacity);
    const QColor fillColor   = QColor::fromHsvF(0, 0, 0.75, 0.66);

    QRectF drawRect = boundingRect();

    // Draw the base rectangle

    painter->setPen(borderColor);
    painter->drawRect(drawRect);

    // Draw the "marching ants" manually if the timer is on

    if (d->marchingAntsTimer->isActive())
    {
        QPen antsPen(Qt::black, 1);
        painter->setPen(antsPen);

        int dashSize = 2; // Size of a chunk
        int gapSize = 2;  // Size of the space between chunks
        int patternLength = dashSize + gapSize;

        // Compute the start position depending of the offset

        int startOffset = d->marchingAntsOffset;

        // Draw the chunks on the 4 sides of the rectangle

        // Top side

        for (int x = 0 ; x < drawRect.width() ; x += patternLength)
        {
            int xStart = x + startOffset;

            if (xStart < drawRect.width())
            {
                int xEnd = std::min(xStart + dashSize, static_cast<int>(drawRect.width()));
                painter->drawLine(xStart, drawRect.top(), xEnd, drawRect.top());
            }
        }

        // Bottom side

        for (int x = 0 ; x < drawRect.width() ; x += patternLength)
        {
            int xStart = x + startOffset;

            if (xStart < drawRect.width())
            {
                int xEnd = std::min(xStart + dashSize, static_cast<int>(drawRect.width()));
                painter->drawLine(xStart, drawRect.bottom(), xEnd, drawRect.bottom());
            }
        }

        // Left side

        for (int y = 0 ; y < drawRect.height() ; y += patternLength)
        {
            int yStart = y + startOffset;

            if (yStart < drawRect.height())
            {
                int yEnd = std::min(yStart + dashSize, static_cast<int>(drawRect.height()));
                painter->drawLine(drawRect.left(), yStart, drawRect.left(), yEnd);
            }
        }

        // Right side

        for (int y = 0 ; y < drawRect.height() ; y += patternLength)
        {
            int yStart = y + startOffset;

            if (yStart < drawRect.height())
            {
                int yEnd = std::min(yStart + dashSize, static_cast<int>(drawRect.height()));
                painter->drawLine(drawRect.right(), yStart, drawRect.right(), yEnd);
            }
        }
    }

    // Draw the anchors to resize thz rectangle.

    if (d->resizeHandleVisibility->isVisible())
    {
        if (d->movingHandle == CH_None)
        {
            painter->setOpacity(d->resizeHandleVisibility->opacity());
            painter->setBrush(fillColor);

            for (const CropHandle& handle : std::as_const(d->cropHandleList))
            {
                QRectF rect = d->handleRect(handle);
                painter->drawRect(rect);
            }
        }
    }
}

void RegionFrameItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    if (boundingRect().contains(e->pos()))
    {
        d->resizeHandleVisibility->controller()->show();
    }
}

void RegionFrameItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    if (!boundingRect().contains(e->pos()))
    {
        d->resizeHandleVisibility->controller()->hide();
    }
}

void RegionFrameItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    if (boundingRect().contains(e->pos()))
    {
        if (d->flags & GeometryEditable)
        {
            CropHandle handle = d->handleAt(e->pos());
            d->updateCursor(handle, false/* buttonDown*/);
        }

        d->resizeHandleVisibility->controller()->show();
    }
}

void RegionFrameItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // FIXME: Fade out?
/*
    d->hudWidget->hide();
*/
    if (!(d->flags & GeometryEditable))
    {
        DImgChildItem::mousePressEvent(event);
        return;
    }

    d->movingHandle = d->handleAt(event->pos());
    d->updateCursor(d->movingHandle, event->buttons() != Qt::NoButton);

    if (d->movingHandle == CH_Content)
    {
        d->lastMouseMovePos = event->scenePos();
    }

    // Update to hide handles

    update();
}

void RegionFrameItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    const QSizeF maxSize = parentDImgItem()->boundingRect().size();
    const QPointF point  = event->scenePos();
    qreal posX           = qBound<qreal>(0., point.x(), maxSize.width());
    qreal posY           = qBound<qreal>(0., point.y(), maxSize.height());
    QRectF r             = rect();

    // Adjust edge

    if      (d->movingHandle & CH_Top)
    {
        r.setTop(posY);
    }
    else if (d->movingHandle & CH_Bottom)
    {
        r.setBottom(posY);
    }

    if      (d->movingHandle & CH_Left)
    {
        r.setLeft(posX);
    }
    else if (d->movingHandle & CH_Right)
    {
        r.setRight(posX);
    }

    // Normalize rect and handles (this is useful when user drag the right side
    // of the crop rect to the left of the left side)

    if (r.height() < 0)
    {
        d->movingHandle = d->movingHandle ^ (CH_Top | CH_Bottom);
    }

    if (r.width() < 0)
    {
        d->movingHandle = d->movingHandle ^ (CH_Left | CH_Right);
    }

    r = r.normalized();

    // Enforce ratio

    if (d->fixedRatio > 0.)
    {
        if      ((d->movingHandle == CH_Top) || (d->movingHandle == CH_Bottom))
        {
            // Top or bottom

            int width = int(r.height() / d->fixedRatio);
            r.setWidth(width);
        }
        else if ((d->movingHandle == CH_Left) || (d->movingHandle == CH_Right))
        {
            // Left or right

            int height = int(r.width() * d->fixedRatio);
            r.setHeight(height);
        }
        else if (d->movingHandle & CH_Top)
        {
            // Top left or top right

            int height = int(r.width() * d->fixedRatio);
            r.setTop(r.bottom() - height);
        }
        else if (d->movingHandle & CH_Bottom)
        {
            // Bottom left or bottom right

            int height = int(r.width() * d->fixedRatio);
            r.setHeight(height);
        }
    }

    if (d->movingHandle == CH_Content)
    {
        QPointF delta       = point - d->lastMouseMovePos;
        r.adjust(delta.x(), delta.y(), delta.x(), delta.y());
        d->lastMouseMovePos = event->scenePos();
    }

    setRect(d->keepRectInsideImage(r));

    d->updateHudWidgetPosition();
}

void RegionFrameItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    // FIXME: Fade in?
/*
    d->hudWidget->show();
*/
    d->movingHandle = CH_None;
    d->updateCursor(d->handleAt(event->pos()), false);

    // Update to show handles

    update();
}

bool RegionFrameItem::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == d->hudWidget) && (event->type() == QEvent::GraphicsSceneResize))
    {
        d->updateHudWidgetPosition();
    }

    return DImgChildItem::eventFilter(watched, event);
}

} // namespace Digikam
