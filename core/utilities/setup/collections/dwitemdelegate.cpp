/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-15
 * Description : widget item delegate for setup collection view
 *
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Rafael Fernández López <ereslibre at kde dot org>
 * SPDX-FileCopyrightText: 2008      by Kevin Ottens <ervin at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dwitemdelegate_p.h"

// Qt includes

#include <QSize>
#include <QTimer>
#include <QTreeView>
#include <QApplication>
#include <QAbstractItemView>

// Local includes

#include "dwitemdelegatepool.h"

namespace Digikam
{

DWItemDelegate::DWItemDelegate(QAbstractItemView* const itemView, QObject* const parent)
    : QAbstractItemDelegate(parent),
      d                    (new DWItemDelegatePrivate(this))
{
    Q_ASSERT(itemView);

    itemView->setMouseTracking(true);
    itemView->viewport()->setAttribute(Qt::WA_Hover);

    d->itemView = itemView;

    itemView->viewport()->installEventFilter(d); // mouse events
    itemView->installEventFilter(d);             // keyboard events

    if (qobject_cast<QTreeView*>(itemView))
    {
        connect(itemView,  SIGNAL(collapsed(QModelIndex)),
                d, SLOT(initializeModel()));

        connect(itemView,  SIGNAL(expanded(QModelIndex)),
                d, SLOT(initializeModel()));
    }
}

DWItemDelegate::~DWItemDelegate()
{
    delete d;
}

QAbstractItemView* DWItemDelegate::itemView() const
{
    return d->itemView;
}

QPersistentModelIndex DWItemDelegate::focusedIndex() const
{
    const QPersistentModelIndex idx = d->widgetPool->d->widgetInIndex.value(QApplication::focusWidget());

    if (idx.isValid())
    {
        return idx;
    }

    // Use the mouse position, if the widget refused to take keyboard focus.

    const QPoint pos = d->itemView->viewport()->mapFromGlobal(QCursor::pos());

    return d->itemView->indexAt(pos);
}

void DWItemDelegate::setBlockedEventTypes(QWidget* const widget, const QList<QEvent::Type>& types) const
{
    widget->setProperty("DigiKam:blockedEventTypes", QVariant::fromValue(types));
}

QList<QEvent::Type> DWItemDelegate::blockedEventTypes(QWidget* const widget) const
{
    return widget->property("Digikam:blockedEventTypes").value<QList<QEvent::Type> >();
}

} // namespace Digikam

#include "moc_dwitemdelegate.cpp"
