/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-06-13
 * Description : Qt item model for camera thumbnails entries
 *
 * SPDX-FileCopyrightText: 2009-2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "importimagemodel.h"
#include "thumbnailsize.h"
#include "camerathumbsctrl.h"
#include "digikam_export.h"

namespace Digikam
{

typedef QPair<CamItemInfo, QPixmap> CachedItem;

class DIGIKAM_GUI_EXPORT ImportThumbnailModel : public ImportItemModel
{
    Q_OBJECT

public:

    /**
     *  This model provides thumbnail loading, it uses the Camera Controller
     *  to retrieve thumbnails for CamItemInfos. It also provides preloading of thumbnails,
     *  and caching facility. Thumbnails size can be adjusted.
     */
    explicit ImportThumbnailModel(QObject* const parent);
    ~ImportThumbnailModel() override;

    /// Sets the camera thumbs controller which is used to get the thumbnails for item infos.
    void setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl)                                override;

    /// Get the thumbnail size
    ThumbnailSize thumbnailSize() const;

    /**
     *  Enable emitting dataChanged() when a thumbnail becomes available.
     *  The thumbnailAvailable() signal will be emitted in any case.
     *  Default is true.
     */
    void setEmitDataChanged(bool emitSignal);

    /**
     *  Handles the ThumbnailRole.
     *  If the pixmap is available, returns it in the QVariant.
     *  If it still needs to be loaded, returns a null QVariant and emits
     *  thumbnailAvailable() as soon as it is available.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const                 override;

    /**
     * You can override the current thumbnail size by giving an integer value for ThumbnailRole.
     * Set a null QVariant to use the thumbnail size set by setThumbnailSize() again.
     * The index given here is ignored for this purpose.
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;

Q_SIGNALS:

    void thumbnailAvailable(const QModelIndex& index, int requestedSize);
    void thumbnailFailed(const QModelIndex& index, int requestedSize);

private Q_SLOTS:

    void slotThumbInfoReady(const CamItemInfo&);

private:

    /// @note disabled
    ImportThumbnailModel(const ImportThumbnailModel&)            = delete;
    ImportThumbnailModel& operator=(const ImportThumbnailModel&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
