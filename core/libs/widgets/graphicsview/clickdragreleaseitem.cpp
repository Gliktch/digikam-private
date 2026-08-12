/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-04
 * Description : A simple item to click, drag and release
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "clickdragreleaseitem.h"

// Qt includes

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QObject>
#include <QPointF>

// Local includes

#include "digikam_debug.h"
#include "itemvisibilitycontroller.h"

namespace Digikam
{

enum ClickDragState
{
    HoverState,
    PressedState,
    PressDragState,
    ClickedMoveState
};

class Q_DECL_HIDDEN ClickDragReleaseItem::Private
{
public:

    Private() = default;

    template <class Event> bool isDrag(Event* e)
    {
        return ((pressPos - e->scenePos()).manhattanLength() > QApplication::startDragDistance());
    }

    template <class Event> QRectF rect(Event* e)
    {
        return QRectF(pressPos, e->scenePos()).normalized();
    }

public:

    ClickDragState state    = HoverState;
    QPointF        pressPos;
};

// ----------------------------------------------------------------------------

ClickDragReleaseItem::ClickDragReleaseItem(QGraphicsItem* const parent)
    : QGraphicsObject(parent),
      d              (new Private)
{
    setCursor(Qt::CrossCursor);
    setFlags(ItemIsFocusable | ItemHasNoContents);
}

ClickDragReleaseItem::~ClickDragReleaseItem()
{
    delete d;
}

QRectF ClickDragReleaseItem::boundingRect() const
{
    if (parentItem())
    {
        return QRectF(QPointF(0, 0), parentItem()->boundingRect().size());
    }

    return QRectF();
}

void ClickDragReleaseItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}

/**
 * 1) Press - Drag - Release:
 *    mousePress, PressedState -> mouseMoveEvent over threshold, PressDragState -> mouseReleaseEvent, finished
 * 2) Click - Move - Click:
 *    mousePressEvent, PressedState -> mouseReleaseEvent, ClickedMoveState ->
 *    hoverMoveEvent -> mouseReleaseEvent, finished
 */

void ClickDragReleaseItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        Q_EMIT cancelled();

        return;
    }

    if (d->state == HoverState)
    {
        d->pressPos = e->scenePos();
        d->state    = PressedState;

        Q_EMIT started(e->scenePos());
    }
}

void ClickDragReleaseItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if ((d->state == PressedState) && d->isDrag(e))
    {
        d->state = PressDragState;
        setCursor(Qt::SizeFDiagCursor);
    }

    if (d->state == PressDragState)
    {
        Q_EMIT moving(d->rect(e));
    }
}

void ClickDragReleaseItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    if (d->state == ClickedMoveState)
    {
        Q_EMIT moving(d->rect(e));
    }
}

void ClickDragReleaseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if      (d->state == PressedState)
    {
        // click-move-click mode first click.
        // It cannot be over the drag threshold in this state, would be caught in moveEvent.

        d->state = ClickedMoveState;
        setCursor(Qt::SizeFDiagCursor);
        setAcceptHoverEvents(true);
    }
    else if (d->state == ClickedMoveState)
    {
        // click-move-click mode second click

        d->state = HoverState;
        setCursor(Qt::CrossCursor);
        setAcceptHoverEvents(false);

        Q_EMIT finished(d->rect(e));
    }
    else if (d->state == PressDragState)
    {
        if (d->isDrag(e))
        {
            Q_EMIT finished(d->rect(e));
        }
        else
        {
            Q_EMIT cancelled();
        }

        d->state = HoverState;
        setCursor(Qt::CrossCursor);
        setAcceptHoverEvents(false);
    }
}

void ClickDragReleaseItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    e->ignore();
}

void ClickDragReleaseItem::keyPressEvent(QKeyEvent* e)
{
    qCDebug(DIGIKAM_WIDGETS_LOG) << e;

    switch (e->key())
    {
        case Qt::Key_Backspace:
        case Qt::Key_Escape:
        {
            Q_EMIT cancelled();

            break;
        }

        default:
        {
            e->ignore();
            break;
        }
    }
}

} // namespace Digikam

#include "moc_clickdragreleaseitem.cpp"
