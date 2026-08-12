/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database - scan operations.
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

void CollectionScanner::completeScan()
{
    QElapsedTimer timer;
    timer.start();

    Q_EMIT startCompleteScan();

    {
        // lock database

        CoreDbTransaction transaction;

        mainEntryPoint(true);
        d->resetRemovedItemsTime();
    }

    // TODO: Implement a mechanism to watch for album root changes while we keep this list

    QList<CollectionLocation> allLocations = CollectionManager::instance()->allAvailableLocations();

    // count for progress info and create album date cache

    int count = 0;

    for (const CollectionLocation& location : std::as_const(allLocations))
    {
        if (!d->checkObserver())
        {
            Q_EMIT cancelled();

            return;
        }

        // cppcheck-suppress useStlAlgorithm
        count += createAlbumDateCache(location, QLatin1String("/"));
    }

    if (d->wantSignals && d->needTotalFiles)
    {
        Q_EMIT totalFilesToScan(count);
    }

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    // clean up all stale albums

    CoreDbAccess().db()->deleteStaleAlbums();

    if (d->wantSignals)
    {
        Q_EMIT startScanningForStaleAlbums();
    }

    for (const CollectionLocation& location : std::as_const(allLocations))
    {
        if (!d->checkObserver())
        {
            Q_EMIT cancelled();

            return;
        }

        scanForStaleAlbums(location, QLatin1String("/"));
    }

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbumRoots();
    }

    for (const CollectionLocation& location : std::as_const(allLocations))
    {
        scanAlbumRoot(location);
    }

    // do not continue to clean up without a complete scan!

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (d->deferredFileScanning)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Complete scan (file scanning deferred) took:" << timer.elapsed() << "msecs.";

        Q_EMIT finishedCompleteScan();

        return;
    }

    CoreDbTransaction transaction;
    completeScanCleanupPart();

    qCDebug(DIGIKAM_DATABASE_LOG) << "Complete scan took:" << timer.elapsed() << "msecs.";
}

void CollectionScanner::finishCompleteScan(const QStringList& albumPaths)
{
    Q_EMIT startCompleteScan();

    {
        // lock database

        CoreDbTransaction transaction;

        mainEntryPoint(true);
        d->resetRemovedItemsTime();
    }

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbumRoots();
    }

    // remove subalbums from list if parent album is already contained

    QStringList sortedPaths = albumPaths;
    std::sort(sortedPaths.begin(), sortedPaths.end());
    QStringList::iterator it, it2;

    for (it = sortedPaths.begin() ; it != sortedPaths.end() ; )
    {
        // remove all following entries as long as they have the same beginning (= are subalbums)

        for (it2 = it + 1 ; it2 != sortedPaths.end() && it2->startsWith(*it) ; )
        {
            it2 = sortedPaths.erase(it2);
        }

        it = it2;
    }

    // count for progress info and create album date cache

    int count = 0;

    for (const QString& path : std::as_const(sortedPaths))
    {
        if (!d->checkObserver())
        {
            Q_EMIT cancelled();

            return;
        }

        CollectionLocation location = CollectionManager::instance()->locationForPath(path);
        QString album               = CollectionManager::instance()->album(path);

        // cppcheck-suppress useStlAlgorithm
        count += createAlbumDateCache(location, album);
    }

    if (d->wantSignals && d->needTotalFiles)
    {
        Q_EMIT totalFilesToScan(count);
    }

    for (const QString& path : std::as_const(sortedPaths))
    {
        if (!d->checkObserver())
        {
            Q_EMIT cancelled();

            return;
        }

        CollectionLocation location = CollectionManager::instance()->locationForPath(path);
        QString album               = CollectionManager::instance()->album(path);

        if (album == QLatin1String("/"))
        {
            scanAlbumRoot(location);
        }
        else
        {
            scanAlbum(location, album);
        }
    }

    // do not continue to clean up without a complete scan!

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    CoreDbTransaction transaction;
    completeScanCleanupPart();
}

