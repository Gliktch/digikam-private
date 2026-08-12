/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-02-01
 * Description : Kinetic Scroller for Thumbnail Bar
 *               based on Razvan Petru implementation.
 *
 * SPDX-FileCopyrightText: 2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotokineticscroller.h"

// Qt includes

#include <QApplication>
#include <QScrollBar>
#include <QMouseEvent>
#include <QEvent>
#include <QTimer>

namespace ShowFoto
{

/**
 *A number of mouse moves are ignored after a press to differentiate
 *it from a press & drag.
 */
static const int gMaxIgnoredMouseMoves = 4;

/**
 * The timer measures the drag speed & handles kinetic scrolling. Adjusting
 * the timer interval will change the scrolling speed and smoothness.
 */
static const int gTimerInterval        = 30;

/**
 * The speed measurement is imprecise, limit it so that the scrolling is not
 * too fast.
 */
static const int gMaxDecelerationSpeed = 30;

/**
 * influences how fast the scroller decelerates
 */
static const int gFriction             = 1;

class Q_DECL_HIDDEN ShowfotoKineticScroller::Private
{
public:

    Private() = default;

    void stopMotion()
    {
        isMoving = false;
        velocity = 0;
        kineticTimer.stop();
    }

public:

    bool                 isPressed              = false;
    bool                 isMoving               = false;

    int                  lastMouseYPos          = 0;
    int                  lastMouseXPos          = 0;
    int                  lastScrollBarPosition  = 0;
    int                  velocity               = 0;
    int                  ignoredMouseMoves      = 0;
    int                  ignoredMouseActions    = 0;

    QAbstractScrollArea* scrollArea             = nullptr;
    QPoint               lastPressPoint;
    QTimer               kineticTimer;
    QListView::Flow      scrollFlow             = QListView::TopToBottom;
};

ShowfotoKineticScroller::ShowfotoKineticScroller(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    connect(&d->kineticTimer, &QTimer::timeout,
            this, &ShowfotoKineticScroller::onKineticTimerElapsed);
}

ShowfotoKineticScroller::~ShowfotoKineticScroller()
{
    delete d;
}

void ShowfotoKineticScroller::enableKineticScrollFor(QAbstractScrollArea* const scrollArea)
{
    if (!scrollArea)
    {
        Q_ASSERT_X(0, "kinetic scroller", "missing scroll area");

        return;
    }

    // remove existing association

    if (d->scrollArea)
    {
        d->scrollArea->viewport()->removeEventFilter(this);
        d->scrollArea->removeEventFilter(this);
        d->scrollArea = nullptr;
    }

    // associate

    scrollArea->installEventFilter(this);
    scrollArea->viewport()->installEventFilter(this);
    d->scrollArea = scrollArea;
}

/// intercepts mouse events to make the scrolling work
bool ShowfotoKineticScroller::eventFilter(QObject* object, QEvent* event)
{
    const QEvent::Type eventType = event->type();
    const bool isMouseAction     = QEvent::MouseButtonPress == eventType || QEvent::MouseButtonRelease == eventType;
    const bool isMouseEvent      = isMouseAction || QEvent::MouseMove == eventType;

    if (!isMouseEvent || !d->scrollArea)
    {
        return false;
    }

    if (isMouseAction && (d->ignoredMouseActions-- > 0)) // don't filter simulated click
    {
        return false;
    }

    QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);

