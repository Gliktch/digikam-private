/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View for DImg preview
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "graphicsdimgview_p.h"

namespace Digikam
{

void GraphicsDImgView::drawForeground(QPainter* p, const QRectF& rect)
{
    QGraphicsView::drawForeground(p, rect);

    QString text = d->item->userLoadingHint();

    if (text.isNull() || !d->showText)
    {
        return;
    }

    QRect viewportRect        = viewport()->rect();
    QRect fontRect            = p->fontMetrics().boundingRect(viewportRect, 0, text);
    QPoint drawingPoint(viewportRect.topRight().x() - fontRect.width() - 10,
                        viewportRect.topRight().y() + 5);

    QPointF sceneDrawingPoint = mapToScene(drawingPoint);
    QRectF sceneDrawingRect(sceneDrawingPoint, fontRect.size());

    if (!rect.intersects(sceneDrawingRect))
    {
        return;
    }

    drawText(p, sceneDrawingRect, text);
}

void GraphicsDImgView::drawText(QPainter* p, const QRectF& rect, const QString& text)
{
    p->save();

    p->setRenderHint(QPainter::Antialiasing, true);
    p->setBackgroundMode(Qt::TransparentMode);

    // increase width by 5 and height by 2

    QRectF textRect    = rect.adjusted(0, 0, 5, 2);

    // Draw background

    p->setPen(Qt::black);
    QColor semiTransBg = palette().color(QPalette::Window);
    semiTransBg.setAlpha(190);
    p->setBrush(semiTransBg);
/*
    p->translate(0.5, 0.5);
*/
    p->drawRoundedRect(textRect, 3.0, 3.0);

    // Draw shadow and text

    p->setPen(palette().color(QPalette::Window).darker(115));
    p->drawText(textRect.translated(3, 1), text);
    p->setPen(palette().color(QPalette::WindowText));
    p->drawText(textRect.translated(2, 0), text);

    p->restore();
}

} // namespace Digikam
