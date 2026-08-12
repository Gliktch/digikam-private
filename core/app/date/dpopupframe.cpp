/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : Frame with popup menu behavior.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 1997      by Tim D. Gilman <tdgilman at best dot org>
 * SPDX-FileCopyrightText: 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * SPDX-FileCopyrightText: 2007      by John Layt <john at layt dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpopupframe.h"

// Qt includes

#include <QApplication>
#include <QEventLoop>
#include <QKeyEvent>
#include <QScreen>
#include <QWindow>

namespace Digikam
{

class Q_DECL_HIDDEN DPopupFrame::Private
{
public:

    explicit Private(DPopupFrame* const qq);
    ~Private();

public:

    DPopupFrame*         q                      = nullptr;

    /**
     * The result. It is returned from exec() when the popup window closes.
     */
    int                  result                 = 0;    // rejected

    /**
     * The only subwidget that uses the whole dialog window.
     */
    QWidget*             main                   = nullptr;

    class OutsideClickCatcher;
    OutsideClickCatcher* outsideClickCatcher    = nullptr;
};

class Q_DECL_HIDDEN DPopupFrame::Private::OutsideClickCatcher : public QObject
{
    Q_OBJECT

public:

    explicit OutsideClickCatcher(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

    ~OutsideClickCatcher() override = default;

    void setPopupFrame(DPopupFrame* const popup)
    {
        m_popup = popup;
        popup->installEventFilter(this);
    }


    bool eventFilter(QObject* object, QEvent* event) override
    {
        Q_UNUSED(object);

        // To catch outside clicks, it is sufficient to check for
        // hide events on Qt::Popup type widgets

        if (event->type() == QEvent::Hide && m_popup)
        {
            // do not set d->result here, because the popup
            // hides itself after leaving the event loop.

            Q_EMIT m_popup->leaveModality();
        }

        return false;
    }

public:

    DPopupFrame* m_popup = nullptr;
};

DPopupFrame::Private::Private(DPopupFrame* const qq)
    : q                  (qq),
      outsideClickCatcher(new OutsideClickCatcher)
{
    outsideClickCatcher->setPopupFrame(q);
}

DPopupFrame::Private::~Private()
{
    delete outsideClickCatcher;
}

DPopupFrame::DPopupFrame(QWidget* const parent)
    : QFrame(parent, Qt::Popup),
      d     (new Private(this))
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setMidLineWidth(2);
}

DPopupFrame::~DPopupFrame()
{
    delete d;
}

void DPopupFrame::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        d->result = 0; // rejected

        Q_EMIT leaveModality();
    }
}

void DPopupFrame::hideEvent(QHideEvent* e)
{
    QFrame::hideEvent(e);
}

void DPopupFrame::close(int r)
{
    d->result = r;

    Q_EMIT leaveModality();
}

void DPopupFrame::setMainWidget(QWidget* const m)
{
    d->main = m;

    if (d->main)
    {
        resize(d->main->width() + 2 * frameWidth(), d->main->height() + 2 * frameWidth());
    }
}

void DPopupFrame::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);

    if (d->main)
    {
        d->main->setGeometry(frameWidth(), frameWidth(),
                             width() - 2 * frameWidth(), height() - 2 * frameWidth());
    }
}

void DPopupFrame::popup(const QPoint& p)
{
    // Make sure the whole popup is visible.

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect desktopGeometry = screen->geometry();

    int x = p.x();
    int y = p.y();
    int w = width();
    int h = height();

    if ((x + w) > (desktopGeometry.x() + desktopGeometry.width()))
    {
        x = desktopGeometry.width() - w;
    }

    if ((y + h) > (desktopGeometry.y() + desktopGeometry.height()))
    {
        y = desktopGeometry.height() - h;
    }

    if (x < desktopGeometry.x())
    {
        x = 0;
    }

    if (y < desktopGeometry.y())
    {
        y = 0;
    }

    // Pop the thingy up.

    move(x, y);
    show();
    d->main->setFocus();
}

int DPopupFrame::exec(const QPoint& p)
{
    popup(p);
    repaint();
    d->result = 0; // rejected
    QEventLoop eventLoop;

    connect(this, &DPopupFrame::leaveModality,
            &eventLoop, &QEventLoop::quit);

    eventLoop.exec();

    hide();

    return d->result;
}

int DPopupFrame::exec(int x, int y)
{
    return exec(QPoint(x, y));
}

} // namespace Digikam

#include "dpopupframe.moc"

#include "moc_dpopupframe.cpp"
