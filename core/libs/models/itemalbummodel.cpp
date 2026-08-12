/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with database job
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemalbummodel.h"

// Qt includes

#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "facetagsiface.h"
#include "coredbwatch.h"
#include "coredburl.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "itemlister.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "dbjobsmanager.h"
#include "dbjobsthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemAlbumModel::Private
{
public:

    Private() = default;

public:

    QList<Album*>     currentAlbums;
    DBJobsThread*     jobThread                 = nullptr;
    QTimer*           incrementalTimer          = nullptr;

    bool              recurseAlbums             = false;
    bool              recurseTags               = false;
    bool              listOnlyAvailableImages   = false;
    QString           specialListing;

    bool              extraValueJob             = false;
};

ItemAlbumModel::ItemAlbumModel(QWidget* const parent)
    : ItemThumbnailModel(parent),
      d                 (new Private)
{
    qRegisterMetaType<QList<ItemListerRecord> >("QList<ItemListerRecord>");

    d->incrementalTimer = new QTimer(this);
    d->incrementalTimer->setSingleShot(true);

    connect(d->incrementalTimer, SIGNAL(timeout()),
            this, SLOT(slotNextIncrementalRefresh()));

    connect(this, SIGNAL(readyForIncrementalRefresh()),
            this, SLOT(incrementalRefresh()));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(searchChange(SearchChangeset)),
            this, SLOT(slotSearchChange(SearchChangeset)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(AlbumManager::instance(), SIGNAL(signalShowOnlyAvailableAlbumsChanged(bool)),
            this, SLOT(setListOnlyAvailableImages(bool)));
}

ItemAlbumModel::~ItemAlbumModel()
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = nullptr;
    }

    delete d;
}

QList<Album*> ItemAlbumModel::currentAlbums() const
{
    return d->currentAlbums;
}

void ItemAlbumModel::setRecurseAlbums(bool recursiveListing)
{
    if (d->recurseAlbums != recursiveListing)
    {
        d->recurseAlbums = recursiveListing;
        refresh();
    }
}

void ItemAlbumModel::setRecurseTags(bool recursiveListing)
{
    if (d->recurseTags != recursiveListing)
    {
        d->recurseTags = recursiveListing;
        refresh();
    }
}

void ItemAlbumModel::setListOnlyAvailableImages(bool onlyAvailable)
{
    if (d->listOnlyAvailableImages!= onlyAvailable)
    {
        d->listOnlyAvailableImages = onlyAvailable;
        refresh();
    }
}

bool ItemAlbumModel::isRecursingAlbums() const
{
    return d->recurseAlbums;
}

bool ItemAlbumModel::isRecursingTags() const
{
    return d->recurseTags;
}

bool ItemAlbumModel::isListingOnlyAvailableImages() const
{
    return d->listOnlyAvailableImages;
}

void ItemAlbumModel::setSpecialTagListing(const QString& specialListing)
{
    if (d->specialListing != specialListing)
    {
        d->specialListing = specialListing;
        refresh();
    }
}

void ItemAlbumModel::openAlbum(const QList<Album*>& albums)
{
    if (d->currentAlbums == albums)
    {
        return;
    }

    d->currentAlbums.clear();

    /**
     * Extra safety, ensure that no null pointers are added
     */

    for (Album* const a : std::as_const(albums))                 // cppcheck-suppress constVariablePointer
    {
        if (a)
        {
            d->currentAlbums << a;
        }
    }
/*
    Q_EMIT listedAlbumChanged(d->currentAlbums);
*/
    refresh();
}

void ItemAlbumModel::refresh()
{
    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = nullptr;
    }

    clearItemInfos();

    if (d->currentAlbums.isEmpty())
    {
        return;
    }

/*
    // TODO: Figure out how to deal with root album

    if (d->currentAlbum->isRoot())
    {
        return;
    }
*/

    startRefresh();

    startListJob(d->currentAlbums);
}

