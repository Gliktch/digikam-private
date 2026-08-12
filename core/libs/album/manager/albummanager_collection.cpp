/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Collection helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummanager_p.h"

namespace Digikam
{

bool AlbumManager::handleCollectionStatusChange(const CollectionLocation& location, int oldStatus)
{
    enum Action
    {
        Add,
        Remove,
        DoNothing
    };
    Action action = DoNothing;

    switch (oldStatus)
    {
        case CollectionLocation::LocationNull:
        case CollectionLocation::LocationHidden:
        case CollectionLocation::LocationUnavailable:
        {
            switch (location.status())
            {
                case CollectionLocation::LocationNull: // not possible
                {
                    break;
                }

                case CollectionLocation::LocationHidden:
                {
                    action = Remove;
                    break;
                }

                case CollectionLocation::LocationAvailable:
                {
                    action = Add;
                    break;
                }

                case CollectionLocation::LocationUnavailable:
                {
                    if (d->showOnlyAvailableAlbums)
                    {
                        action = Remove;
                    }
                    else
                    {
                        action = Add;
                    }

                    break;
                }

                case CollectionLocation::LocationDeleted:
                {
                    action = Remove;
                    break;
                }
            }

            break;
        }

        case CollectionLocation::LocationAvailable:
        {
            switch (location.status())
            {
                case CollectionLocation::LocationNull:
                case CollectionLocation::LocationHidden:
                case CollectionLocation::LocationDeleted:
                {
                    action = Remove;
                    break;
                }

                case CollectionLocation::LocationUnavailable:
                {
                    if (d->showOnlyAvailableAlbums)
                    {
                        action = Remove;
                    }

                    break;
                }

                case CollectionLocation::LocationAvailable: // not possible
                {
                    break;
                }
            }

            break;
        }

        case CollectionLocation::LocationDeleted: // not possible
        {
            break;
        }
    }

    if      ((action == Add) && !d->albumRootAlbumHash.value(location.id()))
    {
        // This is the only place where album root albums are added

        addAlbumRoot(location);
        return true;
    }
    else if ((action == Remove) && d->albumRootAlbumHash.value(location.id()))
    {
        removeAlbumRoot(location);
        return true;
    }

    return false;
}

void AlbumManager::addAlbumRoot(const CollectionLocation& location)
{
    PAlbum* album = d->albumRootAlbumHash.value(location.id());

    if (!album)
    {
        // Create a PAlbum for the Album Root.

        QString label = d->labelForAlbumRootAlbum(location);
        album         = new PAlbum(location.id(), label);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Added root album called: " << album->title();

        // insert album root created into hash

        d->albumRootAlbumHash.insert(location.id(), album);
    }
}

void AlbumManager::removeAlbumRoot(const CollectionLocation& location)
{
    // retrieve and remove from hash

    PAlbum* const album = d->albumRootAlbumHash.take(location.id());

    if (album)
    {
        // delete album and all its children

        removePAlbum(album);
    }
}

void AlbumManager::slotCollectionLocationStatusChanged(const CollectionLocation& location, int oldStatus)
{
    // not before initialization

    if (!d->rootPAlbum)
    {
        return;
    }

    if (handleCollectionStatusChange(location, oldStatus))
    {
        // a change occurred. Possibly albums have appeared or disappeared

        if (!d->scanPAlbumsTimer->isActive())
        {
            d->scanPAlbumsTimer->start();
        }
    }
}

void AlbumManager::slotCollectionLocationPropertiesChanged(const CollectionLocation& location)
{
    PAlbum* const album = d->albumRootAlbumHash.value(location.id());

    if (album)
    {
        QString newLabel = d->labelForAlbumRootAlbum(location);

        if (album->title() != newLabel)
        {
            album->setTitle(newLabel);

            Q_EMIT signalAlbumRenamed(album);
        }
    }
}

void AlbumManager::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (!d->rootDAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
        case CollectionImageChangeset::Deleted:
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
        {
            if (!d->albumItemCountTimer->isActive())
            {
                d->albumItemCountTimer->start();
            }

            if (!d->scanDAlbumsTimer->isActive())
            {
                d->scanDAlbumsTimer->start();
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace Digikam
