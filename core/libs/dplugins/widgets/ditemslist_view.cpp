/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-21
 * Description : widget to display a list of items
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ditemslist_p.h"

namespace Digikam
{

DItemsListView::DItemsListView(DItemsList* const parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setAlternatingRowColors(true);
    setExpandsOnDoubleClick(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    enableDragAndDrop(true);

    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setColumnCount(8);
    setHeaderLabels(QStringList() << i18nc("@title", "Thumbnail")
                                  << i18nc("@title", "File Name")
                                  << i18nc("@title", "User1")
                                  << i18nc("@title", "User2")
                                  << i18nc("@title", "User3")
                                  << i18nc("@title", "User4")
                                  << i18nc("@title", "User5")
                                  << i18nc("@title", "User6"));
    hideColumn(User1);
    hideColumn(User2);
    hideColumn(User3);
    hideColumn(User4);
    hideColumn(User5);
    hideColumn(User6);

    header()->setSectionResizeMode(User1, QHeaderView::Interactive);
    header()->setSectionResizeMode(User2, QHeaderView::Interactive);
    header()->setSectionResizeMode(User3, QHeaderView::Interactive);
    header()->setSectionResizeMode(User4, QHeaderView::Interactive);
    header()->setSectionResizeMode(User5, QHeaderView::Interactive);
    header()->setSectionResizeMode(User6, QHeaderView::Stretch);

    connect(this, &DItemsListView::itemClicked,
            this, &DItemsListView::slotItemClicked);
}

DInfoInterface* DItemsListView::iface() const
{
    const DItemsList* const p = dynamic_cast<DItemsList*>(parent());

    if (p)
    {
        return p->iface();
    }

    return nullptr;
}

void DItemsListView::enableDragAndDrop(const bool enable)
{
    setDragEnabled(enable);
    viewport()->setAcceptDrops(enable);
    setDragDropMode(enable ? QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
    setDragDropOverwriteMode(enable);
    setDropIndicatorShown(enable);
}

void DItemsListView::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    const DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        DItemsList* const view = dynamic_cast<DItemsList*>(parent());

        if (view)
        {
            view->updateThumbnail(item->url());
        }
    }

    QTreeWidget::drawRow(p, opt, index);
}

void DItemsListView::slotItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)

    if (!item)
    {
        return;
    }

    Q_EMIT signalItemClicked(item);
}

void DItemsListView::setColumnLabel(ColumnType column, const QString& label)
{
    headerItem()->setText(column, label);
}

void DItemsListView::setColumnEnabled(ColumnType column, bool enable)
{
    if (enable)
    {
        showColumn(column);
    }
    else
    {
        hideColumn(column);
    }
}

void DItemsListView::setColumn(ColumnType column, const QString& label, bool enable)
{
    setColumnLabel(column, label);
    setColumnEnabled(column, enable);
}

DItemsListViewItem* DItemsListView::findItem(const QUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        DItemsListViewItem* const lvItem = dynamic_cast<DItemsListViewItem*>(*it);

        if (lvItem && (lvItem->url() == url))
        {
            return lvItem;
        }

        ++it;
    }

    return nullptr;
}

QModelIndex DItemsListView::indexFromItem(DItemsListViewItem* item, int column) const
{
    return QTreeWidget::indexFromItem(item, column);
}

void DItemsListView::contextMenuEvent(QContextMenuEvent* e)
{
    QTreeWidget::contextMenuEvent(e);

    Q_EMIT signalContextMenuRequested();
}

void DItemsListView::dragEnterEvent(QDragEnterEvent* e)
{
    m_itemDraged = QTreeWidget::currentItem();

    QTreeWidget::dragEnterEvent(e);

    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void DItemsListView::dragMoveEvent(QDragMoveEvent* e)
{
    QTreeWidget::dragMoveEvent(e);

    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void DItemsListView::dropEvent(QDropEvent* e)
{
    QTreeWidget::dropEvent(e);
    QList<QUrl> list = e->mimeData()->urls();
    QList<QUrl> urls;

    for (const QUrl& url : std::as_const(list))
    {
        QFileInfo fi(url.toLocalFile());

        if (fi.isFile() && fi.exists())
        {
            urls.append(url);
        }
    }

    if (!urls.isEmpty())
    {
        Q_EMIT signalAddedDropedItems(urls);
    }

    scrollToItem(m_itemDraged);
}

DItemsListIsLessThanHandler DItemsListView::isLessThanHandler() const
{
    DItemsList* const p = dynamic_cast<DItemsList*>(parent());

    if (p)
    {
        return p->isLessThanHandler();
    }

    return nullptr;
}

} // namespace Digikam