void ItemAlbumModel::incrementalRefresh()
{
    // The path to this method is:
    // scheduleIncrementalRefresh -> incrementalTimer waits 100ms -> slotNextIncrementalRefresh
    // -> ItemModel::requestIncrementalRefresh -> waits until model is ready, maybe immediately
    // -> to this method via SIGNAL(readyForIncrementalRefresh())

    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = nullptr;
    }

    startIncrementalRefresh();

    startListJob(d->currentAlbums);
}

bool ItemAlbumModel::hasScheduledRefresh() const
{
    return (d->incrementalTimer->isActive() || hasIncrementalRefreshPending());
}

void ItemAlbumModel::scheduleIncrementalRefresh()
{
    if (!hasScheduledRefresh())
    {
        d->incrementalTimer->start(250);
    }
}

void ItemAlbumModel::slotNextIncrementalRefresh()
{
    if (d->jobThread)
    {
        d->incrementalTimer->start(100);
    }
    else
    {
        requestIncrementalRefresh();
    }
}

void ItemAlbumModel::startListJob(const QList<Album*>& albums)
{
    if (albums.isEmpty())
    {
        return;
    }

    if (d->jobThread)
    {
        d->jobThread->cancel();
        d->jobThread = nullptr;
    }

    // stop preloading Thumbnails

    imageInfosCleared();

    if (albums.first()->isTrashAlbum())
    {
        return;
    }

    CoreDbUrl url;
    QList<int> ids;

    if ((albums.first()->type() == Album::TAG) || (albums.first()->type() == Album::SEARCH))
    {
        for (QList<Album*>::const_iterator it = albums.constBegin() ; it != albums.constEnd() ; ++it)
        {
            ids << (*it)->id();
        }

        if (albums.first()->type() == Album::TAG)
        {
            url = CoreDbUrl::fromTagIds(ids);
        }
    }
    else
    {
        url = albums.first()->databaseUrl();
    }

    if      (albums.first()->type() == Album::DATE)
    {
        d->extraValueJob = false;

        DatesDBJobInfo jobInfo;

        jobInfo.setStartDate(url.startDate());
        jobInfo.setEndDate(url.endDate());

        if (d->recurseAlbums)
        {
            jobInfo.setRecursive();
        }

        if (d->listOnlyAvailableImages)
        {
            jobInfo.setListAvailableImagesOnly();
        }

        d->jobThread = DBJobsManager::instance()->startDatesJobThread(jobInfo);
    }
    else if (albums.first()->type() == Album::TAG)
    {
        d->extraValueJob = false;

        TagsDBJobInfo jobInfo;

        if (d->listOnlyAvailableImages)
        {
            jobInfo.setListAvailableImagesOnly();
        }

        if (d->recurseTags)
        {
            jobInfo.setRecursive();
        }

        jobInfo.setTagsIds(ids);

        if (!d->specialListing.isNull())
        {
            jobInfo.setSpecialTag(d->specialListing);
            d->extraValueJob = true;
        }

        d->jobThread = DBJobsManager::instance()->startTagsJobThread(jobInfo);
    }
    else if (albums.first()->type() == Album::PHYSICAL)
    {
        d->extraValueJob = false;
        AlbumsDBJobInfo jobInfo;

        if (d->recurseAlbums)
        {
            jobInfo.setRecursive();
        }

        if (d->listOnlyAvailableImages)
        {
            jobInfo.setListAvailableImagesOnly();
        }

        jobInfo.setAlbumRootId(url.albumRootId());
        jobInfo.setAlbum( url.album() );

        d->jobThread = DBJobsManager::instance()->startAlbumsJobThread(jobInfo);
    }
    else if (albums.first()->type() == Album::SEARCH)
    {
        d->extraValueJob = false;

        SearchesDBJobInfo jobInfo(std::move(ids));

        if (d->listOnlyAvailableImages)
        {
            jobInfo.setListAvailableImagesOnly();
        }

        d->jobThread = DBJobsManager::instance()->startSearchesJobThread(jobInfo);
    }

    connect(d->jobThread, SIGNAL(signalFinished()),
            this, SLOT(slotResult()));

    connect(d->jobThread, SIGNAL(data(QList<ItemListerRecord>)),
            this, SLOT(slotData(QList<ItemListerRecord>)));
}