void CollectionScanner::completeScanCleanupPart()
{
    completeHistoryScanning();

    updateRemovedItemsTime();

    // Items may be set to status removed, without being definitely deleted.
    // This deletion shall be done after a certain time, as checked by checkedDeleteRemoved

    if (checkDeleteRemoved())
    {
        // Mark items that are old enough and have the status trashed as obsolete
        // Only do this in a complete scan!

        CoreDbAccess access;
        QList<qlonglong> trashedItems = access.db()->getImageIds(DatabaseItem::Status::Trashed);

        for (const qlonglong& item : std::as_const(trashedItems))
        {
            access.db()->setItemStatus(item, DatabaseItem::Status::Obsolete);
        }

        resetDeleteRemovedSettings();
    }
    else
    {
        // increment the count of complete scans during which removed items were not deleted

        incrementDeleteRemovedCompleteScanCount();
    }

    markDatabaseAsScanned();

    Q_EMIT finishedCompleteScan();
}

void CollectionScanner::partialScan(const QString& filePath)
{
    QString albumRoot = CollectionManager::instance()->albumRootPath(filePath);
    QString album     = CollectionManager::instance()->album(filePath);
    partialScan(albumRoot, album);
}

void CollectionScanner::partialScan(const QString& albumRoot, const QString& album)
{
    if (albumRoot.isNull() || album.isEmpty())
    {
        // If you want to scan the album root, pass "/"

        qCWarning(DIGIKAM_DATABASE_LOG) << "partialScan(QString, QString) called with invalid values";

        return;
    }

    {
        // lock database

        CoreDbTransaction transaction;
        mainEntryPoint(false);
        d->resetRemovedItemsTime();
    }

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Did not find a CollectionLocation for album root path " << albumRoot;

        return;
    }

    // count for progress info and create album date cache

    int count = createAlbumDateCache(location, album);

    Q_EMIT partialFilesToScan(count);

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    // clean up all stale albums

    CoreDbAccess().db()->deleteStaleAlbums();

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    scanForStaleAlbums(location, album);

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (album == QLatin1String("/"))
    {
        scanAlbumRoot(location);
    }
    else
    {
        scanAlbum(location, album);
    }

    finishHistoryScanning();

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    updateRemovedItemsTime();
}

qlonglong CollectionScanner::scanFile(const QString& filePath, FileScanMode mode)
{
    QFileInfo info(filePath);
    QString dirPath   = info.path(); // strip off filename
    QString albumRoot = CollectionManager::instance()->albumRootPath(dirPath);

    if (albumRoot.isNull())
    {
        return -1;
    }

    QString album = CollectionManager::instance()->album(dirPath);

    return scanFile(albumRoot, album, info.fileName(), mode);
}

qlonglong CollectionScanner::scanFile(const QString& albumRoot, const QString& album,
                                      const QString& fileName, FileScanMode mode)
{
    if (album.isEmpty() || fileName.isEmpty())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "scanFile(QString, QString, QString) called with empty album or empty filename";

        return -1;
    }

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Did not find a CollectionLocation for album root path " << albumRoot;

        return -1;
    }

    QDir dir(location.albumRootPath() + album);
    QFileInfo fi(dir, fileName);

    if (!fi.exists())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "File given to scan does not exist" << albumRoot << album << fileName;

        return -1;
    }

    int albumId       = checkAlbum(location, album);
    qlonglong imageId = CoreDbAccess().db()->getImageId(albumId, fileName);
    imageId           = scanFile(fi, albumId, imageId, mode);

    return imageId;
}

void CollectionScanner::scanFile(const ItemInfo& info, FileScanMode mode)
{
    if (info.isNull() || !info.isLocationAvailable())
    {
        return;
    }

    QFileInfo fi(info.filePath());
    scanFile(fi, info.albumId(), info.id(), mode);
}

