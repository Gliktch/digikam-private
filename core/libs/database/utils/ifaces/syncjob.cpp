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

#include "syncjob.h"

// Qt includes

#include <QEventLoop>

// Local includes

#include "applicationsettings.h"
#include "thumbnailsize.h"
#include "albumthumbnailloader.h"
#include "album.h"

namespace Digikam
{

class Q_DECL_HIDDEN SyncJob::Private
{
public:

    Private() = default;

    QEventLoop* waitingLoop = nullptr;
    QPixmap*    thumbnail   = nullptr;

    Album*      album       = nullptr;
};

SyncJob::SyncJob()
    : d(new Private)
{
    d->waitingLoop = new QEventLoop(this);
}

SyncJob::~SyncJob()
{
    delete d->thumbnail;

    d->thumbnail = nullptr;

    delete d;
}

void SyncJob::enterWaitingLoop() const
{
    d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);
}

void SyncJob::quitWaitingLoop() const
{
    d->waitingLoop->quit();
}

// ---------------------------------------------------------------

QPixmap SyncJob::getTagThumbnail(TAlbum* const album)
{
    SyncJob sj;

    return sj.getTagThumbnailPriv(album);
}

QPixmap SyncJob::getTagThumbnailPriv(TAlbum* const album) const
{
    delete d->thumbnail;

    d->thumbnail                       = new QPixmap();
    AlbumThumbnailLoader* const loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album*,QPixmap)),
            this, SLOT(slotGotThumbnailFromIcon(Album*,QPixmap)),
            Qt::QueuedConnection);

    connect(loader, SIGNAL(signalFailed(Album*)),
            this, SLOT(slotLoadThumbnailFailed(Album*)),
            Qt::QueuedConnection);

    if (!loader->getTagThumbnail(album, *d->thumbnail))
    {
        if (d->thumbnail->isNull())
        {
            if (album->hasProperty(TagPropertyName::person()))
            {
                return loader->getStandardFaceIcon(album);
            }
            else
            {
                return loader->getStandardTagIcon(album);
            }
        }
    }
    else
    {
        d->album = album;
        enterWaitingLoop();
    }

    return *d->thumbnail;
}

void SyncJob::slotLoadThumbnailFailed(Album* album)
{
    if (album == d->album)
    {
        quitWaitingLoop();
    }
}

void SyncJob::slotGotThumbnailFromIcon(Album* album, const QPixmap& pix)
{
    if (album == d->album)
    {
        *d->thumbnail = pix;
        quitWaitingLoop();
    }
}

} // namespace Digikam

#include "moc_syncjob.cpp"
