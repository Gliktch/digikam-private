/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-06-13
 * Description : Qt item model for camera thumbnails entries
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importthumbnailmodel.h"

// Qt includes

#include <QCache>
#include <QReadWriteLock>

// Local includes

#include "digikam_debug.h"
#include "cameracontroller.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportThumbnailModel::Private
{
public:

    Private() = default;

    CameraThumbsCtrl* thumbsCtrl            = nullptr;

    ThumbnailSize     thumbSize             = ThumbnailSize(0);
    ThumbnailSize     lastGlobalThumbSize   = ThumbnailSize(0);
    bool              emitDataChanged       = true;
};

ImportThumbnailModel::ImportThumbnailModel(QObject* const parent)
    : ImportItemModel(parent),
      d              (new Private)
{
    setKeepsFileUrlCache(true);
}

ImportThumbnailModel::~ImportThumbnailModel()
{
    delete d;
}

void ImportThumbnailModel::setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl)
{
    d->thumbsCtrl = thumbsCtrl;

    connect(d->thumbsCtrl, SIGNAL(signalThumbInfoReady(CamItemInfo)),
            this, SLOT(slotThumbInfoReady(CamItemInfo)));

    ImportItemModel::setCameraThumbsController(d->thumbsCtrl);
}

ThumbnailSize ImportThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ImportThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

QVariant ImportThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (
        (role == ThumbnailRole) &&
        d->thumbsCtrl           &&
        index.isValid()
       )
    {
        CamItemInfo info = camItemInfo(index);
        QString     path = info.url().toLocalFile();
        CachedItem  item;

        // use mimetype thumbnail also if the mime is set to something else than to image
        // this is to avoid querying the device for previews with unsupported file formats
        // at least gphoto2 doesn't really like it and will error a lot and slow down

        bool thumbnailPossible = (info.previewPossible                                       ||
                                  d->thumbsCtrl->cameraController()->cameraThumbnailSupport());

        if (info.isNull() || path.isEmpty() || !thumbnailPossible)
        {
            return QVariant(d->thumbsCtrl->cameraController()->mimeTypeThumbnail(path).pixmap(d->thumbSize.size()));
        }

        if (d->thumbsCtrl->getThumbInfo(info, item))
        {
            return QVariant(item.second.scaled(d->thumbSize.size(), d->thumbSize.size(), Qt::KeepAspectRatio));
        }

        return QVariant(d->thumbsCtrl->cameraController()->mimeTypeThumbnail(path).pixmap(d->thumbSize.size()));
    }

    return ImportItemModel::data(index, role);
}

bool ImportThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
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
                    d->lastGlobalThumbSize = d->thumbSize;
                    d->thumbSize           = ThumbnailSize(value.toInt());
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }

    return ImportItemModel::setData(index, value, role);
}

void ImportThumbnailModel::slotThumbInfoReady(const CamItemInfo& info)
{
    CachedItem  item;
    d->thumbsCtrl->getThumbInfo(info, item);

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.

    const auto idx = indexesForUrl(info.url());

    for (const QModelIndex& index : idx)
    {
        if (item.second.isNull())
        {
            Q_EMIT thumbnailFailed(index, d->thumbSize.size());
        }
        else
        {
            Q_EMIT thumbnailAvailable(index, d->thumbSize.size());

            if (d->emitDataChanged)
            {
                Q_EMIT dataChanged(index, index);
            }
        }
    }
}

} // namespace Digikam

#include "moc_importthumbnailmodel.cpp"
