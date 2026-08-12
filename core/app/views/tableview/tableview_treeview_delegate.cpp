/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Tree view subelement
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tableview_treeview_delegate.h"

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "coredbfields.h"
#include "coredbwatch.h"
#include "fileactionmngr.h"
#include "iteminfo.h"
#include "itemmodel.h"
#include "itemposition.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "importui.h"
#include "tableview_column_configuration_dialog.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"

namespace Digikam
{

TableViewItemDelegate::TableViewItemDelegate(TableViewShared* const tableViewShared, QObject* const parent)
    : QItemDelegate(parent),
      s            (tableViewShared)
{
}

void TableViewItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& tableViewModelIndex) const
{
    const int columnIndex                     = tableViewModelIndex.column();
    const int columnCount                     = s->tableViewModel->columnCount(QModelIndex());

    if ((columnIndex < 0) || (columnIndex >= columnCount))
    {
        QItemDelegate::paint(painter, option, tableViewModelIndex);
        return;
    }

    const TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);
    bool useDefaultPainter                    = !columnObject->getColumnFlags().testFlag(TableViewColumn::ColumnCustomPainting);

    if (!useDefaultPainter)
    {
        TableViewModel::Item* const item = s->tableViewModel->itemFromIndex(tableViewModelIndex);
        useDefaultPainter                = !columnObject->paint(painter, option, item);
    }

    if (useDefaultPainter)
    {
        QItemDelegate::paint(painter, option, tableViewModelIndex);
    }
}

QSize TableViewItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& tableViewModelIndex) const
{
    const int columnIndex            = tableViewModelIndex.column();

    /// we have to take the maximum of all columns for the height
    /// @todo somehow cache this calculation
    /// @todo check column flags

    const int columnCount            = s->tableViewModel->columnCount(QModelIndex());
    TableViewModel::Item* const item = s->tableViewModel->itemFromIndex(tableViewModelIndex);
    int maxHeight                    = 0;

    if ((columnIndex < 0) || (columnIndex >= columnCount))
    {
        return QItemDelegate::sizeHint(option, tableViewModelIndex);
    }

    for (int i = 0 ; i < columnCount ; ++i)
    {
        const TableViewColumn* const iColumnObject = s->tableViewModel->getColumnObject(i);

        if (iColumnObject && item)
        {
            const QSize iColumnSize = iColumnObject->sizeHint(option, item);

            if (iColumnSize.isValid())
            {
                maxHeight = qMax(maxHeight, iColumnSize.height());
            }
        }
    }

    QSize columnSize;
    const TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(columnIndex);

    if (columnObject && item)
    {
        columnSize = columnObject->sizeHint(option, item);
    }

    if (!columnSize.isValid())
    {
        columnSize = QItemDelegate::sizeHint(option, tableViewModelIndex);

        /// @todo we have to incorporate the height given by QItemDelegate for the other columns, too

        maxHeight  = qMax(maxHeight, columnSize.height());
    }

    return QSize(columnSize.width(), maxHeight);
}

} // namespace Digikam

#include "moc_tableview_treeview_delegate.cpp"
