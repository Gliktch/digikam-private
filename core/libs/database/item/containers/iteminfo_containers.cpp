/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data - Containers
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfo_p.h"

namespace Digikam
{

ImageCommonContainer ItemInfo::imageCommonContainer() const
{
    if (!m_data)
    {
        return ImageCommonContainer();
    }

    ImageCommonContainer container;
    ItemScanner::fillCommonContainer(m_data->id, &container);

    return container;
}

ImageMetadataContainer ItemInfo::imageMetadataContainer() const
{
    if (!m_data)
    {
        return ImageMetadataContainer();
    }

    ImageMetadataContainer container;
    const DatabaseFieldsHashRaw rawVideoMetadata = getDatabaseFieldsRaw(DatabaseFields::Set(DatabaseFields::ImageMetadataAll));
    bool allFieldsNull                           = true;

    for (DatabaseFields::ImageMetadataIterator it ; !it.atEnd() ; ++it)
    {
        const QVariant fieldValue = rawVideoMetadata.value(*it);

        allFieldsNull &= fieldValue.isNull();

        if (!fieldValue.isNull())
        {
            const MetadataInfo::Field mdField = DatabaseImageMetadataFieldsToMetadataInfoField(*it);
            const QString fieldString         = DMetadata::valueToString(fieldValue, mdField);

            switch (*it)
            {
                case DatabaseFields::Make:
                {
                    container.make = fieldString;
                    break;
                }

                case DatabaseFields::Model:
                {
                    container.model = fieldString;
                    break;
                }

                case DatabaseFields::Lens:
                {
                    container.lens = fieldString;
                    break;
                }

                case DatabaseFields::Aperture:
                {
                    container.aperture = fieldString;
                    break;
                }

                case DatabaseFields::FocalLength:
                {
                    container.focalLength = fieldString;
                    break;
                }

                case DatabaseFields::FocalLength35:
                {
                    container.focalLength35 = fieldString;
                    break;
                }

                case DatabaseFields::ExposureTime:
                {
                    container.exposureTime = fieldString;
                    break;
                }

                case DatabaseFields::ExposureProgram:
                {
                    container.exposureProgram = fieldString;
                    break;
                }

                case DatabaseFields::ExposureMode:
                {
                    container.exposureMode = fieldString;
                    break;
                }

                case DatabaseFields::Sensitivity:
                {
                    container.sensitivity = fieldString;
                    break;
                }

                case DatabaseFields::FlashMode:
                {
                    container.flashMode = fieldString;
                    break;
                }

                case DatabaseFields::WhiteBalance:
                {
                    container.whiteBalance = fieldString;
                    break;
                }

                case DatabaseFields::WhiteBalanceColorTemperature:
                {
                    container.whiteBalanceColorTemperature = fieldString;
                    break;
                }

                case DatabaseFields::SubjectDistance:
                {
                    container.subjectDistance = fieldString;
                    break;
                }

                case DatabaseFields::SubjectDistanceCategory:
                {
                    container.subjectDistanceCategory = fieldString;
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }

    // store whether we have at least one valid field

    container.allFieldsNull = allFieldsNull;

    return container;
}

VideoMetadataContainer ItemInfo::videoMetadataContainer() const
{
    if (!m_data)
    {
        return VideoMetadataContainer();
    }

    VideoMetadataContainer container;
    const DatabaseFieldsHashRaw rawVideoMetadata = getDatabaseFieldsRaw(DatabaseFields::Set(DatabaseFields::VideoMetadataAll));
    bool allFieldsNull                           = true;

    for (DatabaseFields::VideoMetadataIterator it ; !it.atEnd() ; ++it)
    {
        const QVariant fieldValue = rawVideoMetadata.value(*it);

        allFieldsNull &= fieldValue.isNull();

        if (!fieldValue.isNull())
        {
            const MetadataInfo::Field mdField = DatabaseVideoMetadataFieldsToMetadataInfoField(*it);
            const QString fieldString         = DMetadata::valueToString(fieldValue, mdField);

            switch (*it)
            {
                case DatabaseFields::AspectRatio:
                {
                    container.aspectRatio = fieldString;
                    break;
                }

                case DatabaseFields::AudioBitRate:
                {
                    container.audioBitRate = fieldString;
                    break;
                }

                case DatabaseFields::AudioChannelType:
                {
                    container.audioChannelType = fieldString;
                    break;
                }

                case DatabaseFields::AudioCodec:
                {
                    container.audioCodec = fieldString;
                    break;
                }

                case DatabaseFields::Duration:
                {
                    container.duration = fieldString;
                    break;
                }

                case DatabaseFields::FrameRate:
                {
                    container.frameRate = fieldString;
                    break;
                }

                case DatabaseFields::VideoCodec:
                {
                    container.videoCodec = fieldString;
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }

    // store whether we have at least one valid field

    container.allFieldsNull = allFieldsNull;

    return container;
}

PhotoInfoContainer ItemInfo::photoInfoContainer() const
{
    if (!m_data)
    {
        return PhotoInfoContainer();
    }

    ImageMetadataContainer meta = imageMetadataContainer();
    PhotoInfoContainer photoInfo;

    photoInfo.make              = meta.make;
    photoInfo.model             = meta.model;
    photoInfo.lens              = meta.lens;
    photoInfo.exposureTime      = meta.exposureTime;
    photoInfo.exposureMode      = meta.exposureMode;
    photoInfo.exposureProgram   = meta.exposureProgram;
    photoInfo.aperture          = meta.aperture;
    photoInfo.focalLength       = meta.focalLength;
    photoInfo.focalLength35mm   = meta.focalLength35;
    photoInfo.sensitivity       = meta.sensitivity;
    photoInfo.flash             = meta.flashMode;
    photoInfo.whiteBalance      = meta.whiteBalance;
    photoInfo.dateTime          = dateTime();

    return photoInfo;
}

VideoInfoContainer ItemInfo::videoInfoContainer() const
{
    if (!m_data)
    {
        return VideoInfoContainer();
    }

    VideoMetadataContainer meta = videoMetadataContainer();
    VideoInfoContainer videoInfo;

    videoInfo.aspectRatio       = meta.aspectRatio;
    videoInfo.audioBitRate      = meta.audioBitRate;
    videoInfo.audioChannelType  = meta.audioChannelType;
    videoInfo.audioCodec        = meta.audioCodec;
    videoInfo.duration          = meta.duration;
    videoInfo.frameRate         = meta.frameRate;
    videoInfo.videoCodec        = meta.videoCodec;

    return videoInfo;
}

Template ItemInfo::metadataTemplate() const
{
    if (!m_data)
    {
        return Template();
    }

    Template t;

    ItemCopyright ic          = imageCopyright();
    ic.fillTemplate(t);

    ItemExtendedProperties ep = imageExtendedProperties();
    t.setLocationInfo(ep.location());
    t.setIptcSubjects(ep.subjectCode());

    return t;
}

void ItemInfo::setMetadataTemplate(const Template& t)
{
    if (!m_data)
    {
        return;
    }

    removeMetadataTemplate();

    ItemCopyright ic          = imageCopyright();
    ic.setFromTemplate(t);

    ItemExtendedProperties ep = imageExtendedProperties();
    ep.setLocation(t.locationInfo());
    ep.setSubjectCode(t.IptcSubjects());
}

void ItemInfo::removeMetadataTemplate()
{
    if (!m_data)
    {
        return;
    }

    ItemCopyright ic          = imageCopyright();
    ic.removeAll();

    ItemExtendedProperties ep = imageExtendedProperties();
    ep.removeLocation();
    ep.removeSubjectCode();
}

ItemComments ItemInfo::imageComments(const CoreDbAccess& access) const
{
    if (!m_data)
    {
        return ItemComments();
    }

    return ItemComments(access, m_data->id);
}

ItemCopyright ItemInfo::imageCopyright() const
{
    if (!m_data)
    {
        return ItemCopyright();
    }

    return ItemCopyright(m_data->id);
}

ItemExtendedProperties ItemInfo::imageExtendedProperties() const
{
    if (!m_data)
    {
        return ItemExtendedProperties();
    }

    return ItemExtendedProperties(m_data->id);
}

} // namespace Digikam