qlonglong CollectionScanner::scanFile(const QFileInfo& fi, int albumId, qlonglong imageId, FileScanMode mode)
{
    mainEntryPoint(false);

    if (!d->nameFilters.contains(fi.suffix().toLower()))
    {
        return -1;
    }

    if (imageId == -1)
    {
        switch (mode)
        {
            case NormalScan:
            case ModifiedScan:
            {
                imageId = scanNewFile(fi, albumId);
                break;
            }

            case Rescan:
            case CleanScan:
            {
                imageId = scanNewFileFullScan(fi, albumId);
                break;
            }
        }
    }
    else
    {
        ItemScanInfo scanInfo = CoreDbAccess().db()->getItemScanInfo(imageId);

        switch (mode)
        {
            case NormalScan:
            {
                scanFileNormal(fi, scanInfo);
                break;
            }

            case ModifiedScan:
            {
                scanModifiedFile(fi, scanInfo);
                break;
            }

            case Rescan:
            {
                rescanFile(fi, scanInfo);
                break;
            }

            case CleanScan:
            {
                cleanScanFile(fi, scanInfo);
                break;
            }
        }
    }

    finishHistoryScanning();

    return imageId;
}

void CollectionScanner::scanForStaleAlbums(const CollectionLocation& location, const QString& album)
{
    // Only handle albums on available locations

    if (!location.isAvailable())
    {
        return;
    }

    QString relativePath = album;

    if (album != QLatin1String("/"))
    {
        relativePath += QLatin1Char('/');
    }

    QList<AlbumShortInfo> albumList = CoreDbAccess().db()->getAlbumShortInfos();
    QList<AlbumShortInfo>::const_iterator it;
    QList<int> toBeDeleted;

    for (it = albumList.constBegin() ; it != albumList.constEnd() ; ++it)
    {
        if ((location.id() != (*it).albumRootId) || toBeDeleted.contains((*it).id))
        {
            continue;
        }

        if ((*it).relativePath.startsWith(relativePath) || ((*it).relativePath == album))
        {
            QDir dir(location.albumRootPath() + (*it).relativePath);
            QString path = QFileInfo(dir.path()).filePath();

            // Delete entries in the database if they do not exist in the cache.

            if (!d->albumDateCache.contains(path))
            {
                // We have an ignored album, all sub-albums have to be ignored

                QList<int> subAlbums = CoreDbAccess().db()->getAlbumAndSubalbumsForPath((*it).albumRootId,
                                                                                        (*it).relativePath);
                toBeDeleted      << subAlbums;
                d->scannedAlbums << subAlbums;
            }
        }
    }

    safelyRemoveAlbums(toBeDeleted);

    if (d->wantSignals)
    {
        Q_EMIT finishedScanningForStaleAlbums();
    }
}

void CollectionScanner::scanAlbumRoot(const CollectionLocation& location)
{
    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbumRoot(location.albumRootPath());
    }

    bool useFastScan = MetaEngineSettings::instance()->settings().useFastScan;

    if (useFastScan && d->performFastScan)
    {
        const QMap<QString, QDateTime>& pathDateMap = CoreDbAccess().db()->
                                        getAlbumModificationMap(location.id());

        if (!pathDateMap.isEmpty())
        {
            QMap<QString, QDateTime>::const_iterator it;

            for (it = pathDateMap.constBegin() ; it != pathDateMap.constEnd() ; ++it)
            {
                if (!d->checkObserver())
                {
                    return;
                }

                QDir dir(location.albumRootPath() + it.key());
                QString path = QFileInfo(dir.path()).filePath();

                if (s_modificationDateEquals(d->albumDateCache.value(path), it.value()))
                {
                    int albumID = CoreDbAccess().db()->getAlbumForPath(location.id(), it.key(), false);
                    int counter = CoreDbAccess().db()->getNumberOfItemsInAlbum(albumID);

                    d->scannedAlbums << albumID;

                    if (d->wantSignals)
                    {
                        Q_EMIT scannedFiles(counter + 1);
                    }
                }
                else
                {
                    scanAlbum(location, it.key(), true);
                }
            }

            if (d->wantSignals)
            {
                Q_EMIT finishedScanningAlbumRoot(location.albumRootPath());
            }

            return;
        }
    }

    scanAlbum(location, QLatin1String("/"));

    if (d->wantSignals)
    {
        Q_EMIT finishedScanningAlbumRoot(location.albumRootPath());
    }
}

