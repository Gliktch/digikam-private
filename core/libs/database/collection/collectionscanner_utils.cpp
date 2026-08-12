/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database - scan utilities.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionscanner_p.h"

namespace Digikam
{

void CollectionScanner::loadNameFilters()
{
    if (!d->nameFilters.isEmpty())
    {
        return;
    }

    QStringList imageFilter, audioFilter, videoFilter, ignoreDirectory;

    CoreDbAccess().db()->getFilterSettings(&imageFilter, &videoFilter, &audioFilter);
    CoreDbAccess().db()->getIgnoreDirectoryFilterSettings(&ignoreDirectory);

    // Three sets to find category of a file

    d->imageFilterSet  = QSet<QString>(imageFilter.begin(),     imageFilter.end());
    d->audioFilterSet  = QSet<QString>(audioFilter.begin(),     audioFilter.end());
    d->videoFilterSet  = QSet<QString>(videoFilter.begin(),     videoFilter.end());
    d->ignoreDirectory = QSet<QString>(ignoreDirectory.begin(), ignoreDirectory.end());

    d->nameFilters     = d->imageFilterSet + d->audioFilterSet + d->videoFilterSet;
}

void CollectionScanner::mainEntryPoint(bool complete)
{
    loadNameFilters();
    d->recordHistoryIds = !complete;
}

void CollectionScanner::safelyRemoveAlbums(const QList<int>& albumIds)
{
    // Remove the items (orphan items, detach them from the album, but keep entries for a certain time)
    // Make album orphan (no album root, keep entries until next application start)

    CoreDbAccess access;
    CoreDbTransaction transaction(&access);

    for (int albumId : std::as_const(albumIds))
    {
        QList<qlonglong> ids = access.db()->getItemIDsInAlbum(albumId);
        access.db()->removeItemsFromAlbum(albumId, ids);
        access.db()->makeStaleAlbum(albumId);
        itemsWereRemoved(ids);
    }
}

int CollectionScanner::checkAlbum(const CollectionLocation& location, const QString& album)
{
    // Get album id if album exists

    int albumID = CoreDbAccess().db()->getAlbumForPath(location.id(), album, false);

    // Create if necessary

    if (albumID == -1)
    {
        QFileInfo fi(location.albumRootPath() + album);
        albumID = CoreDbAccess().db()->addAlbum(location.id(), album, QString(), fi.lastModified().date(), QString());
    }

    return albumID;
}

void CollectionScanner::copyFileProperties(const ItemInfo& source, const ItemInfo& d)
{
    if (source.isNull() || d.isNull())
    {
        return;
    }

    ItemInfo dest(d);
    CoreDbOperationGroup group;

    qCDebug(DIGIKAM_DATABASE_LOG) << "Copying properties from" << source.id() << "to" << dest.id();

    // Rating, creation dates

    DatabaseFields::ItemInformation imageInfoFields = DatabaseFields::Rating       |
                                                      DatabaseFields::CreationDate |
                                                      DatabaseFields::DigitizationDate;

    QVariantList imageInfos = CoreDbAccess().db()->getItemInformation(source.id(), imageInfoFields);

    if (!imageInfos.isEmpty())
    {
        CoreDbAccess().db()->changeItemInformation(dest.id(), imageInfos, imageInfoFields);
    }

    // Copy public tags

    const auto ids = TagsCache::instance()->publicTags(source.tagIds());

    for (int tagId : ids)
    {
        dest.setTag(tagId);
    }

    // Copy color and pick label

    dest.setPickLabel(source.pickLabel());
    dest.setColorLabel(source.colorLabel());

    // important: skip other internal tags, such a history tags. Therefore CoreDB::copyImageTags is not to be used.

    // GPS data

    QVariantList positionData = CoreDbAccess().db()->getItemPosition(source.id(), DatabaseFields::ItemPositionsAll);

    if (!positionData.isEmpty())
    {
        CoreDbAccess().db()->addItemPosition(dest.id(), positionData, DatabaseFields::ItemPositionsAll);
    }

    // Comments

    {
        CoreDbAccess access;
        ItemComments commentsSource(access, source.id());
        ItemComments commentsDest(access, dest.id());
        commentsDest.replaceFrom(commentsSource);
        commentsDest.apply(access);
    }

    // Copyright info

    ItemCopyright copyrightDest(dest.id());
    copyrightDest.replaceFrom(ItemCopyright(source.id()));

    // Image Properties

    CoreDbAccess().db()->copyImageProperties(source.id(), dest.id());
}

void CollectionScanner::itemsWereRemoved(const QList<qlonglong>& removedIds)
{
    // Set time stamp

    d->removedItems();

    // Manage relations

    QList<qlonglong> relatedImages = CoreDbAccess().db()->getOneRelatedImageEach(removedIds, DatabaseRelation::DerivedFrom);
    qCDebug(DIGIKAM_DATABASE_LOG) << "Removed items:" << removedIds << "related items:" << relatedImages;

    if (d->recordHistoryIds)
    {
        for (const qlonglong& id : std::as_const(relatedImages))
        {
            d->needTaggingHistorySet << id;
        }
    }
    else
    {
        int needTaggingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());
        CoreDbAccess().db()->addTagsToItems(relatedImages, QList<int>() << needTaggingTag);
    }
}

