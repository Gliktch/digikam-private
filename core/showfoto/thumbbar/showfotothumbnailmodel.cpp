/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-22
 * Description : Qt item model for Showfoto thumbnails entries
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotothumbnailmodel.h"

// Qt includes

#include <QFileInfo>
#include <QScopedPointer>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "dmetadata.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoThumbnailModel::Private
{
public:

    Private() = default;

    int preloadThumbnailSize() const
    {
        if (preloadThumbSize.size())
        {
            return preloadThumbSize.size();
        }

        return thumbSize.size();
    }

public:

    QWidget*             displayWidget          = nullptr;

    ThumbnailLoadThread* loadingThread          = nullptr;
    ThumbnailLoadThread* preloadThread          = nullptr;
    ThumbnailSize        thumbSize              = ThumbnailSize(0);
    ThumbnailSize        lastGlobalThumbSize    = ThumbnailSize(0);
    ThumbnailSize        preloadThumbSize       = ThumbnailSize(0);
    QRect                detailRect;
    int                  maxThumbSize           = ThumbnailSize::Huge;
    bool                 emitDataChanged        = true;
};

ShowfotoThumbnailModel::ShowfotoThumbnailModel(QWidget* const parent)
    : ShowfotoItemModel(parent),
      d                (new Private)
{
    d->displayWidget = parent;

    connect(this, &ShowfotoThumbnailModel::signalThumbInfo,
            this, &ShowfotoThumbnailModel::slotThumbInfoLoaded);
}

ShowfotoThumbnailModel::~ShowfotoThumbnailModel()
{
    this->showfotoItemInfosCleared();

    delete d->preloadThread;
    delete d;
}

void ShowfotoThumbnailModel::setThumbnailLoadThread(ThumbnailLoadThread* thread)
{
    d->loadingThread = thread;

    connect(d->loadingThread, &ThumbnailLoadThread::signalThumbnailLoaded,
            this, &ShowfotoThumbnailModel::slotThumbnailLoaded);
}

ThumbnailLoadThread* ShowfotoThumbnailModel::thumbnailLoadThread() const
{
    return d->loadingThread;
}

ThumbnailSize ShowfotoThumbnailModel::thumbnailSize() const
{
    return d->thumbSize;
}

void ShowfotoThumbnailModel::setThumbnailSize(const ThumbnailSize& size)
{
    d->lastGlobalThumbSize = size;
    d->thumbSize           = size;
}

void ShowfotoThumbnailModel::setPreloadThumbnailSize(const ThumbnailSize& size)
{
    d->preloadThumbSize = size;
}

void ShowfotoThumbnailModel::setEmitDataChanged(bool emitSignal)
{
    d->emitDataChanged = emitSignal;
}

void ShowfotoThumbnailModel::showfotoItemInfosCleared()
{
    if (d->preloadThread)
    {
        d->preloadThread->stopAllTasks();
    }
}

QVariant ShowfotoThumbnailModel::data(const QModelIndex& index, int role) const
{
    if ((role == ThumbnailRole) && (d->loadingThread && index.isValid()))
    {
        QImage    thumbnailImage;
        QPixmap   pixmap;
        ShowfotoItemInfo info = showfotoItemInfo(index);
        QString url           = info.url.toDisplayString();
        QString path          = info.folder + QLatin1Char('/') + info.name;

        if (info.isNull() || url.isEmpty())
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QVariant var = QPixmap();

            return var;

#else

            return QVariant(QVariant::Pixmap);

#endif

        }

        if (pixmapForItem(path, pixmap))
        {
            return pixmap;
        }

        double dpr    = d->displayWidget->devicePixelRatio();
        int thumbSize = qRound((double)d->thumbSize.size() * dpr);

        // if pixmapForItem Failed

        if (getThumbnail(info, thumbnailImage))
        {
            thumbnailImage = thumbnailImage.scaled(thumbSize, thumbSize,
                                                   Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation);

            Q_EMIT signalThumbInfo(info, thumbnailImage);

            return thumbnailImage;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            QVariant var = QPixmap();

            return var;

#else

            return QVariant(QVariant::Pixmap);

#endif

    }

    return ShowfotoItemModel::data(index, role);
}

bool ShowfotoThumbnailModel::setData(const QModelIndex& index, const QVariant& value, int role)
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
                    d->thumbSize = ThumbnailSize(d->lastGlobalThumbSize);
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

    return ShowfotoItemModel::setData(index, value, role);
}