void CollectionScanner::scanAlbum(const CollectionLocation& location, const QString& album, bool checkDate)
{
    // + Adds album if it does not yet exist in the db.
    // + Recursively scans subalbums of album.
    // + Adds files if they do not yet exist in the db.
    // + Marks stale files as removed

    if (!d->checkObserver())
    {
        return;
    }

    QDir dir(location.albumRootPath() + album);

    if (!dir.exists() || !dir.isReadable())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Folder does not exist or is not readable: "
                                        << dir.path();
        return;
    }

    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbum(location.albumRootPath(), album);
    }

    int albumID             = checkAlbum(location, album);
    QDateTime albumDateTime = asDateTimeUTC(QFileInfo(dir.path()).lastModified());
    QDateTime albumModified = CoreDbAccess().db()->getAlbumModificationDate(albumID);

    if (checkDate && s_modificationDateEquals(albumDateTime, albumModified))
    {
        // mark album as scanned

        d->scannedAlbums << albumID;

        if (d->wantSignals)
        {
            Q_EMIT finishedScanningAlbum(location.albumRootPath(), album, 1);
        }

        return;
    }

    const QList<ItemScanInfo>& scanInfos = CoreDbAccess().db()->getItemScanInfos(albumID);
    MetaEngineSettingsContainer settings = MetaEngineSettings::instance()->settings();
    QHash<QString, int> fileNameIndexHash;
    QHash<QString, QFileInfo> nameInfos;
    QSet<qlonglong> itemIdSet;

    // create a QHash filename -> index in list

    for (int i = 0 ; i < scanInfos.size() ; ++i)
    {
        fileNameIndexHash[scanInfos.at(i).itemName] = i;
        itemIdSet << scanInfos.at(i).id;
    }

    const QFileInfoList& list = dir.entryInfoList(QDir::Dirs    |
                                                  QDir::Files   |
                                                  QDir::NoDotAndDotDot,
                                                  QDir::Name | QDir::DirsLast);

    for (const QFileInfo& info : std::as_const(list))
    {
        nameInfos.insert(info.fileName(), info);
    }

    int counter          = 0;
    bool updateAlbumDate = false;
    QDate albumDateOld   = albumDateTime.date();
    QDate albumDateNew   = albumDateTime.date();
    const QString xmpExt(QLatin1String(".xmp"));

    for (const QFileInfo& info : std::as_const(list))
    {
        if (!d->checkObserver())
        {
            return; // return directly, do not go to cleanup code after loop!
        }

        if (info.isFile())
        {
            // filter with name filter

            if (!d->nameFilters.contains(info.suffix().toLower()))
            {
                continue;
            }

            ++counter;

            if (d->wantSignals && counter && (counter % 100 == 0))
            {
                Q_EMIT scannedFiles(counter);

                counter = 0;
            }

            int index = fileNameIndexHash.value(info.fileName(), -1);

            if      (index != -1)
            {
                // mark item as "seen"

                itemIdSet.remove(scanInfos.at(index).id);

                bool hasSidecar        = false;
                const QFileInfo* sinfo = nullptr;

                if (settings.useXMPSidecar4Reading)
                {
                    QString sidecarName;

                    if (!settings.useCompatibleFileName)
                    {
                        sidecarName = info.fileName() + xmpExt;
                    }
                    else
                    {
                        sidecarName = info.completeBaseName() + xmpExt;
                    }

                    if (nameInfos.contains(sidecarName))
                    {
                        sinfo      = &nameInfos[sidecarName];
                        hasSidecar = true;
                    }
                }

                scanFileNormal(info, scanInfos.at(index), hasSidecar, sinfo);
            }
            else if (info.completeSuffix().contains(QLatin1String("digikamtempfile.")))
            {
                // ignore temp files we created ourselves

                continue;
            }
            else
            {
                // Read the creation date of each image to determine the oldest one

                qlonglong imageId = scanNewFile(info, albumID);

                if (imageId > 0)
                {
                    ItemInfo itemInfo(imageId);
                    QDate itemDate = itemInfo.dateTime().date();

                    if (itemDate.isValid())
                    {
                        if (
                            (settings.albumDateFrom == MetaEngineSettingsContainer::NewestItemDate) ||
                            (settings.albumDateFrom == MetaEngineSettingsContainer::AverageDate)
                           )
                        {
                            // Change album date only if the item date is newer.

                            if (itemDate > albumDateNew)
                            {
                                albumDateNew    = itemDate;
                                updateAlbumDate = true;
                            }
                        }

                        if (
                            (settings.albumDateFrom == MetaEngineSettingsContainer::OldestItemDate) ||
                            (settings.albumDateFrom == MetaEngineSettingsContainer::AverageDate)
                           )
                        {
                            // Change album date only if the item date is older.

                            if (itemDate < albumDateOld)
                            {
                                albumDateOld    = itemDate;
                                updateAlbumDate = true;
                            }
                        }
                    }
                }

                // Emit signals for scanned files with much higher granularity

                if (d->wantSignals && counter && (counter % 2 == 0))
                {
                    Q_EMIT scannedFiles(counter);

                    counter = 0;
                }
            }
        }
        else if (info.isDir())
        {

#ifdef Q_OS_WIN

            // Hide album that starts with a dot, as under Linux.

            if (info.fileName().startsWith(QLatin1Char('.')))
            {
                continue;
            }

#endif

            if (d->checkIgnoreDirectory(info.fileName()))
            {
                continue;
            }

            ++counter;

            QString subAlbum = album;

            if (subAlbum != QLatin1String("/"))
            {
                subAlbum += QLatin1Char('/');
            }

            scanAlbum(location, subAlbum + info.fileName(), checkDate);
        }
    }

    if (!d->deferredFileScanning && !s_modificationDateEquals(albumDateTime, albumModified))
    {
        CoreDbAccess().db()->setAlbumModificationDate(albumID, albumDateTime);
    }

    if (updateAlbumDate)
    {
        // Write the new album date from the image information

        if      (settings.albumDateFrom == MetaEngineSettingsContainer::OldestItemDate)
        {
            CoreDbAccess().db()->setAlbumDate(albumID, albumDateOld);
        }
        else if (settings.albumDateFrom == MetaEngineSettingsContainer::NewestItemDate)
        {
            CoreDbAccess().db()->setAlbumDate(albumID, albumDateNew);
        }
        else if (settings.albumDateFrom == MetaEngineSettingsContainer::AverageDate)
        {
            qint64 julianDayCount = albumDateOld.toJulianDay();
            julianDayCount       += albumDateNew.toJulianDay();

            CoreDbAccess().db()->setAlbumDate(albumID, QDate::fromJulianDay(julianDayCount / 2));
        }
        else if (settings.albumDateFrom == MetaEngineSettingsContainer::FolderDate)
        {
            CoreDbAccess().db()->setAlbumDate(albumID, albumDateTime.date());
        }
    }

    if (d->wantSignals && counter)
    {
        Q_EMIT scannedFiles(counter);
    }

    // Mark items in the db which we did not see on disk.

    if (!itemIdSet.isEmpty())
    {
        QList<qlonglong> ids = itemIdSet.values();
        CoreDbOperationGroup group;
        CoreDbAccess().db()->removeItems(ids, QList<int>() << albumID);
        itemsWereRemoved(ids);
    }

    // mark album as scanned

    d->scannedAlbums << albumID;

    if (d->wantSignals)
    {
        Q_EMIT finishedScanningAlbum(location.albumRootPath(), album, list.count());
    }
}

