/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Vertical and horizontal layout widget helpers.
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dlayoutbox.h"

// Qt includes

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DHBox::DHBox(QWidget* const parent)
    : QFrame(parent)
{
    QHBoxLayout* const layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    setLayout(layout);
}

DHBox::DHBox(bool /*vertical*/, QWidget* const parent)
    : QFrame(parent)
{
    QVBoxLayout* const layout = new QVBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    setLayout(layout);
}

void DHBox::childEvent(QChildEvent* e)
{
    switch (e->type())
    {
        case QEvent::ChildAdded:
        {
            QChildEvent* const ce = static_cast<QChildEvent*>(e);

            if (ce->child()->isWidgetType())
            {
                QWidget* const w = static_cast<QWidget*>(ce->child());
                static_cast<QBoxLayout*>(layout())->addWidget(w);
            }

            break;
        }

        case QEvent::ChildRemoved:
        {
            QChildEvent* const ce = static_cast<QChildEvent*>(e);

            if (ce->child()->isWidgetType())
            {
                QWidget* const w = static_cast<QWidget*>(ce->child());
                static_cast<QBoxLayout*>(layout())->removeWidget(w);
            }

            break;
        }

        default:
        {
            break;
        }
    }

    QFrame::childEvent(e);
}

QSize DHBox::sizeHint() const
{
    DHBox* const b = const_cast<DHBox*>(this);
    QApplication::sendPostedEvents(b, QEvent::ChildAdded);

    return QFrame::sizeHint();
}

QSize DHBox::minimumSizeHint() const
{
    DHBox* const b = const_cast<DHBox*>(this);
    QApplication::sendPostedEvents(b, QEvent::ChildAdded);

    return QFrame::minimumSizeHint();
}

void DHBox::setSpacing(int spacing)
{
    layout()->setSpacing(spacing);
}

void DHBox::setContentsMargins(const QMargins& margins)
{
    layout()->setContentsMargins(margins);
}

void DHBox::setContentsMargins(int left, int top, int right, int bottom)
{
    layout()->setContentsMargins(left, top, right, bottom);
}

void DHBox::setStretchFactor(QWidget* const widget, int stretch)
{
    static_cast<QBoxLayout*>(layout())->setStretchFactor(widget, stretch);
}

// ------------------------------------------------------------------------------------

DVBox::DVBox(QWidget* const parent)
  : DHBox(true, parent)
{
}

} // namespace Digikam

#include "moc_dlayoutbox.cpp"
