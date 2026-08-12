/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - generic helpers
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QLocale>
#include <QScopedPointer>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

QVariant DMetadata::fromExifOrXmp(const char* const exifTagName, const char* const xmpTagName) const
{
    QVariant var;

    if (exifTagName)
    {
        var = getExifTagVariant(exifTagName, false);

        if (!var.isNull())
        {
            return var;
        }
    }

    if (xmpTagName)
    {
        var = getXmpTagVariant(xmpTagName);

        if (!var.isNull())
        {
            return var;
        }
    }

    return var;
}

QVariant DMetadata::fromIptcOrXmp(const char* const iptcTagName, const char* const xmpTagName) const
{
    if (iptcTagName)
    {
        QString iptcValue = getIptcTagString(iptcTagName);

        if (!iptcValue.isNull())
        {
            return iptcValue;
        }
    }

    if (xmpTagName)
    {
        QVariant var = getXmpTagVariant(xmpTagName);

        if (!var.isNull())
        {
            return var;
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    return QVariant(QMetaType(QMetaType::QString));

#else

    return QVariant(QVariant::String);

#endif

}

QVariant DMetadata::fromExifOrXmpList(const QStringList& tagList) const
{
    QVariant var;

    for (const QString& tagName : EXIV2_AS_CONST(tagList))
    {
        if      (tagName.startsWith(QLatin1String("Exif")))
        {
            var = getExifTagVariant(tagName.toLatin1().constData(), false);
        }
        else if (tagName.startsWith(QLatin1String("Xmp")))
        {
            var = getXmpTagVariant(tagName.toLatin1().constData());
        }

        if (!var.isNull())
        {
            return var;
        }
    }

    return var;
}

QVariant DMetadata::getMetadataField(MetadataInfo::Field field) const
{
    switch (field)
    {
        case MetadataInfo::Comment:
        {
            return getItemComments().value(QLatin1String("x-default")).caption;
        }

        case MetadataInfo::CommentJfif:
        {
            return toStringVariant(getCommentsDecoded());
        }

        case MetadataInfo::CommentExif:
        {
            return toStringVariant(getExifComment());
        }

        case MetadataInfo::CommentIptc:
        {
            return fromIptcOrXmp("Iptc.Application2.Caption", nullptr);
        }

        case MetadataInfo::Description:
        {
            QVariant var = fromXmpLangAlt("Xmp.dc.description");

            if (!var.isNull())
            {
                return var;
            }

            var = fromXmpLangAlt("Xmp.tiff.ImageDescription");

            if (!var.isNull())
            {
                return var;
            }

            return fromIptcEmulateLangAlt("Iptc.Application2.Caption");
        }

        case MetadataInfo::Headline:
        {
            return fromIptcOrXmp("Iptc.Application2.Headline", "Xmp.photoshop.Headline");
        }

        case MetadataInfo::Title:
        {
            QString str = getItemTitles().value(QLatin1String("x-default")).caption;

            if (str.isEmpty())
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                return QVariant(QMetaType(QMetaType::QVariantMap));

#else

                return QVariant(QVariant::Map);

#endif

            }

            QMap<QString, QVariant> map;
            map[QLatin1String("x-default")] = str;

            return map;
        }

        case MetadataInfo::DescriptionWriter:
        {
            return fromIptcOrXmp("Iptc.Application2.Writer", "Xmp.photoshop.CaptionWriter");
        }

        case MetadataInfo::Keywords:
        {
            QStringList list;
            getItemTagsPath(list);

            return toStringListVariant(list);
        }

        case MetadataInfo::Faces:
        {
            QVariant var;
            QMultiMap<QString, QVariant> faceMap;
            getItemFacesMap(faceMap);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            // NOTE: Qt6::QMultiMap do not inherit of QMap as with Qt5 and QVariant do not handle QMultiMap directly.
            // See the thread here: https://lists.qt-project.org/pipermail/interest/2023-April/039056.html

            var = QVariant::fromValue(faceMap);

#else

            var = QVariant(faceMap);

#endif

            return var;
        }

        case MetadataInfo::Rating:
        {
            return getItemRating();
        }

        case MetadataInfo::CreationDate:
        {
            return getItemDateTime();
        }

        case MetadataInfo::DigitizationDate:
        {
            return getDigitizationDateTime(true);
        }

        case MetadataInfo::Orientation:
        {
            return (int)getItemOrientation();
        }

        case MetadataInfo::Make:
        {
            QStringList tagList;
            tagList << QLatin1String("Exif.Image.Make");
            tagList << QLatin1String("Exif.PanasonicRaw.Make");
            tagList << QLatin1String("Xmp.tiff.Make");

            QVariant var = fromExifOrXmpList(tagList);

            return QVariant(var.toString().trimmed());
        }

        case MetadataInfo::Model:
        {
            QStringList tagList;
            tagList << QLatin1String("Exif.Image.Model");
            tagList << QLatin1String("Exif.PanasonicRaw.Model");
            tagList << QLatin1String("Xmp.tiff.Model");

            QVariant var = fromExifOrXmpList(tagList);

            return QVariant(var.toString().trimmed());
        }

        case MetadataInfo::Lens:
        {
            return getLensDescription();
        }

        case MetadataInfo::Aperture:
        {
            QStringList tagList;
            tagList << QLatin1String("Exif.Photo.FNumber");
            tagList << QLatin1String("Exif.Image.FNumber");
            tagList << QLatin1String("Xmp.exif.FNumber");

            QVariant var = fromExifOrXmpList(tagList);

            if (var.isNull())
            {
                var = fromExifOrXmp("Exif.Photo.ApertureValue", "Xmp.exif.ApertureValue");

                if (!var.isNull())
                {
                    var = apexApertureToFNumber(var.toDouble());
                }
            }

            return var;
        }

        case MetadataInfo::FocalLength:
        {
            QStringList tagList;
            tagList << QLatin1String("Exif.Photo.FocalLength");
            tagList << QLatin1String("Exif.Image.FocalLength");
            tagList << QLatin1String("Xmp.exif.FocalLength");

            return fromExifOrXmpList(tagList);
        }

        case MetadataInfo::FocalLengthIn35mm:
        {
            return fromExifOrXmp("Exif.Photo.FocalLengthIn35mmFilm", "Xmp.exif.FocalLengthIn35mmFilm");
        }

        case MetadataInfo::ExposureTime:
        {
            QStringList tagList;
            tagList << QLatin1String("Exif.Photo.ExposureTime");
            tagList << QLatin1String("Exif.Image.ExposureTime");
            tagList << QLatin1String("Xmp.exif.ExposureTime");

            QVariant var = fromExifOrXmpList(tagList);

            if (var.isNull())
            {
                var = fromExifOrXmp("Exif.Photo.ShutterSpeedValue", "Xmp.exif.ShutterSpeedValue");

                if (!var.isNull())
                {
                    var = apexShutterSpeedToExposureTime(var.toDouble());
                }
            }

            return var;
        }

        case MetadataInfo::ExposureProgram:
        {
            return fromExifOrXmp("Exif.Photo.ExposureProgram", "Xmp.exif.ExposureProgram");
        }

        case MetadataInfo::ExposureMode:
        {
            return fromExifOrXmp("Exif.Photo.ExposureMode", "Xmp.exif.ExposureMode");
        }

        case MetadataInfo::Sensitivity:
        {
            QStringList tagList;
            tagList << QLatin1String("Exif.Photo.ISOSpeedRatings");
            tagList << QLatin1String("Exif.Image.ISOSpeedRatings");
            tagList << QLatin1String("Exif.PanasonicRaw.ISOSpeed");
            tagList << QLatin1String("Xmp.exif.ISOSpeedRatings");

            QVariant var = fromExifOrXmpList(tagList);
/*
            if (var.isNull())
            {
                TODO: has this ISO format??? We must convert to the format of ISOSpeedRatings!
                var = fromExifOrXmp("Exif.Photo.ExposureIndex", "Xmp.exif.ExposureIndex");
            }
*/
            return var;
        }

        case MetadataInfo::FlashMode:
        {
            return fromExifOrXmp("Exif.Photo.Flash", "Xmp.exif.Flash/exif:Mode");
        }

        case MetadataInfo::WhiteBalance:
        {
            return fromExifOrXmp("Exif.Photo.WhiteBalance", "Xmp.exif.WhiteBalance");
        }

        case MetadataInfo::MeteringMode:
        {
            return fromExifOrXmp("Exif.Photo.MeteringMode", "Xmp.exif.MeteringMode");
        }

        case MetadataInfo::SubjectDistance:
        {
            return fromExifOrXmp("Exif.Photo.SubjectDistance", "Xmp.exif.SubjectDistance");
        }

        case MetadataInfo::SubjectDistanceCategory:
        {
            return fromExifOrXmp("Exif.Photo.SubjectDistanceRange", "Xmp.exif.SubjectDistanceRange");
        }

        case MetadataInfo::WhiteBalanceColorTemperature:
        {
            //TODO: ??

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            return QVariant(QMetaType(QMetaType::Int));

#else

            return QVariant(QVariant::Int);

#endif

        }

        case MetadataInfo::Longitude:
        {
            return toStringVariant(getGPSLongitudeString());
        }

        case MetadataInfo::LongitudeNumber:
        {
            double longitude;

            if (getGPSLongitudeNumber(&longitude))
            {
                return longitude;
            }
            else
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                return QVariant(QMetaType(QMetaType::Double));

#else

                return QVariant(QVariant::Double);

#endif

            }
        }

        case MetadataInfo::Latitude:
        {
            return toStringVariant(getGPSLatitudeString());
        }

        case MetadataInfo::LatitudeNumber:
        {
            double latitude;

            if (getGPSLatitudeNumber(&latitude))
            {
                return latitude;
            }
            else
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                return QVariant(QMetaType(QMetaType::Double));

#else

                return QVariant(QVariant::Double);

#endif

            }
        }

        case MetadataInfo::Altitude:
        {
            double altitude;

            if (getGPSAltitude(&altitude))
            {
                return altitude;
            }
            else
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                return QVariant(QMetaType(QMetaType::Double));

#else

                return QVariant(QVariant::Double);

#endif

            }
        }

        case MetadataInfo::PositionOrientation:
        case MetadataInfo::PositionTilt:
        case MetadataInfo::PositionRoll:
        case MetadataInfo::PositionAccuracy:
        {
            // TODO or unsupported?

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                return QVariant(QMetaType(QMetaType::Double));

#else

                return QVariant(QVariant::Double);

#endif

        }

        case MetadataInfo::PositionDescription:
        {
            // TODO or unsupported?

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                return QVariant(QMetaType(QMetaType::QString));

#else

                return QVariant(QVariant::String);

#endif

        }

        case MetadataInfo::IptcCoreCopyrightNotice:
        {
            QVariant var = fromXmpLangAlt("Xmp.dc.rights");

            if (!var.isNull())
            {
                return var;
            }

            var = fromXmpLangAlt("Xmp.tiff.Copyright");

            if (!var.isNull())
            {
                return var;
            }

            return fromIptcEmulateLangAlt("Iptc.Application2.Copyright");
        }

        case MetadataInfo::IptcCoreCreator:
        {
            QVariant var = fromXmpList("Xmp.dc.creator");

            if (!var.isNull())
            {
                return var;
            }

            QString artist = getXmpTagString("Xmp.tiff.Artist");

            if (!artist.isNull())
            {
                QStringList list;
                list << artist;

                return list;
            }

            return fromIptcEmulateList("Iptc.Application2.Byline");
        }

        case MetadataInfo::IptcCoreProvider:
        {
            return fromIptcOrXmp("Iptc.Application2.Credit", "Xmp.photoshop.Credit");
        }

        case MetadataInfo::IptcCoreRightsUsageTerms:
        {
            return fromXmpLangAlt("Xmp.xmpRights.UsageTerms");
        }

        case MetadataInfo::IptcCoreSource:
        {
            return fromIptcOrXmp("Iptc.Application2.Source", "Xmp.photoshop.Source");
        }

        case MetadataInfo::IptcCoreCreatorJobTitle:
        {
            return fromIptcOrXmp("Iptc.Application2.BylineTitle", "Xmp.photoshop.AuthorsPosition");
        }

        case MetadataInfo::IptcCoreInstructions:
        {
            return fromIptcOrXmp("Iptc.Application2.SpecialInstructions", "Xmp.photoshop.Instructions");
        }

        case MetadataInfo::IptcCoreLocationInfo:
        {
            IptcCoreLocationInfo location = getIptcCoreLocation();

            if (location.isNull())
            {
                return QVariant();
            }

            return QVariant::fromValue(location);
        }

        case MetadataInfo::IptcCoreCountryCode:
        {
            return fromIptcOrXmp("Iptc.Application2.CountryCode", "Xmp.iptc.CountryCode");
        }

        case MetadataInfo::IptcCoreCountry:
        {
            return fromIptcOrXmp("Iptc.Application2.CountryName", "Xmp.photoshop.Country");
        }

        case MetadataInfo::IptcCoreCity:
        {
            return fromIptcOrXmp("Iptc.Application2.City", "Xmp.photoshop.City");
        }

        case MetadataInfo::IptcCoreLocation:
        {
            return fromIptcOrXmp("Iptc.Application2.SubLocation", "Xmp.iptc.Location");
        }

        case MetadataInfo::IptcCoreProvinceState:
        {
            return fromIptcOrXmp("Iptc.Application2.ProvinceState", "Xmp.photoshop.State");
        }

        case MetadataInfo::IptcCoreIntellectualGenre:
        {
            return fromIptcOrXmp("Iptc.Application2.ObjectAttribute", "Xmp.iptc.IntellectualGenre");
        }

        case MetadataInfo::IptcCoreJobID:
        {
            return fromIptcOrXmp("Iptc.Application2.TransmissionReference", "Xmp.photoshop.TransmissionReference");
        }

        case MetadataInfo::IptcCoreScene:
        {
            return fromXmpList("Xmp.iptc.Scene");
        }

        case MetadataInfo::IptcCoreSubjectCode:
        {
            return toStringListVariant(getIptcCoreSubjects());
        }

        case MetadataInfo::IptcCoreContactInfo:
        {
            IptcCoreContactInfo info = getCreatorContactInfo();

            if (info.isNull())
            {
                return QVariant();
            }

            return QVariant::fromValue(info);
        }

        case MetadataInfo::IptcCoreContactInfoCity:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity");
        }

        case MetadataInfo::IptcCoreContactInfoCountry:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry");
        }

        case MetadataInfo::IptcCoreContactInfoAddress:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr");
        }

        case MetadataInfo::IptcCoreContactInfoPostalCode:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode");
        }

        case MetadataInfo::IptcCoreContactInfoProvinceState:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion");
        }

        case MetadataInfo::IptcCoreContactInfoEmail:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiEmailWork");
        }

        case MetadataInfo::IptcCoreContactInfoPhone:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiTelWork");
        }

        case MetadataInfo::IptcCoreContactInfoWebUrl:
        {
            return getXmpTagVariant("Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiUrlWork");
        }

        case MetadataInfo::AspectRatio:
        {
            long num             = 0;
            long den             = 1;

            // NOTE: there is a bug in Exiv2 xmp::video tag definition as "Rational" value is defined as "Ratio"...
/*
            QList<QVariant> list = getXmpTagVariant("Xmp.video.AspectRatio").toList();
*/
            QString ar       = getXmpTagString("Xmp.video.AspectRatio");
            QStringList list = ar.split(QLatin1Char('/'));

            if (list.size() >= 1)
            {
                num = list[0].toInt();
            }

            if (list.size() >= 2)
            {
                den = list[1].toInt();
            }

            return QString::number((double)num / (double)den);
        }

        case MetadataInfo::AudioBitRate:
        {
            return fromXmpLangAlt("Xmp.audio.SampleRate");
        }

        case MetadataInfo::AudioChannelType:
        {
            return fromXmpLangAlt("Xmp.audio.ChannelType");
        }

        case MetadataInfo::AudioCodec:
        {
            return fromXmpLangAlt("Xmp.audio.Codec");
        }

        case MetadataInfo::Duration:
        {
            return fromXmpLangAlt("Xmp.video.duration"); // duration is in ms
        }

        case MetadataInfo::FrameRate:
        {
            return fromXmpLangAlt("Xmp.video.FrameRate");
        }

        case MetadataInfo::VideoCodec:
        {
            return fromXmpLangAlt("Xmp.video.Codec");
        }

        case MetadataInfo::VideoBitDepth:
        {
            return fromXmpLangAlt("Xmp.video.BitDepth");
        }

        case MetadataInfo::VideoHeight:
        {
            return fromXmpLangAlt("Xmp.video.Height");
        }

        case MetadataInfo::VideoWidth:
        {
            return fromXmpLangAlt("Xmp.video.Width");
        }

        case MetadataInfo::VideoColorSpace:
        {
            QString cs = getXmpTagString("Xmp.video.ColorSpace");

            if      (cs == QLatin1String("sRGB"))
            {
                return QString::number(VIDEOCOLORMODEL_SRGB);
            }
            else if (cs == QLatin1String("CCIR-601"))
            {
                return QString::number(VIDEOCOLORMODEL_BT601);
            }
            else if (cs == QLatin1String("CCIR-709"))
            {
                return QString::number(VIDEOCOLORMODEL_BT709);
            }
            else if (cs == QLatin1String("Other"))
            {
                return QString::number(VIDEOCOLORMODEL_OTHER);
            }
            else
            {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                return QVariant(QMetaType(QMetaType::Int));

#else

                return QVariant(QVariant::Int);

#endif

            }
        }

        default:
        {
            return QVariant();
        }
    }
}

