/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - photo metadata helper.
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

QString ItemScanner::iptcCorePropertyName(MetadataInfo::Field field)
{
    // These strings are specified in DBSCHEMA.ods

    switch (field)
    {
        // Copyright table                                     krazy:exclude=copyright

        case MetadataInfo::IptcCoreCopyrightNotice:
        {
            return QLatin1String("copyrightNotice");        // krazy:exclude=copyright
        }

        case MetadataInfo::IptcCoreCreator:
        {
            return QLatin1String("creator");
        }

        case MetadataInfo::IptcCoreProvider:
        {
            return QLatin1String("provider");
        }

        case MetadataInfo::IptcCoreRightsUsageTerms:
        {
            return QLatin1String("rightsUsageTerms");
        }

        case MetadataInfo::IptcCoreSource:
        {
            return QLatin1String("source");
        }

        case MetadataInfo::IptcCoreCreatorJobTitle:
        {
            return QLatin1String("creatorJobTitle");
        }

        case MetadataInfo::IptcCoreInstructions:
        {
            return QLatin1String("instructions");
        }

        // ImageProperties table

        case MetadataInfo::IptcCoreCountryCode:
        {
            return QLatin1String("countryCode");
        }

        case MetadataInfo::IptcCoreCountry:
        {
            return QLatin1String("country");
        }

        case MetadataInfo::IptcCoreCity:
        {
            return QLatin1String("city");
        }

        case MetadataInfo::IptcCoreLocation:
        {
            return QLatin1String("location");
        }

        case MetadataInfo::IptcCoreProvinceState:
        {
            return QLatin1String("provinceState");
        }

        case MetadataInfo::IptcCoreIntellectualGenre:
        {
            return QLatin1String("intellectualGenre");
        }

        case MetadataInfo::IptcCoreJobID:
        {
            return QLatin1String("jobId");
        }

        case MetadataInfo::IptcCoreScene:
        {
            return QLatin1String("scene");
        }

        case MetadataInfo::IptcCoreSubjectCode:
        {
            return QLatin1String("subjectCode");
        }

        case MetadataInfo::IptcCoreContactInfoCity:
        {
            return QLatin1String("creatorContactInfo.city");
        }

        case MetadataInfo::IptcCoreContactInfoCountry:
        {
            return QLatin1String("creatorContactInfo.country");
        }

        case MetadataInfo::IptcCoreContactInfoAddress:
        {
            return QLatin1String("creatorContactInfo.address");
        }

        case MetadataInfo::IptcCoreContactInfoPostalCode:
        {
            return QLatin1String("creatorContactInfo.postalCode");
        }

        case MetadataInfo::IptcCoreContactInfoProvinceState:
        {
            return QLatin1String("creatorContactInfo.provinceState");
        }

        case MetadataInfo::IptcCoreContactInfoEmail:
        {
            return QLatin1String("creatorContactInfo.email");
        }

        case MetadataInfo::IptcCoreContactInfoPhone:
        {
            return QLatin1String("creatorContactInfo.phone");
        }

        case MetadataInfo::IptcCoreContactInfoWebUrl:
        {
            return QLatin1String("creatorContactInfo.webUrl");
        }

        default:
        {
            return QString();
        }
    }
}

QString ItemScanner::detectImageFormat() const
{
    DImg::FORMAT dimgFormat = d->img.detectedFormat();

    switch (dimgFormat)
    {
        case DImg::JPEG:
        {
            return QLatin1String("JPG");
        }

        case DImg::PNG:
        {
            return QLatin1String("PNG");
        }

        case DImg::TIFF:
        {
            return QLatin1String("TIFF");
        }

        case DImg::JP2K:
        {
            return QLatin1String("JP2");
        }

        case DImg::PGF:
        {
            return QLatin1String("PGF");
        }

        case DImg::HEIF:
        {
            return QLatin1String("HEIF");
        }

        case DImg::RAW:
        {
            QString format = QLatin1String("RAW-");
            format += d->fileInfo.suffix().toUpper();
            return format;
        }
        case DImg::NONE:
        case DImg::QIMAGE:
        {
            QString ext = d->fileInfo.suffix().toUpper();

            if (
                (ext == QLatin1String("AVIF")) ||       // See bug #109060
                (ext == QLatin1String("JPX"))
               )
            {
                return ext;
            }

            QByteArray format = QImageReader::imageFormat(d->fileInfo.filePath());

            if (!format.isEmpty())
            {
                return QString::fromUtf8(format).toUpper();
            }

            break;
        }
    }

    // See BUG #339341: Take file name suffix instead type mime analyze.

    return d->fileInfo.suffix().toUpper();
}

