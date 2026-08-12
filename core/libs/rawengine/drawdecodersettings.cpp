/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-09
 * Description : Raw decoding settings
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2008 by Guillaume Castagnino <casta at xwing dot info>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "drawdecodersettings.h"

namespace Digikam
{

DRawDecoderSettings::DRawDecoderSettings(const DRawDecoderSettings& o)
    : fixColorsHighlights       (o.fixColorsHighlights),
      autoBrightness            (o.autoBrightness),
      sixteenBitsImage          (o.sixteenBitsImage),
      halfSizeColorImage        (o.halfSizeColorImage),
      whiteBalance              (o.whiteBalance),
      customWhiteBalance        (o.customWhiteBalance),
      customWhiteBalanceGreen   (o.customWhiteBalanceGreen),
      RGBInterpolate4Colors     (o.RGBInterpolate4Colors),
      DontStretchPixels         (o.DontStretchPixels),
      unclipColors              (o.unclipColors),
      RAWQuality                (o.RAWQuality),
      medianFilterPasses        (o.medianFilterPasses),
      NRType                    (o.NRType),
      NRThreshold               (o.NRThreshold),
      brightness                (o.brightness),
      enableBlackPoint          (o.enableBlackPoint),
      blackPoint                (o.blackPoint),
      enableWhitePoint          (o.enableWhitePoint),
      whitePoint                (o.whitePoint),
      inputColorSpace           (o.inputColorSpace),
      inputProfile              (o.inputProfile),
      outputColorSpace          (o.outputColorSpace),
      outputProfile             (o.outputProfile),
      deadPixelMap              (o.deadPixelMap),
      whiteBalanceArea          (o.whiteBalanceArea),

      //-- Extended demosaicing settings ----------------------------------------------------------