    switch (eventType)
    {
        case QEvent::MouseButtonPress:
        {
            d->isPressed      = true;
            d->lastPressPoint = mouseEvent->pos();

            if (d->scrollFlow == QListView::TopToBottom)
            {
                d->lastScrollBarPosition = d->scrollArea->verticalScrollBar()->value();
            }
            else
            {
                d->lastScrollBarPosition = d->scrollArea->horizontalScrollBar()->value();
            }

            if (d->isMoving)
            {
                // press while kinetic scrolling, so stop

                d->stopMotion();
            }

            break;
        }

        case QEvent::MouseMove:
        {
            if      (!d->isMoving && d->isPressed)
            {
                // A few move events are ignored as "click jitter", but after that we
                // assume that the user is doing a click & drag

                if (d->ignoredMouseMoves < gMaxIgnoredMouseMoves)
                {
                    ++d->ignoredMouseMoves;
                }
                else
                {
                    d->ignoredMouseMoves = 0;
                    d->isMoving          = true;

                    if (d->scrollFlow == QListView::TopToBottom)
                    {
                        d->lastMouseYPos = mouseEvent->pos().y();
                    }
                    else
                    {
                        d->lastMouseXPos = mouseEvent->pos().x();
                    }

                    if (!d->kineticTimer.isActive())
                    {
                        d->kineticTimer.start(gTimerInterval);
                    }
                }
            }
            else if (d->isPressed)
            {
                // manual scroll

                if (d->scrollFlow == QListView::TopToBottom)
                {
                    const int dragDistance = mouseEvent->pos().y() - d->lastPressPoint.y();
                    d->scrollArea->verticalScrollBar()->setValue(d->lastScrollBarPosition - dragDistance);
                }
                else
                {
                    const int dragDistance = mouseEvent->pos().x() - d->lastPressPoint.x();
                    d->scrollArea->horizontalScrollBar()->setValue(d->lastScrollBarPosition - dragDistance);
                }
            }

            break;
        }

        case QEvent::MouseButtonRelease:
        {
            d->isPressed = false;

            // Looks like the user wanted a single click. Simulate the click,
            // as the events were already consumed

            if (!d->isMoving)
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))

                QMouseEvent* const mousePress   = new QMouseEvent(QEvent::MouseButtonPress, d->lastPressPoint, QCursor::pos(),

#else

                QMouseEvent* const mousePress   = new QMouseEvent(QEvent::MouseButtonPress, d->lastPressPoint,

#endif

                                                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))

                QMouseEvent* const mouseRelease = new QMouseEvent(QEvent::MouseButtonRelease, d->lastPressPoint, QCursor::pos(),

#else

                QMouseEvent* const mouseRelease = new QMouseEvent(QEvent::MouseButtonRelease, d->lastPressPoint,

#endif

                                                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

                d->ignoredMouseActions          = 2;

                QApplication::postEvent(object, mousePress);
                QApplication::postEvent(object, mouseRelease);
            }

            break;
        }

        default:
        {
            // Nothing to do here.

            break;
        }
    }

    return true; // filter event
}

void ShowfotoKineticScroller::onKineticTimerElapsed()
{
    if      (d->isPressed && d->isMoving)
    {
        if (d->scrollFlow == QListView::TopToBottom)
        {
            // the speed is measured between two timer ticks

            const int cursorYPos = d->scrollArea->mapFromGlobal(QCursor::pos()).y();
            d->velocity          = cursorYPos - d->lastMouseYPos;
            d->lastMouseYPos     = cursorYPos;
        }
        else
        {
            const int cursorXPos = d->scrollArea->mapFromGlobal(QCursor::pos()).x();
            d->velocity          = cursorXPos - d->lastMouseXPos;
            d->lastMouseXPos     = cursorXPos;
        }
    }
    else if (!d->isPressed && d->isMoving)
    {
        // use the previously recorded speed and gradually decelerate

        d->velocity = qBound(-gMaxDecelerationSpeed, d->velocity, gMaxDecelerationSpeed);

        if      (d->velocity > 0)
        {
            d->velocity -= gFriction;
        }
        else if (d->velocity < 0)
        {
            d->velocity += gFriction;
        }

        if (qAbs(d->velocity) < qAbs(gFriction))
        {
            d->stopMotion();
        }

        if (d->scrollFlow == QListView::TopToBottom)
        {
            const int scrollBarYPos = d->scrollArea->verticalScrollBar()->value();
            d->scrollArea->verticalScrollBar()->setValue(scrollBarYPos - d->velocity);
        }
        else
        {
            const int scrollBarXPos = d->scrollArea->horizontalScrollBar()->value();
            d->scrollArea->horizontalScrollBar()->setValue(scrollBarXPos - d->velocity);
        }
    }
    else
    {
        d->stopMotion();
    }
}

void ShowfotoKineticScroller::setScrollFlow(QListView::Flow flow)
{
    d->scrollFlow = flow;
}

} // namespace ShowFoto

#include "moc_showfotokineticscroller.cpp"