void CollectionScanner::readDirectoryInCache(int& items, const QString& path)
{
    const auto dirList = QDir(path).entryInfoList(QDir::Dirs    |
                                                  QDir::Files   |
                                                  QDir::NoDotAndDotDot);

    for (const auto& info : dirList)
    {
        if (!d->checkObserver())
        {
            return;
        }

        if (info.isDir())
        {
            if (d->checkIgnoreDirectory(info.fileName()))
            {
                continue;
            }

            d->albumDateCache.insert(info.filePath(),
                                     asDateTimeUTC(info.lastModified()));

            readDirectoryInCache(items, info.filePath());
        }

        ++items;
    }
}

int CollectionScanner::createAlbumDateCache(const CollectionLocation& location, const QString& album)
{
    if (!location.isAvailable())
    {
        return 0;
    }

    QDir dir(location.albumRootPath() + album);

    if (!dir.exists() || !dir.isReadable())
    {
        return 0;
    }

    int items = 1;
    QFileInfo info(dir.path());
    d->albumDateCache.insert(info.filePath(),
                             asDateTimeUTC(info.lastModified()));

    readDirectoryInCache(items, info.filePath());

    return items;
}

DatabaseItem::Category CollectionScanner::category(const QFileInfo& info)
{
    QString suffix = info.suffix().toLower();

    if      (d->imageFilterSet.contains(suffix))
    {
        return DatabaseItem::Image;
    }
    else if (d->audioFilterSet.contains(suffix))
    {
        return DatabaseItem::Audio;
    }
    else if (d->videoFilterSet.contains(suffix))
    {
        return DatabaseItem::Video;
    }
    else
    {
        return DatabaseItem::Other;
    }
}

void CollectionScanner::markDatabaseAsScanned()
{
    CoreDbAccess access;
    access.db()->setSetting(QLatin1String("Scanned"), QDateTime::currentDateTime().toString(Qt::ISODate));
}

void CollectionScanner::updateRemovedItemsTime()
{
    // Called after a complete or partial scan finishes, to write the value
    // held in d->removedItemsTime to the database

    if (!d->removedItemsTime.isNull())
    {
        CoreDbAccess().db()->setSetting(QLatin1String("RemovedItemsTime"), d->removedItemsTime.toString(Qt::ISODate));
        d->removedItemsTime = QDateTime();
    }
}

void CollectionScanner::incrementDeleteRemovedCompleteScanCount()
{
    CoreDbAccess access;
    int count = access.db()->getSetting(QLatin1String("DeleteRemovedCompleteScanCount")).toInt();
    ++count;
    access.db()->setSetting(QLatin1String("DeleteRemovedCompleteScanCount"), QString::number(count));
}

void CollectionScanner::resetDeleteRemovedSettings()
{
    CoreDbAccess().db()->setSetting(QLatin1String("RemovedItemsTime"),               QString());
    CoreDbAccess().db()->setSetting(QLatin1String("DeleteRemovedTime"),              QDateTime::currentDateTime().toString(Qt::ISODate));
    CoreDbAccess().db()->setSetting(QLatin1String("DeleteRemovedCompleteScanCount"), QString::number(0));
}

