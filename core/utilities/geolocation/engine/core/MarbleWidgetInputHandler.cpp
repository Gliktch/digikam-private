/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "MarbleWidgetInputHandler.h"

// Qt includes

#include <QRubberBand>
#include <QToolTip>
#include <QTimer>
#include <QKeyEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

#   include <QEventPoint>

#else

#   include <QTouchEvent>

#endif

// Local includes

#include "MarbleGlobal.h"
#include "MarbleWidget.h"
#include "MarbleMap.h"
#include "ViewportParams.h"
#include "AbstractDataPluginItem.h"
#include "MarbleAbstractPresenter.h"
#include "MarbleWidgetPopupMenu.h"
#include "PopupLayer.h"
#include "RenderPlugin.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN MarbleWidgetInputHandlerPrivate
{
    class Q_DECL_HIDDEN MarbleWidgetSelectionRubber : public AbstractSelectionRubber
    {
    public:

        explicit MarbleWidgetSelectionRubber(MarbleWidget* widget)
            : m_rubberBand(QRubberBand::Rectangle, widget)
        {
            m_rubberBand.hide();
        }

        void show() override
        {
            m_rubberBand.show();
        }

        void hide() override
        {
            m_rubberBand.hide();
        }

        bool isVisible() const override
        {
            return m_rubberBand.isVisible();
        }

        const QRect& geometry() const override
        {
            return m_rubberBand.geometry();
        }

        void setGeometry(const QRect& geometry) override
        {
            m_rubberBand.setGeometry(geometry);
        }

    private:

        QRubberBand m_rubberBand;
    };

public:

    MarbleWidgetInputHandlerPrivate(MarbleWidgetInputHandler* handler, MarbleWidget* widget,
                                    MarbleAbstractPresenter* presenter)
        : m_inputHandler    (handler)
        , m_marbleWidget    (widget)
        , m_marblePresenter (presenter)
        , m_selectionRubber (widget)
    {
        const auto plugs = widget->renderPlugins();

        for (RenderPlugin* const renderPlugin : plugs)
        {
            if (renderPlugin->isInitialized())
            {
                installPluginEventFilter(renderPlugin);
            }
        }

        m_marbleWidget->grabGesture(Qt::PinchGesture);
    }

    void setCursor(const QCursor& cursor)
    {
        m_marbleWidget->setCursor(cursor);
    }

    bool layersEventFilter(QObject* o, QEvent* e)
    {
        // FIXME - this should go up in hierarchy to MarbleInputHandler

        if (m_marbleWidget->popupLayer()->eventFilter(o, e))
        {
            return true;
        }

        return false;
    }

    void installPluginEventFilter(RenderPlugin* renderPlugin)
    {
        m_marbleWidget->installEventFilter(renderPlugin);
    }

    MarbleWidgetInputHandler*   m_inputHandler      = nullptr;
    MarbleWidget*               m_marbleWidget      = nullptr;
    MarbleAbstractPresenter*    m_marblePresenter   = nullptr;
    MarbleWidgetSelectionRubber m_selectionRubber;
    bool                        m_debugModeEnabled  = false;
    bool                        m_pinchDetected     = false;
    bool                        m_panDetected       = false;
};

void MarbleWidgetInputHandler::setCursor(const QCursor& cursor)
{
    d->setCursor(cursor);
}

