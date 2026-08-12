/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Exposure detection  basic factor
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * References  : https://cse.buffalo.edu/~siweilyu/papers/ijcv14.pdf
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exposure_detector.h"

// Qt includes

#include <QtMath>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN ExposureDetector::Private
{

public:

    Private() = default;

public:

    int threshold_overexposed       = 245;
    int threshold_demi_overexposed  = 235;
    int threshold_underexposed      = 15;
    int threshold_demi_underexposed = 30;

    int weight_over_exposure        = 15;
    int weight_demi_over_exposure   = 1;

    int weight_under_exposure       = 15;
    int weight_demi_under_exposure  = 6;
};

ExposureDetector::ExposureDetector()
    :  AbstractDetector(),
       d               (new Private)
{
}

ExposureDetector::~ExposureDetector()
{
    delete d;
}

float ExposureDetector::detect(const cv::Mat& image) const
{
    float overexposed  = percent_overexposed(image);
    float underexposed = percent_underexposed(image);

    return std::max(overexposed, underexposed);
}

float ExposureDetector::percent_overexposed(const cv::Mat& image) const
{
    int over_exposed_pixel      = count_by_condition(image, d->threshold_overexposed, 255);
    int demi_over_exposed_pixel = count_by_condition(image, d->threshold_demi_overexposed,d->threshold_overexposed);
    int normal_pixel            = image.total() - over_exposed_pixel - demi_over_exposed_pixel;

    return (static_cast<float>(static_cast<float>(over_exposed_pixel * d->weight_over_exposure + demi_over_exposed_pixel * d->weight_demi_over_exposure) /
                               static_cast<float>(normal_pixel + over_exposed_pixel * d->weight_over_exposure + demi_over_exposed_pixel * d->weight_demi_over_exposure)));
}

float ExposureDetector::percent_underexposed(const cv::Mat& image) const
{
    int under_exposed_pixel      = count_by_condition(image, 0, d->threshold_underexposed);
    int demi_under_exposed_pixel = count_by_condition(image, d->threshold_underexposed, d->threshold_demi_underexposed);
    int normal_pixel             = image.total() - under_exposed_pixel - demi_under_exposed_pixel;

    return (static_cast<float>(static_cast<float>(under_exposed_pixel * d->weight_under_exposure + demi_under_exposed_pixel * d->weight_demi_under_exposure) /
                               static_cast<float>(normal_pixel + under_exposed_pixel * d->weight_under_exposure + demi_under_exposed_pixel * d->weight_demi_under_exposure)));
}

int ExposureDetector::count_by_condition(const cv::Mat& image, int minVal, int maxVal) const
{
    try
    {
        /**
         * Note: The bitwise operator '&' is intentionally used here for element-wise logical AND between two cv::Mat masks.
         * (image >= minVal) generates a binary mask where pixels >= minVal are set to 255 (true), and others to 0 (false).
         * (image < maxVal) does the same for pixels < maxVal.
         * The '&' operator performs a per-pixel bitwise AND, effectively combining the two conditions:
         *   minVal <= pixel < maxVal.
         * This is the idiomatic way to combine conditions on cv::Mat objects in OpenCV.
         * cv::countNonZero then counts the number of pixels that satisfy both conditions.
         */
        cv::Mat mat = (image >= minVal) & (image < maxVal);         // cppcheck-suppress bitwiseOnBoolean


        return cv::countNonZero(mat);
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "ExposureDetector::count_by_condition: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "ExposureDetector::count_by_condition: Default exception from OpenCV";
    }

    return 0;
}

} // namespace Digikam

#include "moc_exposure_detector.cpp"
