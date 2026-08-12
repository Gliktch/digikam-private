/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : List View Item for List View Model
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagmngrlistitem.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "album.h"

namespace Digikam
{

class Q_DECL_HIDDEN ListItem::Private
{
public:

    Private() = default;

public:

    QList<ListItem*> childItems;
    QList<ListItem*> toDelItems;
    QList<QVariant>  itemData;
    QList<int>       tagIds;
    ListItem*        parentItem = nullptr;
};

ListItem::ListItem(QList<QVariant>& data, ListItem* const parent)
    : d(new Private)
{
    d->parentItem = parent;
    d->itemData.append(data);

    data.removeFirst();

    for (const QVariant& val : std::as_const(data))
    {
        d->tagIds.append(val.toInt());
    }
}

ListItem::~ListItem()
{
    qDeleteAll(d->childItems);
    qDeleteAll(d->toDelItems);

    delete d;
}

void ListItem::deleteChild(ListItem* const item)
{
    int crow = d->childItems.indexOf(item);

    if (crow != -1)
    {
        deleteChild(crow);
    }
}

QList<ListItem*> ListItem::allChildren() const
{
    return d->childItems;
}

QList<int> ListItem::getTagIds() const
{
    return d->tagIds;
}

void ListItem::appendChild(ListItem* const item)
{
    d->childItems.append(item);
}

void ListItem::removeTagId(int tagId)
{
    d->tagIds.removeOne(tagId);
}

ListItem* ListItem::child(int row) const
{
    return d->childItems.value(row);
}

int ListItem::childCount() const
{
    return d->childItems.count();
}

void ListItem::deleteChild(int row)
{
    d->toDelItems << d->childItems.takeAt(row);
}

void ListItem::removeAll()
{
    d->toDelItems << d->childItems;
    d->childItems.clear();
}

void ListItem::appendList(const QList<ListItem*>& items)
{
    d->childItems.append(items);
}

int ListItem::columnCount() const
{
    return d->itemData.count();
}

QVariant ListItem::data(int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
            QString display;

            for (int tagId : std::as_const(d->tagIds))
            {
                const TAlbum* const album = AlbumManager::instance()->findTAlbum(tagId);

                if (!album)
                {
                    continue;
                }

                display.append(album->title() + QLatin1String(", "));

                if ((role == Qt::DisplayRole) && (display.size() > 30))
                {
                    break;
                }
            }

            if (display.isEmpty())
            {
                display.append(i18nc("@info", "All Tags"));
            }
            else
            {
                display.remove(display.size() - 2, 2);
            }

            return QVariant(display);
        }

        default:
        {
            return QVariant();
        }
    }
}

void ListItem::setData(const QList<QVariant>& data)
{
    d->itemData = data;
}

ListItem* ListItem::parent() const
{
    return d->parentItem;
}

int ListItem::row() const
{
    if (d->parentItem)
    {
        return d->parentItem->allChildren().indexOf(const_cast<ListItem*>(this));
    }

    return 0;
}

ListItem* ListItem::containsItem(ListItem* const item) const
{
    // NOTE: We need to compare items and not pointers

    for (int it = 0 ; it < d->childItems.size() ; ++it)
    {
        if (item->equal(d->childItems.at(it)))
        {
            return d->childItems.at(it);
        }
    }

    return nullptr;
}

bool ListItem::equal(ListItem* const item) const
{
    return (d->tagIds == item->getTagIds());
}

} // namespace Digikam

#include "moc_tagmngrlistitem.cpp"
