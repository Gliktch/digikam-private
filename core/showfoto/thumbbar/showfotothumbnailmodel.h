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

#pragma once

// Local Include

#include "showfotoitemmodel.h"
#include "thumbnailsize.h"
#include "loadingdescription.h"
#include "thumbnailloadthread.h"

using namespace Digikam;

namespace ShowFoto
{

typedef QPair<ShowfotoItemInfo, QPixmap> CachedItem;

class ShowfotoThumbnailModel : public ShowfotoItemModel
{
    Q_OBJECT

public:

    /**
     * An ItemModel that supports thumbnail loading.
     * You need to set a ThumbnailLoadThread to enable thumbnail loading.
     * Adjust the thumbnail size to your needs.
     * Note that setKeepsFilePatindexesForPathhCache is enabled per default.
     */
    explicit ShowfotoThumbnailModel(QWidget* const parent);
    ~ShowfotoThumbnailModel()                                                                     override;

    /**
     * Enable thumbnail loading and set the thread that shall be used.
     * The thumbnail size of this thread will be adjusted.
     */
    void setThumbnailLoadThread(ThumbnailLoadThread* thread);
    ThumbnailLoadThread* thumbnailLoadThread()                                              const;

    /**
     * Set the thumbnail size to use.
     */
    void setThumbnailSize(const ThumbnailSize& thumbSize);

    /**
     * If you want to fix a size for preloading, do it here.
     */
    void setPreloadThumbnailSize(const ThumbnailSize& thumbSize);

    void setExifRotate(bool rotate);

    /**
     * Enable emitting dataChanged() when a thumbnail becomes available.
     * The thumbnailAvailable() signal will be emitted in any case.
     * Default is true.
     */
    void setEmitDataChanged(bool emitSignal);

    /**
     * Enable preloading of thumbnails:
     * If preloading is enabled, for every entry in the model a thumbnail generation is started.
     * Default: false.
     */
    void setPreloadThumbnails(bool preload);

    ThumbnailSize thumbnailSize()                                                           const;

    /**
     * Handles the ThumbnailRole.
     * If the pixmap is available, returns it in the QVariant.
     * If it still needs to be loaded, returns a null QVariant and emits
     * thumbnailAvailable() as soon as it is available.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                     const override;

    /**
     * You can override the current thumbnail size by giving an integer value for ThumbnailRole.
     * Set a null QVariant to use the thumbnail size set by setThumbnailSize() again.
     * The index given here is ignored for this purpose.
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole)     override;

    bool pixmapForItem(const QString& url, QPixmap& pix)                                    const;
    bool getThumbnail(const ShowfotoItemInfo& itemInfo, QImage& thumbnail)                  const;

public Q_SLOTS:

    void slotThumbInfoLoaded(const ShowfotoItemInfo& info, const QImage& thumbnailImage);

Q_SIGNALS:

    void thumbnailAvailable(const QModelIndex& index, int requestedSize);
    void thumbnailFailed(const QModelIndex& index, int requestedSize);

    void signalThumbInfo(const ShowfotoItemInfo& info, const QImage& thumbnailImage)        const;   // clazy:exclude=const-signal-or-slot

    void signalItemThumbnail(const ShowfotoItemInfo& info, const QPixmap& pix);

protected:

    void showfotoItemInfosCleared()                                                               override;

protected Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription,
                             const QPixmap& thumb);

private:

    /// @note disabled
    ShowfotoThumbnailModel(const ShowfotoThumbnailModel&)            = delete;
    ShowfotoThumbnailModel& operator=(const ShowfotoThumbnailModel&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
