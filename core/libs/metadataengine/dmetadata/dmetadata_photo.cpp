/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - photo info helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

PhotoInfoContainer DMetadata::getPhotographInformation() const
{
    PhotoInfoContainer photoInfo;

    if (hasExif() || hasXmp())
    {
        photoInfo.dateTime = getItemDateTime();

        // -----------------------------------------------------------------------------------

        photoInfo.make     = getExifTagString("Exif.Image.Make");

        if (photoInfo.make.isEmpty())
        {
            photoInfo.make = getXmpTagString("Xmp.tiff.Make");
        }

        if (photoInfo.make.isEmpty())
        {
            photoInfo.make = getExifTagString("Exif.PanasonicRaw.Make");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.model    = getExifTagString("Exif.Image.Model");

        if (photoInfo.model.isEmpty())
        {
            photoInfo.model = getXmpTagString("Xmp.tiff.Model");
        }

        if (photoInfo.model.isEmpty())
        {
            photoInfo.model = getExifTagString("Exif.PanasonicRaw.Model");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.lens     = getLensDescription();

        // -----------------------------------------------------------------------------------

        photoInfo.aperture = getExifTagString("Exif.Photo.FNumber");

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getExifTagString("Exif.Image.FNumber");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getExifTagString("Exif.Photo.ApertureValue");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.FNumber");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.ApertureValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureTime = getExifTagString("Exif.Photo.ExposureTime");

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getExifTagString("Exif.Image.ExposureTime");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getExifTagString("Exif.Photo.ShutterSpeedValue");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ExposureTime");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ShutterSpeedValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureMode    = getExifTagString("Exif.Photo.ExposureMode");

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getXmpTagString("Xmp.exif.ExposureMode");
        }

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getExifTagString("Exif.CanonCs.MeteringMode");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureProgram = getExifTagString("Exif.Photo.ExposureProgram");

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getXmpTagString("Xmp.exif.ExposureProgram");
        }

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getExifTagString("Exif.CanonCs.ExposureProgram");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength     = getExifTagString("Exif.Photo.FocalLength");

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getExifTagString("Exif.Image.FocalLength");
        }

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getXmpTagString("Xmp.exif.FocalLength");
        }

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getExifTagString("Exif.Canon.FocalLength");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength35mm = getExifTagString("Exif.Photo.FocalLengthIn35mmFilm");

        if (photoInfo.focalLength35mm.isEmpty())
        {
            photoInfo.focalLength35mm = getXmpTagString("Xmp.exif.FocalLengthIn35mmFilm");
        }

        // -----------------------------------------------------------------------------------

        QStringList ISOSpeedTags;

        ISOSpeedTags << QLatin1String("Exif.Photo.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Exif.Photo.ExposureIndex");
        ISOSpeedTags << QLatin1String("Exif.Image.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Exif.CanonSi.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.CanonCs.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon1.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon2.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon3.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO2");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsNew.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsOld.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs5D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs7D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1MltCsA100.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.PanasonicRaw.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Pentax.ISO");
        ISOSpeedTags << QLatin1String("Exif.Olympus.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Samsung2.ISO");

        photoInfo.sensitivity = getExifTagStringFromTagsList(ISOSpeedTags);

        if (photoInfo.sensitivity.isEmpty())
        {
            photoInfo.sensitivity = getXmpTagString("Xmp.exif.ISOSpeedRatings");
        }

        if (photoInfo.sensitivity.isEmpty())
        {
            photoInfo.sensitivity = getXmpTagString("Xmp.exif.ExposureIndex");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.flash = getExifTagString("Exif.Photo.Flash");

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getXmpTagString("Xmp.exif.Flash/exif:Mode");
        }

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getExifTagString("Exif.CanonCs.FlashActivity");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.whiteBalance = getExifTagString("Exif.Photo.WhiteBalance");

        if (photoInfo.whiteBalance.isEmpty())
        {
            photoInfo.whiteBalance = getXmpTagString("Xmp.exif.WhiteBalance");
        }

        // -----------------------------------------------------------------------------------

        double l, L, a;
        photoInfo.hasCoordinates = getGPSInfo(a, l, L);
    }

    return photoInfo;
}

