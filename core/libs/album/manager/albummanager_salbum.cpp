/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Search Album helpers.
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

AlbumList AlbumManager::allSAlbums() const
{
    AlbumList list;

    if (d->rootSAlbum)
    {
        list.append(d->rootSAlbum);
    }

    AlbumIterator it(d->rootSAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

void AlbumManager::scanSAlbums()
{
    d->scanSAlbumsTimer->stop();

    // list SAlbums directly from the db
    // first insert all the current SAlbums into a map for quick lookup

    QMap<int, SAlbum*> oldSearches;

    AlbumIterator it(d->rootSAlbum);

    while (it.current())
    {
        SAlbum* const search      = (SAlbum*)(*it);
        oldSearches[search->id()] = search;
        ++it;
    }

    // scan db and get a list of all albums

    QList<SearchInfo> currentSearches = CoreDbAccess().db()->scanSearches();
    QList<SearchInfo> newSearches;

    // go through all the Albums and see which ones are already present

    for (const SearchInfo& info : std::as_const(currentSearches))
    {
        if (oldSearches.contains(info.id))
        {
            SAlbum* const album = oldSearches[info.id];

            if (
                (info.name  != album->title())      ||
                (info.type  != album->searchType()) ||
                (info.query != album->query())
               )
            {
                QString oldName = album->title();

                album->setSearch(info.type, info.query);
                album->setTitle(info.name);

                if (oldName != album->title())
                {
                    Q_EMIT signalAlbumRenamed(album);
                }

                Q_EMIT signalSearchUpdated(album);
            }

            oldSearches.remove(info.id);
        }
        else
        {
            newSearches << info;
        }
    }

    // remove old albums that have been deleted

    for (SAlbum* const album : std::as_const(oldSearches))
    {
        Q_EMIT signalAlbumAboutToBeDeleted(album);

        d->allAlbumsIdHash.remove(album->globalID());

        Q_EMIT signalAlbumDeleted(album);

        album->prepareForDeletion();

        Q_EMIT signalAlbumHasBeenDeleted(album);

        delete album;
    }

    // add new albums

    for (const SearchInfo& info : std::as_const(newSearches))
    {
        SAlbum* const album                   = new SAlbum(info.name, info.id);
        album->setSearch(info.type, info.query);

        Q_EMIT signalAlbumAboutToBeAdded(album, d->rootSAlbum, d->rootSAlbum->lastChild());

        album->setParent(d->rootSAlbum);
        d->allAlbumsIdHash[album->globalID()] = album;

        Q_EMIT signalAlbumAdded(album);
    }
}

SAlbum* AlbumManager::findSAlbum(int id) const
{
    if (!d->rootSAlbum)
    {
        return nullptr;
    }

    int gid = d->rootSAlbum->globalID() + id;

    return (static_cast<SAlbum*>((d->allAlbumsIdHash.value(gid))));
}

SAlbum* AlbumManager::findSAlbum(const QString& name) const
{
    for (Album* album = d->rootSAlbum->firstChild() ;
         album ; album = album->next())
    {
        if (album->title() == name)
        {
            return (dynamic_cast<SAlbum*>(album));
        }
    }

    return nullptr;
}

QList<SAlbum*> AlbumManager::findSAlbumsBySearchType(int searchType) const
{
    QList<SAlbum*> albums;

    for (Album* album = d->rootSAlbum->firstChild() ;
         album ; album = album->next())
    {
        SAlbum* const sAlbum = dynamic_cast<SAlbum*>(album);

        if ((sAlbum) && (sAlbum->searchType() == searchType))
        {
            albums.append(sAlbum);
        }
    }

    return albums;
}

SAlbum* AlbumManager::createSAlbum(const QString& name,
                                   DatabaseSearch::Type type,
                                   const QString& query)
{
    // first iterate through all the search albums and see if there's an existing
    // SAlbum with same name. (Remember, SAlbums are arranged in a flat list)

    SAlbum* album = findSAlbum(name);
    ChangingDB changing(d);

    if (album)
    {
        updateSAlbum(album, query, name, type);
        return album;
    }

    int id = CoreDbAccess().db()->addSearch(type, name, query);

    if (id == -1)
    {
        return nullptr;
    }

    album = new SAlbum(name, id);

    Q_EMIT signalAlbumAboutToBeAdded(album, d->rootSAlbum, d->rootSAlbum->lastChild());

    album->setSearch(type, query);
    album->setParent(d->rootSAlbum);

    d->allAlbumsIdHash.insert(album->globalID(), album);

    Q_EMIT signalAlbumAdded(album);

    return album;
}

bool AlbumManager::updateSAlbum(SAlbum* album,
                                const QString& changedQuery,
                                const QString& changedName,
                                DatabaseSearch::Type type)
{
    if (!album)
    {
        return false;
    }

    QString newName              = changedName.isNull()                    ? album->title()      : changedName;
    DatabaseSearch::Type newType = (type == DatabaseSearch::UndefinedType) ? album->searchType() : type;

    ChangingDB changing(d);
    CoreDbAccess().db()->updateSearch(album->id(), newType, newName, changedQuery);

    QString oldName              = album->title();

    album->setSearch(newType, changedQuery);
    album->setTitle(newName);

    if (oldName != album->title())
    {
        Q_EMIT signalAlbumRenamed(album);
    }

    if (!d->currentAlbums.isEmpty())
    {
        if (d->currentAlbums.first() == album)
        {
            Q_EMIT signalAlbumCurrentChanged(d->currentAlbums);
        }
    }

    return true;
}

bool AlbumManager::deleteSAlbum(SAlbum* album)
{
    if (!album)
    {
        return false;
    }

    Q_EMIT signalAlbumAboutToBeDeleted(album);

    ChangingDB changing(d);
    CoreDbAccess().db()->deleteSearch(album->id());

    d->allAlbumsIdHash.remove(album->globalID());

    Q_EMIT signalAlbumDeleted(album);

    album->prepareForDeletion();

    Q_EMIT signalAlbumHasBeenDeleted(album);

    delete album;

    return true;
}

void AlbumManager::slotSearchChange(const SearchChangeset& changeset)
{
    if (d->changingDB || !d->rootSAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case SearchChangeset::Added:
        case SearchChangeset::Deleted:
        {
            if (!d->scanSAlbumsTimer->isActive())
            {
                d->scanSAlbumsTimer->start();
            }

            break;
        }

        case SearchChangeset::Changed:
        {
            if (!d->currentAlbums.isEmpty())
            {
                const Album* const currentAlbum = d->currentAlbums.first();

                if (
                    currentAlbum                            &&
                    (currentAlbum->type() == Album::SEARCH) &&
                    (currentAlbum->id()   == changeset.searchId())
                   )
                {
                    // the pointer is the same, but the contents changed

                    Q_EMIT signalAlbumCurrentChanged(d->currentAlbums);
                }
            }

            break;
        }

        case SearchChangeset::Unknown:
        {
            break;
        }
    }
}

} // namespace Digikam
