/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-06-14
 * Description : Alignment by Image Congealing.
 *               Funneling for complex, realistic images
 *               using sequence of distribution fields learned from congealReal
 *               Gary B. Huang, Vidit Jain, and Erik Learned-Miller.
 *               Unsupervised joint alignment of complex images.
 *               International Conference on Computer Vision (ICCV), 2007.
 *               Based on Gary B. Huang, UMass-Amherst implementation.
 *
 * SPDX-FileCopyrightText: 2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_opencv.h"

namespace Digikam
{

class FunnelReal
{

public:

    explicit FunnelReal();
    ~FunnelReal();

    cv::Mat align(const cv::Mat& inputImage);

private:

    /// @note disabled
    FunnelReal(const FunnelReal&)            = delete;
    FunnelReal& operator=(const FunnelReal&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
