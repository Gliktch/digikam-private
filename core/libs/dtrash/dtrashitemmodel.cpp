/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-09
 * Description : DTrash item info model
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dtrashitemmodel.h"

// Qt includes

#include <QIcon>
#include <QTimer>
#include <QPixmap>
#include <QMimeType>
#include <QMimeDatabase>
#include <QPersistentModelIndex>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "thumbnailsize.h"
#include "iojobsmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN DTrashItemModel::Private
{
public:

    Private() = default;

public:

    int                  thumbSize          = ThumbnailSize::Large;
    int                  sortColumn         = DTrashTimeStamp;

    Qt::SortOrder        sortOrder          = Qt::DescendingOrder;
    bool                 sortEnabled        = true;
    bool                 suspendThumbs      = true;

    QWidget*             displayWidget      = nullptr;

    IOJobsThread*        itemsLoadingJob    = nullptr;
    ThumbnailLoadThread* thumbnailThread    = nullptr;

    QString              trashAlbumPath;

    QStringList          failedThumbnails;
    QStringList          collectionThumbs;
    DTrashItemInfoList   data;
};

DTrashItemModel::DTrashItemModel(QObject* const parent, QWidget* const widget)
    : QAbstractTableModel(parent),
      d                  (new Private)
{
    qRegisterMetaType<DTrashItemInfo>("DTrashItemInfo");

    d->displayWidget   = widget;
    d->thumbnailThread = new ThumbnailLoadThread;
    d->thumbnailThread->setSendSurrogatePixmap(false);

    connect(d->thumbnailThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(refreshThumbnails(LoadingDescription,QPixmap)));
}

DTrashItemModel::~DTrashItemModel()
{
    stopLoadingTrash();

    delete d->thumbnailThread;
    delete d;
}

int DTrashItemModel::rowCount(const QModelIndex&) const
{
    return d->data.count();
}

int DTrashItemModel::columnCount(const QModelIndex&) const
{
    return DTrashNumCol;
}

QVariant DTrashItemModel::data(const QModelIndex& index, int role) const
{
    if (
        (role != Qt::DisplayRole)       &&
        (role != Qt::DecorationRole)    &&
        (role != Qt::TextAlignmentRole) &&
        (role != Qt::ToolTipRole)
       )
    {
        return QVariant();
    }

    const DTrashItemInfo& item = d->data[index.row()];

    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    if ((role == Qt::DecorationRole) && (index.column() == DTrashThumb))
    {
        QPixmap pix;
        QString thumbPath;

        if      (!d->failedThumbnails.contains(item.collectionPath))
        {
            d->collectionThumbs << item.collectionPath;
            thumbPath = item.collectionPath;
        }
        else if (!d->failedThumbnails.contains(item.trashPath))
        {
            thumbPath = item.trashPath;
        }

        if (thumbPath.isEmpty() || pixmapForItem(thumbPath, pix))
        {
            if (pix.isNull())
            {
                QMimeType mimeType = QMimeDatabase().mimeTypeForFile(item.trashPath);

                if (mimeType.isValid())
                {
                    pix = QIcon::fromTheme(mimeType.genericIconName()).pixmap(128);
                }
            }

            return pix;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QVariant var = QPixmap();

        return var;

#else

        return QVariant(QVariant::Pixmap);

#endif

    }

    if ((role == Qt::ToolTipRole) && (index.column() == DTrashRelPath))
    {
        return (item.collectionRelativePath);
    }

    switch (index.column())
    {
        case DTrashRelPath:
        {
            return (item.collectionRelativePath);
        }

        case DTrashTimeStamp:
        {
            return (item.deletionTimestamp.toString(DATETIMEFORMATSTRING));
        }

        default:
        {
            return QVariant();
        }
    };
}

void DTrashItemModel::sort(int column, Qt::SortOrder order)
{
    d->sortOrder  = order;
    d->sortColumn = column;

    if (!d->sortEnabled || (d->data.count() < 2))
    {
        return;
    }

    std::sort(d->data.begin(), d->data.end(),
        [column, order](const DTrashItemInfo& a, const DTrashItemInfo& b)
        {
            if ((column == DTrashTimeStamp) && (a.deletionTimestamp != b.deletionTimestamp))
            {
                if (order == Qt::DescendingOrder)
                {
                    return (a.deletionTimestamp > b.deletionTimestamp);
                }
                else
                {
                    return (a.deletionTimestamp < b.deletionTimestamp);
                }
            }

            if (order == Qt::DescendingOrder)
            {
                return (a.collectionRelativePath > b.collectionRelativePath);
            }

            return (a.collectionRelativePath < b.collectionRelativePath);
        }
    );

    const QModelIndex topLeft     = index(0, 0);
    const QModelIndex bottomRight = index(rowCount(QModelIndex())    - 1,
                                          columnCount(QModelIndex()) - 1);

    Q_EMIT dataChanged(topLeft, bottomRight);
}

bool DTrashItemModel::pixmapForItem(const QString& path, QPixmap& pix) const
{
    bool ret = false;

    if (!d->suspendThumbs)
    {
        double ratio  = d->displayWidget->devicePixelRatio();
        int thumbSize = qMin(qRound((double)d->thumbSize * ratio), (int)ThumbnailSize::HD);
        ret           = d->thumbnailThread->find(ThumbnailIdentifier(path), pix, thumbSize);
        pix.setDevicePixelRatio(ratio);
    }

    return ret;
}

QVariant DTrashItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
    {
        return QVariant();
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    switch (section)
    {
        case DTrashThumb:
        {
            return i18n("Thumbnail");
        }

        case DTrashRelPath:
        {
            return i18n("Relative Path");
        }

        case DTrashTimeStamp:
        {
            return i18n("Deletion Time");
        }

        default:
        {
            return QVariant();
        }
    }
}

void DTrashItemModel::append(const DTrashItemInfo& itemInfo)
{
    if (d->itemsLoadingJob != sender())
    {
        return;
    }

    beginInsertRows(QModelIndex(), d->data.count(), d->data.count());
    d->data.append(itemInfo);
    endInsertRows();
}

void DTrashItemModel::removeItems(const QModelIndexList& indexes)
{
    QList<QPersistentModelIndex> persistentIndexes;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        persistentIndexes << index;
    }

    Q_EMIT layoutAboutToBeChanged();

    for (const QPersistentModelIndex& index : std::as_const(persistentIndexes))
    {
        if (!index.isValid())
        {
            continue;
        }

        const DTrashItemInfo& item = d->data[index.row()];

        d->failedThumbnails.removeAll(item.collectionPath);
        d->failedThumbnails.removeAll(item.trashPath);

        beginRemoveRows(QModelIndex(), index.row(), index.row());
        removeRow(index.row());
        d->data.removeAt(index.row());
        endRemoveRows();
    }

    d->suspendThumbs = false;

    Q_EMIT layoutChanged();
    Q_EMIT dataChange();
}

