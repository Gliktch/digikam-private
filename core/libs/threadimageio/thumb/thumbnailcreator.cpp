/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbnailcreator_p.h"

// Local includes

#include "privacysourceresolver.h"
#include "privacycachetransition.h"

namespace Digikam
{

namespace
{

QString privacyThumbnailIdentifier(const ThumbnailIdentifier& identifier)
{
    if (identifier.cacheNamespace.isEmpty())
    {
        return QString();
    }

    QString identity;

    if (identifier.id)
    {
        identity = QLatin1String("id:") + QString::number(identifier.id);
    }
    else
    {
        identity = QLatin1String("path:") + identifier.filePath;
    }
    const QString digest = PrivacySourceResolver::cacheNamespaceDigest(
                               identity + QLatin1Char('\0') + identifier.cacheNamespace +
                               QLatin1Char('\0') +
                               QString::number(identifier.sourceResolverGeneration));

    return (QLatin1String("digikam-private-thumbnail:/v1/") + digest);
}

bool mayPersistCallerSuppliedThumbnail(const QString& logicalFilePath)
{
    PrivacySourceRequest request;
    request.logicalFilePath = logicalFilePath;
    request.consumer        = PrivacySourceRequest::Thumbnail;

    return (PrivacySourceResolver::resolve(request).disposition ==
            PrivacySourceResult::NotHandled);
}

bool thumbnailSourceIsCurrent(const ThumbnailIdentifier& identifier)
{
    if (!identifier.sourceResolutionApplied)
    {
        return true;
    }

    PrivacySourceRequest request;
    request.logicalFilePath = identifier.filePath;
    request.itemReference   = identifier.id;
    request.consumer        = PrivacySourceRequest::Thumbnail;
    request.detailThumbnail = identifier.detailThumbnail;

    const PrivacySourceResult current = PrivacySourceResolver::resolve(request);
    PrivacySourceResult::Disposition expectedDisposition = PrivacySourceResult::NotHandled;

    if (identifier.sourceAccessDenied)
    {
        expectedDisposition = PrivacySourceResult::Denied;
    }
    else if (!identifier.cacheNamespace.isEmpty() ||
             !identifier.sourceFilePath.isEmpty() ||
             !identifier.sourceEncodedBytes.isEmpty())
    {
        expectedDisposition = PrivacySourceResult::Resolved;
    }

    return ((current.disposition       == expectedDisposition)                 &&
            (current.cacheNamespace    == identifier.cacheNamespace)           &&
            (current.resolverGeneration == identifier.sourceResolverGeneration) &&
            ((expectedDisposition != PrivacySourceResult::Resolved) ||
             ((current.physicalFilePath == identifier.sourceFilePath) &&
              (current.encodedBytes == identifier.sourceEncodedBytes))));
}

} // namespace

ThumbnailCreator::ThumbnailCreator(StorageMethod method)
    : d(new Private)
{
    d->thumbnailStorage = method;
    initialize();
}

ThumbnailCreator::ThumbnailCreator(int thumbnailSize, StorageMethod method)
    : d(new Private)
{
    setThumbnailSize(thumbnailSize);
    d->thumbnailStorage = method;
    initialize();
}

ThumbnailCreator::~ThumbnailCreator()
{
    delete d;
}

void ThumbnailCreator::initialize()
{
    QString alphaPath = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                               QLatin1String("thumbnail/background.png"));

    if (QFile::exists(alphaPath))
    {
        if (d->alphaImage.load(alphaPath, "PNG"))
        {
            int max = qMax(d->alphaImage.width(), d->alphaImage.height());
            int min = qMin(d->alphaImage.width(), d->alphaImage.height());

            if ((max > ThumbnailSize::MAX) || (min < 10))
            {
                d->alphaImage = QImage();
            }
        }
    }

    if (d->alphaImage.isNull())
    {
        d->alphaImage = QImage(20, 20, QImage::Format_RGB32);

        // create checkerboard image

        QPainter p(&d->alphaImage);
        p.fillRect( 0,  0, 20, 20, Qt::white);
        p.fillRect( 0, 10 ,10, 10, Qt::lightGray);
        p.fillRect(10,  0, 10, 10, Qt::lightGray);
        p.end();
    }