void ItemScanner::scanImageMetadata()
{
    QVariantList metadataInfos = d->metadata->getMetadataFields(allImageMetadataFields());

    if (hasValidField(metadataInfos))
    {
        d->commit.commitImageMetadata = true;
        d->commit.imageMetadataInfos  = metadataInfos;
    }
}

void ItemScanner::commitImageMetadata()
{
    CoreDbAccess().db()->addImageMetadata(d->scanInfo.id, d->commit.imageMetadataInfos);
}

void ItemScanner::scanItemPosition()
{
    // This list must reflect the order required by CoreDB::addItemPosition

    MetadataFields fields;
    fields << MetadataInfo::Latitude
           << MetadataInfo::LatitudeNumber
           << MetadataInfo::Longitude
           << MetadataInfo::LongitudeNumber
           << MetadataInfo::Altitude
           << MetadataInfo::PositionOrientation
           << MetadataInfo::PositionTilt
           << MetadataInfo::PositionRoll
           << MetadataInfo::PositionAccuracy
           << MetadataInfo::PositionDescription;

    QVariantList metadataInfos = d->metadata->getMetadataFields(fields);

    if (hasValidField(metadataInfos))
    {
        d->commit.commitItemPosition = true;
        d->commit.imagePositionInfos = metadataInfos;
    }
}

void ItemScanner::commitItemPosition()
{
    CoreDbAccess().db()->addItemPosition(d->scanInfo.id, d->commit.imagePositionInfos);
}

void ItemScanner::scanItemComments()
{
    MetadataFields fields;
    fields << MetadataInfo::Headline;

    QVariantList metadataInfos = d->metadata->getMetadataFields(fields);

    // handles all possible fields, multi-language, author, date

    CaptionsMap captions = d->metadata->getItemComments();
    CaptionsMap titles   = d->metadata->getItemTitles();

    if (
        titles.isEmpty()            &&
        captions.isEmpty()          &&
        !hasValidField(metadataInfos)
       )
    {
        return;
    }

    d->commit.commitItemComments = true;
    d->commit.captions           = captions;
    d->commit.titles             = titles;

    // Headline

    if (!metadataInfos.at(0).isNull())
    {
        d->commit.headline = metadataInfos.at(0).toString();
    }
}

void ItemScanner::commitItemComments()
{
    CoreDbAccess access;
    ItemComments comments(access, d->scanInfo.id);

    // Description

    if (!d->commit.captions.isEmpty())
    {
        CaptionsMap::const_iterator it;

        for (it = d->commit.captions.constBegin() ; it != d->commit.captions.constEnd() ; ++it)
        {
            CaptionValues val = it.value();
            comments.addComment(val.caption, it.key(), val.author, val.date);
        }
    }

    // Headline

    if (!d->commit.headline.isNull())
    {
        comments.addHeadline(d->commit.headline);
    }

    // Title

    if (!d->commit.titles.isEmpty())
    {
        CaptionsMap::const_iterator it;

        for (it = d->commit.titles.constBegin() ; it != d->commit.titles.constEnd() ; ++it)
        {
            CaptionValues val = it.value();
            comments.addTitle(val.caption, it.key(), val.author, val.date);
        }
    }
}

void ItemScanner::scanItemCopyright()
{
    Template t;

    if (!d->metadata->getCopyrightInformation(t))
    {
        return;
    }

    d->commit.commitItemCopyright = true;
    d->commit.copyrightTemplate   = t;
}

void ItemScanner::commitItemCopyright()
{
    ItemCopyright copyright(d->scanInfo.id);

    // It is not clear if removeAll() should be called if d->scanMode == Rescan

    copyright.removeAll();
    copyright.setFromTemplate(d->commit.copyrightTemplate);
}

void ItemScanner::scanIPTCCore()
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreLocationInfo
           << MetadataInfo::IptcCoreIntellectualGenre
           << MetadataInfo::IptcCoreJobID
           << MetadataInfo::IptcCoreScene
           << MetadataInfo::IptcCoreSubjectCode;

    QVariantList metadataInfos = d->metadata->getMetadataFields(fields);

    if (!hasValidField(metadataInfos))
    {
        return;
    }

    d->commit.commitIPTCCore        = true;
    d->commit.iptcCoreMetadataInfos = metadataInfos;
}