bool MarbleWidgetInputHandler::handleKeyPress(QKeyEvent* event)
{
    if (d->m_debugModeEnabled)
    {
        if (event->modifiers() == Qt::ControlModifier && d->m_marbleWidget->debugLevelTags())
        {
            switch (event->key())
            {
                case Qt::Key_0:
                {
                    d->m_marbleWidget->setLevelToDebug(0);
                    break;
                }

                case Qt::Key_1:
                {
                    d->m_marbleWidget->setLevelToDebug(1);
                    break;
                }

                case Qt::Key_2:
                {
                    d->m_marbleWidget->setLevelToDebug(2);
                    break;
                }

                case Qt::Key_3:
                {
                    d->m_marbleWidget->setLevelToDebug(3);
                    break;
                }

                case Qt::Key_4:
                {
                    d->m_marbleWidget->setLevelToDebug(4);
                    break;
                }

                case Qt::Key_5:
                {
                    d->m_marbleWidget->setLevelToDebug(5);
                    break;
                }

                case Qt::Key_6:
                {
                    d->m_marbleWidget->setLevelToDebug(6);
                    break;
                }

                case Qt::Key_7:
                {
                    d->m_marbleWidget->setLevelToDebug(7);
                    break;
                }

                case Qt::Key_8:
                {
                    d->m_marbleWidget->setLevelToDebug(8);
                    break;
                }

                case Qt::Key_9:
                {
                    d->m_marbleWidget->setLevelToDebug(9);
                    break;
                }

                case Qt::Key_Plus:
                {
                    d->m_marbleWidget->setLevelToDebug(d->m_marbleWidget->levelToDebug() + 1);
                    break;
                }

                case Qt::Key_Minus:
                {
                    d->m_marbleWidget->setLevelToDebug(d->m_marbleWidget->levelToDebug() - 1);
                    break;
                }
            }
        }
        else
        {
            switch (event->key())
            {
                case Qt::Key_R:
                {
                    d->m_marbleWidget->setShowRuntimeTrace(!d->m_marbleWidget->showRuntimeTrace());
                    break;
                }

                case Qt::Key_O:
                {
                    d->m_marbleWidget->setShowDebugPlacemarks(!d->m_marbleWidget->showDebugPlacemarks());
                    break;
                }

                case Qt::Key_P:
                {
                    d->m_marbleWidget->setShowDebugPolygons(!d->m_marbleWidget->showDebugPolygons());
                    break;
                }

                case Qt::Key_B:
                {
                    d->m_marbleWidget->setShowDebugBatchRender(!d->m_marbleWidget->showDebugBatchRender());
                    break;
                }

                case Qt::Key_L:
                {
                    d->m_marbleWidget->setDebugLevelTags(!d->m_marbleWidget->debugLevelTags());
                    break;
                }
            }
        }
    }

    return MarbleDefaultInputHandler::handleKeyPress(event);
}