bool CollectionScanner::checkDeleteRemoved()
{
    // Returns true if removed items shall be deleted

    CoreDbAccess access;

    // Retrieve last time an item was removed (not deleted, but set to status removed)

    QString removedItemsTimeString = access.db()->getSetting(QLatin1String("RemovedItemsTime"));

    if (removedItemsTimeString.isNull())
    {
        return false;
    }

    // Retrieve last time removed items were (definitely) deleted from db

    QString deleteRemovedTimeString = access.db()->getSetting(QLatin1String("DeleteRemovedTime"));
    QDateTime removedItemsTime      = QDateTime::fromString(removedItemsTimeString, Qt::ISODate);
    QDateTime deleteRemovedTime;

    if (!deleteRemovedTimeString.isNull())
    {
        deleteRemovedTime = QDateTime::fromString(deleteRemovedTimeString, Qt::ISODate);
    }

    QDateTime now     = QDateTime::currentDateTime();

    // Retrieve number of complete collection scans since the last time that removed items were deleted

    int completeScans = access.db()->getSetting(QLatin1String("DeleteRemovedCompleteScanCount")).toInt();

    // No removed items? No need to delete any

    if (!removedItemsTime.isValid())
    {
        return false;
    }

    // Give at least a week between removed item deletions

    if (deleteRemovedTime.isValid())
    {
        if (deleteRemovedTime.daysTo(now) <= 7)
        {
            return false;
        }
    }

    // Now look at time since items were removed, and the number of complete scans
    // since removed items were deleted. Values arbitrarily chosen.

    int daysPast = removedItemsTime.daysTo(now);

    return (
            ((daysPast > 7)  && (completeScans > 2)) ||
            ((daysPast > 30) && (completeScans > 0)) ||
            (completeScans > 30)
           );
}

// ------------------------------------------------------------------------------------------

#if 0

void CollectionScanner::scanForStaleAlbums()
{
    QStringList albumRootPaths = CollectionManager::instance()->allAvailableAlbumRootPaths();

    for (QStringList::const_iterator it = albumRootPaths.constBegin() ;
         it != albumRootPaths.constEnd() ; ++it)
    {
        scanForStaleAlbums(*it);
    }
}

void CollectionScanner::scanForStaleAlbums(const QString& albumRoot)
{
    Q_UNUSED(albumRoot);
    QList<AlbumShortInfo> albumList = CoreDbAccess().db()->getAlbumShortInfos();
    QList<AlbumShortInfo> toBeDeleted;

    QList<AlbumShortInfo>::const_iterator it;

    for (it = albumList.constBegin() ; it != albumList.constEnd() ; ++it)
    {
        QFileInfo fileInfo((*it).albumRoot + (*it).url);

        if (!fileInfo.exists() || !fileInfo.isDir())
        {
            m_foldersToBeDeleted << (*it);
        }
    }
}

QStringList CollectionScanner::formattedListOfStaleAlbums()
{
    QStringList list;
    QList<AlbumShortInfo>::const_iterator it;

    for (it = m_foldersToBeDeleted.constBegin() ;
         it != m_foldersToBeDeleted.constEnd() ; ++it)
    {
        list << (*it).url;
    }

    return list;
}

void CollectionScanner::removeStaleAlbums()
{
    CoreDbAccess access;
    CoreDbTransaction transaction(&access);
    QList<AlbumShortInfo>::const_iterator it;

    for (it = m_foldersToBeDeleted.constBegin() ; it != m_foldersToBeDeleted.constEnd() ; ++it)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Removing album " << (*it).albumRoot + QLatin1Char('/') + (*it).url;
        access.db()->deleteAlbum((*it).id);
    }
}

QStringList CollectionScanner::formattedListOfStaleFiles()
{
    QStringList listToBeDeleted;

    CoreDbAccess access;
    QList<QPair<QString, int> >::const_iterator it;

    for (it = m_filesToBeDeleted.constBegin() ; it != m_filesToBeDeleted.constEnd() ; ++it)
    {
        QString location = QLatin1String(" (") + access.db()->getAlbumPath((*it).second) + QLatin1Char(')');

        listToBeDeleted.append((*it).first + location);
    }

    return listToBeDeleted;
}

void CollectionScanner::removeStaleFiles()
{
    CoreDbAccess access;
    CoreDbTransaction transaction(&access);
    QList<QPair<QString, int> >::const_iterator it;

    for (it = m_filesToBeDeleted.constBegin() ; it != m_filesToBeDeleted.constEnd() ; ++it)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Removing: " << (*it).first << " in " << (*it).second;
        access.db()->deleteItem( (*it).second, (*it).first );
    }
}

void CollectionScanner::scanAlbums()
{
    QStringList albumRootPaths = CollectionManager::instance()->allAvailableAlbumRootPaths();
    int count = 0;

    for (QStringList::const_iterator it = albumRootPaths.constBegin() ; it != albumRootPaths.constEnd() ; ++it)
    {
        count += countItemsInFolder(*it);
    }

    Q_EMIT totalFilesToScan(count);

    for (QStringList::const_iterator it = albumRootPaths.constBegin() ; it != albumRootPaths.constEnd() ; ++it)
    {
        QDir dir(*it);
        QStringList fileList(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));
        CoreDbTransaction transaction;

        for (const QString& dir : std::as_const(fileList))
        {
            scanAlbum(*it, QLatin1Char('/') + dir);
        }
    }
}

