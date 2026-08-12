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

namespace Digikam
{

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

    {
        FileReadLocker lock(identifier.filePath);

        // Get info about path

        info = makeThumbnailInfo(identifier, rect);

        // Load pregenerated thumbnail

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

    // For images in offline collections we can stop here, they are not available on disk

    if (image.isNull() && (onlyStorage || info.filePath.isEmpty()))
    {
        return QImage();
    }

    // If pre-generated thumbnail is not available, generate

    if (image.isNull())
    {
        FileWriteLocker lock(identifier.filePath);

        switch (d->thumbnailStorage)
        {
            case ThumbnailDatabase:
            {
                if (isInDatabase(info))
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

                    if (d->thumbnailStorage == FreeDesktopStandard)
                    {
                        storeFreedesktop(info, image);
                    }
                }

                break;
            }
        }
    }

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
    QUrl url = QUrl::fromLocalFile(info.filePath);
    url.setScheme(QLatin1String("detail"));
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
    if (i.isNull() || (d->thumbnailStorage == NoThumbnailStorage))
    {
        return;
    }

    QImage         qimage = scaleForStorage(i);
    ThumbnailInfo  info   = makeThumbnailInfo(ThumbnailIdentifier(path), rect);
    ThumbnailImage image;
    image.qimage          = qimage;

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
}

void ThumbnailCreator::deleteThumbnailsFromDisk(const QString& filePath) const
{
    switch (d->thumbnailStorage)
    {
        case FreeDesktopStandard:
        {
            deleteFromDiskFreedesktop(filePath);
            break;
        }

        case ThumbnailDatabase:
        {
            ThumbnailInfo info;

            if (d->infoProvider)
            {
                info = d->infoProvider->thumbnailInfo(ThumbnailIdentifier(filePath));
            }
            else
            {
                info = fileThumbnailInfo(filePath);
            }

            deleteFromDatabase(info);
            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace Digikam