void ItemScanner::commitIPTCCore()
{
    ItemExtendedProperties props(d->scanInfo.id);

    if (!d->commit.iptcCoreMetadataInfos.at(0).isNull())
    {
        IptcCoreLocationInfo loc = d->commit.iptcCoreMetadataInfos.at(0).value<IptcCoreLocationInfo>();

        if (!loc.isNull())
        {
            props.setLocation(loc);
        }
    }

    if (!d->commit.iptcCoreMetadataInfos.at(1).isNull())
    {
        props.setIntellectualGenre(d->commit.iptcCoreMetadataInfos.at(1).toString());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(2).isNull())
    {
        props.setJobId(d->commit.iptcCoreMetadataInfos.at(2).toString());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(3).isNull())
    {
        props.setScene(d->commit.iptcCoreMetadataInfos.at(3).toStringList());
    }

    if (!d->commit.iptcCoreMetadataInfos.at(4).isNull())
    {
        props.setSubjectCode(d->commit.iptcCoreMetadataInfos.at(4).toStringList());
    }
}

void ItemScanner::scanTags()
{
    // Check Keywords tag paths.

    QVariant var         = d->metadata->getMetadataField(MetadataInfo::Keywords);
    QStringList keywords = var.toStringList();

    // Extra empty tags check, empty tag = root tag which is not asignable

    for (int index = 0 ; index < keywords.size() ; ++index)
    {
        QString keyword = keywords.at(index);

        if (!keyword.isEmpty())
        {

            // _Digikam_root_tag_ is present in some photos tagged with older
            // version of digiKam, must be removed

            if (keyword.contains(QRegularExpression(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)"))))
            {
                keyword = keyword.replace(QRegularExpression(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)")),
                                          QLatin1String(""));
            }

            d->commit.filteredKeywords.append(keyword);
        }
    }

    if (!d->commit.filteredKeywords.isEmpty())
    {
        // get tag ids, create if necessary

        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(d->commit.filteredKeywords);
        d->commit.tagIds += tagIds;
    }

    // Check Pick Label tag.

    int pickId = d->metadata->getItemPickLabel();

    if (pickId != -1)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Pick Label found :" << pickId;

        // Check if pickId is on the valid range for a PickLabel.

        if ((pickId >= PickLabel::FirstPickLabel) && (pickId <= PickLabel::LastPickLabel))
        {
            int tagId = TagsCache::instance()->tagForPickLabel((PickLabel)pickId);

            if (tagId)
            {
                d->commit.tagIds << tagId;
                d->commit.hasPickTag = true;
                qCDebug(DIGIKAM_DATABASE_LOG) << "Assigned Pick Label Tag :" << tagId;
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Cannot find Pick Label Tag for :" << pickId;
            }
        }
        else
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Invalid Pick Label value (out of range):" << pickId;
        }
    }

    // Check Color Label tag.

    int colorId = d->metadata->getItemColorLabel();

    if (colorId != -1)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Color Label found :" << colorId;

        int tagId = TagsCache::instance()->tagForColorLabel((ColorLabel)colorId);

        if (tagId)
        {
            d->commit.tagIds << tagId;
            d->commit.hasColorTag = true;
            qCDebug(DIGIKAM_DATABASE_LOG) << "Assigned Color Label Tag :" << tagId;
        }
        else
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Cannot find Color Label Tag for :" << colorId;
        }
    }
}

void ItemScanner::commitTags()
{
    const QList<int>& currentTags = CoreDbAccess().db()->getItemTagIDs(d->scanInfo.id);
    const QVector<int>& colorTags = TagsCache::instance()->colorLabelTags();
    const QVector<int>& pickTags  = TagsCache::instance()->pickLabelTags();
    QList<int> removeTags;

    if (d->commit.hasColorTag || d->commit.hasPickTag)
    {
        for (int tag : std::as_const(currentTags))
        {
            if (colorTags.contains(tag) || pickTags.contains(tag))
            {
                removeTags << tag;
            }
        }

    }

    if (!removeTags.isEmpty())
    {
        CoreDbAccess().db()->removeTagsFromItems(QList<qlonglong>() << d->scanInfo.id, removeTags);
    }

    CoreDbAccess().db()->addTagsToItems(QList<qlonglong>() << d->scanInfo.id, d->commit.tagIds);
}

