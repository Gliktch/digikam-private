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
#include <QStyle>
#include <QEvent>
#include <QTimer>
#include <QTreeView>
#include <QApplication>
#include <QStyleOption>
#include <QAbstractItemView>

// Local includes

#include "dwitemdelegatepool.h"

namespace Digikam
{

DWItemDelegatePrivate::DWItemDelegatePrivate(DWItemDelegate* const qq, QObject* const parent)
    : QObject       (parent),
      widgetPool    (new DWItemDelegatePool(qq)),
      q             (qq)
{
}

DWItemDelegatePrivate::~DWItemDelegatePrivate()
{
    delete widgetPool;
}

void DWItemDelegatePrivate::slotDWRowsInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(end);

    // We need to update the rows behind the inserted row as well because the widgets need to be
    // moved to their new position

    updateRowRange(parent, start, model->rowCount(parent), false);
}

void DWItemDelegatePrivate::slotDWRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    updateRowRange(parent, start, end, true);
}

void DWItemDelegatePrivate::slotDWRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(end);

    // We need to update the rows that come behind the deleted rows because the widgets need to be
    // moved to the new position

    updateRowRange(parent, start, model->rowCount(parent), false);
}

void DWItemDelegatePrivate::slotDWDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    for (int i = topLeft.row() ; i <= bottomRight.row() ; ++i)
    {
        for (int j = topLeft.column() ; j <= bottomRight.column() ; ++j)
        {
            const QModelIndex index = model->index(i, j, topLeft.parent());
            widgetPool->findWidgets(index, optionView(index));
        }
    }
}

void DWItemDelegatePrivate::slotDWLayoutChanged()
{
    const auto wdgs = widgetPool->invalidIndexesWidgets();

    for (QWidget* const widget : wdgs)
    {
        widget->setVisible(false);
    }

    QTimer::singleShot(0, this, SLOT(initializeModel()));
}

void DWItemDelegatePrivate::slotDWModelReset()
{
    widgetPool->fullClear();
    QTimer::singleShot(0, this, SLOT(initializeModel()));
}

void DWItemDelegatePrivate::slotDWSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    {
        const auto idx = selected.indexes();

        for (const QModelIndex& index : idx)
        {
            widgetPool->findWidgets(index, optionView(index));
        }
    }

    {
        const auto idx = deselected.indexes();

        for (const QModelIndex& index : idx)
        {
            widgetPool->findWidgets(index, optionView(index));
        }
    }
}

void DWItemDelegatePrivate::updateRowRange(const QModelIndex& parent, int start, int end, bool isRemoving)
{
    int i = start;

    while (i <= end)
    {
        for (int j = 0 ; j < model->columnCount(parent) ; ++j)
        {
            const QModelIndex index    = model->index(i, j, parent);
            QList<QWidget*> widgetList = widgetPool->findWidgets(index, optionView(index), isRemoving ? DWItemDelegatePool::NotUpdateWidgets
                                                                                                      : DWItemDelegatePool::UpdateWidgets);
            if (isRemoving)
            {
                for (QWidget* const widget : std::as_const(widgetList))
                {
                    const QModelIndex idx = widgetPool->d->widgetInIndex[widget];
                    widgetPool->d->usedWidgets.remove(idx);
                    widgetPool->d->widgetInIndex.remove(widget);
                    delete widget;
                }
            }
        }

        i++;
    }
}

inline QStyleOptionViewItem DWItemDelegatePrivate::optionView(const QModelIndex& index)
{
    QStyleOptionViewItem optionView;
    optionView.initFrom(itemView->viewport());
    optionView.rect           = itemView->visualRect(index);
    optionView.decorationSize = itemView->iconSize();

    return optionView;
}

void DWItemDelegatePrivate::initializeModel(const QModelIndex& parent)
{
    if (!model)
    {
        return;
    }

    for (int i = 0 ; i < model->rowCount(parent) ; ++i)
    {
        for (int j = 0 ; j < model->columnCount(parent) ; ++j)
        {
            const QModelIndex index = model->index(i, j, parent);

            if (index.isValid())
            {
                widgetPool->findWidgets(index, optionView(index));
            }
        }

        // Check if we need to go recursively through the children of parent (if any) to initialize
        // all possible indexes that are shown.

        const QModelIndex index = model->index(i, 0, parent);

        if (index.isValid() && model->hasChildren(index))
        {
            initializeModel(index);
        }
    }
}

bool DWItemDelegatePrivate::eventFilter(QObject* watched, QEvent* event)
{
    Q_ASSERT(itemView);

    if (model != itemView->model())
    {
        if (model)
        {
            disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                       q, SLOT(slotDWRowsInserted(QModelIndex,int,int)));

            disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                       q, SLOT(slotDWRowsAboutToBeRemoved(QModelIndex,int,int)));

            disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                       q, SLOT(slotDWRowsRemoved(QModelIndex,int,int)));

            disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                       q, SLOT(slotDWDataChanged(QModelIndex,QModelIndex)));

            disconnect(model, SIGNAL(layoutChanged()),
                       q, SLOT(slotDWLayoutChanged()));

            disconnect(model, SIGNAL(modelReset()),
                       q, SLOT(slotDWModelReset()));
        }

        model = itemView->model();

        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                q, SLOT(slotDWRowsInserted(QModelIndex,int,int)));

        connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                q, SLOT(slotDWRowsAboutToBeRemoved(QModelIndex,int,int)));

        connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                q, SLOT(slotDWRowsRemoved(QModelIndex,int,int)));

        connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                q, SLOT(slotDWDataChanged(QModelIndex,QModelIndex)));

        connect(model, SIGNAL(layoutChanged()),
                q, SLOT(slotDWLayoutChanged()));

        connect(model, SIGNAL(modelReset()),
                q, SLOT(slotDWModelReset()));

        QTimer::singleShot(0, this, SLOT(initializeModel()));
    }

    if (selectionModel != itemView->selectionModel())
    {
        if (selectionModel)
        {
            disconnect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                       q, SLOT(slotDWSelectionChanged(QItemSelection,QItemSelection)));
        }

        selectionModel = itemView->selectionModel();

        connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                q, SLOT(slotDWSelectionChanged(QItemSelection,QItemSelection)));

        QTimer::singleShot(0, this, SLOT(initializeModel()));
    }

    switch (event->type())
    {
        case QEvent::Polish:
        case QEvent::Resize:
        {
            if (!qobject_cast<QAbstractItemView*>(watched))
            {
                QTimer::singleShot(0, this, SLOT(initializeModel()));
            }

            break;
        }

        case QEvent::FocusIn:
        case QEvent::FocusOut:
        {
            if (qobject_cast<QAbstractItemView*>(watched))
            {
                const auto idx = selectionModel->selectedIndexes();

                for (const QModelIndex& index : idx)
                {
                    if (index.isValid())
                    {
                        widgetPool->findWidgets(index, optionView(index));
                    }
                }
            }

            break;
        }

        default:
        {
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

} // namespace Digikam

#include "moc_dwitemdelegate_p.cpp"
