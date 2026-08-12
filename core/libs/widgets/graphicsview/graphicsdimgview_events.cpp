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

bool GraphicsDImgView::viewportEvent(QEvent* event)
{
    if      (event->type() == QEvent::Leave)
    {
        QApplication::restoreOverrideCursor();
        d->magnifier->setVisible(false);
    }
    else if (event->type() == QEvent::Enter)
    {
        if (d->magnifierEnabled)
        {
            QApplication::setOverrideCursor(Qt::CrossCursor);
            d->magnifier->setVisible(true);
        }
    }

    return QGraphicsView::viewportEvent(event);
}

void GraphicsDImgView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QGraphicsView::mouseDoubleClickEvent(e);

    if (!acceptsMouseClick(e))
    {
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        Q_EMIT leftButtonDoubleClicked();

        if (!qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
        {
            Q_EMIT activated();
        }
    }
}

void GraphicsDImgView::mousePressEvent(QMouseEvent* e)
{
    QGraphicsView::mousePressEvent(e);

    d->mousePressPos    = QPoint();
    d->movingInProgress = false;

    if (!acceptsMouseClick(e))
    {
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        Q_EMIT leftButtonClicked();
    }

    if (
        (e->button() == Qt::LeftButton)   ||
        (e->button() == Qt::MiddleButton)
       )
    {
        d->mousePressPos = e->pos();

        if (
            !qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick) ||
            (e->button() == Qt::MiddleButton)
           )
        {
            startPanning(e->pos());
        }

        return;
    }

    if (e->button() == Qt::RightButton)
    {
        Q_EMIT rightButtonClicked();
    }
}

void GraphicsDImgView::mouseReleaseEvent(QMouseEvent* e)
{
    QGraphicsView::mouseReleaseEvent(e);

    // Do not call acceptsMouseClick() here, only on press. Seems that release event are accepted per default.

    if (
        ((e->button() == Qt::LeftButton) || (e->button() == Qt::MiddleButton)) &&
        !d->mousePressPos.isNull()
       )
    {
        if (!d->movingInProgress && (e->button() == Qt::LeftButton))
        {
            if (qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
            {
                Q_EMIT activated();
            }
        }
        else
        {
            finishPanning();
        }
    }

    d->movingInProgress = false;
    d->mousePressPos    = QPoint();
}

bool GraphicsDImgView::acceptsMouseClick(QMouseEvent* e)
{
    // the basic condition is that now item ate the event

    if (e->isAccepted())
    {
        return false;
    }

    return true;
}

void GraphicsDImgView::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
    d->layout->updateZoomAndSize();

    Q_EMIT resized();
    Q_EMIT viewportRectChanged(mapToScene(viewport()->rect()).boundingRect());
}

void GraphicsDImgView::wheelEvent(QWheelEvent* e)
{
    int p = this->verticalScrollBar()->sliderPosition();

    if      (e->modifiers() & Qt::ShiftModifier)
    {
        e->accept();

        if      (e->angleDelta().y() < 0)
        {
            Q_EMIT toNextImage();
        }
        else if (e->angleDelta().y() > 0)
        {
            Q_EMIT toPreviousImage();
        }

        return;
    }
    else if (e->modifiers() & Qt::ControlModifier)
    {
        // When zooming with the mouse-wheel, the image center is kept fixed.

        QPoint point = e->position().toPoint();

        if      (e->angleDelta().y() < 0)
        {
            d->layout->decreaseZoom(point);
        }
        else if (e->angleDelta().y() > 0)
        {
            d->layout->increaseZoom(point);
        }

        return;
    }

    else if (
             ((p == this->verticalScrollBar()->maximum()) && (e->angleDelta().y() < 0)) ||
             ((p == this->verticalScrollBar()->minimum()) && (e->angleDelta().y() > 0))
            )
    {
        // I had to add this condition for "ImageBrushGuideWidget" that subclasses ImageRegionWidget, used
        // in the healingclone tool.
        // If I remove that condition, this event handler gets called recursively and the program
        // crashes.T I couldn't figure out the reason. [Ahmed Fathy]

        return;
    }
    else
    {
        QGraphicsView::wheelEvent(e);
    }
}

void GraphicsDImgView::gestureEvent(QGestureEvent* event)
{
    // Handle pinch gesture event to zoom in/out centered on mouse position.

    if (QGesture* const pinch = event->gesture(Qt::PinchGesture))
    {
        QPinchGesture* const pinchGesture = dynamic_cast<QPinchGesture*>(pinch);

        if (pinchGesture && (pinchGesture->changeFlags() & QPinchGesture::ScaleFactorChanged))
        {
            QPointF center    = pinchGesture->centerPoint();
            qreal scaleFactor = pinchGesture->scaleFactor();
            d->layout->zoomByFactor(scaleFactor, center.toPoint());
            event->accept();

            return;
        }
    }

    // Handle pan gesture event to move contents.

    if (QGesture* const pan = event->gesture(Qt::PanGesture))
    {
        QPanGesture* const panGesture = dynamic_cast<QPanGesture*>(pan);

        if (panGesture)
        {
            if      (panGesture->state() == Qt::GestureStarted)
            {
                d->panningScrollPos = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
                d->mousePressPos    = panGesture->hotSpot().toPoint();
                viewport()->setCursor(Qt::SizeAllCursor);
                d->movingInProgress = true;
                event->accept();

                return;
            }
            else if (panGesture->state() == Qt::GestureUpdated)
            {
                QPointF delta = panGesture->lastOffset();
                horizontalScrollBar()->setValue(d->panningScrollPos.x() - static_cast<int>(delta.x()));
                verticalScrollBar()->setValue(d->panningScrollPos.y() - static_cast<int>(delta.y()));
                viewport()->update();
                event->accept();

                return;
            }
            else if (panGesture->state() == Qt::GestureFinished)
            {
                viewport()->unsetCursor();
                d->movingInProgress = false;
                event->accept();

                return;
            }
        }
    }

    // Handle swipe gesture event to change current item.

    if (QGesture* const swipe = event->gesture(Qt::SwipeGesture))
    {
        QSwipeGesture* const swipeGesture = dynamic_cast<QSwipeGesture*>(swipe);

        if (swipeGesture && (swipeGesture->state() == Qt::GestureFinished))
        {
            if      (swipeGesture->horizontalDirection() == QSwipeGesture::Left)
            {
                Q_EMIT toNextImage();
            }
            else if (swipeGesture->horizontalDirection() == QSwipeGesture::Right)
            {
                Q_EMIT toPreviousImage();
            }

            event->accept();

            return;
        }
    }

    event->ignore();
}

bool GraphicsDImgView::event(QEvent* event)
{
    if (event->type() == QEvent::Gesture)
    {
        gestureEvent(static_cast<QGestureEvent*>(event));

        return true;
    }

    return QGraphicsView::event(event);
}

void GraphicsDImgView::mouseMoveEvent(QMouseEvent* e)
{
    QGraphicsView::mouseMoveEvent(e);

    if (d->magnifierEnabled && d->item)
    {
        updateMagnifier();
    }

    if (
        ((e->buttons() & Qt::LeftButton) || (e->buttons() & Qt::MiddleButton)) &&
        !d->mousePressPos.isNull()
       )
    {
        if (!d->movingInProgress && (e->buttons() & Qt::LeftButton))
        {
            if ((d->mousePressPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
            {
                startPanning(d->mousePressPos);
            }
        }

        if (d->movingInProgress)
        {
            continuePanning(e->pos());
        }
    }
}

} // namespace Digikam