bool MarbleWidgetInputHandler::handleTouch(QTouchEvent* event)
{
    event->accept();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (event->points().count() == 1)
    {
        QEventPoint p = event->points().at(0);

#else

    if (event->touchPoints().count() == 1)
    {
        QTouchEvent::TouchPoint p = event->touchPoints().at(0);

#endif

        if (event->type() == QEvent::TouchBegin)
        {
            d->m_pinchDetected = false;
            d->m_panDetected   = false;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))

            QMouseEvent press(QMouseEvent::MouseButtonPress, p.position(), QCursor::pos(),

#elif (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QMouseEvent press(QMouseEvent::MouseButtonPress, p.position(),

#else

            QMouseEvent press(QMouseEvent::MouseButtonPress, p.pos(),

#endif

                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

            handleMouseEvent(&press);
        }

        else if (event->type() == QEvent::TouchUpdate)
        {
            if (!d->m_pinchDetected)
            {
                d->m_panDetected = true;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))

                QMouseEvent move(QMouseEvent::MouseMove, p.position(), QCursor::pos(),

#elif (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                QMouseEvent move(QMouseEvent::MouseMove, p.position(),

#else

                QMouseEvent move(QMouseEvent::MouseMove, p.pos(),

#endif

                                 Qt::NoButton, Qt::LeftButton, Qt::NoModifier);

                handleMouseEvent(&move);
            }
        }
        else if (event->type() == QEvent::TouchEnd)
        {
            // avoid triggering mouse clicked signal when we just changed the viewport

            if (d->m_pinchDetected || d->m_panDetected)
            {
                blockSignals(true);
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))

            QMouseEvent release(QMouseEvent::MouseButtonRelease, p.position(), QCursor::pos(),

#elif (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QMouseEvent release(QMouseEvent::MouseButtonRelease, p.position(),

#else

            QMouseEvent release(QMouseEvent::MouseButtonRelease, p.pos(),

#endif

                                Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

            handleMouseEvent(&release);

            if (d->m_pinchDetected || d->m_panDetected)
            {
                if (d->m_pinchDetected)
                {
                    stopInertialEarthRotation();    // ensures we keep the viewport
                }

                blockSignals(false);
            }
        }
    }

    return true;
}

bool MarbleWidgetInputHandler::handleGesture(QGestureEvent* e)
{
    QPinchGesture* const pinch = static_cast<QPinchGesture*>(e->gesture(Qt::PinchGesture));

    if (pinch && !d->m_panDetected)
    {
        d->m_pinchDetected = true;
        handlePinchGesture(pinch);

        return true;
    }

    return false;
}

void MarbleWidgetInputHandler::handlePinchGesture(QPinchGesture* pinch)
{
    MarbleAbstractPresenter* marblePresenter = d->m_marblePresenter;

    switch (pinch->state())
    {
        case Qt::NoGesture:
        {
            break;
        }

        case Qt::GestureStarted:
        {
            marblePresenter->setViewContext(Animation);
            break;
        }

        case Qt::GestureUpdated:
        {
            qreal scaleFactor = pinch->scaleFactor();
            QPoint center = d->m_marbleWidget->mapFromGlobal(pinch->centerPoint().toPoint());

            if (scaleFactor > 1.0)
            {
                scaleFactor = 0.2;
            }

            else if (scaleFactor < 1.0)
            {
                scaleFactor = -0.2;
            }

            else
            {
                return;    // 1 .. no change
            }

            qreal zoom = marblePresenter->zoom();
            bool oldAnim = marblePresenter->animationsEnabled();
            qreal newDistance = marblePresenter->distanceFromZoom(zoom + 20 * scaleFactor);
            marblePresenter->setAnimationsEnabled(false);
            marblePresenter->zoomAt(center, newDistance);
            marblePresenter->setAnimationsEnabled(oldAnim);
            break;
        }

        case Qt::GestureFinished:
        case Qt::GestureCanceled:
        {
            restoreViewContext();
            break;
        }
    }
}

AbstractSelectionRubber* MarbleWidgetInputHandler::selectionRubber()
{
    return &d->m_selectionRubber;
}

bool MarbleWidgetInputHandler::layersEventFilter(QObject* o, QEvent* e)
{
    return d->layersEventFilter(o, e);
}

void MarbleWidgetInputHandler::installPluginEventFilter(RenderPlugin* renderPlugin)
{
    d->installPluginEventFilter(renderPlugin);
}

MarbleWidgetInputHandler::MarbleWidgetInputHandler(MarbleAbstractPresenter* marblePresenter, MarbleWidget* widget)
    : MarbleDefaultInputHandler(marblePresenter)
    , d(new MarbleWidgetInputHandlerPrivate(this, widget, marblePresenter))
{
}

void MarbleWidgetInputHandler::setDebugModeEnabled(bool enabled)
{
    d->m_debugModeEnabled = enabled;
}

/**
 * FIXME - these should be moved to superclass and popupMenu should be abstracted in MarbleAbstractPresenter
 */
void MarbleWidgetInputHandler::showLmbMenu(int x, int y)
{
    if (isMouseButtonPopupEnabled(Qt::LeftButton) && !d->m_pinchDetected && !d->m_panDetected)
    {
        d->m_marbleWidget->popupMenu()->showLmbMenu(x, y);
        toolTipTimer()->stop();
    }
}

void MarbleWidgetInputHandler::showRmbMenu(int x, int y)
{
    if (isMouseButtonPopupEnabled(Qt::RightButton))
    {
        d->m_marbleWidget->popupMenu()->showRmbMenu(x, y);
    }
}

void MarbleWidgetInputHandler::openItemToolTip()
{
    if (lastToolTipItem())
    {
        QToolTip::showText(d->m_marbleWidget->mapToGlobal(toolTipPosition()),
                           lastToolTipItem()->toolTip(),
                           d->m_marbleWidget,
                           lastToolTipItem()->containsRect(toolTipPosition()).toRect());
    }
}

} // namespace Marble

#include "moc_MarbleWidgetInputHandler.cpp"