QString DMetadata::getLensDescription() const
{
    QString     lens;
    QStringList lensExifTags;

    // In first, try to get Lens information from Makernotes.

    lensExifTags.append(QLatin1String("Exif.CanonCs.LensType"));      ///< Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.CanonCs.Lens"));          ///< Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Canon.0x0095"));          ///< Alternative Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd1.LensIDNumber")); ///< Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd2.LensIDNumber")); ///< Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd3.LensIDNumber")); ///< Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd4.LensIDNumber")); ///< Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Minolta.LensID"));        ///< Minolta Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sony1.LensID"));          ///< Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sony2.LensID"));          ///< Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.SonyMinolta.LensID"));    ///< Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Pentax.LensType"));       ///< Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.PentaxDng.LensType"));    ///< Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0051"));      ///< Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0310"));      ///< Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sigma.LensRange"));       ///< Sigma Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Samsung2.LensType"));     ///< Samsung Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Photo.0xFDEA"));          ///< Non-standard Exif tag set by Camera Raw.
    lensExifTags.append(QLatin1String("Exif.OlympusEq.LensModel"));   ///< Olympus Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.OlympusEq.LensType"));    ///< Olympus Cameras Makernote.

    // Try Exif.Photo.LensModel for Sony and Canon first.

    QString make      = getExifTagString("Exif.Image.Make");
    QString lensModel = QLatin1String("Exif.Photo.LensModel");

    if      (
             make.contains(QLatin1String("SONY"), Qt::CaseInsensitive)  ||
             make.contains(QLatin1String("NIKON"), Qt::CaseInsensitive) ||
             make.contains(QLatin1String("RICOH"), Qt::CaseInsensitive) ||
             make.contains(QLatin1String("PENTAX"), Qt::CaseInsensitive)
            )
    {
        lensExifTags.prepend(lensModel);
    }
    else if (make.contains(QLatin1String("CANON"), Qt::CaseInsensitive))
    {
        QString canonLt = QLatin1String("Exif.CanonCs.LensType");
        QString canonCs = getExifTagString(canonLt.toLatin1().constData());
        QString exifMod = getExifTagString(lensModel.toLatin1().constData());

        if (
            (exifMod == QLatin1String("RF70-200mm F2.8 L IS USM"))   &&
            (canonCs == QLatin1String("Canon RF 70-200mm F4L IS USM"))
           )
        {
            return QLatin1String("Canon RF 70-200mm F2.8L IS USM");
        }
        else
        {
            lensExifTags.prepend(lensModel);
        }
    }
    else
    {
        lensExifTags.append(lensModel);
    }

    // TODO : add Fuji camera Makernotes.

    // -------------------------------------------------------------------
    // Try to get Lens Data information from Exif.

    for (QStringList::const_iterator it = lensExifTags.constBegin() ; it != lensExifTags.constEnd() ; ++it)
    {
        lens = getExifTagString((*it).toLatin1().constData());

        // To prevent undecoded tag values from Exiv2 as "(65535)"
        // or the value "----" from Exif.Photo.LensModel

        if (
            !lens.isEmpty()                        &&
            (lens != QLatin1String("----"))        &&
            (lens != QLatin1String("65535"))       &&
            !(
              lens.startsWith(QLatin1Char('('))    &&
              lens.endsWith(QLatin1Char(')'))
             )
           )
        {
            return lens;
        }
    }

    // -------------------------------------------------------------------
    // Try to get Lens Data information from XMP.
    // XMP aux tags.

    lens = getXmpTagString("Xmp.aux.Lens");

    if (lens.isEmpty())
    {
        // XMP M$ tags (Lens Maker + Lens Model).

        lens = getXmpTagString("Xmp.MicrosoftPhoto.LensManufacturer");

        if (!lens.isEmpty())
        {
            lens.append(QLatin1Char(' '));
        }

        lens.append(getXmpTagString("Xmp.MicrosoftPhoto.LensModel"));
    }

    return lens;
}