QVariantList DMetadata::getMetadataFields(const MetadataFields& fields) const
{
    QVariantList list;

    for (MetadataInfo::Field field : EXIV2_AS_CONST(fields))
    {
        list << getMetadataField(field);
    }

    return list;
}

QString DMetadata::valueToString(const QVariant& value, MetadataInfo::Field field)
{
    if (value.isNull())
    {
        return QString();
    }

    QScopedPointer<MetaEngine> exiv2Iface(new MetaEngine);

    switch (field)
    {
        case MetadataInfo::Rating:
        {
            return value.toString();
        }

        case MetadataInfo::CreationDate:
        case MetadataInfo::DigitizationDate:
        {
            return asUserDateTime(value.toDateTime());
        }

        case MetadataInfo::Orientation:
        {
            switch (value.toInt())
            {
                // Example why the English text differs from the enum names: ORIENTATION_ROT_90.
                // Rotation by 90 degrees is right (clockwise) rotation.
                // But: The enum names describe what needs to be done to get the image right again.
                // And an image that needs to be rotated 90 degrees is currently rotated 270 degrees = left.

                case ORIENTATION_UNSPECIFIED:
                {
                    return i18nc("@info: rotation", "Unspecified");
                }

                case ORIENTATION_NORMAL:
                {
                    return i18nc("@info: rotation of an unrotated image", "Normal");
                }

                case ORIENTATION_HFLIP:
                {
                    return i18nc("@info: rotation", "Flipped Horizontally");
                }

                case ORIENTATION_ROT_180:
                {
                    return i18nc("@info: rotation", "Rotated by 180 Degrees");
                }

                case ORIENTATION_VFLIP:
                {
                    return i18nc("@info: rotation", "Flipped Vertically");
                }

                case ORIENTATION_ROT_90_HFLIP:
                {
                    return i18nc("@info: rotation", "Flipped Horizontally and Rotated Left");
                }

                case ORIENTATION_ROT_90:
                {
                    return i18nc("@info: rotation", "Rotated Left");
                }

                case ORIENTATION_ROT_90_VFLIP:
                {
                    return i18nc("@info: rotation", "Flipped Vertically and Rotated Left");
                }

                case ORIENTATION_ROT_270:
                {
                    return i18nc("@info: rotation", "Rotated Right");
                }

                default:
                {
                    return i18nc("@info: rotation", "Unknown");
                }
            }

            break;
        }

        case MetadataInfo::Make:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Image.Make", value);
        }

        case MetadataInfo::Model:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Image.Model", value);
        }

        case MetadataInfo::Lens:
        {
            // heterogeneous source, non-standardized string

            return value.toString();
        }

        case MetadataInfo::Aperture:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.FNumber", value);
        }

        case MetadataInfo::FocalLength:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.FocalLength", value);
        }

        case MetadataInfo::FocalLengthIn35mm:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.FocalLengthIn35mmFilm", value);
        }

        case MetadataInfo::ExposureTime:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.ExposureTime", value);
        }

        case MetadataInfo::ExposureProgram:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.ExposureProgram", value);
        }

        case MetadataInfo::ExposureMode:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.ExposureMode", value);
        }

        case MetadataInfo::Sensitivity:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.ISOSpeedRatings", value);
        }

        case MetadataInfo::FlashMode:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.Flash", value);
        }

        case MetadataInfo::WhiteBalance:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.WhiteBalance", value);
        }

        case MetadataInfo::MeteringMode:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.MeteringMode", value);
        }

        case MetadataInfo::SubjectDistance:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.SubjectDistance", value);
        }

        case MetadataInfo::SubjectDistanceCategory:
        {
            return exiv2Iface->createExifUserStringFromValue("Exif.Photo.SubjectDistanceRange", value);
        }

        case MetadataInfo::WhiteBalanceColorTemperature:
        {
            return i18nc("@info: Temperature in Kelvin", "%1 K", value.toInt());
        }

        case MetadataInfo::AspectRatio:
        case MetadataInfo::AudioBitRate:
        case MetadataInfo::AudioChannelType:
        case MetadataInfo::AudioCodec:
        case MetadataInfo::Duration:
        case MetadataInfo::FrameRate:
        case MetadataInfo::VideoCodec:
        {
            return value.toString();
        }

        case MetadataInfo::Longitude:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
            {
                return QString();
            }

            QString direction = (QLatin1Char(directionRef) == QLatin1Char('W'))         ?
                                i18nc("@info: For use in longitude coordinate", "West") :
                                i18nc("@info: For use in longitude coordinate", "East");

            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                        .arg(minutes).arg(QChar(0x2032))
                        .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }

        case MetadataInfo::LongitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (QLatin1Char(directionRef) == QLatin1Char('W'))         ?
                                i18nc("@info: For use in longitude coordinate", "West") :
                                i18nc("@info: For use in longitude coordinate", "East");

            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                        .arg(minutes).arg(QChar(0x2032))
                        .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }

        case MetadataInfo::Latitude:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            if (!convertToUserPresentableNumbers(value.toString(), &degrees, &minutes, &seconds, &directionRef))
            {
                return QString();
            }

            QString direction = (QLatin1Char(directionRef) == QLatin1Char('N'))         ?
                                i18nc("@info: For use in latitude coordinate", "North") :
                                i18nc("@info: For use in latitude coordinate", "South");

            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                        .arg(minutes).arg(QChar(0x2032))
                        .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }

        case MetadataInfo::LatitudeNumber:
        {
            int    degrees, minutes;
            double seconds;
            char   directionRef;

            convertToUserPresentableNumbers(false, value.toDouble(), &degrees, &minutes, &seconds, &directionRef);
            QString direction = (QLatin1Char(directionRef) == QLatin1Char('N'))         ?
                                i18nc("@info: For use in latitude coordinate", "North") :
                                i18nc("@info: For use in latitude coordinate", "South");

            return QString::fromLatin1("%1%2%3%4%L5%6 %7").arg(degrees).arg(QChar(0xB0))
                       .arg(minutes).arg(QChar(0x2032))
                       .arg(seconds, 'f').arg(QChar(0x2033)).arg(direction);
        }

        case MetadataInfo::Altitude:
        {
            QString meters = QString::fromLatin1("%L1").arg(value.toDouble(), 0, 'f', 2);

            // xgettext: no-c-format

            return i18nc("@info: Height in meters", "%1m", meters);
        }

        case MetadataInfo::PositionOrientation:
        case MetadataInfo::PositionTilt:
        case MetadataInfo::PositionRoll:
        case MetadataInfo::PositionAccuracy:
        {
            //TODO
            return value.toString();
        }

        case MetadataInfo::PositionDescription:
        {
            return value.toString();
        }

        // Lang Alt

        case MetadataInfo::IptcCoreCopyrightNotice:
        case MetadataInfo::IptcCoreRightsUsageTerms:
        case MetadataInfo::Description:
        case MetadataInfo::Title:
        {
            QMap<QString, QVariant> map = value.toMap();

            // the most common cases

            if      (map.isEmpty())
            {
                return QString();
            }
            else if (map.size() == 1)
            {
                return map.begin().value().toString();
            }

            // Try "en-us"

            QString spec = QLocale().name().toLower().replace(QLatin1Char('_'), QLatin1Char('-'));

            if (map.contains(spec))
            {
                return map[spec].toString();
            }

            // Try "en-"

            QStringList keys    = map.keys();
            QString spec2       = QLocale().name().toLower();
            QRegularExpression exp(spec2.left(spec2.indexOf(QLatin1Char('_'))) + QLatin1Char('-'));
            QStringList matches = keys.filter(exp);

            if (!matches.isEmpty())
            {
                return map[matches.first()].toString();
            }

            // return default

            if (map.contains(QLatin1String("x-default")))
            {
                return map[QLatin1String("x-default")].toString();
            }

            // return first entry

            return map.begin().value().toString();
        }

        // List

        case MetadataInfo::IptcCoreCreator:
        case MetadataInfo::IptcCoreScene:
        case MetadataInfo::IptcCoreSubjectCode:
        {
            return value.toStringList().join(QLatin1Char(' '));
        }

        // Text

        case MetadataInfo::Comment:
        case MetadataInfo::CommentJfif:
        case MetadataInfo::CommentExif:
        case MetadataInfo::CommentIptc:
        case MetadataInfo::Headline:
        case MetadataInfo::DescriptionWriter:
        case MetadataInfo::IptcCoreProvider:
        case MetadataInfo::IptcCoreSource:
        case MetadataInfo::IptcCoreCreatorJobTitle:
        case MetadataInfo::IptcCoreInstructions:
        case MetadataInfo::IptcCoreCountryCode:
        case MetadataInfo::IptcCoreCountry:
        case MetadataInfo::IptcCoreCity:
        case MetadataInfo::IptcCoreLocation:
        case MetadataInfo::IptcCoreProvinceState:
        case MetadataInfo::IptcCoreIntellectualGenre:
        case MetadataInfo::IptcCoreJobID:
        {
            return value.toString();
        }

        default:
        {
            break;
        }
    }

    return QString();
}

