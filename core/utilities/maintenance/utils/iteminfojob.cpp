/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-22-01
 * Description : interface to get item info from database.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfojob.h"

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "itemlister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "dbjobsmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemInfoJob::Private
{
public:

    Private() = default;

public:

    DBJobsThread* jobThread = nullptr;
};

ItemInfoJob::ItemInfoJob()
    : d(new Private)
{
}

ItemInfoJob::~ItemInfoJob()
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
    }

    delete d;
}

void ItemInfoJob::allItemsFromAlbum(Album* const album)
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = nullptr;
    }

    if (!album)
    {
        return;
    }

    // TODO: Drop Database Url usage

    CoreDbUrl url = album->databaseUrl();

    if      (album->type() == Album::DATE)
    {
        DatesDBJobInfo jobInfo;
        jobInfo.setStartDate(url.startDate());
        jobInfo.setEndDate(url.endDate());

        d->jobThread = DBJobsManager::instance()->startDatesJobThread(jobInfo);
    }
    else if (album->type() == Album::TAG)
    {
        TagsDBJobInfo jobInfo;

        // If we want to search for images with this tag, we only want the tag and not
        // all images in the tag path.

        jobInfo.setTagsIds(QList<int>() << url.tagId());

        d->jobThread = DBJobsManager::instance()->startTagsJobThread(jobInfo);
    }
    else if (album->type() == Album::PHYSICAL)
    {
        AlbumsDBJobInfo jobInfo;
        jobInfo.setAlbumRootId(url.albumRootId());
        jobInfo.setAlbum(url.album());

        d->jobThread = DBJobsManager::instance()->startAlbumsJobThread(jobInfo);
    }
    else if (album->type() == Album::SEARCH)
    {
        QList<int> searchIds = QList<int>() << url.searchId();
        SearchesDBJobInfo jobInfo(std::move(searchIds));

        d->jobThread = DBJobsManager::instance()->startSearchesJobThread(jobInfo);
    }

    connect(d->jobThread, SIGNAL(signalFinished()),
            this, SLOT(slotResult()));

    connect(d->jobThread, SIGNAL(data(QList<ItemListerRecord>)),
            this, SLOT(slotData(QList<ItemListerRecord>)));
}

void ItemInfoJob::stop()
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = nullptr;
    }
}

bool ItemInfoJob::isRunning() const
{
    return d->jobThread;
}

void ItemInfoJob::slotResult()
{
    if (d->jobThread != sender())
    {
        return;
    }

    if (d->jobThread->hasErrors())
    {
        qCWarning(DIGIKAM_MAINTENANCE_LOG) << "Failed to list url: " << d->jobThread->errorsList().first();

        // Pop-up a message about the error.

        DNotificationWrapper(QString(), d->jobThread->errorsList().first(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    d->jobThread = nullptr;

    Q_EMIT signalCompleted();
}

void ItemInfoJob::slotData(const QList<ItemListerRecord>& records)
{
    if (records.isEmpty())
    {
        return;
    }

    ItemInfoList itemsList;

    for (const ItemListerRecord& record : std::as_const(records))
    {
        ItemInfo info(record);
        itemsList.append(info);
    }

    // Sort the itemList based on name

    std::sort(itemsList.begin(), itemsList.end(), ItemInfoList::namefileLessThan);

    Q_EMIT signalItemsInfo(itemsList);
}

} // namespace Digikam

#include "moc_iteminfojob.cpp"
