/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - file metadata helper.
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemscanner_p.h"

namespace Digikam
{

void ItemScanner::fillCommonContainer(qlonglong imageid, ImageCommonContainer* const container)
{
    QVariantList imagesFields;
    QVariantList imageInformationFields;

    {
        CoreDbAccess access;
        imagesFields = access.db()->getImagesFields(imageid,
                                                    DatabaseFields::Name             |
                                                    DatabaseFields::Category         |
                                                    DatabaseFields::ModificationDate |
                                                    DatabaseFields::FileSize);

        imageInformationFields = access.db()->getItemInformation(
                                                                 imageid,
                                                                 DatabaseFields::Rating           |
                                                                 DatabaseFields::CreationDate     |
                                                                 DatabaseFields::DigitizationDate |
                                                                 DatabaseFields::Orientation      |
                                                                 DatabaseFields::Width            |
                                                                 DatabaseFields::Height           |
                                                                 DatabaseFields::Format           |
                                                                 DatabaseFields::ColorDepth       |
                                                                 DatabaseFields::ColorModel
                                                                );
    }

    // We are already checking the field size in the CoreDB.

    if (!imagesFields.isEmpty())
    {
        container->fileName             = imagesFields.at(0).toString();
        container->fileModificationDate = imagesFields.at(2).toDateTime();
        container->fileSize             = imagesFields.at(3).toLongLong();
    }

    if (!imagesFields.isEmpty() && !imageInformationFields.isEmpty())
    {
        container->rating           = imageInformationFields.at(0).toInt();
        container->creationDate     = imageInformationFields.at(1).toDateTime();
        container->digitizationDate = imageInformationFields.at(2).toDateTime();
        container->orientation      = DMetadata::valueToString(imageInformationFields.at(3), MetadataInfo::Orientation);
        container->width            = imageInformationFields.at(4).toInt();
        container->height           = imageInformationFields.at(5).toInt();
        container->format           = formatToString(imageInformationFields.at(6).toString());
        container->colorDepth       = imageInformationFields.at(7).toInt();

        container->colorModel       = (imagesFields.at(1).toInt() == DatabaseItem::Video)                        ?
            DMetadata::videoColorModelToString((DMetadata::VIDEOCOLORMODEL)imageInformationFields.at(8).toInt()) :
            DImg::colorModelToString((DImg::COLORMODEL)imageInformationFields.at(8).toInt());
    }
}

QDateTime ItemScanner::creationDateFromFilesystem(const QFileInfo& info)
{
    // creation date is not what it seems on Unix

    QDateTime ctime = asDateTimeUTC(info.birthTime());
    QDateTime mtime = asDateTimeUTC(info.lastModified());

    if (ctime.isValid() && mtime.isValid())
    {
        return qMin(ctime, mtime);
    }

    if (mtime.isValid())
    {
        return mtime;
    }

    if (ctime.isValid())
    {
        return ctime;
    }

    return asDateTimeUTC(QDateTime::currentDateTime());
}

void ItemScanner::scanItemInformation()
{
    d->commit.commitItemInformation = true;

    if (
        (d->scanMode == NewScan) ||
        (d->scanMode == Rescan)  ||
        (d->scanMode == CleanScan)
       )
    {
        d->commit.imageInformationFields = DatabaseFields::ItemInformationAll;

        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate
               << MetadataInfo::DigitizationDate
               << MetadataInfo::Orientation;
        QVariantList metadataInfos = d->metadata->getMetadataFields(fields);

        checkCreationDateFromMetadata(metadataInfos[1]);

        if (!checkRatingFromMetadata(metadataInfos.at(0)))
        {
            d->commit.imageInformationFields &= ~DatabaseFields::Rating;
            metadataInfos.removeAt(0);
        }

        d->commit.imageInformationInfos = metadataInfos;
    }
    else
    {
        // Does _not_ update rating and orientation (unless dims were exchanged)!
/*
        int orientation = d->metadata->getItemOrientation();
        QVariantList data = CoreDbAccess().db()->getItemInformation(d->scanInfo.id,
                                                                       DatabaseFields::Width |
                                                                       DatabaseFields::Height |
                                                                       DatabaseFields::Orientation);
        if (data.size() == 3 && data[2].isValid() && data[2].toInt() != orientation)
        {
            // be careful not to overwrite our value set in the database
            // But there is a special case: if the dims were
        }
*/

        d->commit.imageInformationFields =
                DatabaseFields::Width      |
                DatabaseFields::Height     |
                DatabaseFields::Format     |
                DatabaseFields::ColorDepth |
                DatabaseFields::ColorModel;
    }

    QSize size = d->img.size();
    d->commit.imageInformationInfos
          << size.width()
          << size.height()
          << detectImageFormat()
          << d->img.originalBitDepth()
          << d->img.originalColorModel();
}

void ItemScanner::commitItemInformation()
{
    if (d->scanMode == NewScan)
    {
        CoreDbAccess().db()->addItemInformation(d->scanInfo.id,
                                                d->commit.imageInformationInfos,
                                                d->commit.imageInformationFields);
    }
    else // d->scanMode == Rescan or CleanScan or ModifiedScan
    {
        CoreDbAccess().db()->changeItemInformation(d->scanInfo.id,
                                                   d->commit.imageInformationInfos,
                                                   d->commit.imageInformationFields);
    }
}

void ItemScanner::prepareUpdateImage()
{
    d->commit.operation = ItemScannerCommit::UpdateItem;
}

void ItemScanner::commitUpdateImage()
{
    CoreDbAccess().db()->updateItem(d->scanInfo.id,
                                    d->scanInfo.category,
                                    d->scanInfo.modificationDate,
                                    d->scanInfo.fileSize,
                                    d->scanInfo.uniqueHash);
}

void ItemScanner::scanFile(ScanMode mode)
{
    d->scanMode = mode;

    if (d->scanMode == ModifiedScan)
    {
        if (d->scanInfo.category == DatabaseItem::Image)
        {
            scanItemInformation();
            scanImageHistoryIfModified();
        }
        else if ((d->scanInfo.category == DatabaseItem::Video) ||
                 (d->scanInfo.category == DatabaseItem::Audio))
        {
            scanVideoInformation();

            // NOTE: Here, we only scan fields which can be expected to have changed, when we detect a change of file data.
            // It seems to me that at the moment video metadata contains such fields (which may change after editing).
            // In contrast, with photos, ImageMetadata contains fields which describe the moment of taking the photo,
            // which means they don't change.

            if (d->hasMetadata)
            {
                scanVideoMetadata();
            }
        }
        else if (d->scanInfo.category == DatabaseItem::Other)
        {
            // unsupported
        }
    }
    else
    {
        if (d->scanMode == CleanScan)
        {
            cleanDatabaseMetadata();
        }

        if      (d->scanInfo.category == DatabaseItem::Image)
        {
            scanItemInformation();

            if (d->hasMetadata)
            {
                scanImageMetadata();
                scanItemPosition();
                scanItemComments();
                scanItemCopyright();
                scanIPTCCore();
                scanTags();
                scanFaces();
                scanImageHistory();
                scanBalooInfo();
            }
        }
        else if (
                 (d->scanInfo.category == DatabaseItem::Video) ||
                 (d->scanInfo.category == DatabaseItem::Audio)
                )
        {
            scanVideoInformation();

            if (d->hasMetadata)
            {
                scanVideoMetadata();
                scanImageMetadata();
                scanItemPosition();
                scanItemComments();
                scanItemCopyright();
                scanIPTCCore();
                scanTags();
            }
        }
        else if (d->scanInfo.category == DatabaseItem::Other)
        {
            // unsupported
        }
    }
}

bool ItemScanner::scanFromIdenticalFile()
{
    // Get a list of other images that are identical. Source image shall not be included.
    // When using the Commit functionality, d->scanInfo.id can be null.

    QList<ItemScanInfo> candidates = CoreDbAccess().db()->getIdenticalFiles(d->scanInfo.uniqueHash,
                                                                            d->scanInfo.fileSize,
                                                                            d->scanInfo.id);

    if (!candidates.isEmpty())
    {
        // Sort by priority, as implemented by custom lessThan()

        std::stable_sort(candidates.begin(), candidates.end(), lessThanForIdentity);
        qlonglong imageId = candidates.first().id;

        // Prefer item found in the same album

        for (const ItemScanInfo& info : std::as_const(candidates))
        {
            if (info.albumID == d->scanInfo.albumID)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Found identical item" << info.itemName
                                              << "in the same album" << info.albumID;
                imageId = info.id;
                break;
            }
        }

        qCDebug(DIGIKAM_DATABASE_LOG) << "Recognized" << d->fileInfo.filePath()
                                      << "as identical to item" << imageId;

        // Copy attributes.
        // Todo for the future is to worry about syncing identical files.

        d->commit.copyImageAttributesId = imageId;

        return true;
    }

    return false;
}

void ItemScanner::fileModified()
{
    loadFromDisk();
    prepareUpdateImage();
    scanFile(ModifiedScan);
}

} // namespace Digikam