void CollectionScanner::scan(const QString& folderPath)
{
    CollectionManager* const manager = CollectionManager::instance();
    QUrl url;
    url.setPath(folderPath);
    QString albumRoot = manager->albumRootPath(url);
    QString album     = manager->album(url);

    if (albumRoot.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "scanAlbums(QString): folder " << folderPath << " not found in collection.";
        return;
    }

    scan(albumRoot, album);
}

void CollectionScanner::scan(const QString& albumRoot, const QString& album)
{
    // Step one: remove invalid albums

    scanForStaleAlbums(albumRoot);
    removeStaleAlbums();

    Q_EMIT totalFilesToScan(countItemsInFolder(albumRoot + album));

    // Step two: Scan directories

    if (album == QLatin1String("/"))
    {
        // Don't scan files under album root, only descend into directories (?)

        QDir dir(albumRoot + album);
        QStringList fileList(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));

        CoreDbTransaction transaction;

        for (QStringList::const_iterator fileIt = fileList.constBegin() ; fileIt != fileList.constEnd() ; ++fileIt)
        {
            scanAlbum(albumRoot, QLatin1Char('/') + (*fileIt));
        }
    }
    else
    {
        CoreDbTransaction transaction;
        scanAlbum(albumRoot, album);
    }

    // Step three: Remove invalid files

    removeStaleFiles();
}

void CollectionScanner::scanAlbum(const QString& filePath)
{
    QUrl url;
    url.setPath(filePath);
    scanAlbum(CollectionManager::instance()->albumRootPath(url), CollectionManager::instance()->album(url));
}

void CollectionScanner::scanAlbum(const QString& albumRoot, const QString& album)
{
    /**
     * This method perform following rules:
     *
     * + Adds album if it does not yet exist in the db.
     * + Recursively scans subalbums of album.
     * + Adds files if they do not yet exist in the db.
     * + Adds stale files from the db to m_filesToBeDeleted
     * - Does not add stale albums to m_foldersToBeDeleted.
     */

    QDir dir( albumRoot + album );

    if (!dir.exists() || !dir.isReadable())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Folder does not exist or is not readable: " << dir.path();

        return;
    }

    Q_EMIT startScanningAlbum(albumRoot, album);

    // Get album id if album exists

    int albumID = CoreDbAccess().db()->getAlbumForPath(albumRoot, album, false);

    if (albumID == -1)
    {
        QFileInfo fi(albumRoot + album);
        albumID = CoreDbAccess().db()->addAlbum(albumRoot, album, QString(), fi.lastModified().date(), QString());
    }

    QStringList filesInAlbum = CoreDbAccess().db()->getItemNamesInAlbum( albumID );

    QSet<QString> filesFoundInDB;

    for (QStringList::const_iterator it = filesInAlbum.constBegin() ;
         it != filesInAlbum.constEnd() ; ++it)
    {
        filesFoundInDB << *it;
    }

    const QFileInfoList list = dir.entryInfoList(m_nameFilters, QDir::AllDirs | QDir::Files  | QDir::NoDotAndDotDot /*not CaseSensitive*/);

    QFileInfoList::const_iterator fi;

    for (fi = list.constBegin() ; fi != list.constEnd() ; ++fi)
    {
        if      (fi->isFile())
        {
            if      (filesFoundInDB.contains(fi->fileName()) )
            {
                filesFoundInDB.remove(fi->fileName());
            }
            else if (fi->completeSuffix() == QLatin1String("digikamtempfile.tmp"))
            {
                // Ignore temp files we created ourselves

                continue;
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Adding item " << fi->fileName();
                addItem(albumID, albumRoot, album, fi->fileName());
            }
        }
        else if (fi->isDir())
        {
            scanAlbum( albumRoot, album + QLatin1Char('/') + fi->fileName() );
        }
    }

    // Removing items from the db which we did not see on disk.

    if (!filesFoundInDB.isEmpty())
    {
        QSetIterator<QString> it(filesFoundInDB);

        while (it.hasNext())
        {
            QPair<QString, int> pair(it.next(),albumID);

            if (m_filesToBeDeleted.indexOf(pair) == -1)
            {
                m_filesToBeDeleted << pair;
            }
        }
    }

    Q_EMIT finishedScanningAlbum(albumRoot, album, list.count());
}

