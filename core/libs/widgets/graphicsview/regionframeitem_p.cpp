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

RegionFrameItem::Private::Private(RegionFrameItem* const qq)
    : q(qq)
{
    cropHandleList << CH_Left       << CH_Right
                   << CH_Top        << CH_Bottom
                   << CH_TopLeft    << CH_TopRight
                   << CH_BottomLeft << CH_BottomRight;
}

QRectF RegionFrameItem::Private::handleRect(CropHandle handle) const
{
    QSizeF size = q->boundingRect().size();
    double left, top;

    if      (handle & CH_Top)
    {
        top = 0;
    }
    else if (handle & CH_Bottom)
    {
        top = size.height() - HANDLE_SIZE;
    }
    else
    {
        top = (size.height() - HANDLE_SIZE) / 2;
    }

    if      (handle & CH_Left)
    {
        left = 0;
    }
    else if (handle & CH_Right)
    {
        left = size.width() - HANDLE_SIZE;
    }
    else
    {
        left = (size.width() - HANDLE_SIZE) / 2;
    }

    return QRectF(left, top, HANDLE_SIZE, HANDLE_SIZE);
}

CropHandle RegionFrameItem::Private::handleAt(const QPointF& pos) const
{
    if (flags & ShowResizeHandles)
    {
        for (const CropHandle& handle : std::as_const(cropHandleList))
        {
            QRectF rect = handleRect(handle);

            if (rect.contains(pos))
            {
                return handle;
            }
        }
    }

    if (flags & MoveByDrag)
    {
        if (q->boundingRect().contains(pos))
        {
            return CH_Content;
        }
    }


    return CH_None;
}

void RegionFrameItem::Private::updateCursor(CropHandle handle, bool buttonDown)
{
    Qt::CursorShape shape;

    switch (handle)
    {
        case CH_TopLeft:
        case CH_BottomRight:
        {
            shape = Qt::SizeFDiagCursor;
            break;
        }

        case CH_TopRight:
        case CH_BottomLeft:
        {
            shape = Qt::SizeBDiagCursor;
            break;
        }

        case CH_Left:
        case CH_Right:
        {
            shape = Qt::SizeHorCursor;
            break;
        }

        case CH_Top:
        case CH_Bottom:
        {
            shape = Qt::SizeVerCursor;
            break;
        }

        case CH_Content:
        {
            shape = buttonDown ? Qt::ClosedHandCursor : Qt::OpenHandCursor;
            break;
        }

        default:
        {
            shape = Qt::ArrowCursor;
            break;
        }
    }

    q->setCursor(shape);
}

QRectF RegionFrameItem::Private::keepRectInsideImage(const QRectF& rect, bool moving) const
{
    QRectF r(rect);
    const QSizeF imageSize = q->parentDImgItem()->boundingRect().size();

    if ((r.width() > imageSize.width()) || (r.height() > imageSize.height()))
    {
        // This can happen when the crop ratio changes

        QSizeF rectSize = r.size();
        rectSize.scale(imageSize, Qt::KeepAspectRatio);
        r.setSize(rectSize);
    }

    if      (r.right() > imageSize.width())
    {
        moving ? r.moveRight(imageSize.width()) : r.setRight(imageSize.width());
    }
    else if (r.left() < 0)
    {
        moving ? r.moveLeft(0) : r.setLeft(0);
    }

    if      (r.bottom() > imageSize.height())
    {
        moving ? r.moveBottom(imageSize.height()) : r.setBottom(imageSize.height());
    }
    else if (r.top() < 0)
    {
        moving ? r.moveTop(0) : r.setTop(0);
    }

    return r;
}

OptimalPosition RegionFrameItem::Private::computeOptimalHudWidgetPosition() const
{
    const QRectF visibleSceneRect = viewportRect.isValid() ? viewportRect : q->scene()->sceneRect();
    const QRectF rect             = q->sceneBoundingRect();

    const int margin              = HANDLE_SIZE;
    const int hudHeight           = hudWidget->boundingRect().height();
    const QRectF hudMaxRect       = visibleSceneRect.adjusted(0, 0, 0, -hudHeight);

    OptimalPosition ret;

    // Compute preferred and fallback positions. Preferred is outside rect
    // on the same side, fallback is outside on the other side.

    OptimalPosition preferred     = OptimalPosition(QPointF(rect.left(), rect.bottom() + margin),          HS_Bottom);
    OptimalPosition fallback      = OptimalPosition(QPointF(rect.left(), rect.top() - margin - hudHeight), HS_Top);

    if (hudSide & HS_Top)
    {
        std::swap(preferred, fallback);
    }

    // Check if a position outside rect fits

    if      (hudMaxRect.contains(preferred.first))
    {
        ret = preferred;
    }
    else if (hudMaxRect.contains(fallback.first))
    {
        ret= fallback;
    }
    else
    {
        // Does not fit outside, use a position inside rect

        QPoint pos;

        if (hudSide & HS_Top)
        {
            pos = QPoint(rect.left() + margin, rect.top() + margin);
        }
        else
        {
            pos = QPoint(rect.left() + margin, rect.bottom() - margin - hudHeight);
        }

        ret = OptimalPosition(pos, HudSide(hudSide | HS_Inside));
    }

    // Ensure it's always fully visible

    ret.first.rx() = qMin(ret.first.rx(), hudMaxRect.width() - hudWidget->boundingRect().width());

    // map from scene to item coordinates

    ret.first      = q->mapFromScene(ret.first);

    return ret;
}

void RegionFrameItem::Private::updateHudWidgetPosition()
{
    if (!hudWidget || !q->scene())
    {
        return;
    }

    OptimalPosition result = computeOptimalHudWidgetPosition();

    if ((result.first == hudWidget->pos()) && (result.second == hudSide))
    {
        return;
    }

    if (hudSide == HS_None)
    {
        hudSide = result.second;
    }

    if ((hudSide == result.second) && !hudTimer->isActive())
    {
        // Not changing side and not in an animation, move directly the hud
        // to the final position to avoid lagging effect

        hudWidget->setPos(result.first);
    }
    else
    {
        hudEndPos = result.first;
        hudSide   = result.second;

        if (!hudTimer->isActive())
        {
            hudTimer->start();
        }
    }
}

} // namespace Digikam