void CollectionScanner::scanFileNormal(const QFileInfo& fi, const ItemScanInfo& scanInfo,
                                       bool checkSidecar, const QFileInfo* const sidecarInfo)
{
    bool hasMetadataHint       = (d->hints && d->hints->hasMetadataHint(scanInfo.id));
    QDateTime modificationDate = asDateTimeUTC(fi.lastModified());

    if (!s_modificationDateEquals(modificationDate, scanInfo.modificationDate))
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Scanner debug:" << modificationDate
                                                          << scanInfo.modificationDate;
    }

    // if the date is null, this signals a full rescan

    if      (scanInfo.modificationDate.isNull())
    {
        rescanFile(fi, scanInfo);

        return;
    }
    else if (hasMetadataHint) // metadata adjustment hints
    {
        if (d->hints->hasMetadataAboutToAdjustHint(scanInfo.id))
        {
            // postpone scan

            return;
        }
        else // hasMetadataAdjustedHint
        {
            {
                QWriteLocker locker(&d->hints->lock);
                d->hints->metadataAdjustedHints.remove(scanInfo.id);
            }

            scanFileUpdateHashReuseThumbnail(fi, scanInfo, true);

            return;
        }
    }
    else if (d->updatingHashHint)
    {
        // if the file need not be scanned because of modification, update the hash

        if (
            s_modificationDateEquals(modificationDate, scanInfo.modificationDate) &&
            (fi.size() == scanInfo.fileSize)
           )
        {
            scanFileUpdateHashReuseThumbnail(fi, scanInfo, false);

            return;
        }
    }

    MetaEngineSettingsContainer settings = MetaEngineSettings::instance()->settings();

    if (checkSidecar && settings.useXMPSidecar4Reading)
    {
        if      (sidecarInfo)
        {
            QDateTime sidecarDate = asDateTimeUTC(sidecarInfo->lastModified());

            if (sidecarDate > modificationDate)
            {
                modificationDate = sidecarDate;
            }
        }
        else if (DMetadata::hasSidecar(fi.filePath()))
        {
            QString filePath      = DMetadata::sidecarPath(fi.filePath());
            QDateTime sidecarDate = asDateTimeUTC(QFileInfo(filePath).lastModified());

            if (sidecarDate > modificationDate)
            {
                modificationDate = sidecarDate;
            }
        }
    }

    if (
        !s_modificationDateEquals(modificationDate, scanInfo.modificationDate) ||
        (fi.size() != scanInfo.fileSize)
       )
    {
        if (settings.rescanImageIfModified)
        {
            cleanScanFile(fi, scanInfo);
        }
        else
        {
            scanModifiedFile(fi, scanInfo);
        }
    }
}

