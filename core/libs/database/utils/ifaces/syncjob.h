/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-10-04
 * Description : synchronize Input/Output jobs.
 *
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

class QString;

namespace Digikam
{

class Album;
class TAlbum;

// -------------------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT SyncJob : public QObject
{
    Q_OBJECT

public:

    /**
     * Load the image or icon for the tag thumbnail.
     */
    static QPixmap getTagThumbnail(TAlbum* const album);
    static QPixmap getTagThumbnail(const QString& name, int size);

private:

    void enterWaitingLoop()                          const;
    void quitWaitingLoop()                           const;

    QPixmap getTagThumbnailPriv(TAlbum* const album) const;

private Q_SLOTS:

    void slotGotThumbnailFromIcon(Album* album, const QPixmap& pix);
    void slotLoadThumbnailFailed(Album* album);

private:

    /// @note disabled
    SyncJob();
    explicit SyncJob(QObject*) = delete;
    ~SyncJob() override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
