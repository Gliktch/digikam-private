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

PanIconWidget* GraphicsDImgView::installPanIcon()
{
    d->pan = new PanIconWidget(this);
    d->pan->setVisible(false);

    connect(d->pan, &PanIconWidget::signalSelectionMoved,
            this, &GraphicsDImgView::slotPanIconSelectionMoved);

    connect(this, &GraphicsDImgView::contentsMoving,
            this, &GraphicsDImgView::slotRefreshPanIconSelection);

    connect(this, &GraphicsDImgView::resized,
            this, &GraphicsDImgView::slotRefreshPanIconSelection);

    connect(this, &GraphicsDImgView::signalZoomFactorChanged,
            this, &GraphicsDImgView::slotRefreshPanIconSelection);

    return d->pan;
}

void GraphicsDImgView::updatePanIconWidget()
{
    viewport()->update();
    d->pan->setImage(180, 120, item()->image());
    QRectF sceneRect(mapToScene(viewport()->rect().topLeft()), mapToScene(viewport()->rect().bottomRight()));
    d->pan->setRegionSelection(item()->zoomSettings()->sourceRect(sceneRect).toRect());
}

void GraphicsDImgView::slotRefreshPanIconSelection()
{
    viewport()->update();
    QRectF sceneRect(mapToScene(viewport()->rect().topLeft()), mapToScene(viewport()->rect().bottomRight()));
    d->pan->setRegionSelection(item()->zoomSettings()->sourceRect(sceneRect).toRect());

    // Manage the visibility of the pan icon widget automatically if one scrollbar is visible.

    QTimer::singleShot(1000, this, [this]()
        {
            d->pan->setVisible(verticalScrollBar()->isVisible() || horizontalScrollBar()->isVisible());
            viewport()->update();
        }
    );
}

void GraphicsDImgView::slotPanIconSelectionMoved(const QRect& imageRect, bool /*b*/)
{
    QRectF zoomRect = item()->zoomSettings()->mapImageToZoom(imageRect);
    qCDebug(DIGIKAM_WIDGETS_LOG) << imageRect << zoomRect;
    centerOn(item()->mapToScene(zoomRect.center()));
    viewport()->update();
}

void GraphicsDImgView::startPanning(const QPoint& pos)
{
    if (horizontalScrollBar()->maximum() || verticalScrollBar()->maximum())
    {
        d->movingInProgress = true;
        d->mousePressPos    = pos;
        d->panningScrollPos = QPoint(horizontalScrollBar()->value(),
                                     verticalScrollBar()->value());
        viewport()->setCursor(Qt::SizeAllCursor);
    }
}

void GraphicsDImgView::continuePanning(const QPoint& pos)
{
    QPoint delta = pos - d->mousePressPos;
    horizontalScrollBar()->setValue(d->panningScrollPos.x() + (isRightToLeft() ? delta.x() : -delta.x()));
    verticalScrollBar()->setValue(d->panningScrollPos.y() - delta.y());

    Q_EMIT contentsMoved(false);

    viewport()->update();
}

void GraphicsDImgView::finishPanning()
{
    Q_EMIT contentsMoved(true);

    viewport()->unsetCursor();
}

} // namespace Digikam
