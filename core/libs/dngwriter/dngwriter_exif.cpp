/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG - Exif Metadata storage.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Jens Mueller <tschenser at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngwriter_p.h"

// Local includes

#include "dngwriterhost.h"

namespace Digikam
{

int DNGWriter::Private::storeExif(DNGWriterHost& /*host*/,
                                  AutoPtr<dng_negative>& negative,
                                  DRawInfo* const identify,
                                  DRawInfo* const /*identifyMake*/,
                                  DMetadata* const meta)
{
    // Self-write text information.

    exif->fModel.Set_ASCII(identify->model.toLatin1().constData());
    exif->fMake.Set_ASCII(identify->make.toLatin1().constData());
    exif->fSoftware.Set_ASCII(QString::fromLatin1("digiKam %1").arg(digiKamVersion()).toLatin1().constData());
    exif->fExifVersion = DNG_CHAR4('0', '2', '3', '0');

    // Self-Write CFAPattern (section G) manually from mosaic info container.

    const dng_mosaic_info* mosaicInfo = negative->GetMosaicInfo();

    if (mosaicInfo)
    {
        exif->fCFARepeatPatternCols = mosaicInfo->fCFAPatternSize.v;
        exif->fCFARepeatPatternRows = mosaicInfo->fCFAPatternSize.h;

        for (uint16 c = 0 ; c < exif->fCFARepeatPatternCols ; ++c)
        {
            for (uint16 r = 0 ; r < exif->fCFARepeatPatternRows ; ++r)
            {
                exif->fCFAPattern[r][c] = mosaicInfo->fCFAPattern[c][r];
            }
        }
    }

    // Write information with Exiv2

    metaLoaded = meta->load(parent->inputFile());

    if (metaLoaded)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Updating Exif metadata to DNG Negative";

        QString  str;
        long int num, den;
        long     val;

        fileDate                 = meta->getItemDateTime();

        // Time from original shot

        orgDateTimeInfo.SetDateTime(dngDateTime(meta->getItemDateTime()));
        exif->fDateTimeOriginal  = orgDateTimeInfo;

        dng_date_time_info digiDateTimeInfo;
        digiDateTimeInfo.SetDateTime(dngDateTime(meta->getDigitizationDateTime(true)));
        exif->fDateTimeDigitized = digiDateTimeInfo;

        negative->UpdateDateTime(orgDateTimeInfo);

        // String Tags

        str = meta->getExifTagString("Exif.Image.Make");
        if (!str.isEmpty()) exif->fMake.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.Image.Model");
        if (!str.isEmpty()) exif->fModel.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.Image.Software");
        if (!str.isEmpty()) exif->fSoftware.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.Image.ImageDescription");
        if (!str.isEmpty()) exif->fImageDescription.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.Image.Artist");
        if (!str.isEmpty()) exif->fArtist.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.Image.Copyright");
        if (!str.isEmpty()) exif->fCopyright.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.Photo.UserComment");
        if (!str.isEmpty()) exif->fUserComment.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.Image.CameraSerialNumber");
        if (!str.isEmpty()) exif->fCameraSerialNumber.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSLatitudeRef");
        if (!str.isEmpty()) exif->fGPSLatitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSLongitudeRef");
        if (!str.isEmpty()) exif->fGPSLongitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSSatellites");
        if (!str.isEmpty()) exif->fGPSSatellites.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSStatus");
        if (!str.isEmpty()) exif->fGPSStatus.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSMeasureMode");
        if (!str.isEmpty()) exif->fGPSMeasureMode.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSSpeedRef");
        if (!str.isEmpty()) exif->fGPSSpeedRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSTrackRef");
        if (!str.isEmpty()) exif->fGPSTrackRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSSpeedRef");
        if (!str.isEmpty()) exif->fGPSSpeedRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSImgDirectionRef");
        if (!str.isEmpty()) exif->fGPSSpeedRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSMapDatum");
        if (!str.isEmpty()) exif->fGPSMapDatum.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSDestLatitudeRef");
        if (!str.isEmpty()) exif->fGPSDestLatitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSDestLongitudeRef");
        if (!str.isEmpty()) exif->fGPSDestLongitudeRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSDestBearingRef");
        if (!str.isEmpty()) exif->fGPSDestBearingRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSDestDistanceRef");
        if (!str.isEmpty()) exif->fGPSDestDistanceRef.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSProcessingMethod");
        if (!str.isEmpty()) exif->fGPSProcessingMethod.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSAreaInformation");
        if (!str.isEmpty()) exif->fGPSAreaInformation.Set_ASCII(str.trimmed().toLatin1().constData());

        str = meta->getExifTagString("Exif.GPSInfo.GPSDateStamp");
        if (!str.isEmpty()) exif->fGPSDateStamp.Set_ASCII(str.trimmed().toLatin1().constData());

        // Rational Tags

        if (meta->getExifTagRational("Exif.Photo.ExposureTime", num, den))          exif->fExposureTime             = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.FNumber", num, den))               exif->fFNumber                  = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.ShutterSpeedValue", num, den))     exif->fShutterSpeedValue        = dng_srational(num, den);
        if (meta->getExifTagRational("Exif.Photo.ApertureValue", num, den))         exif->fApertureValue            = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.BrightnessValue", num, den))       exif->fBrightnessValue          = dng_srational(num, den);
        if (meta->getExifTagRational("Exif.Photo.ExposureBiasValue", num, den))     exif->fExposureBiasValue        = dng_srational(num, den);
        if (meta->getExifTagRational("Exif.Photo.MaxApertureValue", num, den))      exif->fMaxApertureValue         = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.FocalLength", num, den))           exif->fFocalLength              = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.DigitalZoomRatio", num, den))      exif->fDigitalZoomRatio         = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.SubjectDistance", num, den))       exif->fSubjectDistance          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Image.BatteryLevel", num, den))          exif->fBatteryLevelR            = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.FocalPlaneXResolution", num, den)) exif->fFocalPlaneXResolution    = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.Photo.FocalPlaneYResolution", num, den)) exif->fFocalPlaneYResolution    = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSAltitude", num, den))         exif->fGPSAltitude              = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDOP", num, den))              exif->fGPSDOP                   = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSSpeed", num, den))            exif->fGPSSpeed                 = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSTrack", num, den))            exif->fGPSTrack                 = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSImgDirection", num, den))     exif->fGPSImgDirection          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestBearing", num, den))      exif->fGPSDestBearing           = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestDistance", num, den))     exif->fGPSDestDistance          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSLatitude", num, den, 0))      exif->fGPSLatitude[0]           = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSLatitude", num, den, 1))      exif->fGPSLatitude[1]           = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSLatitude", num, den, 2))      exif->fGPSLatitude[2]           = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSLongitude", num, den, 0))     exif->fGPSLongitude[0]          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSLongitude", num, den, 1))     exif->fGPSLongitude[1]          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSLongitude", num, den, 2))     exif->fGPSLongitude[2]          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSTimeStamp", num, den, 0))     exif->fGPSTimeStamp[0]          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSTimeStamp", num, den, 1))     exif->fGPSTimeStamp[1]          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSTimeStamp", num, den, 2))     exif->fGPSTimeStamp[2]          = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestLatitude", num, den, 0))  exif->fGPSDestLatitude[0]       = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestLatitude", num, den, 1))  exif->fGPSDestLatitude[1]       = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestLatitude", num, den, 2))  exif->fGPSDestLatitude[2]       = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestLongitude", num, den, 0)) exif->fGPSDestLongitude[0]      = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestLongitude", num, den, 1)) exif->fGPSDestLongitude[1]      = dng_urational(num, den);
        if (meta->getExifTagRational("Exif.GPSInfo.GPSDestLongitude", num, den, 2)) exif->fGPSDestLongitude[2]      = dng_urational(num, den);

        // Integer Tags

        if (meta->getExifTagLong("Exif.Photo.ExposureProgram", val))                exif->fExposureProgram          = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.ISOSpeedRatings", val))                exif->fISOSpeedRatings[0]       = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.MeteringMode", val))                   exif->fMeteringMode             = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.LightSource", val))                    exif->fLightSource              = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.Flash", val))                          exif->fFlash                    = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.SensingMethod", val))                  exif->fSensingMethod            = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.FileSource", val))                     exif->fFileSource               = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.SceneType", val))                      exif->fSceneType                = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.CustomRendered", val))                 exif->fCustomRendered           = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.ExposureMode", val))                   exif->fExposureMode             = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.WhiteBalance", val))                   exif->fWhiteBalance             = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.SceneCaptureType", val))               exif->fSceneCaptureType         = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.GainControl", val))                    exif->fGainControl              = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.Contrast", val))                       exif->fContrast                 = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.Saturation", val))                     exif->fSaturation               = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.Sharpness", val))                      exif->fSharpness                = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.SubjectDistanceRange", val))           exif->fSubjectDistanceRange     = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.FocalLengthIn35mmFilm", val))          exif->fFocalLengthIn35mmFilm    = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.ComponentsConfiguration", val))        exif->fComponentsConfiguration  = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.PixelXDimension", val))                exif->fPixelXDimension          = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.PixelYDimension", val))                exif->fPixelYDimension          = (uint32)val;
        if (meta->getExifTagLong("Exif.Photo.FocalPlaneResolutionUnit", val))       exif->fFocalPlaneResolutionUnit = (uint32)val;
        if (meta->getExifTagLong("Exif.GPSInfo.GPSAltitudeRef", val))               exif->fGPSAltitudeRef           = (uint32)val;
        if (meta->getExifTagLong("Exif.GPSInfo.GPSDifferential", val))              exif->fGPSDifferential          = (uint32)val;

        long gpsVer1        = 0;
        long gpsVer2        = 0;
        long gpsVer3        = 0;
        long gpsVer4        = 0;
        meta->getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer1, 0);
        meta->getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer2, 1);
        meta->getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer3, 2);
        meta->getExifTagLong("Exif.GPSInfo.GPSVersionID", gpsVer4, 3);

        long gpsVer         = 0;
        gpsVer             += gpsVer1 << 24;
        gpsVer             += gpsVer2 << 16;
        gpsVer             += gpsVer3 <<  8;
        gpsVer             += gpsVer4;
        exif->fGPSVersionID = (uint32)gpsVer;
    }

    if (cancel)
    {
        return PROCESS_CANCELED;
    }

    return PROCESS_CONTINUE;
}

} // namespace Digikam
