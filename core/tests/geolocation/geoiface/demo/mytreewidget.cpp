/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-06
 * Description : sub class of QTreeWidget for drag-and-drop support
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mytreewidget.h"

// Qt includes

#include <QApplication>
#include <QMouseEvent>
#include <QDrag>

// geoiface includes

#include "geoifacetypes.h"

// Local includes

#include "mydragdrophandler.h"

class Q_DECL_HIDDEN MyTreeWidget::Private
{
public:

    explicit Private()
        : dragStartPos()
    {
    }

    QPoint dragStartPos;
};

MyTreeWidget::MyTreeWidget(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    setDragEnabled(true);
    setUniformRowHeights(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

MyTreeWidget::~MyTreeWidget()
{
    delete d;
}

void MyTreeWidget::startDrag(Qt::DropActions /*supportedActions*/)
{
    QMimeData* const dragMimeData = mimeData(selectionModel()->selectedIndexes());
    QDrag* const drag             = new QDrag(this);
    drag->setMimeData(dragMimeData);
    drag->exec(Qt::CopyAction);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

QMimeData* MyTreeWidget::mimeData(const QList<QTreeWidgetItem*>& items) const

#else

// cppcheck-suppress passedByValue
QMimeData* MyTreeWidget::mimeData(const QList<QTreeWidgetItem*> items) const    // clazy:exclude=function-args-by-ref

#endif

{
    return QTreeWidget::mimeData(items);
}

// cppcheck-suppress passedByValue
QMimeData* MyTreeWidget::mimeData(const QModelIndexList itemsToDrag) const          // clazy:exclude=function-args-by-ref
{
    MyDragData* const mimeData = new MyDragData;

    // TODO: determine the indices of the items to drag!

    for (int i = 0 ; i < itemsToDrag.count() ; ++i)
    {
        mimeData->draggedIndices << QPersistentModelIndex(itemsToDrag.at(i));
    }

    return mimeData;
}

#include "moc_mytreewidget.cpp"
