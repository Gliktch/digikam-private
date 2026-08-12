/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : A model to hold GPS information about items.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsitemmodel.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN GPSItemModel::Private
{
public:

    Private() = default;

public:

    QList<GPSItemContainer*>        items;
    int                             columnCount         = 0;
    QMap<QPair<int, int>, QVariant> headerData;
    ThumbnailLoadThread*            thumbnailLoadThread = nullptr;
};

GPSItemModel::GPSItemModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->thumbnailLoadThread = new ThumbnailLoadThread(this);

    connect(d->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

GPSItemModel::~GPSItemModel()
{
    // TODO: send a signal before deleting the items?

    qDeleteAll(d->items);
    delete d;
}

int GPSItemModel::columnCount(const QModelIndex& /*parent*/) const
{
    return d->columnCount;
}

QVariant GPSItemModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == this);
    }

    const int rowNumber = index.row();

    if ((rowNumber < 0) || (rowNumber >= d->items.count()))
    {
        return QVariant();
    }

    return d->items.at(rowNumber)->data(index.column(), role);
}

QModelIndex GPSItemModel::index(int row, int column, const QModelIndex& parent) const
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG)<<row<<column<<parent;
*/
    if (parent.isValid())
    {
        // there are no child items, only top level items

        return QModelIndex();
    }

    if (
        (column < 0)               ||
        (column >= d->columnCount) ||
        (row < 0)                  ||
        (row >= d->items.count())
       )
    {
        return QModelIndex();
    }

    return createIndex(row, column, (void*)nullptr);
}

QModelIndex GPSItemModel::parent(const QModelIndex& /*index*/) const
{
    // we have only top level items

    return QModelIndex();
}

void GPSItemModel::addItem(GPSItemContainer* const newItem)
{
    beginInsertRows(QModelIndex(), d->items.count(), d->items.count());
    newItem->setModel(this);
    d->items << newItem;
    endInsertRows();
}

void GPSItemModel::setColumnCount(const int nColumns)
{
    Q_EMIT layoutAboutToBeChanged();

    d->columnCount = nColumns;

    Q_EMIT layoutChanged();
}

void GPSItemModel::itemChanged(GPSItemContainer* const changedItem)
{
    const int itemIndex = d->items.indexOf(changedItem);

    if (itemIndex < 0)
    {
        return;
    }

    const QModelIndex itemModelIndexStart = createIndex(itemIndex, 0, (void*)nullptr);
    const QModelIndex itemModelIndexEnd   = createIndex(itemIndex, d->columnCount - 1, (void*)nullptr);

    Q_EMIT dataChanged(itemModelIndexStart, itemModelIndexEnd);
}

GPSItemContainer* GPSItemModel::itemFromIndex(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == this);
    }

    if (!index.isValid())
    {
        return nullptr;
    }

    const int row = index.row();

    if ((row < 0) || (row >= d->items.count()))
    {
        return nullptr;
    }

    return d->items.at(row);
}

int GPSItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->items.count();
}

bool GPSItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if ((section >= d->columnCount) || (orientation != Qt::Horizontal))
    {
        return false;
    }

    const QPair<int, int> headerIndex = QPair<int, int>(section, role);
    d->headerData[headerIndex]        = value;

    return true;
}

QVariant GPSItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((section >= d->columnCount) || (orientation != Qt::Horizontal))
    {
        return false;
    }

    const QPair<int, int> headerIndex = QPair<int, int>(section, role);

    return d->headerData.value(headerIndex);
}

bool GPSItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

Qt::ItemFlags GPSItemModel::flags(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Q_ASSERT(index.model() == this);
    }

    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return (QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled);
}

GPSItemContainer* GPSItemModel::itemFromUrl(const QUrl& url) const
{
    for (int i = 0 ; i < d->items.count() ; ++i)
    {
        if (d->items.at(i)->url() == url)
        {
            return d->items.at(i);
        }
    }

    return nullptr;
}

QModelIndex GPSItemModel::indexFromUrl(const QUrl& url) const
{
    for (int i = 0 ; i < d->items.count() ; ++i)
    {
        if (d->items.at(i)->url() == url)
        {
            return index(i, 0, QModelIndex());
        }
    }

    return QModelIndex();
}

QPixmap GPSItemModel::getPixmapForIndex(const QPersistentModelIndex& itemIndex, const int size)
{
    if (itemIndex.isValid())
    {
        Q_ASSERT(itemIndex.model() == this);
    }

    // TODO: should we cache the pixmap on our own here or does the interface usually cache it for us?
    // TODO: do we need to make sure we do not request the same pixmap twice in a row?
    // construct the key under which we stored the pixmap in the cache

    const GPSItemContainer* const imageItem = itemFromIndex(itemIndex);

    if (!imageItem)
    {
        return QPixmap();
    }

    QPixmap thumbnail;

    if (d->thumbnailLoadThread->find(ThumbnailIdentifier(imageItem->url().toLocalFile()), thumbnail, size))
    {
        return thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2);
    }

    return QPixmap();
}

void GPSItemModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    const QModelIndex currentIndex = indexFromUrl(QUrl::fromLocalFile(loadingDescription.filePath));

    if (currentIndex.isValid())
    {
        QPersistentModelIndex goodIndex(currentIndex);

        Q_EMIT signalThumbnailForIndexAvailable(goodIndex, thumb.copy(1, 1, thumb.size().width()-2, thumb.size().height()-2));
    }
}

Qt::DropActions GPSItemModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

} // namespace Digikam

#include "moc_gpsitemmodel.cpp"