      dcbIterations             (o.dcbIterations),
      dcbEnhanceFl              (o.dcbEnhanceFl),
      expoCorrection            (o.expoCorrection),
      expoCorrectionShift       (o.expoCorrectionShift),
      expoCorrectionHighlight   (o.expoCorrectionHighlight)
{
}

DRawDecoderSettings& DRawDecoderSettings::operator=(const DRawDecoderSettings& o)
{
    fixColorsHighlights     = o.fixColorsHighlights;
    autoBrightness          = o.autoBrightness;
    sixteenBitsImage        = o.sixteenBitsImage;
    brightness              = o.brightness;
    RAWQuality              = o.RAWQuality;
    inputColorSpace         = o.inputColorSpace;
    outputColorSpace        = o.outputColorSpace;
    RGBInterpolate4Colors   = o.RGBInterpolate4Colors;
    DontStretchPixels       = o.DontStretchPixels;
    unclipColors            = o.unclipColors;
    whiteBalance            = o.whiteBalance;
    customWhiteBalance      = o.customWhiteBalance;
    customWhiteBalanceGreen = o.customWhiteBalanceGreen;
    halfSizeColorImage      = o.halfSizeColorImage;
    enableBlackPoint        = o.enableBlackPoint;
    blackPoint              = o.blackPoint;
    enableWhitePoint        = o.enableWhitePoint;
    whitePoint              = o.whitePoint;
    NRType                  = o.NRType;
    NRThreshold             = o.NRThreshold;
    medianFilterPasses      = o.medianFilterPasses;
    inputProfile            = o.inputProfile;
    outputProfile           = o.outputProfile;
    deadPixelMap            = o.deadPixelMap;
    whiteBalanceArea        = o.whiteBalanceArea;

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations           = o.dcbIterations;
    dcbEnhanceFl            = o.dcbEnhanceFl;
    expoCorrection          = o.expoCorrection;
    expoCorrectionShift     = o.expoCorrectionShift;
    expoCorrectionHighlight = o.expoCorrectionHighlight;

    return *this;
}

bool DRawDecoderSettings::operator==(const DRawDecoderSettings& o) const
{
    return (
            (fixColorsHighlights     == o.fixColorsHighlights)      &&
            (autoBrightness          == o.autoBrightness)           &&
            (sixteenBitsImage        == o.sixteenBitsImage)         &&
            (brightness              == o.brightness)               &&
            (RAWQuality              == o.RAWQuality)               &&
            (inputColorSpace         == o.inputColorSpace)          &&
            (outputColorSpace        == o.outputColorSpace)         &&
            (RGBInterpolate4Colors   == o.RGBInterpolate4Colors)    &&
            (DontStretchPixels       == o.DontStretchPixels)        &&
            (unclipColors            == o.unclipColors)             &&
            (whiteBalance            == o.whiteBalance)             &&
            (customWhiteBalance      == o.customWhiteBalance)       &&
            (customWhiteBalanceGreen == o.customWhiteBalanceGreen)  &&
            (halfSizeColorImage      == o.halfSizeColorImage)       &&
            (enableBlackPoint        == o.enableBlackPoint)         &&
            (blackPoint              == o.blackPoint)               &&
            (enableWhitePoint        == o.enableWhitePoint)         &&
            (whitePoint              == o.whitePoint)               &&
            (NRType                  == o.NRType)                   &&
            (NRThreshold             == o.NRThreshold)              &&
            (medianFilterPasses      == o.medianFilterPasses)       &&
            (inputProfile            == o.inputProfile)             &&
            (outputProfile           == o.outputProfile)            &&
            (deadPixelMap            == o.deadPixelMap)             &&
            (whiteBalanceArea        == o.whiteBalanceArea)         &&

            //-- Extended demosaicing settings ----------------------------------------------------------

            (dcbIterations           == o.dcbIterations)            &&
            (dcbEnhanceFl            == o.dcbEnhanceFl)             &&
            (expoCorrection          == o.expoCorrection)           &&
            (expoCorrectionShift     == o.expoCorrectionShift)      &&
            (expoCorrectionHighlight == o.expoCorrectionHighlight)
           );
}

void DRawDecoderSettings::optimizeTimeLoading()
{
    fixColorsHighlights     = false;
    autoBrightness          = true;
    sixteenBitsImage        = true;
    brightness              = 1.0;
    RAWQuality              = BILINEAR;
    inputColorSpace         = NOINPUTCS;
    outputColorSpace        = SRGB;
    RGBInterpolate4Colors   = false;
    DontStretchPixels       = false;
    unclipColors            = 0;
    whiteBalance            = CAMERA;
    customWhiteBalance      = 6500;
    customWhiteBalanceGreen = 1.0;
    halfSizeColorImage      = true;
    medianFilterPasses      = 0;

    enableBlackPoint        = false;
    blackPoint              = 0;

    enableWhitePoint        = false;
    whitePoint              = 0;

    NRType                  = NONR;
    NRThreshold             = 0;

    inputProfile            = QString();
    outputProfile           = QString();

    deadPixelMap            = QString();

    whiteBalanceArea        = QRect();

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations           = -1;
    dcbEnhanceFl            = false;
    expoCorrection          = false;
    expoCorrectionShift     = 1.0;
    expoCorrectionHighlight = 0.0;
}

QDebug operator<<(QDebug dbg, const DRawDecoderSettings& s)
{
    dbg.nospace() << Qt::endl;
    dbg.nospace() << "-- RAW DECODING SETTINGS --------------------------------" << Qt::endl;
    dbg.nospace() << "-- autoBrightness:          " << s.autoBrightness          << Qt::endl;
    dbg.nospace() << "-- sixteenBitsImage:        " << s.sixteenBitsImage        << Qt::endl;
    dbg.nospace() << "-- brightness:              " << s.brightness              << Qt::endl;
    dbg.nospace() << "-- RAWQuality:              " << s.RAWQuality              << Qt::endl;
    dbg.nospace() << "-- inputColorSpace:         " << s.inputColorSpace         << Qt::endl;
    dbg.nospace() << "-- outputColorSpace:        " << s.outputColorSpace        << Qt::endl;
    dbg.nospace() << "-- RGBInterpolate4Colors:   " << s.RGBInterpolate4Colors   << Qt::endl;
    dbg.nospace() << "-- DontStretchPixels:       " << s.DontStretchPixels       << Qt::endl;
    dbg.nospace() << "-- unclipColors:            " << s.unclipColors            << Qt::endl;
    dbg.nospace() << "-- whiteBalance:            " << s.whiteBalance            << Qt::endl;
    dbg.nospace() << "-- customWhiteBalance:      " << s.customWhiteBalance      << Qt::endl;
    dbg.nospace() << "-- customWhiteBalanceGreen: " << s.customWhiteBalanceGreen << Qt::endl;
    dbg.nospace() << "-- halfSizeColorImage:      " << s.halfSizeColorImage      << Qt::endl;
    dbg.nospace() << "-- enableBlackPoint:        " << s.enableBlackPoint        << Qt::endl;
    dbg.nospace() << "-- blackPoint:              " << s.blackPoint              << Qt::endl;
    dbg.nospace() << "-- enableWhitePoint:        " << s.enableWhitePoint        << Qt::endl;
    dbg.nospace() << "-- whitePoint:              " << s.whitePoint              << Qt::endl;
    dbg.nospace() << "-- NoiseReductionType:      " << s.NRType                  << Qt::endl;
    dbg.nospace() << "-- NoiseReductionThreshold: " << s.NRThreshold             << Qt::endl;
    dbg.nospace() << "-- medianFilterPasses:      " << s.medianFilterPasses      << Qt::endl;
    dbg.nospace() << "-- inputProfile:            " << s.inputProfile            << Qt::endl;
    dbg.nospace() << "-- outputProfile:           " << s.outputProfile           << Qt::endl;
    dbg.nospace() << "-- deadPixelMap:            " << s.deadPixelMap            << Qt::endl;
    dbg.nospace() << "-- whiteBalanceArea:        " << s.whiteBalanceArea        << Qt::endl;

    //-- Extended demosaicing settings ----------------------------------------------------------

    dbg.nospace() << "-- dcbIterations:           " << s.dcbIterations           << Qt::endl;
    dbg.nospace() << "-- dcbEnhanceFl:            " << s.dcbEnhanceFl            << Qt::endl;
    dbg.nospace() << "-- expoCorrection:          " << s.expoCorrection          << Qt::endl;
    dbg.nospace() << "-- expoCorrectionShift:     " << s.expoCorrectionShift     << Qt::endl;
    dbg.nospace() << "-- expoCorrectionHighlight: " << s.expoCorrectionHighlight << Qt::endl;
    dbg.nospace() << "---------------------------------------------------------" << Qt::endl;

    return dbg.space();
}

} // namespace Digikam
