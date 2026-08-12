/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-29
 * Description : image levels manipulation methods.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "dcolor.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageHistogram;

class DIGIKAM_EXPORT ImageLevels
{

public:

    explicit ImageLevels(bool sixteenBit);
    ~ImageLevels();

    bool   isDirty();
    bool   isSixteenBits();
    void   reset();

    /**
     * Methods to manipulate the levels data.
     */
    void   levelsChannelReset(int channel);
    void   levelsAuto(const ImageHistogram* const hist);
    void   levelsChannelAuto(const ImageHistogram* const hist, int channel);
    int    levelsInputFromColor(int channel, const DColor& color);
    void   levelsBlackToneAdjustByColors(int channel, const DColor& color);
    void   levelsGrayToneAdjustByColors(int channel, const DColor& color);
    void   levelsWhiteToneAdjustByColors(int channel, const DColor& color);
    void   levelsCalculateTransfers();
    float  levelsLutFunc(int nchannels, int channel, float value);
    void   levelsLutSetup(int nchannels);
    void   levelsLutProcess(uchar* const srcPR, uchar* const destPR, uint w, uint h);

    /**
     * Methods to set manually the levels values.
     */
    void   setLevelGammaValue(int channel, double val);
    void   setLevelLowInputValue(int channel, int val);
    void   setLevelHighInputValue(int channel, int val);
    void   setLevelLowOutputValue(int channel, int val);
    void   setLevelHighOutputValue(int channel, int val);

    double getLevelGammaValue(int channel);
    int    getLevelLowInputValue(int channel);
    int    getLevelHighInputValue(int channel);
    int    getLevelLowOutputValue(int channel);
    int    getLevelHighOutputValue(int channel);

    /**
     * Methods to save/load the levels values to/from a Gimp levels text file.
     */
    bool   saveLevelsToGimpLevelsFile(const QUrl& fileUrl);
    bool   loadLevelsFromGimpLevelsFile(const QUrl& fileUrl);

private:

    /// @note disabled
    ImageLevels(const ImageLevels&)            = delete;
    ImageLevels& operator=(const ImageLevels&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