    if (d->thumbnailStorage == FreeDesktopStandard)
    {
        initThumbnailDirs();
    }
}

int ThumbnailCreator::Private::storageSize() const
{
    // on-disk thumbnail sizes according to freedesktop spec
    // for thumbnail db it's always max size

    double dpr = qApp->devicePixelRatio();

    if (onlyLargeThumbnails)
    {
        if ((dpr > 1.0) && (thumbnailStorage == ThumbnailDatabase))
        {
            return ThumbnailSize::getUseLargeThumbs() ? ThumbnailSize::MAX
                                                      : ThumbnailSize::HD;
        }
        else
        {
            return ThumbnailSize::maxThumbsSize();
        }
    }
    else
    {
        if ((dpr > 1.0) && (thumbnailStorage == ThumbnailDatabase))
        {
            return (thumbnailSize <= ThumbnailSize::Small) ? ThumbnailSize::Huge
                                                           : ThumbnailSize::HD;
        }
        else
        {
            return (thumbnailSize <= ThumbnailSize::Medium) ? ThumbnailSize::Medium
                                                            : ThumbnailSize::Huge;
        }
    }
}

void ThumbnailCreator::setThumbnailSize(int thumbnailSize)
{
    d->thumbnailSize = thumbnailSize;
}

void ThumbnailCreator::setExifRotate(bool rotate)
{
    d->exifRotate = rotate;
}

void ThumbnailCreator::setOnlyLargeThumbnails(bool onlyLarge)
{
    d->onlyLargeThumbnails = onlyLarge;
}

void ThumbnailCreator::setRemoveAlphaChannel(bool removeAlpha)
{
    d->removeAlphaChannel = removeAlpha;
}

void ThumbnailCreator::setLoadingProperties(DImgLoaderObserver* const observer, const DRawDecoding& settings)
{
    d->observer    = observer;
    d->rawSettings = settings;
}

void ThumbnailCreator::setThumbnailInfoProvider(ThumbnailInfoProvider* const provider)
{
    d->infoProvider = provider;
}

int ThumbnailCreator::thumbnailSize() const
{
    return d->thumbnailSize;
}

int ThumbnailCreator::storedSize() const
{
    return d->storageSize();
}

QString ThumbnailCreator::errorString() const
{
    return d->error;
}

QImage ThumbnailCreator::load(const ThumbnailIdentifier& identifier, bool onlyStorage) const
{
    return load(identifier, QRect(), false, onlyStorage);
}

QImage ThumbnailCreator::loadDetail(const ThumbnailIdentifier& identifier,
                                    const QRect& rect, bool onlyStorage) const
{
    if (!rect.isValid())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid rectangle" << rect;

        return QImage();
    }

    return load(identifier, rect, false, onlyStorage);
}

void ThumbnailCreator::pregenerate(const ThumbnailIdentifier& identifier) const
{
    load(identifier, QRect(), true);
}

void ThumbnailCreator::pregenerateDetail(const ThumbnailIdentifier& identifier, const QRect& rect) const
{
    if (!rect.isValid())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid rectangle" << rect;

        return;
    }

    load(identifier, rect, true);
}