qlonglong CollectionScanner::scanNewFile(const QFileInfo& info, int albumId)
{
    if (d->checkDeferred(info))
    {
        return -1;
    }

    ItemScanner scanner(info);
    scanner.setCategory(category(info));

    scanner.newFile(albumId);
    d->finishScanner(scanner);

    d->newIdsList << scanner.id();

    return scanner.id();
}

qlonglong CollectionScanner::scanNewFileFullScan(const QFileInfo& info, int albumId)
{
    if (d->checkDeferred(info))
    {
        return -1;
    }

    ItemScanner scanner(info);
    scanner.setCategory(category(info));

    scanner.newFileFullScan(albumId);
    d->finishScanner(scanner);

    return scanner.id();
}

void CollectionScanner::scanModifiedFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));

    scanner.fileModified();
    d->finishScanner(scanner);
}

void CollectionScanner::scanFileUpdateHashReuseThumbnail(const QFileInfo& info, const ItemScanInfo& scanInfo,
                                                         bool fileWasEdited)
{
    QString oldHash   = scanInfo.uniqueHash;
    qlonglong oldSize = scanInfo.fileSize;

    // same code as scanModifiedFile

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));

    scanner.fileModified();

    QString newHash   = scanner.itemScanInfo().uniqueHash;
    qlonglong newSize = scanner.itemScanInfo().fileSize;

    if (ThumbsDbAccess::isInitialized())
    {
        if (fileWasEdited)
        {
            // The file was edited in such a way that we know that the pixel content did not change, so we can reuse the thumbnail.
            // We need to add a link to the thumbnail data with the new hash/file size _and_ adjust
            // the file modification date in the data table.

            ThumbsDbInfo thumbDbInfo = ThumbsDbAccess().db()->findByHash(oldHash, oldSize);

            if (thumbDbInfo.id != -1)
            {
                ThumbsDbAccess().db()->insertUniqueHash(newHash, newSize, thumbDbInfo.id);
                ThumbsDbAccess().db()->updateModificationDate(thumbDbInfo.id, scanner.itemScanInfo().modificationDate);

                // TODO: also update details thumbnails (by file path and URL scheme)
            }
        }
        else
        {
            ThumbsDbAccess().db()->replaceUniqueHash(oldHash, oldSize, newHash, newSize);
        }
    }

    d->finishScanner(scanner);
}

