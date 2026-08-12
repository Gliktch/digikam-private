/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-07
 * Description : a command line tool to compare info from 2 DNG files
 *
 * SPDX-FileCopyrightText: 2011      by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C++ includes

#include <cmath>
#include <limits>

// Qt includes

#include <QString>
#include <QCoreApplication>

// DNG SDK includes

#include "dng_file_stream.h"
#include "dng_host.h"
#include "dng_info.h"
#include "dng_xmp_sdk.h"
#include "dng_sdk_limits.h"

// Local includes

#include "digikam_debug.h"

bool AreSame(real64 a, real64 b)
{
    return (std::fabs(a - b) < std::numeric_limits<real32>::epsilon());
}

void compareExif(dng_exif* const exif1, dng_exif* const exif2)
{
    if (!AreSame(exif1->fApertureValue.As_real64(), exif2->fApertureValue.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ApertureValue: %1 %2").arg(exif1->fApertureValue.As_real64()).arg(exif2->fApertureValue.As_real64());

    if (exif1->fArtist != exif2->fArtist)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Artist: %1 %2").arg(QLatin1String(exif1->fArtist.Get())).arg(QLatin1String(exif2->fArtist.Get()));

    if (exif1->fBatteryLevelA != exif2->fBatteryLevelA)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BatteryLevelA: %1 %2").arg(QLatin1String(exif1->fBatteryLevelA.Get())).arg(QLatin1String(exif2->fBatteryLevelA.Get()));

    if (!AreSame(exif1->fBatteryLevelR.As_real64(), exif2->fBatteryLevelR.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BatteryLevelR: %1 %2").arg(exif1->fBatteryLevelR.As_real64()).arg(exif2->fBatteryLevelR.As_real64());

    if (!AreSame(exif1->fBrightnessValue.As_real64(), exif2->fBrightnessValue.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BrightnessValue: %1 %2").arg(exif1->fBrightnessValue.As_real64()).arg(exif2->fBrightnessValue.As_real64());

    if (exif1->fCameraSerialNumber != exif2->fCameraSerialNumber)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CameraSerialNumber: %1 %2").arg(QLatin1String(exif1->fCameraSerialNumber.Get())).arg(QLatin1String(exif2->fCameraSerialNumber.Get()));

    if (0 != memcmp(exif1->fCFAPattern, exif2->fCFAPattern, kMaxCFAPattern * kMaxCFAPattern * sizeof(uint8)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CFAPattern");

    if (exif1->fCFARepeatPatternCols != exif2->fCFARepeatPatternCols)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CFARepeatPatternCols: %1 %2").arg(exif1->fCFARepeatPatternCols).arg(exif2->fCFARepeatPatternCols);

    if (exif1->fCFARepeatPatternRows != exif2->fCFARepeatPatternRows)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CFARepeatPatternRows: %1 %2").arg(exif1->fCFARepeatPatternRows).arg(exif2->fCFARepeatPatternRows);

    if (exif1->fColorSpace != exif2->fColorSpace)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ColorSpace: %1 %2").arg(exif1->fColorSpace).arg(exif2->fColorSpace);

    if (exif1->fComponentsConfiguration != exif2->fComponentsConfiguration)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ComponentsConfiguration: %1 %2").arg(exif1->fComponentsConfiguration).arg(exif2->fComponentsConfiguration);

    if (!AreSame(exif1->fCompresssedBitsPerPixel.As_real64(), exif2->fCompresssedBitsPerPixel.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CompresssedBitsPerPixel: %1 %2").arg(exif1->fCompresssedBitsPerPixel.As_real64()).arg(exif2->fCompresssedBitsPerPixel.As_real64());

    if (exif1->fContrast != exif2->fContrast)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Contrast: %1 %2").arg(exif1->fContrast).arg(exif2->fContrast);

    if (exif1->fCopyright != exif2->fCopyright)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Copyright: %1 %2").arg(QLatin1String(exif1->fCopyright.Get())).arg(QLatin1String(exif2->fCopyright.Get()));

    if (exif1->fCopyright2 != exif2->fCopyright2)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Copyright2: %1 %2").arg(QLatin1String(exif1->fCopyright2.Get())).arg(QLatin1String(exif2->fCopyright2.Get()));

    if (exif1->fCustomRendered != exif2->fCustomRendered)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CustomRendered: %1 %2").arg(exif1->fCustomRendered).arg(exif2->fCustomRendered);

    if (exif1->fDateTime.DateTime() != exif2->fDateTime.DateTime())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DateTime: %1 %2").arg(QLatin1String(exif1->fDateTime.Encode_ISO_8601().Get())).arg(QLatin1String(exif2->fDateTime.Encode_ISO_8601().Get()));

    if (exif1->fDateTimeDigitized.DateTime() != exif2->fDateTimeDigitized.DateTime())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DateTimeDigitized: %1 %2").arg(QLatin1String(exif1->fDateTimeDigitized.Encode_ISO_8601().Get())).arg( QLatin1String(exif2->fDateTimeDigitized.Encode_ISO_8601().Get()));

    //if ((exif1->fDateTimeDigitizedStorageInfo.IsValid() != exif2->fDateTimeDigitizedStorageInfo.IsValid()) ||
    //    ((exif1->fDateTimeDigitizedStorageInfo.IsValid()) &&
    //     (exif1->fDateTimeDigitizedStorageInfo.Format() != exif2->fDateTimeDigitizedStorageInfo.Format()) ||
    //     (exif1->fDateTimeDigitizedStorageInfo.Offset() != exif2->fDateTimeDigitizedStorageInfo.Offset())))
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DateTimeDigitizedStorageInfo");

    if (exif1->fDateTimeOriginal.DateTime() != exif2->fDateTimeOriginal.DateTime())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DateTimeOriginal: %1 %2").arg(QLatin1String(exif1->fDateTimeOriginal.Encode_ISO_8601().Get())).arg(QLatin1String(exif2->fDateTimeOriginal.Encode_ISO_8601().Get()));

    //if ((exif1->fDateTimeOriginalStorageInfo.IsValid() != exif2->fDateTimeOriginalStorageInfo.IsValid()) ||
    //    ((exif1->fDateTimeOriginalStorageInfo.IsValid()) &&
    //     (exif1->fDateTimeOriginalStorageInfo.Format() != exif2->fDateTimeOriginalStorageInfo.Format()) ||
    //     (exif1->fDateTimeOriginalStorageInfo.Offset() != exif2->fDateTimeOriginalStorageInfo.Offset())))
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DateTimeOriginalStorageInfo");

    //if ((exif1->fDateTimeStorageInfo.IsValid() != exif2->fDateTimeStorageInfo.IsValid()) ||
    //    ((exif1->fDateTimeStorageInfo.IsValid()) &&
    //     (exif1->fDateTimeStorageInfo.Format() != exif2->fDateTimeStorageInfo.Format()) ||
    //     (exif1->fDateTimeStorageInfo.Offset() != exif2->fDateTimeStorageInfo.Offset())))
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DateTimeStorageInfo");

    if (!AreSame(exif1->fDigitalZoomRatio.As_real64(), exif2->fDigitalZoomRatio.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DigitalZoomRatio: %1 %2").arg(exif1->fDigitalZoomRatio.As_real64()).arg(exif2->fDigitalZoomRatio.As_real64());

    if (exif1->fExifVersion != exif2->fExifVersion)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExifVersion: %1 %2").arg(exif1->fExifVersion).arg(exif2->fExifVersion);

    if (!AreSame(exif1->fExposureBiasValue.As_real64(), exif2->fExposureBiasValue.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExposureBiasValue: %1 %2").arg(exif1->fExposureBiasValue.As_real64()).arg(exif2->fExposureBiasValue.As_real64());

    if (!AreSame(exif1->fExposureIndex.As_real64(), exif2->fExposureIndex.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExposureIndex: %1 %2").arg(exif1->fExposureIndex.As_real64()).arg(exif2->fExposureIndex.As_real64());

    if (exif1->fExposureMode != exif2->fExposureMode)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExposureMode: %1 %2").arg(exif1->fExposureMode).arg(exif2->fExposureMode);

    if (exif1->fExposureProgram != exif2->fExposureProgram)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExposureProgram: %1 %2").arg(exif1->fExposureProgram).arg(exif2->fExposureProgram);

    if (!AreSame(exif1->fExposureTime.As_real64(), exif2->fExposureTime.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExposureTime: %1 %2").arg(exif1->fExposureTime.As_real64()).arg(exif2->fExposureTime.As_real64());

    if (exif1->fFileSource != exif2->fFileSource)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FileSource: %1 %2").arg(exif1->fFileSource).arg(exif2->fFileSource);

    if (exif1->fFirmware != exif2->fFirmware)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Firmware: %1 %2").arg(QLatin1String(exif1->fFirmware.Get())).arg(QLatin1String(exif2->fFirmware.Get()));

    if (exif1->fFlash != exif2->fFlash)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Flash: %1 %2").arg(exif1->fFlash).arg(exif2->fFlash);

    if (!AreSame(exif1->fFlashCompensation.As_real64(), exif2->fFlashCompensation.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FlashCompensation: %1 %2").arg(exif1->fFlashCompensation.As_real64()).arg(exif2->fFlashCompensation.As_real64());

    if (exif1->fFlashMask != exif2->fFlashMask)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FlashMask: %1 %2").arg(exif1->fFlashMask).arg(exif2->fFlashMask);

    if (exif1->fFlashPixVersion != exif2->fFlashPixVersion)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FlashPixVersion: %1 %2").arg(exif1->fFlashPixVersion).arg(exif2->fFlashPixVersion);

    if (!AreSame(exif1->fFNumber.As_real64(), exif2->fFNumber.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FNumber: %1 %2").arg(exif1->fFNumber.As_real64()).arg(exif2->fFNumber.As_real64());

    if (!AreSame(exif1->fFocalLength.As_real64(), exif2->fFocalLength.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FocalLength: %1 %2").arg(exif1->fFocalLength.As_real64()).arg(exif2->fFocalLength.As_real64());

    if (exif1->fFocalLengthIn35mmFilm != exif2->fFocalLengthIn35mmFilm)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FocalLengthIn35mmFilm: %1 %2").arg(exif1->fFocalLengthIn35mmFilm).arg(exif2->fFocalLengthIn35mmFilm);

    if (exif1->fFocalPlaneResolutionUnit != exif2->fFocalPlaneResolutionUnit)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FocalPlaneResolutionUnit: %1 %2").arg(exif1->fFocalPlaneResolutionUnit).arg(exif2->fFocalPlaneResolutionUnit);

    if (!AreSame(exif1->fFocalPlaneXResolution.As_real64(), exif2->fFocalPlaneXResolution.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FocalPlaneXResolution: %1 %2").arg(exif1->fFocalPlaneXResolution.As_real64()).arg(exif2->fFocalPlaneXResolution.As_real64());

    if (!AreSame(exif1->fFocalPlaneYResolution.As_real64(), exif2->fFocalPlaneYResolution.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FocalPlaneYResolution: %1 %2").arg(exif1->fFocalPlaneYResolution.As_real64()).arg(exif2->fFocalPlaneYResolution.As_real64());

    if (exif1->fGainControl != exif2->fGainControl)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GainControl: %1 %2").arg(exif1->fGainControl).arg(exif2->fGainControl);

    if (!AreSame(exif1->fGamma.As_real64(), exif2->fGamma.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Gamma: %1 %2").arg(exif1->fGamma.As_real64()).arg(exif2->fGamma.As_real64());

    if (!AreSame(exif1->fGPSAltitude.As_real64(), exif2->fGPSAltitude.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSAltitude: %1 %2").arg(exif1->fGPSAltitude.As_real64()).arg(exif2->fGPSAltitude.As_real64());

    if (exif1->fGPSAltitudeRef != exif2->fGPSAltitudeRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSAltitudeRef: %1 %2").arg(exif1->fGPSAltitudeRef).arg(exif2->fGPSAltitudeRef);

    if (exif1->fGPSAreaInformation != exif2->fGPSAreaInformation)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSAreaInformation: %1 %2").arg(QLatin1String(exif1->fGPSAreaInformation.Get())).arg(QLatin1String(exif2->fGPSAreaInformation.Get()));

    if (exif1->fGPSDateStamp != exif2->fGPSDateStamp)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDateStamp: %1 %2").arg(QLatin1String(exif1->fGPSDateStamp.Get())).arg(QLatin1String(exif2->fGPSDateStamp.Get()));

    if (!AreSame(exif1->fGPSDestBearing.As_real64(), exif2->fGPSDestBearing.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestBearing: %1 %2").arg(exif1->fGPSDestBearing.As_real64()).arg(exif2->fGPSDestBearing.As_real64());

    if (exif1->fGPSDestBearingRef != exif2->fGPSDestBearingRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestBearingRef: %1 %2").arg(QLatin1String(exif1->fGPSDestBearingRef.Get())).arg(QLatin1String(exif2->fGPSDestBearingRef.Get()));

    if (!AreSame(exif1->fGPSDestDistance.As_real64(), exif2->fGPSDestDistance.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestDistance: %1 %2").arg(exif1->fGPSDestDistance.As_real64()).arg(exif2->fGPSDestDistance.As_real64());

    if (exif1->fGPSDestDistanceRef != exif2->fGPSDestDistanceRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestDistanceRef: %1 %2").arg(QLatin1String(exif1->fGPSDestDistanceRef.Get())).arg(QLatin1String(exif2->fGPSDestDistanceRef.Get()));

    if ((!AreSame(exif1->fGPSDestLatitude[0].As_real64(), exif2->fGPSDestLatitude[0].As_real64())) ||
        (!AreSame(exif1->fGPSDestLatitude[1].As_real64(), exif2->fGPSDestLatitude[1].As_real64())) ||
        (!AreSame(exif1->fGPSDestLatitude[2].As_real64(), exif2->fGPSDestLatitude[2].As_real64())))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestLatitude");

    if (exif1->fGPSDestLatitudeRef != exif2->fGPSDestLatitudeRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestLatitudeRef: %1 %2").arg(QLatin1String(exif1->fGPSDestLatitudeRef.Get())).arg(QLatin1String(exif2->fGPSDestLatitudeRef.Get()));

    if ((!AreSame(exif1->fGPSDestLongitude[0].As_real64(), exif2->fGPSDestLongitude[0].As_real64())) ||
        (!AreSame(exif1->fGPSDestLongitude[1].As_real64(), exif2->fGPSDestLongitude[1].As_real64())) ||
        (!AreSame(exif1->fGPSDestLongitude[2].As_real64(), exif2->fGPSDestLongitude[2].As_real64())))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestLongitude");

    if (exif1->fGPSDestLongitudeRef != exif2->fGPSDestLongitudeRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDestLongitudeRef: %1 %2").arg(QLatin1String(exif1->fGPSDestLongitudeRef.Get())).arg(QLatin1String(exif2->fGPSDestLongitudeRef.Get()));

    if (exif1->fGPSDifferential != exif2->fGPSDifferential)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDifferential: %1 %2").arg(exif1->fGPSDifferential).arg(exif2->fGPSDifferential);

    if (!AreSame(exif1->fGPSDOP.As_real64(), exif2->fGPSDOP.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSDOP: %1 %2").arg(exif1->fGPSDOP.As_real64()).arg(exif2->fGPSDOP.As_real64());

    if (!AreSame(exif1->fGPSImgDirection.As_real64(), exif2->fGPSImgDirection.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSImgDirection: %1 %2").arg(exif1->fGPSImgDirection.As_real64()).arg(exif2->fGPSImgDirection.As_real64());

    if (exif1->fGPSImgDirectionRef != exif2->fGPSImgDirectionRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSImgDirectionRef: %1 %2").arg(QLatin1String(exif1->fGPSImgDirectionRef.Get())).arg(QLatin1String(exif2->fGPSImgDirectionRef.Get()));

    if ((!AreSame(exif1->fGPSLatitude[0].As_real64(), exif2->fGPSLatitude[0].As_real64())) ||
        (!AreSame(exif1->fGPSLatitude[1].As_real64(), exif2->fGPSLatitude[1].As_real64())) ||
        (!AreSame(exif1->fGPSLatitude[2].As_real64(), exif2->fGPSLatitude[2].As_real64())))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSLatitude");

    if (exif1->fGPSLatitudeRef != exif2->fGPSLatitudeRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSLatitudeRef: %1 %2").arg(QLatin1String(exif1->fGPSLatitudeRef.Get())).arg(QLatin1String(exif2->fGPSLatitudeRef.Get()));

    if ((!AreSame(exif1->fGPSLongitude[0].As_real64(), exif2->fGPSLongitude[0].As_real64())) ||
        (!AreSame(exif1->fGPSLongitude[1].As_real64(), exif2->fGPSLongitude[1].As_real64())) ||
        (!AreSame(exif1->fGPSLongitude[2].As_real64(), exif2->fGPSLongitude[2].As_real64())))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSLongitude");

    if (exif1->fGPSLongitudeRef != exif2->fGPSLongitudeRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSLongitudeRef: %1 %2").arg(QLatin1String(exif1->fGPSLongitudeRef.Get())).arg(QLatin1String(exif2->fGPSLongitudeRef.Get()));

    if (exif1->fGPSMapDatum != exif2->fGPSMapDatum)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSMapDatum: %1 %2").arg(QLatin1String(exif1->fGPSMapDatum.Get())).arg(QLatin1String(exif2->fGPSMapDatum.Get()));

    if (exif1->fGPSMeasureMode != exif2->fGPSMeasureMode)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSMeasureMode: %1 %2").arg(QLatin1String(exif1->fGPSMeasureMode.Get())).arg(QLatin1String(exif2->fGPSMeasureMode.Get()));

    if (exif1->fGPSProcessingMethod != exif2->fGPSProcessingMethod)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSProcessingMethod: %1 %2").arg(QLatin1String(exif1->fGPSProcessingMethod.Get())).arg(QLatin1String(exif2->fGPSProcessingMethod.Get()));

    if (exif1->fGPSSatellites != exif2->fGPSSatellites)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSSatellites: %1 %2").arg(QLatin1String(exif1->fGPSSatellites.Get())).arg(QLatin1String(exif2->fGPSSatellites.Get()));

    if (!AreSame(exif1->fGPSSpeed.As_real64(), exif2->fGPSSpeed.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSSpeed: %1 %2").arg(exif1->fGPSSpeed.As_real64()).arg(exif2->fGPSSpeed.As_real64());

    if (exif1->fGPSSpeedRef != exif2->fGPSSpeedRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSSpeedRef: %1 %2").arg(QLatin1String(exif1->fGPSSpeedRef.Get())).arg(QLatin1String(exif2->fGPSSpeedRef.Get()));

    if (exif1->fGPSStatus != exif2->fGPSStatus)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSStatus: %1 %2").arg(QLatin1String(exif1->fGPSStatus.Get())).arg(QLatin1String(exif2->fGPSStatus.Get()));

    if ((!AreSame(exif1->fGPSTimeStamp[0].As_real64(), exif2->fGPSTimeStamp[0].As_real64())) ||
        (!AreSame(exif1->fGPSTimeStamp[1].As_real64(), exif2->fGPSTimeStamp[1].As_real64())) ||
        (!AreSame(exif1->fGPSTimeStamp[2].As_real64(), exif2->fGPSTimeStamp[2].As_real64())))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSTimeStamp");

    if (!AreSame(exif1->fGPSTrack.As_real64(), exif2->fGPSTrack.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSTrack: %1 %2").arg(exif1->fGPSTrack.As_real64()).arg(exif2->fGPSTrack.As_real64());

    if (exif1->fGPSTrackRef != exif2->fGPSTrackRef)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSTrackRef: %1 %2").arg(QLatin1String(exif1->fGPSTrackRef.Get())).arg(QLatin1String(exif2->fGPSTrackRef.Get()));

    if (exif1->fGPSVersionID != exif2->fGPSVersionID)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    GPSVersionID %1 %2").arg(exif1->fGPSVersionID).arg(exif2->fGPSVersionID);

    if (exif1->fImageDescription != exif2->fImageDescription)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ImageDescription: %1 %2").arg(QLatin1String(exif1->fImageDescription.Get())).arg(QLatin1String(exif2->fImageDescription.Get()));

    if (exif1->fImageNumber != exif2->fImageNumber)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ImageNumber: %1 %2").arg(exif1->fImageNumber).arg(exif2->fImageNumber);

    if (exif1->fImageUniqueID != exif2->fImageUniqueID)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ImageUniqueID");

    if (exif1->fInteroperabilityIndex != exif2->fInteroperabilityIndex)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    InteroperabilityIndex: %1 %2").arg(QLatin1String(exif1->fInteroperabilityIndex.Get())).arg(QLatin1String(exif2->fInteroperabilityIndex.Get()));

    if (exif1->fInteroperabilityVersion != exif2->fInteroperabilityVersion)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    InteroperabilityVersion: %1 %2").arg(exif1->fInteroperabilityVersion).arg(exif2->fInteroperabilityVersion);

    if ((exif1->fISOSpeedRatings[0] != exif2->fISOSpeedRatings[0]) ||
        (exif1->fISOSpeedRatings[1] != exif2->fISOSpeedRatings[1]) ||
        (exif1->fISOSpeedRatings[2] != exif2->fISOSpeedRatings[2]))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ISOSpeedRatings");

    if (exif1->fLensID != exif2->fLensID)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LensID: %1 %2").arg(QLatin1String(exif1->fLensID.Get())).arg(QLatin1String(exif2->fLensID.Get()));

    if ((!AreSame(exif1->fLensInfo[0].As_real64(), exif2->fLensInfo[0].As_real64())) ||
        (!AreSame(exif1->fLensInfo[1].As_real64(), exif2->fLensInfo[1].As_real64())) ||
        (!AreSame(exif1->fLensInfo[2].As_real64(), exif2->fLensInfo[2].As_real64())) ||
        (!AreSame(exif1->fLensInfo[3].As_real64(), exif2->fLensInfo[3].As_real64())))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LensInfo");

    if (exif1->fLensName != exif2->fLensName)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LensName: %1 %2").arg(QLatin1String(exif1->fLensName.Get())).arg(QLatin1String(exif2->fLensName.Get()));

    if (exif1->fLensSerialNumber != exif2->fLensSerialNumber)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LensSerialNumber: %1 %2").arg(QLatin1String(exif1->fLensSerialNumber.Get())).arg(QLatin1String(exif2->fLensSerialNumber.Get()));

    if (exif1->fLightSource != exif2->fLightSource)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LightSource: %1 %2").arg(exif1->fLightSource).arg(exif2->fLightSource);

    if (exif1->fMake != exif2->fMake)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Make: %1 %2").arg(QLatin1String(exif1->fMake.Get())).arg(QLatin1String(exif2->fMake.Get()));

    if (!AreSame(exif1->fMaxApertureValue.As_real64(), exif2->fMaxApertureValue.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    MaxApertureValue: %1 %2").arg(exif1->fMaxApertureValue.As_real64()).arg(exif2->fMaxApertureValue.As_real64());

    if (exif1->fMeteringMode != exif2->fMeteringMode)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    MeteringMode: %1 %2").arg(exif1->fMeteringMode).arg(exif2->fMeteringMode);

    if (exif1->fModel != exif2->fModel)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Model: %1 %2").arg(QLatin1String(exif1->fModel.Get())).arg(QLatin1String(exif2->fModel.Get()));

    if (exif1->fOwnerName != exif2->fOwnerName)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OwnerName: %1 %2").arg(QLatin1String(exif1->fOwnerName.Get())).arg(QLatin1String(exif2->fOwnerName.Get()));

    if (exif1->fPixelXDimension != exif2->fPixelXDimension)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PixelXDimension: %1 %2").arg(exif1->fPixelXDimension).arg(exif2->fPixelXDimension);

    if (exif1->fPixelYDimension != exif2->fPixelYDimension)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PixelYDimension: %1 %2").arg(exif1->fPixelYDimension).arg(exif2->fPixelYDimension);

    if (exif1->fRelatedImageFileFormat != exif2->fRelatedImageFileFormat)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RelatedImageFileFormat: %1 %2").arg(QLatin1String(exif1->fRelatedImageFileFormat.Get())).arg(QLatin1String(exif2->fRelatedImageFileFormat.Get()));

    if (exif1->fRelatedImageLength != exif2->fRelatedImageLength)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RelatedImageLength: %1 %2").arg(exif1->fRelatedImageLength).arg(exif2->fRelatedImageLength);

    if (exif1->fRelatedImageWidth != exif2->fRelatedImageWidth)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RelatedImageWidth: %1 %2").arg(exif1->fRelatedImageWidth).arg(exif2->fRelatedImageWidth);

    if (exif1->fSaturation != exif2->fSaturation)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Saturation: %1 %2").arg(exif1->fSaturation).arg(exif2->fSaturation);

    if (exif1->fSceneCaptureType != exif2->fSceneCaptureType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SceneCaptureType: %1 %2").arg(exif1->fSceneCaptureType).arg(exif2->fSceneCaptureType);

    if (exif1->fSceneType != exif2->fSceneType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SceneType: %1 %2").arg(exif1->fSceneType).arg(exif2->fSceneType);

    if (exif1->fSelfTimerMode != exif2->fSelfTimerMode)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SelfTimerMode: %1 %2").arg(exif1->fSelfTimerMode).arg(exif2->fSelfTimerMode);

    if (exif1->fSensingMethod != exif2->fSensingMethod)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SensingMethod: %1 %2").arg(exif1->fSensingMethod).arg(exif2->fSensingMethod);

    if (exif1->fSharpness != exif2->fSharpness)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Sharpness: %1 %2").arg(exif1->fSharpness).arg(exif2->fSharpness);

    if (!AreSame(exif1->fShutterSpeedValue.As_real64(), exif2->fShutterSpeedValue.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ShutterSpeedValue: %1 %2").arg(exif1->fShutterSpeedValue.As_real64()).arg(exif2->fShutterSpeedValue.As_real64());

    if (exif1->fSoftware != exif2->fSoftware)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Software: %1 %2").arg(QLatin1String(exif1->fSoftware.Get())).arg(QLatin1String(exif2->fSoftware.Get()));

    if (0 != memcmp(exif1->fSubjectArea, exif2->fSubjectArea, 4 * sizeof(uint32)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubjectArea");

    if (exif1->fSubjectAreaCount != exif2->fSubjectAreaCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubjectAreaCount: %1 %2").arg(exif1->fSubjectAreaCount).arg(exif2->fSubjectAreaCount);

    if (!AreSame(exif1->fSubjectDistance.As_real64(), exif2->fSubjectDistance.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubjectDistance: %1 %2").arg(exif1->fSubjectDistance.As_real64()).arg(exif2->fSubjectDistance.As_real64());

    if (exif1->fSubjectDistanceRange != exif2->fSubjectDistanceRange)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubjectDistanceRange: %1 %2").arg(exif1->fSubjectDistanceRange).arg(exif2->fSubjectDistanceRange);

    if (exif1->fTIFF_EP_StandardID != exif2->fTIFF_EP_StandardID)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TIFF_EP_StandardID: %1 %2").arg(exif1->fTIFF_EP_StandardID).arg(exif2->fTIFF_EP_StandardID);

    if (exif1->fUserComment != exif2->fUserComment)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    UserComment: %1 %2").arg(QLatin1String(exif1->fUserComment.Get())).arg(QLatin1String(exif2->fUserComment.Get()));

    if (exif1->fWhiteBalance != exif2->fWhiteBalance)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    WhiteBalance: %1 %2").arg(exif1->fWhiteBalance).arg(exif2->fWhiteBalance);
}

void compareIfd(dng_ifd* const ifd1, dng_ifd* const ifd2)
{
    if (ifd1->fActiveArea != ifd2->fActiveArea)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ActiveArea (t/l/b/r): %1/%2/%3/%4 %5/%6/%7/%8",
               ifd1->fActiveArea.t).arg(ifd1->fActiveArea.l).arg(ifd1->fActiveArea.b).arg(ifd1->fActiveArea.r,
               ifd2->fActiveArea.t).arg(ifd2->fActiveArea.l).arg(ifd2->fActiveArea.b).arg(ifd2->fActiveArea.r);

    if (!AreSame(ifd1->fAntiAliasStrength.As_real64(), ifd2->fAntiAliasStrength.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    AntiAliasStrength: %1 %2").arg(ifd1->fAntiAliasStrength.As_real64()).arg(ifd2->fAntiAliasStrength.As_real64());

    if (ifd1->fBayerGreenSplit != ifd2->fBayerGreenSplit)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BayerGreenSplit: %1 %2").arg(ifd1->fBayerGreenSplit).arg(ifd2->fBayerGreenSplit);

    if (!AreSame(ifd1->fBestQualityScale.As_real64(), ifd2->fBestQualityScale.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BestQualityScale: %1 %2").arg(ifd1->fBestQualityScale.As_real64()).arg(ifd2->fBestQualityScale.As_real64());

    if (0 != memcmp(ifd1->fBitsPerSample, ifd2->fBitsPerSample, kMaxSamplesPerPixel * sizeof(uint32)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BitsPerSample");

    if (0 != memcmp(ifd1->fBlackLevel, ifd2->fBlackLevel, kMaxBlackPattern * kMaxBlackPattern * kMaxSamplesPerPixel * sizeof(real64)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevel");

    if (ifd1->fBlackLevelDeltaHCount != ifd2->fBlackLevelDeltaHCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelDeltaHCount: %1 %2").arg(ifd1->fBlackLevelDeltaHCount).arg(ifd2->fBlackLevelDeltaHCount);

    //if (ifd1->fBlackLevelDeltaHOffset != ifd2->fBlackLevelDeltaHOffset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelDeltaHOffset: %lld %lld").arg(ifd1->fBlackLevelDeltaHOffset).arg(ifd2->fBlackLevelDeltaHOffset);

    if (ifd1->fBlackLevelDeltaHType != ifd2->fBlackLevelDeltaHType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelDeltaHType: %1 %2").arg(ifd1->fBlackLevelDeltaHType).arg(ifd2->fBlackLevelDeltaHType);

    if (ifd1->fBlackLevelDeltaVCount != ifd2->fBlackLevelDeltaVCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelDeltaVCount: %1 %2").arg(ifd1->fBlackLevelDeltaVCount).arg(ifd2->fBlackLevelDeltaVCount);

    //if (ifd1->fBlackLevelDeltaVOffset != ifd2->fBlackLevelDeltaVOffset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelDeltaVOffset: %lld %lld").arg(ifd1->fBlackLevelDeltaVOffset).arg(ifd2->fBlackLevelDeltaVOffset);

    if (ifd1->fBlackLevelDeltaVType != ifd2->fBlackLevelDeltaVType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelDeltaVType: %1 %2").arg(ifd1->fBlackLevelDeltaVType).arg(ifd2->fBlackLevelDeltaVType);

    if (ifd1->fBlackLevelRepeatCols != ifd2->fBlackLevelRepeatCols)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelRepeatCols: %1 %2").arg(ifd1->fBlackLevelRepeatCols).arg(ifd2->fBlackLevelRepeatCols);

    if (ifd1->fBlackLevelRepeatRows != ifd2->fBlackLevelRepeatRows)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BlackLevelRepeatRows: %1 %2").arg(ifd1->fBlackLevelRepeatRows).arg(ifd2->fBlackLevelRepeatRows);

    if (!AreSame(ifd1->fChromaBlurRadius.As_real64(), ifd2->fChromaBlurRadius.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ChromaBlurRadius: %1 %2").arg(ifd1->fChromaBlurRadius.As_real64()).arg(ifd2->fChromaBlurRadius.As_real64());

    if (ifd1->fCompression != ifd2->fCompression)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Compression: %1 %2").arg(ifd1->fCompression).arg(ifd2->fCompression);

    if (!AreSame(ifd1->fDefaultCropOriginH.As_real64(), ifd2->fDefaultCropOriginH.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropOriginH: %1 %2").arg(ifd1->fDefaultCropOriginH.As_real64()).arg(ifd2->fDefaultCropOriginH.As_real64());

    if (!AreSame(ifd1->fDefaultCropOriginV.As_real64(), ifd2->fDefaultCropOriginV.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropOriginV: %1 %2").arg(ifd1->fDefaultCropOriginV.As_real64()).arg(ifd2->fDefaultCropOriginV.As_real64());

    if (!AreSame(ifd1->fDefaultCropSizeH.As_real64(), ifd2->fDefaultCropSizeH.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropSizeH: %1 %2").arg(ifd1->fDefaultCropSizeH.As_real64()).arg(ifd2->fDefaultCropSizeH.As_real64());

    if (!AreSame(ifd1->fDefaultCropSizeV.As_real64(), ifd2->fDefaultCropSizeV.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropSizeV: %1 %2").arg(ifd1->fDefaultCropSizeV.As_real64()).arg(ifd2->fDefaultCropSizeV.As_real64());

    if (!AreSame(ifd1->fDefaultScaleH.As_real64(), ifd2->fDefaultScaleH.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultScaleH: %1 %2").arg(ifd1->fDefaultScaleH.As_real64()).arg(ifd2->fDefaultScaleH.As_real64());

    if (!AreSame(ifd1->fDefaultScaleV.As_real64(), ifd2->fDefaultScaleV.As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultScaleV: %1 %2").arg(ifd1->fDefaultScaleV.As_real64()).arg(ifd2->fDefaultScaleV.As_real64());

    if (0 != memcmp(ifd1->fExtraSamples, ifd2->fExtraSamples, kMaxSamplesPerPixel * sizeof(uint32)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExtraSamples");

    if (ifd1->fExtraSamplesCount != ifd2->fExtraSamplesCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ExtraSamplesCount: %1 %2").arg(ifd1->fExtraSamplesCount).arg(ifd2->fExtraSamplesCount);

    if (ifd1->fFillOrder != ifd2->fFillOrder)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    FillOrder: %1 %2").arg(ifd1->fFillOrder).arg(ifd2->fFillOrder);

    if (ifd1->fImageLength != ifd2->fImageLength)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ImageLength: %1 %2").arg(ifd1->fImageLength).arg(ifd2->fImageLength);

    if (ifd1->fImageWidth != ifd2->fImageWidth)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ImageWidth: %1 %2").arg(ifd1->fImageWidth).arg(ifd2->fImageWidth);

    if (ifd1->fJPEGInterchangeFormat != ifd2->fJPEGInterchangeFormat)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    JPEGInterchangeFormat: %1 %2").arg(ifd1->fJPEGInterchangeFormat).arg(ifd2->fJPEGInterchangeFormat);

    if (ifd1->fJPEGInterchangeFormatLength != ifd2->fJPEGInterchangeFormatLength)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    JPEGInterchangeFormatLength: %1 %2").arg(ifd1->fJPEGInterchangeFormatLength).arg(ifd2->fJPEGInterchangeFormatLength);

    if (ifd1->fJPEGTablesCount != ifd2->fJPEGTablesCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    JPEGTablesCount: %1 %2").arg(ifd1->fJPEGTablesCount).arg(ifd2->fJPEGTablesCount);

    //if (ifd1->fJPEGTablesOffset != ifd2->fJPEGTablesOffset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    JPEGTablesOffset: %lld %lld").arg(ifd1->fJPEGTablesOffset).arg(ifd2->fJPEGTablesOffset);

    if (ifd1->fLinearizationTableCount != ifd2->fLinearizationTableCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LinearizationTableCount: %1 %2").arg(ifd1->fLinearizationTableCount).arg(ifd2->fLinearizationTableCount);

    //if (ifd1->fLinearizationTableOffset != ifd2->fLinearizationTableOffset)
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LinearizationTableOffset: %1 %2").arg(ifd1->fLinearizationTableOffset).arg(ifd2->fLinearizationTableOffset);

    if (ifd1->fLinearizationTableType != ifd2->fLinearizationTableType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LinearizationTableType: %1 %2").arg(ifd1->fLinearizationTableType).arg(ifd2->fLinearizationTableType);

    if (ifd1->fLosslessJPEGBug16 != ifd2->fLosslessJPEGBug16)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LosslessJPEGBug16: %1 %2").arg(ifd1->fLosslessJPEGBug16).arg(ifd2->fLosslessJPEGBug16);

    for (unsigned int i = 0 ; i < kMaxMaskedAreas ; i++)
    {
        if (ifd1->fMaskedArea[i] != ifd2->fMaskedArea[i])
        {
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    MaskedArea");
        }
    }

    if (ifd1->fMaskedAreaCount != ifd2->fMaskedAreaCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    MaskedAreaCount: %1 %2").arg(ifd1->fMaskedAreaCount).arg(ifd2->fMaskedAreaCount);

    if (ifd1->fNewSubFileType != ifd2->fNewSubFileType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    NewSubFileType: %1 %2").arg(ifd1->fNewSubFileType).arg(ifd2->fNewSubFileType);

    if (ifd1->fNextIFD != ifd2->fNextIFD)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    NextIFD: %1 %2").arg(ifd1->fNextIFD).arg(ifd2->fNextIFD);

    if (ifd1->fOpcodeList1Count != ifd2->fOpcodeList1Count)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OpcodeList1Count: %1 %2").arg(ifd1->fOpcodeList1Count).arg(ifd2->fOpcodeList1Count);

    //if (ifd1->fOpcodeList1Offset != ifd2->fOpcodeList1Offset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OpcodeList1Offset");

    if (ifd1->fOpcodeList2Count != ifd2->fOpcodeList2Count)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OpcodeList2Count: %1 %2").arg(ifd1->fOpcodeList2Count).arg(ifd2->fOpcodeList2Count);

    //if (ifd1->fOpcodeList2Offset != ifd2->fOpcodeList2Offset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OpcodeList2Offset");

    if (ifd1->fOpcodeList3Count != ifd2->fOpcodeList3Count)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OpcodeList3Count: %1 %2").arg(ifd1->fOpcodeList3Count).arg(ifd2->fOpcodeList3Count);

    //if (ifd1->fOpcodeList3Offset != ifd2->fOpcodeList3Offset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OpcodeList3Offset");

    if (ifd1->fOrientation != ifd2->fOrientation)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Orientation: %1 %2").arg(ifd1->fOrientation).arg(ifd2->fOrientation);

    if (ifd1->fOrientationBigEndian != ifd2->fOrientationBigEndian)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OrientationBigEndian: %1 %2").arg(ifd1->fOrientationBigEndian).arg(ifd2->fOrientationBigEndian);

    //if (ifd1->fOrientationOffset != ifd2->fOrientationOffset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OrientationOffset: %lld %lld").arg(ifd1->fOrientationOffset).arg(ifd2->fOrientationOffset);

    if (ifd1->fOrientationType != ifd2->fOrientationType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OrientationType: %1 %2").arg(ifd1->fOrientationType).arg(ifd2->fOrientationType);

    if (ifd1->fPhotometricInterpretation != ifd2->fPhotometricInterpretation)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PhotometricInterpretation: %1 %2").arg(ifd1->fPhotometricInterpretation).arg(ifd2->fPhotometricInterpretation);

    if (ifd1->fPlanarConfiguration != ifd2->fPlanarConfiguration)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PlanarConfiguration: %1 %2").arg(ifd1->fPlanarConfiguration).arg(ifd2->fPlanarConfiguration);

    if (ifd1->fPredictor != ifd2->fPredictor)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Predictor: %1 %2").arg(ifd1->fPredictor).arg(ifd2->fPredictor);

    if (ifd1->fPreviewInfo.fApplicationName != ifd2->fPreviewInfo.fApplicationName)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PreviewInfo ApplicationName: %1 %2").arg(QLatin1String(ifd1->fPreviewInfo.fApplicationName.Get())).arg(QLatin1String(ifd2->fPreviewInfo.fApplicationName.Get()));

    if (ifd1->fPreviewInfo.fApplicationVersion != ifd2->fPreviewInfo.fApplicationVersion)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PreviewInfo ApplicationVersion: %1 %2").arg(QLatin1String(ifd1->fPreviewInfo.fApplicationVersion.Get())).arg(QLatin1String(ifd2->fPreviewInfo.fApplicationVersion.Get()));

    if (ifd1->fPreviewInfo.fColorSpace != ifd2->fPreviewInfo.fColorSpace)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PreviewInfo ColorSpace");

    if (ifd1->fPreviewInfo.fDateTime != ifd2->fPreviewInfo.fDateTime)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PreviewInfo DateTime");

    if (ifd1->fPreviewInfo.fIsPrimary != ifd2->fPreviewInfo.fIsPrimary)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PreviewInfo");

    if (ifd1->fPreviewInfo.fSettingsDigest != ifd2->fPreviewInfo.fSettingsDigest)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PreviewInfo");

    if (ifd1->fPreviewInfo.fSettingsName != ifd2->fPreviewInfo.fSettingsName)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PreviewInfo");

    if (0 != memcmp(ifd1->fReferenceBlackWhite, ifd2->fReferenceBlackWhite, 6 * sizeof(real64)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ReferenceBlackWhite");

    if (ifd1->fResolutionUnit != ifd2->fResolutionUnit)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ResolutionUnit: %1 %2").arg(ifd1->fResolutionUnit).arg(ifd2->fResolutionUnit);

    if (ifd1->fRowInterleaveFactor != ifd2->fRowInterleaveFactor)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RowInterleaveFactor: %1 %2").arg(ifd1->fRowInterleaveFactor).arg(ifd2->fRowInterleaveFactor);

    if (ifd1->fSampleBitShift != ifd2->fSampleBitShift)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SampleBitShift: %1 %2").arg(ifd1->fSampleBitShift).arg(ifd2->fSampleBitShift);

    if (0 != memcmp(ifd1->fSampleFormat, ifd2->fSampleFormat, kMaxSamplesPerPixel * sizeof(uint32)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SampleFormat");

    if (ifd1->fSamplesPerPixel != ifd2->fSamplesPerPixel)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SamplesPerPixel: %1 %2").arg(ifd1->fSamplesPerPixel).arg(ifd2->fSamplesPerPixel);

    if (ifd1->fSubIFDsCount != ifd2->fSubIFDsCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubIFDsCount: %1 %2").arg(ifd1->fSubIFDsCount).arg(ifd2->fSubIFDsCount);

    //if (ifd1->fSubIFDsOffset != ifd2->fSubIFDsOffset)
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubIFDsOffset");

    if (ifd1->fSubTileBlockCols != ifd2->fSubTileBlockCols)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubTileBlockCols: %1 %2").arg(ifd1->fSubTileBlockCols).arg(ifd2->fSubTileBlockCols);

    if (ifd1->fSubTileBlockRows != ifd2->fSubTileBlockRows)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SubTileBlockRows: %1 %2").arg(ifd1->fSubTileBlockRows).arg(ifd2->fSubTileBlockRows);

    //if (ifd1->fThisIFD != ifd2->fThisIFD)
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ThisIFD:");

    if (0 != memcmp(ifd1->fTileByteCount, ifd2->fTileByteCount, dng_ifd::kMaxTileInfo * sizeof(uint32)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileByteCount");

    if (ifd1->fTileByteCountsCount != ifd2->fTileByteCountsCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileByteCountsCount: %1 %2").arg(ifd1->fTileByteCountsCount).arg(ifd2->fTileByteCountsCount);

    //if (ifd1->fTileByteCountsOffset != ifd2->fTileByteCountsOffset)
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileByteCountsOffset");

    if (ifd1->fTileByteCountsType != ifd2->fTileByteCountsType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileByteCountsType: %1 %2").arg(ifd1->fTileByteCountsType).arg(ifd2->fTileByteCountsType);

    if (ifd1->fTileLength != ifd2->fTileLength)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileLength: %1 %2").arg(ifd1->fTileLength).arg(ifd2->fTileLength);

    //if (ifd1->fTileOffset != ifd2->fTileOffset)
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileOffset");

    if (ifd1->fTileOffsetsCount != ifd2->fTileOffsetsCount)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileOffsetsCount: %1 %2").arg(ifd1->fTileOffsetsCount).arg(ifd2->fTileOffsetsCount);

    //if (ifd1->fTileOffsetsOffset != ifd2->fTileOffsetsOffset)
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileOffsetsOffset");

    if (ifd1->fTileOffsetsType != ifd2->fTileOffsetsType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileOffsetsType: %1 %2").arg(ifd1->fTileOffsetsType).arg(ifd2->fTileOffsetsType);

    if (ifd1->fTileWidth != ifd2->fTileWidth)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    TileWidth: %1 %2").arg(ifd1->fTileWidth).arg(ifd2->fTileWidth);

    if (ifd1->fUsesNewSubFileType != ifd2->fUsesNewSubFileType)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    UsesNewSubFileType: %1 %2").arg(ifd1->fUsesNewSubFileType).arg(ifd2->fUsesNewSubFileType);

    if (ifd1->fUsesStrips != ifd2->fUsesStrips)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    UsesStrips: %1 %2").arg(ifd1->fUsesStrips).arg(ifd2->fUsesStrips);

    if (ifd1->fUsesTiles != ifd2->fUsesTiles)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    UsesTiles: %1 %2").arg(ifd1->fUsesTiles).arg(ifd2->fUsesTiles);

    if (0 != memcmp(ifd1->fWhiteLevel, ifd2->fWhiteLevel, kMaxSamplesPerPixel * sizeof(real64)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    WhiteLevel");

    if (!AreSame(ifd1->fXResolution, ifd2->fXResolution))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    XResolution: %1 %2").arg(ifd1->fXResolution).arg(ifd2->fXResolution);

    if (!AreSame(ifd1->fYCbCrCoefficientB, ifd2->fYCbCrCoefficientB))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    YCbCrCoefficientB: %1 %2").arg(ifd1->fYCbCrCoefficientB).arg(ifd2->fYCbCrCoefficientB);

    if (!AreSame(ifd1->fYCbCrCoefficientG, ifd2->fYCbCrCoefficientG))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    YCbCrCoefficientG: %1 %2").arg(ifd1->fYCbCrCoefficientG).arg(ifd2->fYCbCrCoefficientG);

    if (!AreSame(ifd1->fYCbCrCoefficientR, ifd2->fYCbCrCoefficientR))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    YCbCrCoefficientR: %1 %2").arg(ifd1->fYCbCrCoefficientR).arg(ifd2->fYCbCrCoefficientR);

    if (ifd1->fYCbCrPositioning != ifd2->fYCbCrPositioning)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    YCbCrPositioning: %1 %2").arg(ifd1->fYCbCrPositioning).arg(ifd2->fYCbCrPositioning);

    if (ifd1->fYCbCrSubSampleH != ifd2->fYCbCrSubSampleH)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    YCbCrSubSampleH: %1 %2").arg(ifd1->fYCbCrSubSampleH).arg(ifd2->fYCbCrSubSampleH);

    if (ifd1->fYCbCrSubSampleV != ifd2->fYCbCrSubSampleV)
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    YCbCrSubSampleV: %1 %2").arg(ifd1->fYCbCrSubSampleV).arg(ifd2->fYCbCrSubSampleV);

    if (!AreSame(ifd1->fYResolution, ifd2->fYResolution))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    YResolution: %1 %2").arg(ifd1->fYResolution).arg(ifd2->fYResolution);
}

void compareNegative(dng_negative* const negative1, dng_negative* const negative2)
{
    if (negative1->AsShotProfileName() != negative2->AsShotProfileName())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    AsShotProfileName: %1 %2").arg(QLatin1String(negative1->AsShotProfileName().Get())).arg(QLatin1String(negative2->AsShotProfileName().Get()));

    if (!AreSame(negative1->AntiAliasStrength().As_real64(), negative2->AntiAliasStrength().As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BaselineExposure: %1 %2").arg(negative1->AntiAliasStrength().As_real64()).arg(negative2->AntiAliasStrength().As_real64());

    if (negative1->AspectRatio() != negative2->AspectRatio())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    AspectRatio: %1 %2").arg(negative1->AspectRatio()).arg(negative2->AspectRatio());

    if (negative1->BaselineExposure() != negative2->BaselineExposure())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BaselineExposure: %1 %2").arg(negative1->BaselineExposure()).arg(negative2->BaselineExposure());

    if (negative1->BaselineNoise() != negative2->BaselineNoise())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BaselineNoise: %1 %2").arg(negative1->BaselineNoise()).arg(negative2->BaselineNoise());

    if (negative1->BaselineSharpness() != negative2->BaselineSharpness())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BaselineSharpness: %1 %2").arg(negative1->BaselineSharpness()).arg(negative2->BaselineSharpness());

    if (negative1->BaseOrientation() != negative2->BaseOrientation())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BaseOrientation: %1 %2").arg(negative1->BaseOrientation().GetAdobe()).arg(negative2->BaseOrientation().GetAdobe());

    //if (negative1->BestQualityFinalHeight() != negative2->BestQualityFinalHeight())
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BestQualityFinalHeight: %1 %2").arg(negative1->BestQualityFinalHeight()).arg(negative2->BestQualityFinalHeight());

    //if (negative1->BestQualityFinalWidth() != negative2->BestQualityFinalWidth())
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BestQualityFinalWidth: %1 %2").arg(negative1->BestQualityFinalWidth()).arg(negative2->BestQualityFinalWidth());

    if (!AreSame(negative1->BestQualityScale().As_real64(), negative2->BestQualityScale().As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BestQualityScale");

    if (negative1->HasCameraNeutral() &&
        negative2->HasCameraNeutral())
    {
        if (negative1->CameraNeutral() != negative2->CameraNeutral())
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CameraNeutral");
    }

    if (negative1->HasCameraWhiteXY() &&
        negative2->HasCameraWhiteXY())
    {
        if (negative1->CameraWhiteXY() != negative2->CameraWhiteXY())
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CameraWhiteXY");
    }

    if (!AreSame(negative1->ChromaBlurRadius().As_real64(), negative2->ChromaBlurRadius().As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ChromaBlurRadius: %1 %2").arg(negative1->ChromaBlurRadius().As_real64()).arg(negative2->ChromaBlurRadius().As_real64());

    if (negative1->ColorChannels() != negative2->ColorChannels())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ColorChannels: %1 %2").arg(negative1->ColorChannels()).arg(negative2->ColorChannels());

    if (negative1->ColorimetricReference() != negative2->ColorimetricReference())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ColorimetricReference: %1 %2").arg(negative1->ColorimetricReference()).arg(negative2->ColorimetricReference());

    //if (!AreSame(negative1->DefaultCropOriginH().As_real64(), negative2->DefaultCropOriginH().As_real64()))
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropOriginH: %1 %2").arg(negative1->DefaultCropOriginH().As_real64()).arg(negative2->DefaultCropOriginH().As_real64());

    //if (!AreSame(negative1->DefaultCropOriginV().As_real64(), negative2->DefaultCropOriginV().As_real64()))
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropOriginV: %1 %2").arg(negative1->DefaultCropOriginV().As_real64()).arg(negative2->DefaultCropOriginV().As_real64());

    //if (!AreSame(negative1->DefaultCropSizeH().As_real64(), negative2->DefaultCropSizeH().As_real64()))
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropSizeH: %1 %2").arg(negative1->DefaultCropSizeH().As_real64()).arg(negative2->DefaultCropSizeH().As_real64());

    //if (!AreSame(negative1->DefaultCropSizeV().As_real64(), negative2->DefaultCropSizeV().As_real64()))
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultCropSizeV: %1 %2").arg(negative1->DefaultCropSizeV().As_real64()).arg(negative2->DefaultCropSizeV().As_real64());

    if (negative1->DefaultScale() != negative2->DefaultScale())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultScale: %1 %2").arg(negative1->DefaultScale()).arg(negative2->DefaultScale());

    if (!AreSame(negative1->DefaultScaleH().As_real64(), negative2->DefaultScaleH().As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultScaleH: %1 %2").arg(negative1->DefaultScaleH().As_real64()).arg(negative2->DefaultScaleH().As_real64());

    if (!AreSame(negative1->DefaultScaleV().As_real64(), negative2->DefaultScaleV().As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultScaleV: %1 %2").arg(negative1->DefaultScaleV().As_real64()).arg(negative2->DefaultScaleV().As_real64());

    //if (negative1->DefaultFinalHeight() != negative2->DefaultFinalHeight())
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultFinalHeight: %1 %2").arg(negative1->DefaultFinalHeight()).arg(negative2->DefaultFinalHeight());

    //if (negative1->DefaultFinalWidth() != negative2->DefaultFinalWidth())
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    DefaultFinalWidth: %1 %2").arg(negative1->DefaultFinalWidth()).arg(negative2->DefaultFinalWidth());

    if (negative1->HasBaseOrientation() != negative2->HasBaseOrientation())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    HasBaseOrientation: %1 %2").arg(negative1->HasBaseOrientation()).arg(negative2->HasBaseOrientation());

    if (negative1->HasCameraNeutral() != negative2->HasCameraNeutral())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    HasCameraNeutral: %1 %2").arg(negative1->HasCameraNeutral()).arg(negative2->HasCameraNeutral());

    if (negative1->HasCameraWhiteXY() != negative2->HasCameraWhiteXY())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    HasCameraWhiteXY: %1 %2").arg(negative1->HasCameraWhiteXY()).arg(negative2->HasCameraWhiteXY());

    if (negative1->HasNoiseProfile() != negative2->HasNoiseProfile())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    HasNoiseProfile: %1 %2").arg(negative1->HasNoiseProfile()).arg(negative2->HasNoiseProfile());

    if (negative1->HasOriginalRawFileName() != negative2->HasOriginalRawFileName())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    HasOriginalRawFileName: %1 %2").arg(negative1->HasOriginalRawFileName()).arg(negative2->HasOriginalRawFileName());

    if (negative1->IsMakerNoteSafe() != negative2->IsMakerNoteSafe())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    IsMakerNoteSafe: %1 %2").arg(negative1->IsMakerNoteSafe()).arg(negative2->IsMakerNoteSafe());

    if (negative1->IsMonochrome() != negative2->IsMonochrome())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    IsMonochrome: %1 %2").arg(negative1->IsMonochrome()).arg(negative2->IsMonochrome());

    if (negative1->LinearResponseLimit() != negative2->LinearResponseLimit())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LinearResponseLimit: %1 %2").arg(negative1->LinearResponseLimit()).arg(negative2->LinearResponseLimit());

    if (negative1->LocalName() != negative2->LocalName())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    LocalName: %1 %2").arg(QLatin1String(negative1->LocalName().Get())).arg(QLatin1String(negative2->LocalName().Get()));

    if (negative1->MakerNoteLength() != negative2->MakerNoteLength())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    MakerNoteLength: %1 %2").arg(negative1->MakerNoteLength()).arg(negative2->MakerNoteLength());

    if (negative1->ModelName() != negative2->ModelName())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ModelName: %1 %2").arg(QLatin1String(negative1->ModelName().Get())).arg(QLatin1String(negative2->ModelName().Get()));

    if (!AreSame(negative1->NoiseReductionApplied().As_real64(), negative2->NoiseReductionApplied().As_real64()))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    NoiseReductionApplied: %1 %2").arg(negative1->NoiseReductionApplied().As_real64()).arg(negative2->NoiseReductionApplied().As_real64());

    //if (negative1->Orientation() != negative2->Orientation())
    //    qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    Orientation: %1 %2").arg(negative1->Orientation()).arg(negative2->Orientation());

    if (negative1->OriginalRawFileDataLength() != negative2->OriginalRawFileDataLength())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OriginalRawFileDataLength: %1 %2").arg(negative1->OriginalRawFileDataLength()).arg(negative2->OriginalRawFileDataLength());

    if (0 != memcmp(negative1->OriginalRawFileDigest().data, negative2->OriginalRawFileDigest().data, 16 * sizeof(uint8)))
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OriginalRawFileDigest");

    if (negative1->OriginalRawFileName() != negative2->OriginalRawFileName())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    OriginalRawFileName: %1 %2").arg(QLatin1String(negative1->OriginalRawFileName().Get())).arg(QLatin1String(negative2->OriginalRawFileName().Get()));

    if (negative1->PixelAspectRatio() != negative2->PixelAspectRatio())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PixelAspectRatio: %1 %2").arg(negative1->PixelAspectRatio()).arg(negative2->PixelAspectRatio());

    if (negative1->PrivateLength() != negative2->PrivateLength())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    PrivateLength: %1 %2").arg(negative1->PrivateLength()).arg(negative2->PrivateLength());

    if (negative1->ProfileCount() != negative2->ProfileCount())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ProfileCount: %1 %2").arg(negative1->ProfileCount()).arg(negative2->ProfileCount());

    if (negative1->RawDataUniqueID().Collapse32() != negative2->RawDataUniqueID().Collapse32())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RawDataUniqueID");

    if (negative1->RawImageDigest().Collapse32() != negative2->RawImageDigest().Collapse32())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RawImageDigest");

    if (negative1->RawToFullScaleH() != negative2->RawToFullScaleH())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RawToFullScaleH: %1 %2").arg(negative1->RawToFullScaleH()).arg(negative2->RawToFullScaleH());

    if (negative1->RawToFullScaleV() != negative2->RawToFullScaleV())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    RawToFullScaleV: %1 %2").arg(negative1->RawToFullScaleV()).arg(negative2->RawToFullScaleV());

    if (negative1->ShadowScale() != negative2->ShadowScale())
        qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ShadowScale: %1 %2").arg(negative1->ShadowScale()).arg(negative2->ShadowScale());

    //if (negative1->SquareHeight() != negative2->SquareHeight())
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SquareHeight: %1 %2").arg(negative1->SquareHeight()).arg(negative2->SquareHeight());

    //if (negative1->SquareWidth() != negative2->SquareWidth())
    //  qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    SquareWidth: %1 %2").arg(negative1->SquareWidth()).arg(negative2->SquareWidth());

    const dng_mosaic_info* mosaicInfo1 = negative1->GetMosaicInfo();
    const dng_mosaic_info* mosaicInfo2 = negative2->GetMosaicInfo();

    if ((mosaicInfo1 != nullptr) && (mosaicInfo2 != nullptr))
    {
        if (mosaicInfo1->fBayerGreenSplit != mosaicInfo2->fBayerGreenSplit)
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    BayerGreenSplit: %1 %2").arg(mosaicInfo1->fBayerGreenSplit).arg(mosaicInfo2->fBayerGreenSplit);

        if (mosaicInfo1->fCFALayout != mosaicInfo2->fCFALayout)
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CFALayout: %1 %2").arg(mosaicInfo1->fCFALayout).arg(mosaicInfo2->fCFALayout);

        if (0 != memcmp(mosaicInfo1->fCFAPattern, mosaicInfo2->fCFAPattern, kMaxCFAPattern * kMaxCFAPattern * sizeof(uint8)))
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CFAPattern");

        if (mosaicInfo1->fCFAPatternSize != mosaicInfo2->fCFAPatternSize)
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CFAPatternSize: %1,%2 %3,%4").arg(mosaicInfo1->fCFAPatternSize.h).arg(mosaicInfo1->fCFAPatternSize.v).arg(mosaicInfo2->fCFAPatternSize.h).arg(mosaicInfo2->fCFAPatternSize.v);

        if (0 != memcmp(mosaicInfo1->fCFAPlaneColor, mosaicInfo2->fCFAPlaneColor, kMaxColorPlanes * sizeof(uint8)))
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    CFAPlaneColor");

        if (mosaicInfo1->fColorPlanes != mosaicInfo2->fColorPlanes)
            qCDebug(DIGIKAM_TESTS_LOG).noquote() << QString::fromLatin1("    ColorPlanes: %1 %2").arg(mosaicInfo1->fColorPlanes).arg(mosaicInfo2->fColorPlanes);
    }
}

int main(int argc, char** argv)
{
    try
    {
        QCoreApplication app(argc, argv);

        if (argc == 1)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << ""
                        "dngcompare - DNG comparison tool"
                        "Usage: %s [options] <dngfile1> <dngfile2>"
                     << argv[0];

            return -1;
        }

        char* const fileName1 = argv[1];
        char* const fileName2 = argv[2];

        dng_xmp_sdk::InitializeSDK();

        dng_file_stream stream1(fileName1);
        dng_host host1;
        host1.SetKeepOriginalFile(true);

        dng_info info1;
        AutoPtr<dng_negative> negative1;
        {
            info1.Parse(host1, stream1);
            info1.PostParse(host1);

            if (!info1.IsValidDNG())
            {
                return dng_error_bad_format;
            }

            negative1.Reset(host1.Make_dng_negative());
            negative1->Parse(host1, stream1, info1);
            negative1->PostParse(host1, stream1, info1);
        }

        dng_file_stream stream2(fileName2);
        dng_host host2;
        host2.SetKeepOriginalFile(true);

        dng_info info2;
        AutoPtr<dng_negative> negative2;
        {
            info2.Parse(host2, stream2);
            info2.PostParse(host2);

            if (!info2.IsValidDNG())
            {
                return dng_error_bad_format;
            }

            negative2.Reset(host2.Make_dng_negative());
            negative2->Parse(host2, stream2, info2);
            negative2->PostParse(host2, stream2, info2);
        }

        negative1->SynchronizeMetadata();
        negative2->SynchronizeMetadata();

        dng_exif* const exif1 = negative1->GetExif();
        dng_exif* const exif2 = negative2->GetExif();

        qCDebug(DIGIKAM_TESTS_LOG) << " Exif";
        compareExif(exif1, exif2);

        qCDebug(DIGIKAM_TESTS_LOG) << " Main Ifd";
        compareIfd(info1.fIFD[info1.fMainIndex], info2.fIFD[info1.fMainIndex]);

        qCDebug(DIGIKAM_TESTS_LOG) << " Negative";
        compareNegative(negative1.Get(), negative2.Get());

        dng_xmp_sdk::TerminateSDK();

        return 0;
    }

    catch (const dng_exception& exception)
    {
        int ret = exception.ErrorCode();
        qCCritical(DIGIKAM_TESTS_LOG) << "DNGWriter: DNG SDK exception code (" << ret << ")";

        return (-1);
    }

    catch (...)
    {
        qCCritical(DIGIKAM_TESTS_LOG) << "DNGWriter: DNG SDK exception code unknown";

        return (-1);
    }
}
