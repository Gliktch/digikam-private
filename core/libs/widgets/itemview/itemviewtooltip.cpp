/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : A DItemToolTip prepared for use in QAbstractItemViews
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemviewtooltip.h"

// Qt includes

#include <QApplication>
#include <QToolTip>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemViewToolTip::Private
{
public:

    Private() = default;

    QAbstractItemView* view             = nullptr;
    QModelIndex        index;
    QRect              rect;
    QString            text;
    bool               filterInstalled  = false;
};

ItemViewToolTip::ItemViewToolTip(QAbstractItemView* const view)
    : DItemToolTip(view),
      d           (new Private)
{
    d->view = view;

    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setMouseTracking(true);
}

ItemViewToolTip::~ItemViewToolTip()
{
    delete d;
}

QAbstractItemView* ItemViewToolTip::view() const
{
    return d->view;
}

QAbstractItemModel* ItemViewToolTip::model() const
{
    return (d->view ? d->view->model() : nullptr);
}

QModelIndex ItemViewToolTip::currentIndex() const
{
    return d->index;
}

void ItemViewToolTip::show(const QStyleOptionViewItem& option, const QModelIndex& index)
{
    d->index = index;
    d->rect  = option.rect;
    d->rect.moveTopLeft(d->view->viewport()->mapToGlobal(d->rect.topLeft()));
    updateToolTip();
    reposition();

    if (isHidden() && !toolTipIsEmpty())
    {
        if (!d->filterInstalled)
        {
            qApp->installEventFilter(this);
            d->filterInstalled = true;
        }

        DItemToolTip::show();
    }
}

void ItemViewToolTip::setTipContents(const QString& tipContents)
{
    d->text = tipContents;
    updateToolTip();
}

QString ItemViewToolTip::tipContents()
{
    return d->text;
}

QRect ItemViewToolTip::repositionRect()
{
    return d->rect;
}

void ItemViewToolTip::hideEvent(QHideEvent*)
{
    d->rect  = QRect();
    d->index = QModelIndex();

    if (d->filterInstalled)
    {
        d->filterInstalled = false;
        qApp->removeEventFilter(this);
    }
}

// The following code is inspired by qtooltip.cpp,
// Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).

bool ItemViewToolTip::eventFilter(QObject* o, QEvent* e)
{
    switch (e->type())
    {

#ifdef Q_OS_MACOS

        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        {
            int key                    = static_cast<QKeyEvent*>(e)->key();
            Qt::KeyboardModifiers mody = static_cast<QKeyEvent*>(e)->modifiers();

            if (
                !(mody & Qt::KeyboardModifierMask) &&
                (key != Qt::Key_Shift)             &&
                (key != Qt::Key_Control)           &&
                (key != Qt::Key_Alt)               &&
                (key != Qt::Key_Meta)
               )
            {
                hide();
            }

            break;
        }

#endif // Q_OS_MACOS

        case QEvent::Leave:
        {
            hide(); // could add a 300ms timer here, like Qt
            break;
        }

        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
        {
            hide();
            break;
        }

        case QEvent::MouseMove:
        {
            // needs mouse tracking, obviously
            if (
                (o == d->view->viewport()) &&
                !d->rect.isNull()          &&

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                !d->rect.contains(static_cast<QMouseEvent*>(e)->globalPosition().toPoint())

#else

                !d->rect.contains(static_cast<QMouseEvent*>(e)->globalPos())

#endif

               )
            {
                hide();
            }

            break;
        }

        default:
        {
            break;
        }
    }

    return false;
}

void ItemViewToolTip::mouseMoveEvent(QMouseEvent* e)
{
    if (d->rect.isNull())
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QPoint pos = e->globalPosition().toPoint();

#else

    QPoint pos = e->globalPos();

#endif

    pos        = d->view->viewport()->mapFromGlobal(pos);

    if (!d->rect.contains(pos))
    {
        hide();
    }

    DItemToolTip::mouseMoveEvent(e);
}

} // namespace Digikam

#include "moc_itemviewtooltip.cpp"