void DTrashItemModel::refreshLayout()
{
    const QModelIndex topLeft     = index(0, 0);
    const QModelIndex bottomRight = index(rowCount(QModelIndex()) - 1, 0);

    Q_EMIT dataChanged(topLeft, bottomRight);
    Q_EMIT layoutAboutToBeChanged();
    Q_EMIT layoutChanged();
}

void DTrashItemModel::refreshThumbnails(const LoadingDescription& desc, const QPixmap& pix)
{
    if (pix.isNull())
    {
        if (!d->failedThumbnails.contains(desc.filePath))
        {
            d->failedThumbnails << desc.filePath;
        }
    }

    if (d->collectionThumbs.contains(desc.filePath))
    {
        d->collectionThumbs.removeAll(desc.filePath);
        d->failedThumbnails << desc.filePath;
    }

    const QModelIndex topLeft     = index(0, 0);
    const QModelIndex bottomRight = index(rowCount(QModelIndex()) - 1, 0);

    Q_EMIT dataChanged(topLeft, bottomRight);
}

void DTrashItemModel::clearCurrentData()
{
    d->failedThumbnails.clear();
    d->suspendThumbs = false;
    beginResetModel();
    d->data.clear();
    endResetModel();

    Q_EMIT dataChange();
}

void DTrashItemModel::loadItemsForCollection(const QString& colPath)
{
    stopLoadingTrash();

    Q_EMIT signalLoadingStarted();

    d->sortEnabled    = false;
    d->suspendThumbs  = false;
    d->trashAlbumPath = colPath;

    clearCurrentData();

    d->itemsLoadingJob = IOJobsManager::instance()->startDTrashItemsListingForCollection(colPath);

    connect(d->itemsLoadingJob, SIGNAL(collectionTrashItemInfo(DTrashItemInfo)),
            this, SLOT(append(DTrashItemInfo)),
            Qt::QueuedConnection);

    connect(d->itemsLoadingJob, SIGNAL(signalFinished()),
            this, SLOT(slotLoadItemsFinished()),
            Qt::QueuedConnection);
}

DTrashItemInfo DTrashItemModel::itemForIndex(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return DTrashItemInfo();
    }

    return d->data.at(index.row());
}

DTrashItemInfoList DTrashItemModel::itemsForIndexes(const QList<QModelIndex>& indexes)
{
    DTrashItemInfoList items;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (!index.isValid())
        {
            continue;
        }

        items << itemForIndex(index);
    }

    return items;
}

QModelIndex DTrashItemModel::indexForItem(const DTrashItemInfo& itemInfo) const
{
    int index = d->data.indexOf(itemInfo);

    if (index != -1)
    {
        return createIndex(index, 0);
    }

    return QModelIndex();
}

DTrashItemInfoList DTrashItemModel::allItems()
{
    return d->data;
}

bool DTrashItemModel::isEmpty()
{
    return d->data.isEmpty();
}

void DTrashItemModel::changeThumbSize(int size)
{
    d->thumbSize = size;

    if (isEmpty())
    {
        return;
    }

    QTimer::singleShot(100, this, SLOT(refreshLayout()));
}

void DTrashItemModel::stopLoadingTrash()
{
    if (d->itemsLoadingJob)
    {
        disconnect(d->itemsLoadingJob, nullptr, this, nullptr);

        d->itemsLoadingJob->cancel();
        d->itemsLoadingJob = nullptr;
    }

    d->thumbnailThread->stopAllTasks();
    d->thumbnailThread->wait();
}

void DTrashItemModel::suspendLoadingTrashThumbs()
{
    d->suspendThumbs = true;

    d->thumbnailThread->stopAllTasks();
    d->thumbnailThread->wait();
}

QString DTrashItemModel::trashAlbumPath() const
{
    return d->trashAlbumPath;
}

void DTrashItemModel::slotLoadItemsFinished()
{
    d->sortEnabled     = true;
    d->itemsLoadingJob = nullptr;
    sort(d->sortColumn, d->sortOrder);

    Q_EMIT dataChange();
    Q_EMIT signalLoadingFinished();
}

} // namespace Digikam

#include "moc_dtrashitemmodel.cpp"
