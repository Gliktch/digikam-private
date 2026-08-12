/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-02
 * Description : RAW file identification information container
 * Note:         this container permit to not expose the rest of
 *               digiKam code to libraw API and use Qt internal
 *               containers when possible.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QSize>
#include <QDebug>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DRawInfo
{

public:

    /**
     * The RAW image orientation values
     */
    enum ImageOrientation
    {
        ORIENTATION_NONE        = 0,
        ORIENTATION_180         = 3,
        ORIENTATION_Mirror90CCW = 4,
        ORIENTATION_90CCW       = 5,
        ORIENTATION_90CW        = 6
    };

public:

    /**
     * Standard constructor
     */
    explicit DRawInfo();

    /**
     * Standard destructor
     */
    ~DRawInfo() = default;

public:

    /**
     * True if RAW file include an ICC color profile.
     */
    bool             hasIccProfile          = false;

    /**
     * True is RAW file is decodable by dcraw.
     */
    bool             isDecodable            = false;

    /**
     * The number of RAW colors.
     */
    int              rawColors              = -1;

    /**
     * The number of RAW images.
     */
    int              rawImages              = -1;

    /**
     * Black level from Raw histogram.
     */
    unsigned int     blackPoint             = 0;

    /**
     * Channel black levels from Raw histogram.
     */
    unsigned int     blackPointCh[4]        = { 0 };

    /**
     * White level from Raw histogram.
     */
    unsigned int     whitePoint             = 0;

    /**
     * Top margin of raw image.
     */
    unsigned int     topMargin              = 0;

    /**
     * Left margin of raw image.
     */
    unsigned int     leftMargin             = 0;

    /**
     * The raw image orientation
     */
    ImageOrientation orientation            = ORIENTATION_NONE;

    /**
     * The sensitivity in ISO used by camera to take the picture.
     */
    float            sensitivity            = -1.0F;

    /**
     * 1/exposureTime = exposure time in seconds.
     */
    float            exposureTime           = -1.0F;

    /**
     * Aperture value in APEX.
     */
    float            aperture               = -1.0F;

    /**
     * Focal Length value in mm.
     */
    float            focalLength            = -1.0F;

    /**
     * The pixel Aspect Ratio if != 1.0. NOTE: if == 1.0, libraw CLI tool do not show this value.
     * Default value = 1.0. This can be unavailable (depending of camera model).
     */
    float            pixelAspectRatio       = 1.0F;

    /**
     * Exposure compensation to be applied during raw conversion.
     * -999 is an invalid exposure.
     */
    float            baselineExposure       = -999.0F;

    /**
     * Ambient temperature in Celsius degrees.
     * -1000 is an invalid temperature.
     */
    float            ambientTemperature     = -1000.0F;

    /**
     * Ambient relative humidity in percent.
     * -1000 is an invalid humidity.
     */
    float            ambientHumidity        = -1000.0F;

    /**
     * Ambient air pressure in hPa or mbar.
     * -1000 is an invalid pressure.
     */
    float            ambientPressure        = -1000.0F;

    /**
     * Depth under water in metres, negative for above water.
     * 1000 is an invalid water depth.
     */
    float            ambientWaterDepth      = 1000.0F;

    /**
     * Directionless camera acceleration in units of mGal, or 10-5 m/s2.
     * -1000 is an invalid acceleration.
     */
    float            ambientAcceleration    = -1000.0F;

    /**
     * Camera elevation angle in degrees.
     * -1000 is an invalid angle.
     */
    float            ambientElevationAngle  = -1000.0F;

    /**
     * Describe how flash has been used by camera.
     * Valid value is unsigned.
     */
    int              flashUsed              = -1;

    /**
     * The metering mode used by camera.
     * Valid value is unsigned.
     */
    int              meteringMode           = -1;

    /**
     * The exposure program used by camera.
     * Valid value is unsigned.
     */
    int              exposureProgram        = -1;

    /**
     * Exposure Index from the camera.
     * Valid value is unsigned.
     */
    float            exposureIndex          = -1.0F;

    /**
     * White color balance settings.
     */
    double           daylightMult[3]        = { 0.0 };

    /**
     * Camera multipliers used for White Balance adjustments
     */
    double           cameraMult[4]          = { 0.0 };

    /**
     * Camera Color Matrix
     */
    float            cameraColorMatrix1[3][4];
    float            cameraColorMatrix2[3][4];
    float            cameraXYZMatrix[4][3];

    /**
     * GPS information
     */
    double           latitude               = 0.0F;
    double           longitude              = 0.0F;
    double           altitude               = 0.0F;
    bool             hasGpsInfo             = false;          ///< true if GPS info are parsed from RAW file.

    /**
     * The used Color Keys
     */
    QString          colorKeys;

    /**
     * The camera maker.
     */
    QString          make;

    /**
     * The camera model.
     */
    QString          model;

    /**
     * The artist name who have picture owner.
     */
    QString          owner;

    /**
     * The software name which process raw image.
     */
    QString          software;

    /**
     * The Firmware name or version which create raw image.
     */
    QString          firmware;

    /**
     * The image description of raw image.
     */
    QString          description;

    /**
     * Serial number of raw image.
     */
    unsigned int     serialNumber           = 0;

    /**
     * The demosaising filter pattern.
     */
    QString          filterPattern;

    /**
     * The DNG version. NOTE: it is only shown with DNG RAW files.
     */
    QString          DNGVersion;

    /**
     * Non-localized name for the camera model that created the raw file
     */
    QString          uniqueCameraModel;

    /**
     * Localized name for the camera model that created the raw file
     */
    QString          localizedCameraModel;

    /**
     * An unique image ID generated by camera.
     */
    QString          imageID;

    /**
     * An unique RAW data ID.
     */
    QString          rawDataUniqueID;

    /**
     * The original RAW file name.
     */
    QString          originalRawFileName;

    /**
     * Date & time when the picture has been taken.
     */
    QDateTime        dateTime;

    /**
     * The image dimensions in pixels.
     */
    QSize            imageSize;

    /**
     * The thumb dimensions in pixels.
     */
    QSize            thumbSize;

    /**
     * The full RAW image dimensions in pixels.
     */
    QSize            fullSize;

    /**
     * The output dimensions in pixels.
     */
    QSize            outputSize;

    /**
     * Xmp metadata container extracted from RAW file, if present.
     */
    QByteArray       xmpData;

    /**
     * ICC color profilr container extracted from RAW file, if present.
     */
    QByteArray       iccData;

    /**
     * Thumbnail image data extracted from raw file.
     */
    QByteArray       thumbnail;

    /**
     * Description of lens properties.
     */
    QString          lensModel;
    QString          lensMake;
    QString          lensSerial;
    int              focalLengthIn35mmFilm  = -1;        ///< Valid value is unsigned.
    float            maxAperture            = -1.0F;     ///< Valid value is unsigned.
};

//! qDebug() stream operator. Writes container @a c to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const DRawInfo& c);

} // namespace Digikam
