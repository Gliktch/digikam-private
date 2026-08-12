/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : widget for displaying an item in view with used filters on current image
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemfiltershistorytreeitem.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemFiltersHistoryTreeItem::Private
{
public:

    Private() = default;

    QList<ItemFiltersHistoryTreeItem*> childItems;
    QList<QVariant>                    itemData;
    ItemFiltersHistoryTreeItem*        parentItem   = nullptr;
    bool                               disabled     = false;
};

ItemFiltersHistoryTreeItem::ItemFiltersHistoryTreeItem(const QList<QVariant>& data,
                                                       ItemFiltersHistoryTreeItem* const parent)
    : d(new Private)
{
    d->parentItem = parent;
    d->itemData   = data;
}

ItemFiltersHistoryTreeItem::ItemFiltersHistoryTreeItem(const QString& data,
                                                       ItemFiltersHistoryTreeItem* const parent)
    : d(new Private)
{
    d->parentItem = parent;
    d->itemData.append(data);
}

ItemFiltersHistoryTreeItem::ItemFiltersHistoryTreeItem(const ItemFiltersHistoryTreeItem& other)
    : d(new Private(*other.d))
{
}

ItemFiltersHistoryTreeItem::~ItemFiltersHistoryTreeItem()
{
    qDeleteAll(d->childItems);
    delete d;
}

void ItemFiltersHistoryTreeItem::appendChild(ItemFiltersHistoryTreeItem* const item)
{
    d->childItems.append(item);
}

ItemFiltersHistoryTreeItem* ItemFiltersHistoryTreeItem::child(int row) const
{
    return d->childItems.value(row);
}

int ItemFiltersHistoryTreeItem::childCount() const
{
    return d->childItems.count();
}

int ItemFiltersHistoryTreeItem::columnCount() const
{
    return d->itemData.count();
}

QVariant ItemFiltersHistoryTreeItem::data(int column) const
{
    return d->itemData.value(column);
}

ItemFiltersHistoryTreeItem* ItemFiltersHistoryTreeItem::parent() const
{
    return d->parentItem;
}

int ItemFiltersHistoryTreeItem::row() const
{
    if (d->parentItem)
    {
        return d->parentItem->d->childItems.indexOf(const_cast<ItemFiltersHistoryTreeItem*>(this));
    }

    return 0;
}

void ItemFiltersHistoryTreeItem::removeChild(int row)
{
    delete child(row);
    d->childItems.removeAt(row);
}

bool ItemFiltersHistoryTreeItem::isDisabled() const
{
    return d->disabled;
}

void ItemFiltersHistoryTreeItem::setDisabled(bool disabled) const
{
    d->disabled = disabled;
}

} // namespace Digikam
