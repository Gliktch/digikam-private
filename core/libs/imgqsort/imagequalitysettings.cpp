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

#include "imagequalitysettings.h"

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

namespace Digikam
{

ImageQualitySettings::ImageQualitySettings(const ImageQualitySettings& other)
    : scanMode          (other.scanMode),
      useFullCpu        (other.useFullCpu),
      detectBlur        (other.detectBlur),
      detectNoise       (other.detectNoise),
      detectCompression (other.detectCompression),
      detectExposure    (other.detectExposure),
      detectAesthetic   (other.detectAesthetic),
      lowQRejected      (other.lowQRejected),
      mediumQPending    (other.mediumQPending),
      highQAccepted     (other.highQAccepted),
      rejectedThreshold (other.rejectedThreshold),
      pendingThreshold  (other.pendingThreshold),
      acceptedThreshold (other.acceptedThreshold),
      blurWeight        (other.blurWeight),
      noiseWeight       (other.noiseWeight),
      compressionWeight (other.compressionWeight),
      exposureWeight    (other.exposureWeight),
      wholeAlbums       (other.wholeAlbums),
      albums            (other.albums)
{
}

ImageQualitySettings& ImageQualitySettings::operator=(const ImageQualitySettings& other)
{
    scanMode           = other.scanMode;
    useFullCpu         = other.useFullCpu;
    detectBlur         = other.detectBlur;
    detectNoise        = other.detectNoise;
    detectCompression  = other.detectCompression;
    detectExposure     = other.detectExposure;
    detectAesthetic    = other.detectAesthetic;
    lowQRejected       = other.lowQRejected;
    mediumQPending     = other.mediumQPending;
    highQAccepted      = other.highQAccepted;
    rejectedThreshold  = other.rejectedThreshold;
    pendingThreshold   = other.pendingThreshold;
    acceptedThreshold  = other.acceptedThreshold;
    blurWeight         = other.blurWeight;
    noiseWeight        = other.noiseWeight;
    compressionWeight  = other.compressionWeight;
    exposureWeight     = other.exposureWeight;
    wholeAlbums        = other.wholeAlbums;
    albums             = other.albums;

    return *this;
}

void ImageQualitySettings::readFromConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configName);
    readFromConfig(group);
}

void ImageQualitySettings::readFromConfig(const KConfigGroup& group)
{
    scanMode                  = (ScanMode)group.readEntry(configScanMode, (int)ScanMode::AllItems);
    useFullCpu                = group.readEntry(configUseFullCpu,         false);
    detectBlur                = group.readEntry(configDetectBlur,         true);
    detectNoise               = group.readEntry(configDetectNoise,        true);
    detectCompression         = group.readEntry(configDetectCompression,  true);
    detectExposure            = group.readEntry(configDetectExposure,     true);
    detectAesthetic           = group.readEntry(configDetectAesthetic,    true);
    lowQRejected              = group.readEntry(configLowQRejected,       true);
    mediumQPending            = group.readEntry(configMediumQPending,     true);
    highQAccepted             = group.readEntry(configHighQAccepted,      true);
    rejectedThreshold         = group.readEntry(configRejectedThreshold,  10);
    pendingThreshold          = group.readEntry(configPendingThreshold,   40);
    acceptedThreshold         = group.readEntry(configAcceptedThreshold,  60);
    blurWeight                = group.readEntry(configBlurWeight,         100);
    noiseWeight               = group.readEntry(configNoiseWeight,        100);
    compressionWeight         = group.readEntry(configCompressionWeight,  100);
    exposureWeight            = group.readEntry(configExposureWeight,     100);

    // NOTE: Album settings are handled by AlbumSelector widget.
}

void ImageQualitySettings::writeToConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configName);
    writeToConfig(group);
}

void ImageQualitySettings::writeToConfig(KConfigGroup& group)
{
    group.writeEntry(configScanMode,           (int)scanMode);
    group.writeEntry(configUseFullCpu,         useFullCpu);
    group.writeEntry(configDetectBlur,         detectBlur);
    group.writeEntry(configDetectNoise,        detectNoise);
    group.writeEntry(configDetectCompression,  detectCompression);
    group.writeEntry(configDetectExposure,     detectExposure);
    group.writeEntry(configDetectAesthetic,    detectAesthetic);
    group.writeEntry(configLowQRejected,       lowQRejected);
    group.writeEntry(configMediumQPending,     mediumQPending);
    group.writeEntry(configHighQAccepted,      highQAccepted);
    group.writeEntry(configRejectedThreshold,  rejectedThreshold);
    group.writeEntry(configPendingThreshold,   pendingThreshold);
    group.writeEntry(configAcceptedThreshold,  acceptedThreshold);
    group.writeEntry(configBlurWeight,         blurWeight);
    group.writeEntry(configNoiseWeight,        noiseWeight);
    group.writeEntry(configCompressionWeight,  compressionWeight);
    group.writeEntry(configExposureWeight,     exposureWeight);

    // NOTE: Album settings are handled by AlbumSelector widget.
}

QDebug operator<<(QDebug dbg, const ImageQualitySettings& s)
{
    dbg.nospace()                                                   << Qt::endl;
    dbg.nospace() << "Scan Mode          :" << s.scanMode           << Qt::endl;
    dbg.nospace() << "Use Full CPU       :" << s.useFullCpu         << Qt::endl;
    dbg.nospace() << "DetectBlur         :" << s.detectBlur         << Qt::endl;
    dbg.nospace() << "DetectNoise        :" << s.detectNoise        << Qt::endl;
    dbg.nospace() << "DetectCompression  :" << s.detectCompression  << Qt::endl;
    dbg.nospace() << "DetectExposure     :" << s.detectExposure     << Qt::endl;
    dbg.nospace() << "DetectAesthetic    :" << s.detectAesthetic    << Qt::endl;
    dbg.nospace() << "LowQRejected       :" << s.lowQRejected       << Qt::endl;
    dbg.nospace() << "MediumQPending     :" << s.mediumQPending     << Qt::endl;
    dbg.nospace() << "HighQAccepted      :" << s.highQAccepted      << Qt::endl;
    dbg.nospace() << "Rejected Threshold :" << s.rejectedThreshold  << Qt::endl;
    dbg.nospace() << "Pending Threshold  :" << s.pendingThreshold   << Qt::endl;
    dbg.nospace() << "Accepted Threshold :" << s.acceptedThreshold  << Qt::endl;
    dbg.nospace() << "Blur Weight        :" << s.blurWeight         << Qt::endl;
    dbg.nospace() << "Noise Weight       :" << s.noiseWeight        << Qt::endl;
    dbg.nospace() << "Compression Weight :" << s.compressionWeight  << Qt::endl;
    dbg.nospace() << "Exposure Weight    :" << s.exposureWeight     << Qt::endl;
    dbg.nospace() << "Whole Albums       :" << s.wholeAlbums        << Qt::endl;
    dbg.nospace() << "Albums             :" << s.albums.count()     << Qt::endl;

    return dbg.space();
}

} // namespace Digikam

#include "moc_imagequalitysettings.cpp"