void ItemAlbumModel::slotResult()
{
    if (d->jobThread != sender())
    {
        return;
    }

    if (d->jobThread->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list url: "
                                       << d->jobThread->errorsList().first();

        // Pop-up a message about the error.

        DNotificationWrapper(QString(), d->jobThread->errorsList().first(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }

    d->jobThread->cancel();
    d->jobThread = nullptr;

    // either of the two

    finishRefresh();
    finishIncrementalRefresh();
}

void ItemAlbumModel::slotData(const QList<ItemListerRecord>& records)
{
    if (d->jobThread != sender())
    {
        return;
    }

    if (records.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Data From DBJobsThread is null: " << records.isEmpty();
        return;
    }

    ItemInfoList newItemsList;

    if (d->extraValueJob)
    {
        QList<QVariant> extraValues;

        for (const ItemListerRecord& record : std::as_const(records))
        {
            ItemInfo info(record);
            newItemsList << info;

            if (d->specialListing == QLatin1String("faces"))
            {
                FaceTagsIface face = FaceTagsIface::fromListing(info.id(), record.extraValues);
                extraValues << face.toVariant();
            }
            else
            {
                // default handling: just pass extraValue

                if      (record.extraValues.isEmpty())
                {
                    extraValues  << QVariant();
                }
                else if (record.extraValues.size() == 1)
                {
                    extraValues  << record.extraValues.first();
                }
                else
                {
                    extraValues  << QVariant(record.extraValues);    // uh-uh. List in List.
                }
            }
        }

        addItemInfos(newItemsList, extraValues);
    }
    else
    {
        for (const ItemListerRecord& record : std::as_const(records))
        {
            ItemInfo info(record);
            newItemsList << info;
        }

        addItemInfos(newItemsList);
    }

    // A refresh has just been completed, but another one has already
    // started, then slow down the refresh significantly.

    if (hasScheduledRefresh())
    {
        d->incrementalTimer->start(1000);
    }
}

void ItemAlbumModel::slotImageChange(const ImageChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    ItemModel::slotImageChange(changeset);

    // already scheduled to refresh?

    if (hasScheduledRefresh())
    {
        return;
    }

    // this is for the case that _only_ the status changes, i.e., explicit setVisible()

    if ((DatabaseFields::Images)changeset.changes() == DatabaseFields::Status)
    {
        scheduleIncrementalRefresh();

        return;
    }

    // If we list a search, a change to a property may alter the search result

    QList<Album*>::iterator it;

    /**
     * QList is designed for multiple selection, for now, only tags are supported
     * for SAlbum it will be a list with one element
     */

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        if ((*it)->type() == Album::SEARCH)
        {
            const SAlbum* const salbum = static_cast<SAlbum*>(*it);
            bool needCheckRefresh      = false;

            if      (salbum->isNormalSearch())
            {
                // For searches any touched field can require a refresh.
                // We cannot easily find out which fields are searched for, so we refresh for any change.

                needCheckRefresh = true;
            }
            else if (salbum->isTimelineSearch())
            {
                if (changeset.changes() & DatabaseFields::CreationDate)
                {
                    needCheckRefresh = true;
                }
            }
            else if (salbum->isMapSearch())
            {
                if (changeset.changes() & DatabaseFields::ItemPositionsAll)
                {
                    needCheckRefresh = true;
                }
            }

            if (needCheckRefresh)
            {
                const auto ids = changeset.ids();

                for (const qlonglong& id : ids)
                {
                    // if one matching image id is found, trigger a refresh

                    if (hasImage(id))
                    {   // cppcheck-suppress useStlAlgorithm
                        scheduleIncrementalRefresh();
                        break;
                    }
                }
            }
        }
    }
}

void ItemAlbumModel::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    ItemModel::slotImageTagChange(changeset);

    // already scheduled to refresh?

    if (hasScheduledRefresh())
    {
        return;
    }

    bool doRefresh = false;
    QList<Album*>::iterator it;

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        if ((*it)->type() == Album::TAG)
        {
            doRefresh = changeset.containsTag((*it)->id());

            if (!doRefresh && d->recurseTags)
            {
                const auto ids = changeset.tags();

                for (int tagId : ids)
                {
                    const Album* const a = AlbumManager::instance()->findTAlbum(tagId);

                    if (a && (*it)->isAncestorOf(a))
                    {
                        doRefresh = true;
                        break;
                    }
                }
            }
        }

        if (doRefresh)
        {
            break;
        }
    }

    if (doRefresh)
    {
        scheduleIncrementalRefresh();
    }
}

