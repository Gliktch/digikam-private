/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2012-01-03
 * Description : Calculates the TanTriggs Preprocessing as described in:
 *               Tan, X., and Triggs, B. "Enhanced local texture feature sets for face
 *               recognition under difficult lighting conditions.". IEEE Transactions
 *               on Image Processing 19 (2010), 1635–650.
 *               Default parameters are taken from the paper.
 *
 * SPDX-FileCopyrightText: 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText:      2012 Philipp Wagner <bytefish at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_opencv.h"

namespace Digikam
{

class TanTriggsPreprocessor
{
public:

    explicit TanTriggsPreprocessor();

    /**
     * Performs the Tan Triggs preprocessing to reduce the influence of lightning conditions.
     * Returns a grey-scale 8-bit image.
     */
    cv::Mat preprocess(const cv::Mat& inputImage);

    /**
     * Performs the Tan Triggs preprocessing to reduce the influence of lightning conditions.
     * Expects a one-channel image.
     * Returns a one-channel floating point (CV_32F1) image.
     */
    cv::Mat preprocessRaw(const cv::Mat& inputImage);

    /**
     * Converts CV_32F1 -> CV_8UC1.
     */
    cv::Mat normalize(const cv::Mat& preprocessedImage);

public:

    /// Parameters, initialized with the default values from the paper.

    float alpha  = 0.0F;
    float tau    = 0.0F;
    float gamma  = 0.0F;
    float sigma0 = 0.0F;
    float sigma1 = 0.0F;
};

} // namespace Digikam
