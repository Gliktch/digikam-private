/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fileworkeriface.h"

// Qt includes

#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "fileactionmngr_p.h"
#include "metaenginesettings.h"
#include "itemattributeswatch.h"
#include "iteminfotasksplitter.h"
#include "filereadwritelock.h"
#include "scancontroller.h"
#include "autorotator.h"
#include "faceutils.h"
#include "jpegutils.h"
#include "dimg.h"

namespace Digikam
{

void FileActionMngrFileWorker::writeOrientationToFiles(const FileActionItemInfoList& infos, int orientation)
{
    MetaEngineSettingsContainer msettings = MetaEngineSettings::instance()->settings();
    MetaEngineSettingsContainer::RotationBehaviorFlags behavior;
    behavior                              = msettings.rotationBehavior;

    QStringList failedItems;

    for (const ItemInfo& info : std::as_const(infos))
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        QString filePath              = info.filePath();
        QScopedPointer<DMetadata> metadata(new DMetadata(filePath));
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata->setItemOrientation(o);

        if (
            (behavior & MetaEngineSettingsContainer::RotateByMetadataFlag)   &&
            (msettings.metadataWritingMode == DMetadata::WRITE_TO_SIDECAR_ONLY)
           )
        {
            metadata->setMetadataWritingMode(DMetadata::WRITE_TO_SIDECAR_AND_FILE);
        }

        if (!metadata->applyChanges())
        {
            failedItems.append(info.name());
        }
        else
        {
            Q_EMIT imageDataChanged(filePath, true, true);

            QUrl url = QUrl::fromLocalFile(filePath);
            ItemAttributesWatch::instance()->fileMetadataChanged(url);
        }

        infos.writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        Q_EMIT imageChangeFailed(i18n("Failed to revise Exif orientation these files:"), failedItems);
    }

    infos.finishedWriting();
}

void FileActionMngrFileWorker::writeMetadataToFiles(const FileActionItemInfoList& infos)
{
    d->startingToWrite(infos);

    ScanController::instance()->suspendCollectionScan();

    for (const ItemInfo& info : std::as_const(infos))
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        MetadataHub hub;
        hub.load(info);

        if (MetaEngineSettings::instance()->settings().useLazySync)
        {
            hub.writeToMetadata(info, MetadataHub::WRITE_ALL);
        }
        else
        {
            ScanController::FileMetadataWrite writeScope(info);
            writeScope.changed(hub.writeToMetadata(info, MetadataHub::WRITE_ALL));
        }

        // hub emits fileMetadataChanged

        infos.writtenToOne();
    }

    ScanController::instance()->resumeCollectionScan();

    infos.finishedWriting();
}

void FileActionMngrFileWorker::writeMetadata(const FileActionItemInfoList& infos, int flags)
{
    d->startingToWrite(infos);

    ScanController::instance()->suspendCollectionScan();

    for (const ItemInfo& info : std::as_const(infos))
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        MetadataHub hub;
        hub.load(info);

        // apply to file metadata

        if (MetaEngineSettings::instance()->settings().useLazySync)
        {
            hub.writeToMetadata(info, (MetadataHub::WriteComponents)flags);
        }
        else
        {
            ScanController::FileMetadataWrite writeScope(info);
            writeScope.changed(hub.writeToMetadata(info, (MetadataHub::WriteComponents)flags));
        }

        // hub emits fileMetadataChanged

        infos.writtenToOne();
    }

    ScanController::instance()->resumeCollectionScan();

    infos.finishedWriting();
}