QImage ThumbnailCreator::load(const ThumbnailIdentifier& identifier,
                              const QRect& rect, bool pregenerate, bool onlyStorage) const
{
    if (identifier.sourceAccessDenied)
    {
        return QImage();
    }

    if (d->storageSize() <= 0)
    {
        d->error = i18n("No or invalid size specified");
        qCWarning(DIGIKAM_GENERAL_LOG) << "No or invalid size specified";

        return QImage();
    }

    if (d->thumbnailStorage == ThumbnailDatabase)
    {
        d->dbIdForReplacement = -1;    // Just to prevent bugs
    }

    ThumbnailInfo info;
    ThumbnailImage image;
    const QString lockPath = identifier.effectiveFilePath();
    const bool usePersistentStorage = identifier.persistentCacheAllowed;
    PrivacyPersistentCacheWriteGuard persistentWrite(identifier.filePath,
                                                      usePersistentStorage);

    if (!persistentWrite.isAcquired())
    {
        return QImage();
    }

    {
        FileReadLocker lock(lockPath);

        // Get info about path

        info = makeThumbnailInfo(identifier, rect);

        // Load pregenerated thumbnail

        if (usePersistentStorage)
        {
            switch (d->thumbnailStorage)
            {
                case ThumbnailDatabase:
                {
                    if (pregenerate)
                    {
                        if (isInDatabase(info))
                        {
                            return QImage();
                        }

                        // Otherwise, fall through and generate
                    }
                    else
                    {
                        image = loadFromDatabase(info);
                    }

                    break;
                }

                case NoThumbnailStorage:
                case FreeDesktopStandard:
                {
                    image = loadFreedesktop(info);
                    break;
                }
            }
        }
    }

    // A pregeneration-only request must never force a handled memory-only
    // source into ThumbsDB or freedesktop storage.

    if (pregenerate && !usePersistentStorage)
    {
        return QImage();
    }

    // For images in offline collections we can stop here, they are not available on disk

    if (image.isNull() && (onlyStorage || info.filePath.isEmpty()))
    {
        return QImage();
    }

    // If pre-generated thumbnail is not available, generate

    if (image.isNull())
    {
        FileWriteLocker lock(lockPath);

        switch (d->thumbnailStorage)
        {
            case ThumbnailDatabase:
            {
                if (!usePersistentStorage)
                {
                    image = createThumbnail(info, rect);
                }
                else if (isInDatabase(info))
                {
                    image = loadFromDatabase(info);
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG)
                        << "Generating thumbnail for:" << identifier.filePath;

                    image = createThumbnail(info, rect);

                    if (!image.isNull())
                    {
                        storeInDatabase(info, image);

                        qCDebug(DIGIKAM_GENERAL_LOG)
                            << "Thumbnail stored in database for:" << identifier.filePath;
                    }
                    else
                    {
                        qCWarning(DIGIKAM_GENERAL_LOG)
                            << "Failed to generate thumbnail for:" << identifier.filePath;
                    }
                }

                break;
            }

            case NoThumbnailStorage:
            case FreeDesktopStandard:
            {
                image = createThumbnail(info, rect);

                if (!image.isNull())
                {
                    // Image is stored rotated

                    if (d->exifRotate)
                    {
                        image.qimage = exifRotate(image.qimage, image.exifOrientation);
                    }

                    if ((d->thumbnailStorage == FreeDesktopStandard) &&
                        usePersistentStorage)
                    {
                        storeFreedesktop(info, image);
                    }
                }

                break;
            }
        }
    }

    // A protect/relock/presentation transition may begin after the task's
    // pre-write freshness check. Remove the exact row/file addressed by this
    // operation before returning any pixels. The transition barrier prevents
    // a new old-generation writer, and its drain then guarantees that the
    // inventory purge runs after every such cleanup attempt has completed.

    if (usePersistentStorage && !thumbnailSourceIsCurrent(identifier))
    {
        switch (d->thumbnailStorage)
        {
            case ThumbnailDatabase:
            {
                if (!identifier.cacheNamespace.isEmpty() || !rect.isNull())
                {
                    deleteFromDatabase(info);
                }

                break;
            }

            case FreeDesktopStandard:
            {
                const QString storageIdentifier = info.customIdentifier.isNull()
                                                    ? info.filePath
                                                    : info.customIdentifier;
                deleteFromDiskFreedesktop(storageIdentifier);
                break;
            }

            default:
            {
                break;
            }
        }

        image = ThumbnailImage();
    }

    persistentWrite.release();

    if (image.isNull())
    {
        d->error = i18n("Thumbnail is null");
        qCWarning(DIGIKAM_GENERAL_LOG) << "Thumbnail is null for " << identifier.filePath;

        return image.qimage;
    }

    // If we only pregenerate, we have now created and stored in the database

    if (pregenerate)
    {
        return QImage();
    }

    // Prepare for usage in digikam

    image.qimage = image.qimage.scaled(d->thumbnailSize,
                                       d->thumbnailSize,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);

    image.qimage = handleAlphaChannel(image.qimage);

    if (d->thumbnailStorage == ThumbnailDatabase)
    {
        // Image is stored, or created, unrotated, and is now rotated for display
        // detail thumbnails are stored readily rotated

        if ((d->exifRotate && rect.isNull()) || (info.mimeType == QLatin1String("video")))
        {
            image.qimage = exifRotate(image.qimage, image.exifOrientation);
        }
    }

    if (!info.customIdentifier.isNull())
    {
        image.qimage.setText(QLatin1String("customIdentifier"), info.customIdentifier);
    }

    return image.qimage;
}

