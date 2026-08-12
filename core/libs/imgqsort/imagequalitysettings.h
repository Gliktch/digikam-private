/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image quality Settings Container.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDebug>

// Local includes

#include "digikam_export.h"
#include "album.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_GUI_EXPORT ImageQualitySettings
{
    Q_GADGET

public:

    /**
     * @brief Different possible mode processed while scanning operation.
     */
    enum ScanMode
    {
        AllItems = 0,        ///< Scan all items
        NonAssignedItems     ///< Scan only items with no tags assigned.
    };
    Q_ENUM(ScanMode)

public:

    ImageQualitySettings()  = default;
    ImageQualitySettings(const ImageQualitySettings& other);
    ~ImageQualitySettings() = default;

    ImageQualitySettings& operator=(const ImageQualitySettings& other);

public:

    void readFromConfig();
    void readFromConfig(const KConfigGroup&);
    void writeToConfig();
    void writeToConfig(KConfigGroup&);

public:

    ScanMode      scanMode                = ScanMode::AllItems;   ///< Scanning mode. @note turn all items by default to prevent clearing whole Pick Labels from Collection.
    bool          useFullCpu              = false;                ///< Processing using all CPU available.

    bool          detectBlur              = true;                 ///< Enable image blur detection.
    bool          detectNoise             = true;                 ///< Enable image noise detection.
    bool          detectCompression       = true;                 ///< Enable image compression detection.
    bool          detectExposure          = true;                 ///< Enable image over and under exposure detection.
    bool          detectAesthetic         = true;                 ///< Enable image aesthetic detection.

    bool          lowQRejected            = true;                 ///< Assign Rejected property to low quality.
    bool          mediumQPending          = true;                 ///< Assign Pending property to medium quality.
    bool          highQAccepted           = true;                 ///< Assign Accepted property to high quality.

    int           rejectedThreshold       = 10;                   ///< Item rejection threshold.
    int           pendingThreshold        = 40;                   ///< Item pending threshold.
    int           acceptedThreshold       = 60;                   ///< Item accepted threshold.
    int           blurWeight              = 100;                  ///< Item blur level.
    int           noiseWeight             = 100;                  ///< Item noise level.
    int           compressionWeight       = 100;                  ///< Item compression level.
    int           exposureWeight          = 100;                  ///< Item exposure level.

    bool          wholeAlbums             = true;                 ///< Processing power.
    AlbumList     albums;                                         ///< Albums to scan.

    const QString configName              = QLatin1String("Image Quality Settings");
    const QString configScanMode          = QLatin1String("Image Quality Scan Mode");
    const QString configUseFullCpu        = QLatin1String("Image Quality Use Full CPU");
    const QString configDetectBlur        = QLatin1String("Image Quality Detect Blur");
    const QString configDetectNoise       = QLatin1String("Image Quality Detect Noise");
    const QString configDetectCompression = QLatin1String("Image Quality Detect Compression");
    const QString configDetectExposure    = QLatin1String("Image Quality Detect Exposure");
    const QString configDetectAesthetic   = QLatin1String("Image Quality Detect Aesthetic");
    const QString configLowQRejected      = QLatin1String("Image Quality LowQ Rejected");
    const QString configMediumQPending    = QLatin1String("Image Quality MediumQ Pending");
    const QString configHighQAccepted     = QLatin1String("Image Quality HighQ Accepted");
    const QString configRejectedThreshold = QLatin1String("Image Quality Rejected Threshold");
    const QString configPendingThreshold  = QLatin1String("Image Quality Pending Threshold");
    const QString configAcceptedThreshold = QLatin1String("Image Quality Accepted Threshold");
    const QString configBlurWeight        = QLatin1String("Image Quality Blur Weight");
    const QString configNoiseWeight       = QLatin1String("Image Quality Noise Weight");
    const QString configCompressionWeight = QLatin1String("Image Quality Compression Weight");
    const QString configExposureWeight    = QLatin1String("Image Quality Exposure Weight");
};

//! qDebug() stream operator. Writes property @a s to the debug output in a nicely formatted way.
DIGIKAM_GUI_EXPORT QDebug operator<<(QDebug dbg, const ImageQualitySettings& s);

} // namespace Digikam
