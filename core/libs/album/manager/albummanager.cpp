/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
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

class Q_DECL_HIDDEN AlbumManagerCreator
{
public:

    AlbumManager object;
};

Q_GLOBAL_STATIC(AlbumManagerCreator, creator)

AlbumManager* AlbumManager::instance()
{
    return &creator->object;
}

// -----------------------------------------------------------------------------------

AlbumManager::AlbumManager()
    : QObject(),
      d      (new Private)
{
    qRegisterMetaType<QHash<QDateTime,int>>("QHash<QDateTime,int>");
    qRegisterMetaType<QMap<int,int>>("QMap<int,int>");
    qRegisterMetaType<QHash<int,int>>("QHash<int,int>");
    qRegisterMetaType<QMap<QString,QHash<int,int> >>("QMap<QString,QHash<int,int> >");

    d->albumWatch       = new AlbumWatch(this);

    // these operations are pretty fast, no need for long queuing

    d->scanPAlbumsTimer = new QTimer(this);
    d->scanPAlbumsTimer->setInterval(150);
    d->scanPAlbumsTimer->setSingleShot(true);

    connect(d->scanPAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanPAlbums()));

    d->scanTAlbumsTimer = new QTimer(this);
    d->scanTAlbumsTimer->setInterval(150);
    d->scanTAlbumsTimer->setSingleShot(true);

    connect(d->scanTAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanTAlbums()));

    d->scanSAlbumsTimer = new QTimer(this);
    d->scanSAlbumsTimer->setInterval(150);
    d->scanSAlbumsTimer->setSingleShot(true);

    connect(d->scanSAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanSAlbums()));

    d->updatePAlbumsTimer = new QTimer(this);
    d->updatePAlbumsTimer->setInterval(150);
    d->updatePAlbumsTimer->setSingleShot(true);

    connect(d->updatePAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(updateChangedPAlbums()));

    // this operation is much more expensive than the other scan methods

    d->scanDAlbumsTimer = new QTimer(this);
    d->scanDAlbumsTimer->setInterval(30 * 1000);
    d->scanDAlbumsTimer->setSingleShot(true);

    connect(d->scanDAlbumsTimer, SIGNAL(timeout()),
            this, SLOT(scanDAlbumsScheduled()));

    // moderately expensive

    d->albumItemCountTimer = new QTimer(this);
    d->albumItemCountTimer->setInterval(1000);
    d->albumItemCountTimer->setSingleShot(true);

    connect(d->albumItemCountTimer, SIGNAL(timeout()),
            this, SLOT(getAlbumItemsCount()));

    // more expensive

    d->tagItemCountTimer = new QTimer(this);
    d->tagItemCountTimer->setInterval(2500);
    d->tagItemCountTimer->setSingleShot(true);

    connect(d->tagItemCountTimer, SIGNAL(timeout()),
            this, SLOT(getTagItemsCount()));
}

AlbumManager::~AlbumManager()
{
    delete d->rootPAlbum;
    delete d->rootTAlbum;
    delete d->rootDAlbum;
    delete d->rootSAlbum;

    delete d;
}

void AlbumManager::cleanUp()
{
    // This is what we prefer to do before Application destruction

    if (d->dateListJob)
    {
        disconnect(d->dateListJob, nullptr,
                   this, nullptr);

        d->dateListJob->cancel();
        d->dateListJob = nullptr;
    }

    if (d->albumListJob)
    {
        disconnect(d->albumListJob, nullptr,
                   this, nullptr);

        d->albumListJob->cancel();
        d->albumListJob = nullptr;
    }

    if (d->tagListJob)
    {
        disconnect(d->tagListJob, nullptr,
                   this, nullptr);

        d->tagListJob->cancel();
        d->tagListJob = nullptr;
    }

    if (d->personListJob)
    {
        disconnect(d->personListJob, nullptr,
                   this, nullptr);

        d->personListJob->cancel();
        d->personListJob = nullptr;
    }

    d->scanPAlbumsTimer->stop();
    d->scanTAlbumsTimer->stop();
    d->scanSAlbumsTimer->stop();
    d->scanDAlbumsTimer->stop();
    d->tagItemCountTimer->stop();
    d->updatePAlbumsTimer->stop();
    d->albumItemCountTimer->stop();
}

