/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails - Database thumbnail storage
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

void ThumbnailCreator::storeInDatabase(const ThumbnailInfo& info, const ThumbnailImage& image) const
{
    ThumbsDbInfo dbInfo;

    // We rely on loadThumbsDbInfo() being called before, so we do not need to look up
    // by filepath of uniqueHash to find out if a thumb need to be replaced.

    dbInfo.id               = d->dbIdForReplacement;
    d->dbIdForReplacement   = -1;
    dbInfo.type             = DatabaseThumbnail::PGF;
    dbInfo.modificationDate = info.modificationDate;
    dbInfo.orientationHint  = image.exifOrientation;

    if      (dbInfo.type == DatabaseThumbnail::PGF)
    {
        // NOTE: see bug #233094: using PGF compression level 4 there. Do not use a value > 4,
        // else image is blurred due to down-sampling.

        if (!PGFUtils::writePGFImageData(image.qimage, dbInfo.data, 4))
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot save PGF thumb in DB";
            return;
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::WriteOnly);
        image.qimage.save(&buffer, "JPEG", 90);  // Here we will use JPEG quality = 90 to reduce artifacts.
        buffer.close();

        if (dbInfo.data.isNull())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot save JPEG thumb in DB";
            return;
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG2000)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::WriteOnly);
        image.qimage.save(&buffer, "JP2");
        buffer.close();

        if (dbInfo.data.isNull())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot save JPEG2000 thumb in DB";
            return;
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::PNG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::WriteOnly);
        image.qimage.save(&buffer, "PNG", 0);
        buffer.close();

        if (dbInfo.data.isNull())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot save PNG thumb in DB";
            return;
        }
    }

    ThumbsDbAccess access;
    BdEngineBackend::QueryState lastQueryState = BdEngineBackend::QueryState(BdEngineBackend::ConnectionError);

    while (lastQueryState == BdEngineBackend::ConnectionError)
    {
        lastQueryState = access.backend()->beginTransaction();

        if (BdEngineBackend::NoErrors != lastQueryState)
        {
            continue;
        }

        // Insert thumbnail data

        if (dbInfo.id == -1)
        {
            QVariant id;
            lastQueryState = access.db()->insertThumbnail(dbInfo, &id);

            if (BdEngineBackend::NoErrors != lastQueryState)
            {
                continue;
            }
            else
            {
                dbInfo.id = id.toInt();
            }
        }
        else
        {
            lastQueryState = access.db()->replaceThumbnail(dbInfo);

            if (BdEngineBackend::NoErrors != lastQueryState)
            {
                continue;
            }
        }

        // Insert lookup data used to locate thumbnail data

        if (!info.customIdentifier.isNull())
        {
            lastQueryState = access.db()->insertCustomIdentifier(info.customIdentifier, dbInfo.id);

            if (BdEngineBackend::NoErrors != lastQueryState)
            {
                continue;
            }
        }
        else
        {
            if (!info.uniqueHash.isNull())
            {
                lastQueryState = access.db()->insertUniqueHash(info.uniqueHash, info.fileSize, dbInfo.id);

                if (BdEngineBackend::NoErrors != lastQueryState)
                {
                    continue;
                }
            }

            if (!info.filePath.isNull())
            {
                lastQueryState = access.db()->insertFilePath(info.filePath, dbInfo.id);

                if (BdEngineBackend::NoErrors != lastQueryState)
                {
                    continue;
                }
            }
        }

        lastQueryState = access.backend()->commitTransaction();

        if (BdEngineBackend::NoErrors != lastQueryState)
        {
            continue;
        }
    }
}

ThumbsDbInfo ThumbnailCreator::loadThumbsDbInfo(const ThumbnailInfo& info) const
{
    ThumbsDbAccess access;
    ThumbsDbInfo   dbInfo;

    // Custom identifier takes precedence

    if (!info.customIdentifier.isEmpty())
    {
        dbInfo = access.db()->findByCustomIdentifier(info.customIdentifier);
    }
    else
    {
        if (!info.uniqueHash.isEmpty())
        {
            dbInfo = access.db()->findByHash(info.uniqueHash, info.fileSize);
        }

        if (dbInfo.data.isNull() && !info.filePath.isEmpty())
        {
            dbInfo = access.db()->findByFilePath(info.filePath, info.uniqueHash);
        }
    }

    // Store for use in storeInDatabase()

    d->dbIdForReplacement = dbInfo.id;

    return dbInfo;
}