QImage ThumbnailCreator::scaleForStorage(const QImage& qimage) const
{
    if ((qimage.width() > d->storageSize()) || (qimage.height() > d->storageSize()))
    {
/*
        Cheat scaling is disabled because of quality problems - see bug #224999

        // Perform cheat scaling (https://www.qtcentre.org/threads/28415-Creating-thumbnails-efficiently)

        int cheatSize = maxSize - (3*(maxSize - d->storageSize()) / 4);
        qimage        = qimage.scaled(cheatSize, cheatSize, Qt::KeepAspectRatio, Qt::FastTransformation);
*/
        QImage scaledThumb = qimage.scaled(d->storageSize(),
                                           d->storageSize(),
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

        return scaledThumb;
    }

    return qimage;
}

QString ThumbnailCreator::identifierForDetail(const ThumbnailInfo& info, const QRect& rect)
{
    QUrl url;

    if (info.cacheNamespace.isEmpty())
    {
        url = QUrl::fromLocalFile(info.filePath);
        url.setScheme(QLatin1String("detail"));
    }
    else
    {
        url.setScheme(QLatin1String("detail"));
        url.setPath(QLatin1String("/privacy"));
    }
/*
    A scheme to support loading by database id, but this is a hack. Solve cleanly later (schema update)

    url.setPath(identifier.fileName);

    if (!identifier.uniqueHash.isNull())
    {
        url.addQueryItem("hash", identifier.uniqueHash);
        url.addQueryItem("filesize", QString::number(identifier.fileSize));
    }
    else
    {
        url.addQueryItem("path", identifier.filePath);
    }
*/
    QString r = QString::fromLatin1("%1,%2-%3x%4")
                .arg(rect.x())
                .arg(rect.y())
                .arg(rect.width())
                .arg(rect.height());

    QUrlQuery q(url);

    if (!info.cacheNamespace.isEmpty())
    {
        q.addQueryItem(QLatin1String("source"), info.customIdentifier);
    }

    q.addQueryItem(QLatin1String("rect"), r);
    url.setQuery(q);

    return url.toString();
}

ThumbnailInfo ThumbnailCreator::makeThumbnailInfo(const ThumbnailIdentifier& identifier, const QRect& rect) const
{
    ThumbnailInfo info;

    if (d->infoProvider)
    {
        info = d->infoProvider->thumbnailInfo(identifier);
    }
    else
    {
        info = fileThumbnailInfo(identifier.filePath);
    }

    info.sourceFilePath     = identifier.sourceFilePath;
    info.cacheNamespace     = identifier.cacheNamespace;
    info.sourceResolverGeneration = identifier.sourceResolverGeneration;
    info.sourceResolutionApplied = identifier.sourceResolutionApplied;
    info.sourceAccessDenied = identifier.sourceAccessDenied;
    info.persistentCacheAllowed = identifier.persistentCacheAllowed;
    info.detailThumbnail    = identifier.detailThumbnail;

    if (!identifier.cacheNamespace.isEmpty())
    {
        info.customIdentifier = privacyThumbnailIdentifier(identifier);
        info.orientationHint  = DMetadata::ORIENTATION_UNSPECIFIED;
    }

    if (identifier.sourceAccessDenied)
    {
        info.filePath.clear();
        info.isAccessible = false;
    }
    else if (!identifier.sourceFilePath.isEmpty())
    {
        const ThumbnailInfo sourceInfo = fileThumbnailInfo(identifier.sourceFilePath);

        info.filePath         = sourceInfo.filePath;
        info.fileName         = sourceInfo.fileName;
        info.isAccessible     = sourceInfo.isAccessible;
        info.mimeType         = sourceInfo.mimeType;
        info.modificationDate = sourceInfo.modificationDate;
    }
    else if (!identifier.sourceEncodedBytes.isEmpty())
    {
        info.filePath     = identifier.filePath;
        info.fileName     = QLatin1String("private-clear-thumbnail.jpg");
        info.isAccessible = true;
        info.mimeType     = QLatin1String("image");
    }

    info.sourceEncodedBytes = identifier.sourceEncodedBytes;

    if (!rect.isNull())
    {
        // Important: Pass the filled info, not the possibly half-filled identifier here because the hash is preferred for the customIdentifier!

        info.customIdentifier = identifierForDetail(info, rect);
    }

    return info;
}

void ThumbnailCreator::store(const QString& path, const QImage& i) const
{
    store(path, i, QRect());
}

void ThumbnailCreator::storeDetailThumbnail(const QString& path, const QRect& detailRect, const QImage& i) const
{
    store(path, i, detailRect);
}

void ThumbnailCreator::store(const QString& path, const QImage& i, const QRect& rect) const
{
    // Caller-supplied pixels have no proof that they came from the currently
    // selected locked source. Never place them in an ordinary persistent cache
    // for a handled privacy item; the protected derivative store owns those
    // crops instead.

    if (i.isNull() || (d->thumbnailStorage == NoThumbnailStorage))
    {
        return;
    }

    PrivacyPersistentCacheWriteGuard persistentWrite(path);

    if (!persistentWrite.isAcquired() ||
        !mayPersistCallerSuppliedThumbnail(path))
    {
        return;
    }

    QImage         qimage = scaleForStorage(i);
    ThumbnailInfo  info   = makeThumbnailInfo(ThumbnailIdentifier(path), rect);
    ThumbnailImage image;
    image.qimage          = qimage;

    if (!mayPersistCallerSuppliedThumbnail(path))
    {
        return;
    }

    switch (d->thumbnailStorage)
    {
        case ThumbnailDatabase:
        {
            // We must call isInDatabase or loadFromDatabase before storeInDatabase for d->dbIdForReplacement!

            if (!isInDatabase(info))
            {
                storeInDatabase(info, image);
            }

            break;
        }

        case FreeDesktopStandard:
        {
            storeFreedesktop(info, image);
            break;
        }

        default:
        {
            break;
        }
    }

    // Protection may have been enabled while the persistent store operation
    // was in progress. Recheck afterward and remove the just-addressed entry
    // if it is no longer known to be an ordinary unprotected item.

    if (!mayPersistCallerSuppliedThumbnail(path))
    {
        switch (d->thumbnailStorage)
        {
            case ThumbnailDatabase:
            {
                if (!rect.isNull())
                {
                    deleteFromDatabase(info);
                }

                break;
            }

            case FreeDesktopStandard:
            {
                const QString identifier = info.customIdentifier.isNull()
                                               ? info.filePath
                                               : info.customIdentifier;
                deleteFromDiskFreedesktop(identifier);
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

void ThumbnailCreator::deleteThumbnailsFromDisk(const QString& filePath) const
{
    deleteThumbnailsFromDisk(ThumbnailIdentifier(filePath));
}

bool ThumbnailCreator::deleteThumbnailsFromDisk(const ThumbnailIdentifier& identifier,
                                                const QRect& detailRect) const
{
    const ThumbnailInfo info = makeThumbnailInfo(identifier, detailRect);

    switch (d->thumbnailStorage)
    {
        case FreeDesktopStandard:
        {
            const QString storageIdentifier = info.customIdentifier.isNull()
                                                ? info.filePath
                                                : info.customIdentifier;
            return deleteFromDiskFreedesktop(storageIdentifier);
        }

        case ThumbnailDatabase:
        {
            return deleteFromDatabase(info);
        }

        case NoThumbnailStorage:
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}

} // namespace Digikam