void FileActionMngrFileWorker::transform(const FileActionItemInfoList& infos, int action)
{
    d->startingToWrite(infos);

    QStringList failedItems;
    ScanController::instance()->suspendCollectionScan();

    for (const ItemInfo& info : std::as_const(infos))
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        if (MetaEngineRotation::RotateAuto == action)
        {
            if (!AIToolsPipeline::instance()->autoRotate(info))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "AI Tools pipeline not available, auto-rotation skipped";

                failedItems.append(info.name());
            }
            else
            {
                Q_EMIT imageDataChanged(info.filePath(), true, true);

                ItemAttributesWatch::instance()->fileMetadataChanged(info.fileUrl());
            }

            infos.writtenToOne();
        }
        else
        {
            FileWriteLocker lock(info.filePath());

            QString format                                  = info.format();
            QString filePath                                = info.filePath();
            QSize originalSize                              = info.dimensions();
            MetaEngine::ImageOrientation currentOrientation = (MetaEngine::ImageOrientation)info.orientation();
            bool isRaw                                      = info.format().startsWith(QLatin1String("RAW"));
            bool isDng                                      = (info.format() == QLatin1String("RAW-DNG"));
            bool isWritable                                 = QFileInfo(filePath).isWritable();
            bool rotateAsJpeg                               = false;
            bool rotateLossy                                = false;

            MetaEngineSettingsContainer msettings           = MetaEngineSettings::instance()->settings();
            MetaEngineSettingsContainer::RotationBehaviorFlags behavior;
            behavior                = msettings.rotationBehavior;
            bool rotateByMetadata   = (behavior & MetaEngineSettingsContainer::RotateByMetadataFlag);
            bool writeOnlyToSidecar = (msettings.metadataWritingMode == DMetadata::WRITE_TO_SIDECAR_ONLY);

            // Check if rotation by content, as desired, is feasible
            // We'll later check again if it was successful

            if (isWritable && (behavior & MetaEngineSettingsContainer::RotatingPixels))
            {
                if ((format == QLatin1String("JPG")) && JPEGUtils::isJpegImage(filePath))
                {
                    rotateAsJpeg = true;
                }

                rotateLossy       = (behavior & MetaEngineSettingsContainer::RotateByLossyRotation);
                DImg::FORMAT frmt = DImg::fileFormat(filePath);

                switch (frmt)
                {
                    case DImg::PNG:
                    case DImg::TIFF:
                    {
                        rotateLossy = true;
                        break;
                    }

                    case DImg::RAW:
                    {
                        rotateLossy = false;
                        break;
                    }

                    default:
                    {
                        // QImage and ImageMagick codecs support

                        if (format == QLatin1String("BMP"))
                        {
                            rotateLossy = true;
                        }

                        break;
                    }
                }
            }

            MetaEngineRotation matrix;
            matrix                                        *= currentOrientation;
            matrix                                        *= (MetaEngineRotation::TransformationAction)action;
            MetaEngine::ImageOrientation finalOrientation  = matrix.exifOrientation();
            bool rotatedPixels                             = false;

            if      (rotateAsJpeg)
            {
                JPEGUtils::JpegRotator rotator(filePath);
                rotator.setCurrentOrientation(currentOrientation);

                if (action == MetaEngineRotation::NoTransformation)
                {
                    rotatedPixels = rotator.autoExifTransform();
                }
                else
                {
                    rotatedPixels = rotator.exifTransform((MetaEngineRotation::TransformationAction)action);
                }

                if (!rotatedPixels)
                {
                    failedItems.append(info.name());
                }
            }
            else if (rotateLossy)
            {
                // Non-JPEG image: DImg

                DImg image;

                if (!image.load(filePath))
                {
                    failedItems.append(info.name());
                }
                else
                {
                    if (action == MetaEngineRotation::NoTransformation)
                    {
                        image.rotateAndFlip(currentOrientation);
                    }
                    else
                    {
                        image.transform(action);
                    }

                    // TODO: Atomic operation!!
                    // prepare metadata, including to reset Exif tag

                    image.prepareMetadataToSave(filePath, image.format(), true);

                    if (image.save(filePath, image.detectedFormat()))
                    {
                        rotatedPixels = true;
                    }
                    else
                    {
                        failedItems.append(info.name());
                    }
                }
            }

            MetaEngine::ImageOrientation metaOrientation = finalOrientation;

            if (rotatedPixels)
            {
                metaOrientation = MetaEngine::ORIENTATION_NORMAL;

                if (writeOnlyToSidecar)
                {
                    QScopedPointer<DMetadata> metadata(new DMetadata(filePath));
                    metadata->setMetadataWritingMode(DMetadata::WRITE_TO_SIDECAR_AND_FILE);
                    metadata->setItemOrientation(metaOrientation);
                    metadata->applyChanges();
                }
            }

            // Setting the rotation flag on Raws with embedded JPEG is a mess
            // Can apply to the RAW data, or to the embedded JPEG, or to both.

            if ((rotateByMetadata && !isRaw) || isDng)
            {
                QScopedPointer<DMetadata> metadata(new DMetadata(filePath));
                metadata->setItemOrientation(metaOrientation);
                metadata->applyChanges();
            }

            // DB rotation

            ItemInfo(info).setOrientation(metaOrientation);

            ScanController::instance()->scannedInfo(filePath,
                                                    CollectionScanner::ModifiedScan);

            // Adjust faces in the DB

            FaceUtils utils;

            bool confirmed = utils.rotateFaces(info.id(), originalSize,
                                               currentOrientation, finalOrientation);

            // Write faces to metadata when confirmed names exists

            if (confirmed)
            {
                MetadataHub hub;
                hub.load(info);

                ScanController::FileMetadataWrite writeScope(info);
                writeScope.changed(hub.writeToMetadata(info, MetadataHub::WRITE_TAGS, true));
            }
            else
            {
                // Set the newly rotated image as not processed by the face recognition pipeline

                utils.markAsScanned(info, false);
            }

            if (!failedItems.contains(info.name()))
            {
                Q_EMIT imageDataChanged(filePath, true, true);

                ItemAttributesWatch::instance()->fileMetadataChanged(info.fileUrl());
            }

            infos.writtenToOne();
        }
    }

    if (!failedItems.isEmpty())
    {
        Q_EMIT imageChangeFailed(i18n("Failed to transform these files:"), failedItems);
    }

    infos.finishedWriting();

    ScanController::instance()->resumeCollectionScan();
}

} // namespace Digikam

#include "moc_fileworkeriface.cpp"
