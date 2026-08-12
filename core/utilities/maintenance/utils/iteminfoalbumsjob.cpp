/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-14-02
 * Description : interface to get item info from an albums list.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfoalbumsjob.h"

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "iteminfojob.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemInfoAlbumsJob::Private
{
public:

    Private() = default;

public:

    AlbumList           albumsList;
    AlbumList::Iterator albumIt;

    ItemInfoList        itemsList;

    ItemInfoJob         imageInfoJob;
};

ItemInfoAlbumsJob::ItemInfoAlbumsJob(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    connect(&d->imageInfoJob, SIGNAL(signalItemsInfo(ItemInfoList)),
            this, SLOT(slotItemsInfo(ItemInfoList)));

    connect(&d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotComplete()));
}

ItemInfoAlbumsJob::~ItemInfoAlbumsJob()
{
    delete d;
}

void ItemInfoAlbumsJob::allItemsFromAlbums(const AlbumList& albumsList)
{
    if (albumsList.isEmpty())
    {
        return;
    }

    d->albumsList = albumsList;
    d->albumIt    = d->albumsList.begin();
    parseAlbum();
}

void ItemInfoAlbumsJob::parseAlbum()
{
    d->imageInfoJob.allItemsFromAlbum(*d->albumIt);
}

void ItemInfoAlbumsJob::stop()
{
    d->imageInfoJob.stop();
    d->albumsList.clear();
}

void ItemInfoAlbumsJob::slotItemsInfo(const ItemInfoList& items)
{
    d->itemsList += items;
}

void ItemInfoAlbumsJob::slotComplete()
{
    ++d->albumIt;

    if (d->albumIt == d->albumsList.end())
    {
        stop();

        Q_EMIT signalCompleted(d->itemsList);

        return;
    }

    parseAlbum();
}

} // namespace Digikam

#include "moc_iteminfoalbumsjob.cpp"