void ItemAlbumModel::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    // already scheduled to refresh?

    if (hasScheduledRefresh())
    {
        return;
    }

    bool doRefresh = false;

    QList<Album*>::iterator it;

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        switch (changeset.operation())
        {
            case CollectionImageChangeset::Added:
            {
                switch ((*it)->type())
                {
                    case Album::PHYSICAL:
                    {
                        // that's easy: try if our album is affected

                        doRefresh = changeset.containsAlbum((*it)->id());

                        if (!doRefresh && d->recurseAlbums)
                        {
                            const auto ids = changeset.albums();

                            for (int albumId : ids)
                            {
                                const Album* const a = AlbumManager::instance()->findPAlbum(albumId);

                                if (a && (*it)->isAncestorOf(a))
                                {
                                    doRefresh = true;
                                    break;
                                }
                            }
                        }

                        break;
                    }

                    default:
                    {
                        // we cannot easily know if we are affected

                        doRefresh = true;
                        break;
                    }
                }

                break;
            }

            case CollectionImageChangeset::Deleted:
            case CollectionImageChangeset::Removed:
            case CollectionImageChangeset::RemovedAll:
            {
                // is one of our images affected?

                const auto ids = changeset.ids();

                for (const qlonglong& id : ids )
                {
                    // if one matching image id is found, trigger a refresh

                    if (hasImage(id))
                    {   // cppcheck-suppress useStlAlgorithm
                        doRefresh = true;
                        break;
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }

        if (doRefresh)
        {
            break;
        }
    }

    if (doRefresh)
    {
        scheduleIncrementalRefresh();
    }
}

void ItemAlbumModel::slotSearchChange(const SearchChangeset& changeset)
{
    if (d->currentAlbums.isEmpty())
    {
        return;
    }

    if (changeset.operation() != SearchChangeset::Changed)
    {
        return;
    }

    const SAlbum* const album = AlbumManager::instance()->findSAlbum(changeset.searchId());

    QList<Album*>::iterator it;

    for (it = d->currentAlbums.begin() ; it != d->currentAlbums.end() ; ++it)
    {
        if (album && (*it) == album)
        {
            scheduleIncrementalRefresh();
        }
    }
}

void ItemAlbumModel::slotAlbumAdded(Album* /*album*/)
{
}

void ItemAlbumModel::slotAlbumDeleted(Album* album)
{
    if (d->currentAlbums.contains(album))
    {
        d->currentAlbums.removeOne(album);
        clearItemInfos();
        return;
    }

    // display changed tags

    if (album->type() == Album::TAG)
    {
        emitDataChangedForAll();
    }
}

void ItemAlbumModel::slotAlbumRenamed(Album* album)
{
    // display changed names

    if (d->currentAlbums.contains(album))
    {
        emitDataChangedForAll();
    }
}

void ItemAlbumModel::slotAlbumsCleared()
{
    d->currentAlbums.clear();
    clearItemInfos();
}

} // namespace Digikam

#include "moc_itemalbummodel.cpp"
