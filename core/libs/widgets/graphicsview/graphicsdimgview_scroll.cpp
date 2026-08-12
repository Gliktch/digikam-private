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

void GraphicsDImgView::slotContentsMoved()
{
    Q_EMIT contentsMoving(horizontalScrollBar()->value(),
                          verticalScrollBar()->value());

    if (d->magnifierEnabled && d->item)
    {
        updateMagnifier();
    }

    viewport()->update();
}

void GraphicsDImgView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);

    Q_EMIT viewportRectChanged(mapToScene(viewport()->rect()).boundingRect());
}

void GraphicsDImgView::scrollPointOnPoint(const QPointF& scenePos, const QPoint& viewportPos)
{
    // This is inspired from QGraphicsView's centerOn()

    QPointF viewPoint = transform().map(scenePos);

    if (horizontalScrollBar()->maximum())
    {
        if (isRightToLeft())
        {
            qint64 horizontal = 0;
            horizontal       += horizontalScrollBar()->minimum();
            horizontal       += horizontalScrollBar()->maximum();
            horizontal       -= int(viewPoint.x() - viewportPos.x());
            horizontalScrollBar()->setValue(horizontal);
        }
        else
        {
            horizontalScrollBar()->setValue(int(viewPoint.x() - viewportPos.x()));
        }
    }

    if (verticalScrollBar()->maximum())
    {
        verticalScrollBar()->setValue(int(viewPoint.y() - viewportPos.y()));
    }

    viewport()->update();
}

} // namespace Digikam