void ShowfotoThumbnailModel::slotThumbnailLoaded(const LoadingDescription& loadingDescription,
                                                 const QPixmap& thumb)
{
    if (thumb.isNull())
    {
        return;
    }

    // In case of multiple occurrence, we currently do not know which thumbnail is this. Signal change on all.

    const auto idx = indexesForUrl(QUrl::fromLocalFile(loadingDescription.filePath));

    for (const QModelIndex& index : idx)
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
                Q_EMIT dataChanged(index, index);
            }

            Q_EMIT signalItemThumbnail(showfotoItemInfo(index), thumb);
        }
    }
}

bool ShowfotoThumbnailModel::getThumbnail(const ShowfotoItemInfo& itemInfo,
                                          QImage& thumbnail) const
{
    QString path = itemInfo.folder + QLatin1Char('/') + itemInfo.name;

    // Try to get preview from Exif data (good quality). Can work with Raw files

    QScopedPointer<DMetadata> metadata(new DMetadata(path));
    metadata->getItemPreview(thumbnail);

    if (!thumbnail.isNull())
    {
        return true;
    }

    // RAW files : try to extract embedded thumbnail using RawEngine

    DRawDecoder::loadRawPreview(thumbnail, path);

    if (!thumbnail.isNull())
    {
        return true;
    }

    KSharedConfig::Ptr config  = KSharedConfig::openConfig();
    KConfigGroup group         = config->group(QLatin1String("Camera Settings"));
    bool turnHighQualityThumbs = group.readEntry(QLatin1String("TurnHighQualityThumbs"), false);

    // Try to get thumbnail from Exif data (poor quality).

    if (!turnHighQualityThumbs)
    {
        thumbnail = metadata->getExifThumbnail(true);

        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // THM files: try to get thumbnail from '.thm' files if we didn't manage to get
    // thumbnail from Exif. Any cameras provides *.thm files like JPEG files with RAW files.
    // Using this way is always speed up than ultimate loading using DImg.
    // Note: the thumbnail extracted with this method can be in poor quality.
    // 2006/27/01 - Gilles - Tested with my Minolta Dynax 5D USM camera.

    QFileInfo fi(path);

    if      (thumbnail.load(itemInfo.folder + QLatin1Char('/') + fi.baseName() + QLatin1String(".thm")))        // Lowercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }
    else if (thumbnail.load(itemInfo.folder + QLatin1Char('/') + fi.baseName() + QLatin1String(".THM")))   // Uppercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // Finally, we trying to get thumbnail using DImg API (slow).
/*
    qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Use DImg loader to get thumbnail from : " << path;

    DImg dimgThumb(path);

    if (!dimgThumb.isNull())
    {
        thumbnail = dimgThumb.copyQImage();
        return true;
    }
*/
    return false;
}

bool ShowfotoThumbnailModel::pixmapForItem(const QString& url, QPixmap& pix) const
{
    double dpr    = d->displayWidget->devicePixelRatio();
    int thumbSize = qRound((double)d->thumbSize.size() * dpr);

    if (thumbSize > d->maxThumbSize)
    {
        // TODO: Install a widget maximum size to prevent this situation

        bool hasPixmap = d->loadingThread->find(ThumbnailIdentifier(url), pix, d->maxThumbSize);

        if (hasPixmap)
        {
/*
            qCWarning(DIGIKAM_GENERAL_LOG) << "Thumbbar: Requested thumbnail size" << d->thumbSize.size()
                                           << "is larger than the maximum thumbnail size" << d->maxThumbSize
                                           << ". Returning a scaled-up image.";
*/
            pix = pix.scaled(thumbSize, thumbSize,
                             Qt::KeepAspectRatio, Qt::SmoothTransformation);

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return d->loadingThread->find(ThumbnailIdentifier(url), pix, thumbSize);
    }
}

void ShowfotoThumbnailModel::slotThumbInfoLoaded(const ShowfotoItemInfo& info, const QImage& thumbnailImage)
{
    QImage thumbnail = thumbnailImage;

    if (thumbnail.isNull())
    {
        thumbnail = QImage();
    }

    const auto idx = indexesForUrl(info.url);

    for (const QModelIndex& index : idx)
    {
        if (thumbnail.isNull())
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

} // namespace ShowFoto

#include "moc_showfotothumbnailmodel.cpp"
