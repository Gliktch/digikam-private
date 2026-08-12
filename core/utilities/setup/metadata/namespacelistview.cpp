/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-22
 * Description : Reimplemented QListView for metadata setup, with support for
 *               drag-n-drop
 *
 * SPDX-FileCopyrightText: 2013-2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "namespacelistview.h"

// Qt includes

#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QStandardItem>
#include <QStandardItemModel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

NamespaceListView::NamespaceListView(QWidget* const parent)
    : QListView(parent)
{
    setAlternatingRowColors(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void NamespaceListView::startDrag(Qt::DropActions supportedActions)
{
    QListView::startDrag(supportedActions);
}

QModelIndexList NamespaceListView::mySelectedIndexes()
{
    return selectedIndexes();
}

void NamespaceListView::dropEvent(QDropEvent* e)
{
    QListView::dropEvent(e);

    Q_EMIT signalItemsChanged();
}

QModelIndex NamespaceListView::indexVisuallyAt(const QPoint& p)
{
    if (viewport()->rect().contains(p))
    {
        QModelIndex index = indexAt(p);

        if (index.isValid() && visualRect(index).contains(p))
        {
            return index;
        }
    }

    return QModelIndex();
}

void NamespaceListView::slotDeleteSelected()
{
    QModelIndexList sel = selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    QStandardItemModel* const model = dynamic_cast<QStandardItemModel*>(this->model());

    if (!model)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    for (const QModelIndex& index : std::as_const(sel))
    {
        QStandardItem* const root = model->invisibleRootItem();
        root->removeRow(index.row());
    }

    Q_EMIT signalItemsChanged();
}

void NamespaceListView::slotMoveItemDown()
{
    QModelIndexList sel = selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    QStandardItemModel* const model = dynamic_cast<QStandardItemModel*>(this->model());

    if (!model)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    QModelIndex index         = sel.first();
    QStandardItem* const root = model->invisibleRootItem();

    if (index.row() == (root->rowCount() - 1))
    {
        return;
    }

    root->insertRow(index.row() + 1, root->takeRow(index.row()));
    setCurrentIndex(model->index(index.row() + 1, index.column(), index.parent()));

    Q_EMIT signalItemsChanged();
}

void NamespaceListView::slotMoveItemUp()
{
    QModelIndexList sel = selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    QStandardItemModel* const model = dynamic_cast<QStandardItemModel*>(this->model());

    if (!model)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";
        return;
    }

    QModelIndex index         = sel.first();
    QStandardItem* const root = model->invisibleRootItem();

    if (index.row() == 0)
    {
        return;
    }

    root->insertRow(index.row() - 1, root->takeRow(index.row()));
    setCurrentIndex(model->index(index.row() - 1, index.column(), index.parent()));

    Q_EMIT signalItemsChanged();
}

} // namespace Digikam

#include "moc_namespacelistview.cpp"