QStringList DMetadata::valuesToString(const QVariantList& values, const MetadataFields& fields)
{
    int size = values.size();

    QStringList list;

    for (int i = 0 ; i < size ; ++i)
    {
        list << valueToString(values.at(i), fields.at(i));
    }

    return list;
}

QMap<int, QString> DMetadata::possibleValuesForEnumField(MetadataInfo::Field field)
{
    QMap<int, QString> map;
    int min, max;

    switch (field)
    {
        case MetadataInfo::Orientation:                      /// Int, enum from libMetaEngine
        {
            min = ORIENTATION_UNSPECIFIED;
            max = ORIENTATION_ROT_270;
            break;
        }

        case MetadataInfo::ExposureProgram:                  /// Int, enum from Exif
        {
            min = 0;
            max = 8;
            break;
        }

        case MetadataInfo::ExposureMode:                     /// Int, enum from Exif
        {
            min = 0;
            max = 2;
            break;
        }

        case MetadataInfo::WhiteBalance:                     /// Int, enum from Exif
        {
            min = 0;
            max = 1;
            break;
        }

        case MetadataInfo::MeteringMode:                     /// Int, enum from Exif
        {
            min = 0;
            max = 6;
            map[255] = valueToString(255, field);
            break;
        }

        case MetadataInfo::SubjectDistanceCategory:          /// int, enum from Exif
        {
            min = 0;
            max = 3;
            break;
        }

        case MetadataInfo::FlashMode:                        /// Int, bit mask from Exif
        {
            // This one is a bit special.
            // We return a bit mask for binary AND searching.

            map[0x1]  = i18nc("@info", "Flash has been fired");
            map[0x40] = i18nc("@info", "Flash with red-eye reduction mode");

            //more: TODO?

            return map;
        }

        default:
        {
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Unsupported field " << field << " in DMetadata::possibleValuesForEnumField";
            return map;
        }
    }

    for (int i = min ; i <= max ; ++i)
    {
        map[i] = valueToString(i, field);
    }

    return map;
}

bool DMetadata::hasValidField(const QVariantList& list) const
{
    for (QVariantList::const_iterator it = list.constBegin() ;
         it != list.constEnd() ; ++it)
    {
        if (!(*it).isNull())
        {
            return true;
        }
    }

    return false;
}

QVariant DMetadata::toStringListVariant(const QStringList& list) const
{
    if (list.isEmpty())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            return QVariant(QMetaType(QMetaType::QStringList));

#else

            return QVariant(QVariant::StringList);

#endif

    }

    return list;
}

QVariant DMetadata::toStringVariant(const QString& str) const
{
    if (str.isEmpty())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            return QVariant(QMetaType(QMetaType::QString));

#else

            return QVariant(QVariant::String);

#endif

    }

    return str;
}

} // namespace Digikam
