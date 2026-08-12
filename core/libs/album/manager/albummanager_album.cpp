/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Generic Album helpers.
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

void AlbumManager::setCurrentAlbums(const QList<Album*>& albums)
{
    if (albums.isEmpty())
    {
        return;
    }

    d->currentAlbums.clear();

    /**
     * Filter out the null pointers
     */
    for (Album* const album : std::as_const(albums))        // cppcheck-suppress constVariablePointer
    {
        if (album)
        {
            d->currentAlbums << album;
        }
    }

    /**
     * Sort is needed to identify selection correctly, ex AlbumHistory
     */
    std::sort(d->currentAlbums.begin(), d->currentAlbums.end());

    Q_EMIT signalAlbumCurrentChanged(d->currentAlbums);
}

AlbumList AlbumManager::currentAlbums() const
{
    return d->currentAlbums;
}

void AlbumManager::clearCurrentAlbums()
{
    d->currentAlbums.clear();

    Q_EMIT signalAlbumCurrentChanged(d->currentAlbums);
}

Album* AlbumManager::findAlbum(int gid) const
{
    return d->allAlbumsIdHash.value(gid);
}

Album* AlbumManager::findAlbum(Album::Type type, int id) const
{
    return findAlbum(Album::globalID(type, id));
}

QHash<int, QString> AlbumManager::albumTitles() const
{
    QHash<int, QString> hash;
    AlbumIterator it(d->rootPAlbum);

    while (it.current())
    {
        const PAlbum* const a = (PAlbum*)(*it);
        hash.insert(a->id(), a->title());
        ++it;
    }

    return hash;
}

bool AlbumManager::isMovingAlbum(Album* album) const
{
    return d->currentlyMovingAlbum == album;
}

bool AlbumManager::hasDirectChildAlbumWithTitle(Album* parent, const QString& title)
{
    const Album* sibling = parent->firstChild();

    while (sibling)
    {
        if (sibling->title() == title)
        {
            return true;
        }

        sibling = sibling->next();
    }

    return false;
}

void AlbumManager::notifyAlbumDeletion(Album* album)
{
    invalidateGuardedPointers(album);
}

void AlbumManager::slotAlbumsJobResult()
{
    if (!d->albumListJob)
    {
        return;
    }

    if (d->albumListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list albums";

        // Pop-up a message about the error.

        DNotificationWrapper(QString(), d->albumListJob->errorsList().first(),
                             nullptr, i18n("digiKam"));
    }

    d->albumListJob = nullptr;
}

void AlbumManager::slotAlbumsJobData(const QHash<int, int>& albumsStatHash)
{
    if (albumsStatHash.isEmpty())
    {
        return;
    }

    d->pAlbumsCount = albumsStatHash;

    Q_EMIT signalPAlbumsDirty(albumsStatHash);
}

void AlbumManager::updateAlbumPathHash()
{
    // Update AlbumDict. basically clear it and rebuild from scratch

    d->albumPathHash.clear();
    AlbumIterator it(d->rootPAlbum);
    PAlbum* subAlbum = nullptr;

    while ((subAlbum = static_cast<PAlbum*>(it.current())) != nullptr)
    {
        d->albumPathHash[PAlbumPath(subAlbum)] = subAlbum;
        ++it;
    }
}

void AlbumManager::addGuardedPointer(Album* album, Album** pointer)
{
    if (album)
    {
        d->guardedPointers.insert(album, pointer);
    }
}

void AlbumManager::removeGuardedPointer(Album* album, Album** pointer)
{
    if (album)
    {
        d->guardedPointers.remove(album, pointer);
    }
}

void AlbumManager::changeGuardedPointer(Album* oldAlbum, Album* album, Album** pointer)
{
    if (oldAlbum)
    {
        d->guardedPointers.remove(oldAlbum, pointer);
    }

    if (album)
    {
        d->guardedPointers.insert(album, pointer);
    }
}

void AlbumManager::invalidateGuardedPointers(Album* album)
{
    if (!album)
    {
        return;
    }

    QMultiHash<Album*, Album**>::iterator it = d->guardedPointers.find(album);

    for ( ; (it != d->guardedPointers.end()) && (it.key() == album) ; ++it)
    {
        if (it.value())
        {
            *(it.value()) = nullptr;
        }
    }
}

void AlbumManager::getAlbumItemsCount()
{
    d->albumItemCountTimer->stop();

    if (!ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        return;
    }

    if (d->albumListJob)
    {
        disconnect(d->albumListJob, nullptr,
                   this, nullptr);

        d->albumListJob->cancel();
        d->albumListJob = nullptr;
    }

    AlbumsDBJobInfo jInfo;
    jInfo.setFoldersJob();
    d->albumListJob = DBJobsManager::instance()->startAlbumsJobThread(jInfo);

    connect(d->albumListJob, SIGNAL(signalFinished()),
            this, SLOT(slotAlbumsJobResult()));

    connect(d->albumListJob, SIGNAL(foldersData(QHash<int,int>)),
            this, SLOT(slotAlbumsJobData(QHash<int,int>)));
}

void AlbumManager::slotAlbumChange(const AlbumChangeset& changeset)
{
    if (d->changingDB || !d->rootPAlbum)
    {
        return;
    }

    switch (changeset.operation())
    {
        case AlbumChangeset::Added:
        case AlbumChangeset::Deleted:
        {
            if (!d->scanPAlbumsTimer->isActive())
            {
                d->scanPAlbumsTimer->start();
            }

            break;
        }

        case AlbumChangeset::Renamed:
        case AlbumChangeset::PropertiesChanged:
        {
            // mark for rescan

            d->changedPAlbums << changeset.albumId();

            if (!d->updatePAlbumsTimer->isActive())
            {
                d->updatePAlbumsTimer->start();
            }

            break;
        }

        case AlbumChangeset::Unknown:
        {
            break;
        }
    }
}

qlonglong AlbumManager::getItemFromAlbum(Album* const album, const QString& fileName)
{
    return CoreDbAccess().db()->getItemFromAlbum(album->id(), fileName);
}

} // namespace Digikam