void CollectionScanner::updateItemsWithoutDate()
{
    QStringList urls  = CoreDbAccess().db()->getAllItemURLsWithoutDate();

    Q_EMIT totalFilesToScan(urls.count());

    QString albumRoot = CoreDbAccess::albumRoot();

    {
        CoreDbTransaction transaction;

        for (QStringList::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
        {
            Q_EMIT scanningFile(*it);

            QFileInfo fi(*it);
            QString albumURL = fi.path();
            albumURL         = QDir::cleanPath(albumURL.remove(albumRoot));
            int albumID      = CoreDbAccess().db()->getAlbumForPath(albumRoot, albumURL);

            if (albumID <= 0)
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Album ID == -1: " << albumURL;
            }

            if (fi.exists())
            {
                CollectionScanner::updateItemDate(albumID, albumRoot, albumURL, fi.fileName());
            }
            else
            {
                QPair<QString, int> pair(fi.fileName(), albumID);

                if (m_filesToBeDeleted.indexOf(pair) == -1)
                {
                    m_filesToBeDeleted << pair;
                }
            }
        }
    }
}

int CollectionScanner::countItemsInFolder(const QString& directory)
{
    int items = 0;

    QDir dir(directory);

    if (!dir.exists() || !dir.isReadable())
    {
        return 0;
    }

    QFileInfoList list = dir.entryInfoList();

    items += list.count();

    QFileInfoList::const_iterator fi;

    for (fi = list.constBegin() ; fi != list.constEnd() ; ++fi)
    {
        if (
            fi->isDir()                            &&
            (fi->fileName() != QLatin1String(".")) &&
            (fi->fileName() != QLatin1String(".."))
           )
        {
            items += countItemsInFolder( fi->filePath() );
        }
    }

    return items;
}

void CollectionScanner::markDatabaseAsScanned()
{
    CoreDbAccess access;
    access.db()->setSetting("Scanned", QDateTime::currentDateTime().toString(Qt::ISODate));
}

// ------------------- Tools ------------------------

void CollectionScanner::addItem(int albumID, const QString& albumRoot,
                                const QString& album, const QString& fileName)
{
    CoreDbAccess access;
    addItem(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::addItem(Digikam::CoreDbAccess& access, int albumID,
                                const QString& albumRoot, const QString& album,
                                const QString& fileName)
{
    QString filePath = albumRoot + album + QLatin1Char('/') + fileName;

    QString     comment;
    QStringList keywords;
    QDateTime   datetime;
    int         rating;

    QScopedPointer<DMetadata> metadata(new DMetadata(filePath));

    // Try to get comments from image :
    // In first, from standard JPEG comments, or
    // In second, from EXIF comments tag, or
    // In third, from IPTC comments tag.

    comment  = metadata->getImageComment();

    // Try to get date and time from image :
    // In first, from EXIF date & time tags, or
    // In second, from IPTC date & time tags.

    datetime = metadata->getItemDateTime();

    // Try to get image rating from IPTC Urgency tag
    // else use file system time stamp.

    rating   = metadata->getItemRating();

    if (!datetime.isValid())
    {
        QFileInfo info(filePath);
        datetime = info.lastModified();
    }

    // Try to get image tags from IPTC keywords tags.

    metadata->getItemTagsPath(keywords);

    access.db()->addItem(albumID, fileName, datetime, comment, rating, keywords);
}

void CollectionScanner::updateItemDate(int albumID, const QString& albumRoot,
                                       const QString& album, const QString& fileName)
{
    CoreDbAccess access;
    updateItemDate(access, albumID, albumRoot, album, fileName);
}

void CollectionScanner::updateItemDate(Digikam::CoreDbAccess& access, int albumID,
                                       const QString& albumRoot, const QString& album,
                                       const QString& fileName)
{
    QString filePath = albumRoot + album + QLatin1Char('/') + fileName;

    QDateTime datetime;

    QScopedPointer<DMetadata> metadata(new DMetadata(filePath));

    // Trying to get date and time from image :
    // In first, from EXIF date & time tags, or
    // In second, from IPTC date & time tags.

    datetime = metadata->getItemDateTime();

    if (!datetime.isValid())
    {
        QFileInfo info(filePath);
        datetime = info.lastModified();
    }

    access.db()->setItemDate(albumID, fileName, datetime);
}

#endif // 0

} // namespace Digikam
