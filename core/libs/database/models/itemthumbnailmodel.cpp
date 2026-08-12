/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries with support for thumbnail loading
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemthumbnailmodel.h"

// Qt includes

#include <QHash>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemThumbnailModel::Private
{
public:

    Private()
    {
        staticListContainingThumbnailRole << ItemModel::ThumbnailRole;
    }

public:

    QWidget*               displayWidget        = nullptr;

    ThumbnailLoadThread*   loadingThread        = nullptr;
    ThumbnailLoadThread*   storageThread        = nullptr;
    ThumbnailLoadThread*   preloadThread        = nullptr;
    ThumbnailSize          thumbSize            = ThumbnailSize(0);
    ThumbnailSize          lastGlobalThumbSize  = ThumbnailSize(0);
    ThumbnailSize          preloadThumbSize     = ThumbnailSize(0);
    QRect                  detailRect;
    QVector<int>           staticListContainingThumbnailRole;

    bool                   emitDataChanged      = true;

public:

    int preloadThumbnailSize() const
    {
        if (preloadThumbSize.size())
        {
            return preloadThumbSize.size();
        }

        return thumbSize.size();
    }
};

ItemThumbnailModel::ItemThumbnailModel(QWidget* const parent)
    : ItemModel(parent),
      d        (new Private)
{
    d->displayWidget = parent;
    d->storageThread = new ThumbnailLoadThread;
    d->storageThread->setSendSurrogatePixmap(false);

    connect(d->storageThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoadedFromStorage(LoadingDescription,QPixmap)));

    setKeepsFilePathCache(true);
}

ItemThumbnailModel::~ItemThumbnailModel()
{
    d->storageThread->stopAllTasks();
    d->storageThread->wait();

    if (d->preloadThread)
    {
        d->preloadThread->stopAllTasks();
        d->preloadThread->wait();
    }

    delete d->storageThread;
    delete d->preloadThread;
    delete d;
}