void ItemScanner::scanFaces()
{
    QSize size = d->img.size();

    if (!size.isValid())
    {
        return;
    }

    QMultiMap<QString, QVariant> metadataFacesMap;

    if (!d->metadata->getItemFacesMap(metadataFacesMap))
    {
        return;
    }

    d->commit.commitFaces      = true;
    d->commit.metadataFacesMap = metadataFacesMap;
}

void ItemScanner::commitFaces()
{
    FaceTagsEditor editor;
    QList<QRect> assignedRects;
    QMultiMap<QString, QVariant>::const_iterator it;
    QSize size                         = d->img.size();
    int orientation                    = d->img.orientation();
    QList<FaceTagsIface> databaseFaces = editor.databaseFaces(d->scanInfo.id);

    for (it = d->commit.metadataFacesMap.constBegin() ; it != d->commit.metadataFacesMap.constEnd() ; ++it)
    {
        QString name = it.key();
        QRectF rectF = it.value().toRectF();

        if (!rectF.isValid())
        {
            int tagId = FaceTags::getOrCreateTagForPerson(name);

            if (tagId)
            {
                ItemInfo(d->scanInfo.id).setTag(tagId);
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Failed to create a person tag for name" << name;
            }

            continue;
        }

        QRect rect = TagRegion::relativeToAbsolute(rectF, size);
        TagRegion::adjustToOrientation(rect, orientation, size);
        TagRegion newRegion(rect);

        if (assignedRects.contains(rect))
        {
            continue;
        }

        assignedRects << rect;
        QList<FaceTagsIface>::iterator it1;

        for (it1 = databaseFaces.begin() ; it1 != databaseFaces.end() ; )
        {
             double minOverlap = (*it1).isConfirmedName() ? 0.25 : 0.5;

            if ((*it1).region().intersects(newRegion, minOverlap))
            {
                // Remove the duplicate face in the database.

                editor.removeFace((*it1));
                it1 = databaseFaces.erase(it1);

                continue;
            }

            ++it1;
        }

        if (name.isEmpty())
        {
            int tagId = FaceTags::unknownPersonTagId();
            FaceTagsIface face(FaceTagsIface::UnknownName, d->scanInfo.id, tagId, newRegion);

            editor.addManually(face);
        }
        else
        {
            int tagId = 0;

            for (const QString& keyword : std::as_const(d->commit.filteredKeywords))
            {
                if (keyword.endsWith(QLatin1Char('/') + name))
                {
                    tagId = TagsCache::instance()->getOrCreateTag(keyword);

                    if (tagId)
                    {
                        FaceTags::ensureIsPerson(tagId, name);
                    }

                    break;
                }
            }

            if (!tagId)
            {
                tagId = FaceTags::getOrCreateTagForPerson(name);
            }

            if (tagId)
            {
                editor.add(d->scanInfo.id, tagId, newRegion, false);
            }
            else
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Failed to create a person tag for name" << name;
            }
        }
    }
}

void ItemScanner::checkCreationDateFromMetadata(QVariant& dateFromMetadata) const
{
    // creation date: fall back to file system property

    if (dateFromMetadata.isNull() || !dateFromMetadata.toDateTime().isValid())
    {
        dateFromMetadata = creationDateFromFilesystem(d->fileInfo);
    }
}

bool ItemScanner::checkRatingFromMetadata(const QVariant& ratingFromMetadata) const
{
    // should only be overwritten if set in metadata

    if ((d->scanMode == Rescan) || (d->scanMode == CleanScan))
    {
        if (ratingFromMetadata.isNull() || (ratingFromMetadata.toInt() == -1))
        {
            return false;
        }
    }

    return true;
}

MetadataFields ItemScanner::allImageMetadataFields()
{
    // This list must reflect the order required by CoreDB::addImageMetadata

    MetadataFields fields;
    fields << MetadataInfo::Make
           << MetadataInfo::Model
           << MetadataInfo::Lens
           << MetadataInfo::Aperture
           << MetadataInfo::FocalLength
           << MetadataInfo::FocalLengthIn35mm
           << MetadataInfo::ExposureTime
           << MetadataInfo::ExposureProgram
           << MetadataInfo::ExposureMode
           << MetadataInfo::Sensitivity
           << MetadataInfo::FlashMode
           << MetadataInfo::WhiteBalance
           << MetadataInfo::WhiteBalanceColorTemperature
           << MetadataInfo::MeteringMode
           << MetadataInfo::SubjectDistance
           << MetadataInfo::SubjectDistanceCategory;
    return fields;
}

} // namespace Digikam
