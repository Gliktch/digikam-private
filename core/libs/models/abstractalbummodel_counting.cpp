/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-23
 * Description : Qt Model for Albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "abstractalbummodel_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN AbstractCountingAlbumModel::Private
{
public:

    Private() = default;

    bool            showCount       = false;
    QHash<int, int> countHash;
    QHash<int, int> countHashReady;
    QSet<int>       includeChildrenAlbums;
};

AbstractCountingAlbumModel::AbstractCountingAlbumModel(Album::Type albumType,
                                                       Album* const rootAlbum,
                                                       RootAlbumBehavior rootBehavior,
                                                       QObject* const parent)
    : AbstractSpecificAlbumModel(albumType, rootAlbum, rootBehavior, parent),
      d                         (new Private)
{
}

void AbstractCountingAlbumModel::setup()
{
    connect(AlbumManager::instance(), SIGNAL(signalAlbumMoved(Album*)),
            this, SLOT(slotAlbumMoved(Album*)));
}

AbstractCountingAlbumModel::~AbstractCountingAlbumModel()
{
    delete d;
}

void AbstractCountingAlbumModel::setShowCount(bool show)
{
    if (d->showCount != show)
    {
        d->showCount = show;
        emitDataChangedForChildren(rootAlbum());
    }
}

bool AbstractCountingAlbumModel::showCount() const
{
    return d->showCount;
}

void AbstractCountingAlbumModel::excludeChildrenCount(const QModelIndex& index)
{
    Album* const album = albumForIndex(index);

    if (!album)
    {
        return;
    }

    d->includeChildrenAlbums.remove(album->id());
    updateCount(album);
}

void AbstractCountingAlbumModel::includeChildrenCount(const QModelIndex& index)
{
    Album* const album = albumForIndex(index);

    if (!album)
    {
        return;
    }

    d->includeChildrenAlbums << album->id();
    updateCount(album);
}

void AbstractCountingAlbumModel::setCountHash(const QHash<int, int>& idCountHash)
{
    d->countHash                       = idCountHash;
    QHash<int, int>::const_iterator it = d->countHash.constBegin();

    for ( ; it != d->countHash.constEnd() ; ++it)
    {
        updateCount(albumForId(it.key()));
    }
}

void AbstractCountingAlbumModel::updateCount(Album* const album)
{
    if (!album)
    {
        return;
    }

    QHash<int, int>::iterator includeIt = d->countHashReady.find(album->id());
    bool changed                        = false;

    // get count for album without children

    int count                           = d->countHash.value(album->id());

    // if wanted, add up children's counts

    if (d->includeChildrenAlbums.contains(album->id()))
    {
        AlbumIterator it(album);

        while (it.current())
        {
            count += d->countHash.value((*it)->id());
            ++it;
        }
    }

    // insert or update

    if (includeIt == d->countHashReady.end())
    {
        changed                        = true;
        d->countHashReady[album->id()] = count;
    }
    else
    {
        changed           = (includeIt.value() != count);
        includeIt.value() = count;
    }

    // notify views

    if (changed)
    {
        // update only if the model contains the album

        QModelIndex index = indexForAlbum(album);

        if (index.isValid())
        {
            Q_EMIT dataChanged(index, index);
        }
    }
}

void AbstractCountingAlbumModel::setCount(Album* album, int count)
{
    if (!album)
    {
        return;
    }

    // if the model does not contain the album, do nothing.

    QModelIndex index = indexForAlbum(album);

    if (!index.isValid())
    {
        return;
    }

    QHash<int, int>::iterator includeIt = d->countHashReady.find(album->id());
    bool changed                        = false;

    // insert or update

    if (includeIt == d->countHashReady.end())
    {
        changed                        = true;
        d->countHashReady[album->id()] = count;
    }
    else
    {
        changed           = (includeIt.value() != count);
        includeIt.value() = count;
    }

    // notify views

    if (changed)
    {
        Q_EMIT dataChanged(index, index);
    }
}

QVariant AbstractCountingAlbumModel::albumData(Album* album, int role) const
{
    if ((role == Qt::DisplayRole) && d->showCount && !album->isRoot())
    {
        QHash<int, int>::const_iterator it = d->countHashReady.constFind(album->id());

        if (it != d->countHashReady.constEnd())
        {
            return QString::fromUtf8("%1 (%2)").arg(albumName(album)).arg(it.value());
        }
    }

    return AbstractSpecificAlbumModel::albumData(album, role);
}

int AbstractCountingAlbumModel::albumCount(Album* album) const
{
    QHash<int, int>::const_iterator it = d->countHashReady.constFind(album->id());

    if (it != d->countHashReady.constEnd())
    {
        return it.value();
    }

    return -1;
}

QString AbstractCountingAlbumModel::albumName(Album* album) const
{
    return album->title();
}

void AbstractCountingAlbumModel::albumCleared(Album* album)
{
    if (!AlbumManager::instance()->isMovingAlbum(album))
    {
        d->countHash.remove(album->id());
        d->countHashReady.remove(album->id());
        d->includeChildrenAlbums.remove(album->id());
    }
}

void AbstractCountingAlbumModel::allAlbumsCleared()
{
    d->countHash.clear();
    d->countHashReady.clear();
    d->includeChildrenAlbums.clear();
}

void AbstractCountingAlbumModel::slotAlbumMoved(Album*)
{
    // need to update counts of all parents

    setCountHash(d->countHash);
}

} // namespace Digikam