bool ThumbnailCreator::isInDatabase(const ThumbnailInfo& info) const
{
    ThumbsDbInfo dbInfo = loadThumbsDbInfo(info);

    if (dbInfo.data.isNull())
    {
        return false;
    }

    // Check modification date

    if (dbInfo.modificationDate < info.modificationDate)
    {
        return false;
    }

    return true;
}

ThumbnailImage ThumbnailCreator::loadFromDatabase(const ThumbnailInfo& info) const
{
    ThumbsDbInfo dbInfo = loadThumbsDbInfo(info);
    ThumbnailImage image;

    if (dbInfo.data.isNull())
    {
        return ThumbnailImage();
    }

    // Check modification date

    if (dbInfo.modificationDate < info.modificationDate)
    {
        return ThumbnailImage();
    }

    // Read QImage from data blob

    if      (dbInfo.type == DatabaseThumbnail::PGF)
    {
        if (!PGFUtils::readPGFImageData(dbInfo.data, image.qimage))
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot load PGF thumb from DB";
            return ThumbnailImage();
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::ReadOnly);
        image.qimage.load(&buffer, "JPEG");
        buffer.close();

        if (dbInfo.data.isNull())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot load JPEG thumb from DB";
            return ThumbnailImage();
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::JPEG2000)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::ReadOnly);
        image.qimage.load(&buffer, "JP2");
        buffer.close();

        if (dbInfo.data.isNull())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot load JPEG2000 thumb from DB";
            return ThumbnailImage();
        }
    }
    else if (dbInfo.type == DatabaseThumbnail::PNG)
    {
        QBuffer buffer(&dbInfo.data);
        buffer.open(QIODevice::ReadOnly);
        image.qimage.load(&buffer, "PNG");
        buffer.close();

        if (dbInfo.data.isNull())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot load PNG thumb from DB";
            return ThumbnailImage();
        }
    }

    // Give priority to main database's rotation flag
    // NOTE: Breaks rotation of RAWs which do not contain JPEG previews

    image.exifOrientation = info.orientationHint;

    if ((image.exifOrientation == DMetadata::ORIENTATION_UNSPECIFIED) &&
        !info.filePath.isEmpty()                                      &&
        LoadSaveThread::infoProvider())
    {
        image.exifOrientation = LoadSaveThread::infoProvider()->orientationHint(info.filePath);
    }

    if (image.exifOrientation == DMetadata::ORIENTATION_UNSPECIFIED)
    {
        image.exifOrientation = dbInfo.orientationHint;
    }

    return image;
}

void ThumbnailCreator::deleteFromDatabase(const ThumbnailInfo& info) const
{
    ThumbsDbAccess access;
    BdEngineBackend::QueryState lastQueryState = BdEngineBackend::QueryState(BdEngineBackend::ConnectionError);

    while (BdEngineBackend::ConnectionError == lastQueryState)
    {
        lastQueryState = access.backend()->beginTransaction();

        if (BdEngineBackend::NoErrors != lastQueryState)
        {
            continue;
        }

        if (!info.uniqueHash.isNull())
        {
            lastQueryState = access.db()->removeByUniqueHash(info.uniqueHash, info.fileSize);

            if (BdEngineBackend::NoErrors != lastQueryState)
            {
                continue;
            }
        }

        if (!info.filePath.isNull())
        {
            lastQueryState = access.db()->removeByFilePath(info.filePath);

            if (BdEngineBackend::NoErrors != lastQueryState)
            {
                continue;
            }
        }

        lastQueryState = access.backend()->commitTransaction();

        if (BdEngineBackend::NoErrors != lastQueryState)
        {
            continue;
        }
    }
}

} // namespace Digikam