void ItemThumbnailModel::setThumbnailLoadThread(ThumbnailLoadThread* const thread)
{
    d->loadingThread = thread;

    connect(d->loadingThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

ThumbnailLoadThread* ItemThumbnailModel::thumbnailLoadThread() const
{
    return d->loadingThread;
}

ThumbnailSize ItemThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ItemThumbnailModel::setThumbnailSize(const ThumbnailSize& size)
{
    d->lastGlobalThumbSize = size;
    d->thumbSize           = size;
}

void ItemThumbnailModel::setPreloadThumbnailSize(const ThumbnailSize& size)
{
    d->preloadThumbSize = size;
}

void ItemThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

void ItemThumbnailModel::setPreloadThumbnails(bool preload)
{
    if (preload)
    {
        if (!d->preloadThread)
        {
            d->preloadThread = new ThumbnailLoadThread;
            d->preloadThread->setPixmapRequested(false);
            d->preloadThread->setPriority(QThread::LowestPriority);
        }

        connect(this, SIGNAL(allRefreshingFinished()),
                this, SLOT(preloadAllThumbnails()));
    }
    else
    {
        delete d->preloadThread;
        d->preloadThread = nullptr;

        disconnect(this, SIGNAL(allRefreshingFinished()),
                   this, SLOT(preloadAllThumbnails()));
    }
}

void ItemThumbnailModel::preloadThumbnails(const QList<ItemInfo>& infos)
{
    if (!d->preloadThread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;

    for (const ItemInfo& info : std::as_const(infos))
    {
        ids << info.thumbnailIdentifier();
    }

    d->preloadThread->stopAllTasks();
    d->preloadThread->pregenerateGroup(ids, d->preloadThumbnailSize());
}

void ItemThumbnailModel::preloadThumbnails(const QList<QModelIndex>& indexesToPreload)
{
    if (!d->preloadThread)
    {
        return;
    }

    QList<ThumbnailIdentifier> ids;

    for (const QModelIndex& index : std::as_const(indexesToPreload))
    {
        ids << imageInfoRef(index).thumbnailIdentifier();
    }

    d->preloadThread->stopAllTasks();
    d->preloadThread->pregenerateGroup(ids, d->preloadThumbnailSize());
}

void ItemThumbnailModel::preloadAllThumbnails()
{
    preloadThumbnails(imageInfos());
}

void ItemThumbnailModel::imageInfosCleared()
{
    if (d->preloadThread)
    {
        d->preloadThread->stopAllTasks();
    }
}

QVariant ItemThumbnailModel::data(const QModelIndex& index, int role) const
{
    if ((role == ThumbnailRole) && (d->loadingThread && index.isValid()))
    {
        QPixmap thumbnail;
        ItemInfo info = imageInfo(index);

        if (info.isNull())
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QVariant var = QPixmap();

            return var;

#else

            return QVariant(QVariant::Pixmap);

#endif

        }

        double dpr    = d->displayWidget->devicePixelRatio();
        int thumbSize = qRound((double)d->thumbSize.size() * dpr);

        if (!d->detailRect.isNull())
        {
            if      (d->storageThread->find(info.thumbnailIdentifier(),
                                            d->detailRect, thumbnail, thumbSize, true))
            {
                return thumbnail;
            }
            else if (d->storageThread->findBuffered(info.thumbnailIdentifier(),
                                                    d->detailRect, thumbnail, thumbSize))
            {
                return thumbnail;
            }
        }
        else
        {
            if      (d->storageThread->find(info.thumbnailIdentifier(),
                                            thumbnail, thumbSize, true))
            {
                return thumbnail;
            }
            else if (d->storageThread->findBuffered(info.thumbnailIdentifier(),
                                                    QRect(), thumbnail, thumbSize))
            {
                return thumbnail;
            }
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QVariant var = QPixmap();
        return var;

#else

        return QVariant(QVariant::Pixmap);

#endif

    }

    return ItemModel::data(index, role);
}

bool ItemThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == ThumbnailRole)
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        switch (value.typeId())

#else

        switch (value.type())

#endif

        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::UnknownType:

#else

            case QVariant::Invalid:

#endif

            {
                d->thumbSize  = d->lastGlobalThumbSize;
                d->detailRect = QRect();

                break;
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::Int:

#else

            case QVariant::Int:

#endif

            {
                if (value.isNull())
                {
                    d->thumbSize = d->lastGlobalThumbSize;
                }
                else
                {
                    d->thumbSize = ThumbnailSize(value.toInt());
                }

                break;
            }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            case QMetaType::QRect:

#else

            case QVariant::Rect:

#endif

            {
                if (value.isNull())
                {
                    d->detailRect = QRect();
                }
                else
                {
                    d->detailRect = value.toRect();
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }

    return ItemModel::setData(index, value, role);
}

void ItemThumbnailModel::slotThumbnailLoadedFromStorage(const LoadingDescription& loadingDescription, const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        LoadingDescription description       = loadingDescription;
        description.previewParameters.flags &= ~LoadingDescription::PreviewParameters::OnlyFromStorage;
        d->loadingThread->load(description);
    }
    else
    {
        slotThumbnailLoaded(loadingDescription, thumb);
    }
}

void ItemThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription,
                                             const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.

    QModelIndexList indexes;
    ThumbnailIdentifier thumbId = loadingDescription.thumbnailIdentifier();

    if (thumbId.filePath.isEmpty())
    {
        indexes = indexesForImageId(thumbId.id);
    }
    else
    {
        indexes = indexesForPath(thumbId.filePath);
    }

    for (const QModelIndex& index : std::as_const(indexes))
    {
        if (thumb.isNull())
        {
            Q_EMIT thumbnailFailed(index, loadingDescription.previewParameters.size);
        }
        else
        {
            Q_EMIT thumbnailAvailable(index, loadingDescription.previewParameters.size);

            if (d->emitDataChanged)
            {
                Q_EMIT dataChanged(index, index, d->staticListContainingThumbnailRole);
            }
        }
    }
}

} // namespace Digikam

#include "moc_itemthumbnailmodel.cpp"