void CollectionScanner::cleanScanFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));

    scanner.cleanScan();
    d->finishScanner(scanner);
}

void CollectionScanner::rescanFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.rescan();
    d->finishScanner(scanner);
}

void CollectionScanner::completeHistoryScanning()
{
    // scan tagged images

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    QList<qlonglong> ids = CoreDbAccess().db()->getItemIDsInTag(needResolvingTag);
    historyScanningStage2(ids);

    ids                  = CoreDbAccess().db()->getItemIDsInTag(needTaggingTag);
    qCDebug(DIGIKAM_DATABASE_LOG) << "items to tag" << ids;
    historyScanningStage3(ids);
}

void CollectionScanner::finishHistoryScanning()
{
    // scan recorded ids

    QList<qlonglong> ids;

    // stage 2

    ids = d->needResolveHistorySet.values();
    d->needResolveHistorySet.clear();
    historyScanningStage2(ids);

    if (!d->checkObserver())
    {
        return;
    }

    // stage 3

    ids = d->needTaggingHistorySet.values();
    d->needTaggingHistorySet.clear();
    historyScanningStage3(ids);
}

void CollectionScanner::historyScanningStage2(const QList<qlonglong>& ids)
{
    for (const qlonglong& id : std::as_const(ids))
    {
        if (!d->checkObserver())
        {
            return;
        }

        CoreDbOperationGroup group;

        if (d->recordHistoryIds)
        {
            QList<qlonglong> needTaggingIds;
            ItemScanner::resolveImageHistory(id, &needTaggingIds);

            for (const qlonglong& needTag : std::as_const(needTaggingIds))
            {
                d->needTaggingHistorySet << needTag;
            }
        }
        else
        {
            ItemScanner::resolveImageHistory(id);
        }
    }
}

void CollectionScanner::historyScanningStage3(const QList<qlonglong>& ids)
{
    for (const qlonglong& id : std::as_const(ids))
    {
        if (!d->checkObserver())
        {
            return;
        }

        CoreDbOperationGroup group;
        ItemScanner::tagItemHistoryGraph(id);
    }
}

bool CollectionScanner::databaseInitialScanDone()
{
    CoreDbAccess access;

    return (!access.db()->getSetting(QLatin1String("Scanned")).isEmpty());
}

} // namespace Digikam