void AlbumManager::startScan()
{
    if (!d->changed)
    {
        return;
    }

    d->changed = false;

    // create root albums

    d->rootPAlbum = new PAlbum(i18n("Albums"));
    insertPAlbum(d->rootPAlbum, nullptr);

    d->rootTAlbum = new TAlbum(i18n("Tags"), 0, true);
    insertTAlbum(d->rootTAlbum, nullptr);

    d->rootSAlbum = new SAlbum(i18n("Searches"), 0, true);

    Q_EMIT signalAlbumAboutToBeAdded(d->rootSAlbum, nullptr, nullptr);

    d->allAlbumsIdHash[d->rootSAlbum->globalID()] = d->rootSAlbum;

    Q_EMIT signalAlbumAdded(d->rootSAlbum);

    d->rootDAlbum = new DAlbum(QDate(), true);

    Q_EMIT signalAlbumAboutToBeAdded(d->rootDAlbum, nullptr, nullptr);

    d->allAlbumsIdHash[d->rootDAlbum->globalID()] = d->rootDAlbum;

    Q_EMIT signalAlbumAdded(d->rootDAlbum);

    // Create albums for album roots. Reuse logic implemented in the method

    const auto locs = CollectionManager::instance()->allLocations();

    for (const CollectionLocation& location : locs)
    {
        handleCollectionStatusChange(location, CollectionLocation::LocationNull);
    }

    // listen to location status changes

    connect(CollectionManager::instance(), SIGNAL(locationStatusChanged(CollectionLocation,int)),
            this, SLOT(slotCollectionLocationStatusChanged(CollectionLocation,int)));

    connect(CollectionManager::instance(), SIGNAL(locationPropertiesChanged(CollectionLocation)),
            this, SLOT(slotCollectionLocationPropertiesChanged(CollectionLocation)));

    // reload albums

    refresh();

    // listen to album database changes

    connect(CoreDbAccess::databaseWatch(), SIGNAL(albumChange(AlbumChangeset)),
            this, SLOT(slotAlbumChange(AlbumChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(tagChange(TagChangeset)),
            this, SLOT(slotTagChange(TagChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(searchChange(SearchChangeset)),
            this, SLOT(slotSearchChange(SearchChangeset)));

    // listen to collection image changes

    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));

    connect(CoreDbAccess::databaseWatch(), SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));

    // listen to image attribute changes

    connect(ItemAttributesWatch::instance(), SIGNAL(signalImageDateChanged(qlonglong)),
            d->scanDAlbumsTimer, SLOT(start()));

    Q_EMIT signalAllAlbumsLoaded();
}

bool AlbumManager::isShowingOnlyAvailableAlbums() const
{
    return d->showOnlyAvailableAlbums;
}

void AlbumManager::setShowOnlyAvailableAlbums(bool onlyAvailable)
{
    if (d->showOnlyAvailableAlbums == onlyAvailable)
    {
        return;
    }

    d->showOnlyAvailableAlbums = onlyAvailable;

    Q_EMIT signalShowOnlyAvailableAlbumsChanged(d->showOnlyAvailableAlbums);

    // We need to update the unavailable locations.
    // We assume the handleCollectionStatusChange does the right thing (even though old status == current status)

    const auto locs = CollectionManager::instance()->allLocations();

    for (const CollectionLocation& location : locs)
    {
        if (location.status() == CollectionLocation::LocationUnavailable)
        {
            handleCollectionStatusChange(location, CollectionLocation::LocationUnavailable);
        }
    }
}

void AlbumManager::refresh()
{
    scanPAlbums();
    scanTAlbums();
    scanSAlbums();
    scanDAlbums();
}

void AlbumManager::prepareItemCounts()
{
    // There is no way to find out if any data we had collected
    // previously is still valid - recompute

    scanDAlbums();
    getAlbumItemsCount();
    getTagItemsCount();
}

void AlbumManager::slotImagesDeleted(const QList<qlonglong>& imageIds)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Got image deletion notification from ItemViewUtilities for " << imageIds.size() << " images.";

    QSet<SAlbum*> sAlbumsToUpdate;
    QSet<qlonglong> deletedImages(imageIds.begin(), imageIds.end());

    QList<SAlbum*> sAlbums = findSAlbumsBySearchType(DatabaseSearch::DuplicatesSearch);

    for (SAlbum* const sAlbum : std::as_const(sAlbums))
    {
        // Read the search query XML and save the image ids

        SearchXmlReader reader(sAlbum->query());
        SearchXml::Element element;
        QSet<qlonglong> images;

        while ((element = reader.readNext()) != SearchXml::End)
        {
            if ((element == SearchXml::Field) && (reader.fieldName().compare(QLatin1String("imageid")) == 0))
            {
                const auto list = reader.valueToLongLongList();
                images          = QSet<qlonglong>(list.begin(), list.end());
            }
        }

        // If the deleted images are part of the SAlbum,
        // mark the album as ready for deletion and the images as ready for rescan.

        if (images.intersects(deletedImages))
        {
            sAlbumsToUpdate.insert(sAlbum);
        }
    }

    if (!sAlbumsToUpdate.isEmpty())
    {
        Q_EMIT signalUpdateDuplicatesAlbums(sAlbumsToUpdate.values(), deletedImages.values());
    }
}

} // namespace Digikam

#include "moc_albummanager.cpp"
