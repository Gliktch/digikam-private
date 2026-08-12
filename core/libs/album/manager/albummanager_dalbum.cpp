/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Date Album helpers.
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

void AlbumManager::scanDAlbums()
{
    d->scanDAlbumsTimer->stop();

    if (d->dateListJob)
    {
        disconnect(d->dateListJob, nullptr, this, nullptr);

        d->dateListJob->cancel();
        d->dateListJob = nullptr;
    }

    DatesDBJobInfo jInfo;
    jInfo.setFoldersJob();
    d->dateListJob = DBJobsManager::instance()->startDatesJobThread(jInfo);

    connect(d->dateListJob, SIGNAL(signalFinished()),
            this, SLOT(slotDatesJobResult()));

    connect(d->dateListJob, SIGNAL(foldersData(QMap<QDateTime,int>)),
            this, SLOT(slotDatesJobData(QMap<QDateTime,int>)));
}

AlbumList AlbumManager::allDAlbums() const
{
    AlbumList list;

    if (d->rootDAlbum)
    {
        list.append(d->rootDAlbum);
    }

    AlbumIterator it(d->rootDAlbum);

    while (it.current())
    {
        list.append(*it);
        ++it;
    }

    return list;
}

DAlbum* AlbumManager::findDAlbum(int id) const
{
    if (!d->rootDAlbum)
    {
        return nullptr;
    }

    int gid = d->rootDAlbum->globalID() + id;

    return static_cast<DAlbum*>((d->allAlbumsIdHash.value(gid)));
}

QMap<YearMonth, int> AlbumManager::getDAlbumsCount() const
{
    return d->dAlbumsCount;
}

void AlbumManager::slotDatesJobResult()
{
    if (!d->dateListJob)
    {
        return;
    }

    if (d->dateListJob->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list dates";

        // Pop-up a message about the error.

        DNotificationWrapper(QString(), d->dateListJob->errorsList().first(),
                             nullptr, i18n("digiKam"));
    }

    d->dateListJob = nullptr;

    Q_EMIT signalAllDAlbumsLoaded();
}

void AlbumManager::slotDatesJobData(const QMap<QDateTime, int>& datesStatMap)
{
    if (datesStatMap.isEmpty() || !d->rootDAlbum)
    {
        return;
    }

    // insert all the DAlbums into a qmap for quick access

    QMap<QDate, DAlbum*> mAlbumMap;
    QMap<int, DAlbum*>   yAlbumMap;

    AlbumIterator it(d->rootDAlbum);

    while (it.current())
    {
        DAlbum* const a = (DAlbum*)(*it);       // cppcheck-suppress constVariablePointer

        if (a->range() == DAlbum::Month)
        {
            mAlbumMap.insert(a->date(), a);
        }
        else
        {
            yAlbumMap.insert(a->date().year(), a);
        }

        ++it;
    }

    QMap<YearMonth, int> yearMonthMap;

    for (QMap<QDateTime, int>::const_iterator it3 = datesStatMap.constBegin() ;
         it3 != datesStatMap.constEnd() ; ++it3)
    {
        YearMonth yearMonth = YearMonth(it3.key().date().year(), it3.key().date().month());

        QMap<YearMonth, int>::iterator it2 = yearMonthMap.find(yearMonth);

        if (it2 == yearMonthMap.end())
        {
            yearMonthMap.insert(yearMonth, *it3);
        }
        else
        {
            *it2 += *it3;
        }
    }

    int year, month;

    for (QMap<YearMonth, int>::const_iterator it4 = yearMonthMap.constBegin() ;
         it4 != yearMonthMap.constEnd() ; ++it4)
    {
        year  = it4.key().first;
        month = it4.key().second;

        QDate md(year, month, 1);

        // Do we already have this Month album

        if (mAlbumMap.contains(md))
        {
            // already there. remove Month album from map

            mAlbumMap.remove(md);

            if (yAlbumMap.contains(year))
            {
                // already there. remove from map

                yAlbumMap.remove(year);
            }

            continue;
        }

        // Check if Year Album already exist.

        DAlbum* yAlbum = nullptr;
        AlbumIterator it5(d->rootDAlbum);

        while (it5.current())
        {
            DAlbum* const a = (DAlbum*)(*it5);

            if ((a->date() == QDate(year, 1, 1)) && (a->range() == DAlbum::Year))
            {
                yAlbum = a;
                break;
            }

            ++it5;
        }

        // If no, create Year album.

        if (!yAlbum)
        {
            yAlbum = new DAlbum(QDate(year, 1, 1), false, DAlbum::Year);

            Q_EMIT signalAlbumAboutToBeAdded(yAlbum, d->rootDAlbum, d->rootDAlbum->lastChild());

            yAlbum->setParent(d->rootDAlbum);
            d->allAlbumsIdHash.insert(yAlbum->globalID(), yAlbum);

            Q_EMIT signalAlbumAdded(yAlbum);
        }

        // Create Month album

        DAlbum* const mAlbum = new DAlbum(md);

        Q_EMIT signalAlbumAboutToBeAdded(mAlbum, yAlbum, yAlbum->lastChild());

        mAlbum->setParent(yAlbum);
        d->allAlbumsIdHash.insert(mAlbum->globalID(), mAlbum);

        Q_EMIT signalAlbumAdded(mAlbum);
    }

    // Now the items contained in the maps are the ones which
    // have been deleted.

    for (QMap<QDate, DAlbum*>::const_iterator it6 = mAlbumMap.constBegin() ;
         it6 != mAlbumMap.constEnd() ; ++it6)
    {
        DAlbum* const album = it6.value();

        Q_EMIT signalAlbumAboutToBeDeleted(album);

        d->allAlbumsIdHash.remove(album->globalID());

        Q_EMIT signalAlbumDeleted(album);

        album->prepareForDeletion();

        Q_EMIT signalAlbumHasBeenDeleted(album);

        delete album;
    }

    for (QMap<int, DAlbum*>::const_iterator it7 = yAlbumMap.constBegin() ;
         it7 != yAlbumMap.constEnd() ; ++it7)
    {
        DAlbum* const album = it7.value();

        Q_EMIT signalAlbumAboutToBeDeleted(album);

        d->allAlbumsIdHash.remove(album->globalID());

        Q_EMIT signalAlbumDeleted(album);

        album->prepareForDeletion();

        Q_EMIT signalAlbumHasBeenDeleted(album);

        delete album;
    }

    d->dAlbumsCount = yearMonthMap;

    Q_EMIT signalDAlbumsDirty(yearMonthMap);
    Q_EMIT signalDatesMapDirty(datesStatMap);
}

void AlbumManager::scanDAlbumsScheduled()
{
    // Avoid a cycle of killing a job which takes longer than the timer interval

    if (d->dateListJob)
    {
        d->scanDAlbumsTimer->start();
        return;
    }

    scanDAlbums();
}

} // namespace Digikam