QString DMetadata::getCameraSerialNumber() const
{
    QString sn = getExifTagString("Exif.Image.CameraSerialNumber");

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Canon.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Fujifilm.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Nikon3.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Nikon3.SerialNO");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.OlympusEq.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Olympus.SerialNumber2");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.OlympusEq.InternalSerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Panasonic.InternalSerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Sigma.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Sigma.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Sigma.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getExifTagString("Exif.Pentax.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getXmpTagString("Xmp.exifEX.BodySerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getXmpTagString("Xmp.aux.SerialNumber");
    }

    if (sn.isEmpty())
    {
        sn = getXmpTagString("Xmp.MicrosoftPhoto.CameraSerialNumber");
    }

    return sn;
}

double DMetadata::apexApertureToFNumber(double aperture)
{
    // convert from APEX. See Exif spec, Annex C.

    if      (aperture == 0.0)
    {
        return 1;
    }
    else if (aperture == 1.0)
    {
        return 1.4;
    }
    else if (aperture == 2.0)
    {
        return 2;
    }
    else if (aperture == 3.0)
    {
        return 2.8;
    }
    else if (aperture == 4.0)
    {
        return 4;
    }
    else if (aperture == 5.0)
    {
        return 5.6;
    }
    else if (aperture == 6.0)
    {
        return 8;
    }
    else if (aperture == 7.0)
    {
        return 11;
    }
    else if (aperture == 8.0)
    {
        return 16;
    }
    else if (aperture == 9.0)
    {
        return 22;
    }
    else if (aperture == 10.0)
    {
        return 32;
    }

    return exp(log(2) * aperture / 2.0);
}

double DMetadata::apexShutterSpeedToExposureTime(double shutterSpeed)
{
    // convert from APEX. See Exif spec, Annex C.

    if      (shutterSpeed == -5.0)
    {
        return 30;
    }
    else if (shutterSpeed == -4.0)
    {
        return 15;
    }
    else if (shutterSpeed == -3.0)
    {
        return 8;
    }
    else if (shutterSpeed == -2.0)
    {
        return 4;
    }
    else if (shutterSpeed == -1.0)
    {
        return 2;
    }
    else if (shutterSpeed == 0.0)
    {
        return 1;
    }
    else if (shutterSpeed == 1.0)
    {
        return 0.5;
    }
    else if (shutterSpeed == 2.0)
    {
        return 0.25;
    }
    else if (shutterSpeed == 3.0)
    {
        return 0.125;
    }
    else if (shutterSpeed == 4.0)
    {
        return 1.0 / 15.0;
    }
    else if (shutterSpeed == 5.0)
    {
        return 1.0 / 30.0;
    }
    else if (shutterSpeed == 6.0)
    {
        return 1.0 / 60.0;
    }
    else if (shutterSpeed == 7.0)
    {
        return 0.008;    // 1/125
    }
    else if (shutterSpeed == 8.0)
    {
        return 0.004;    // 1/250
    }
    else if (shutterSpeed == 9.0)
    {
        return 0.002;    // 1/500
    }
    else if (shutterSpeed == 10.0)
    {
        return 0.001;    // 1/1000
    }
    else if (shutterSpeed == 11.0)
    {
        return 0.0005;    // 1/2000
    }

    // supplemental rules

    else if (shutterSpeed == 12.0)
    {
        return 0.00025;    // 1/4000
    }
    else if (shutterSpeed == 13.0)
    {
        return 0.000125;    // 1/8000
    }

    return exp( - log(2) * shutterSpeed);
}

} // namespace Digikam
