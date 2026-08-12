/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Compression detection basic factor
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * References  : http://www.arpnjournals.org/jeas/research_papers/rp_2016/jeas_1216_5505.pdf // krazy:exclude=insecurenet
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "compression_detector.h"

// Qt includes

#include <QtMath>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN CompressionDetector::Private
{
public:

    Private() = default;

public:

    int   threshold_edges_block = 2;
    float weight_edges_block    = 120.0F;

    float weight_mono_color     = 10.0F;
    float threshold_mono_color  = 0.1F;

    float alpha                 = 0.026F;
    float beta                  = -0.002F;
};

CompressionDetector::CompressionDetector()
    :  AbstractDetector(),
       d               (new Private)
{
}

CompressionDetector::~CompressionDetector()
{
    delete d;
}

auto accessRow = [](const cv::Mat& mat)
{
    return [mat](int index)
    {
        return mat.row(index);
    };
};

auto accessCol = [](const cv::Mat& mat)
{
    return [mat](int index)
    {
        return mat.col(index);
    };
};

float CompressionDetector::detect(const cv::Mat& image) const
{
    try
    {
        cv::Mat gray_image;

        cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);

        cv::Mat verticalBlock    = checkEdgesBlock(gray_image, gray_image.cols, accessCol);
        cv::Mat horizontalBlock  = checkEdgesBlock(gray_image, gray_image.rows, accessRow);
        cv::Mat mono_color_map   = detectMonoColorRegion(image);
        cv::Mat block_map        = mono_color_map.mul(verticalBlock + horizontalBlock);

        int nb_pixels_edge_block = cv::countNonZero(block_map);
        int nb_pixels_mono_color = cv::countNonZero(mono_color_map);
        int nb_pixels_normal     = image.total() - nb_pixels_edge_block - nb_pixels_edge_block;

        float res                = static_cast<float>(
                                                      (nb_pixels_mono_color * d->weight_mono_color +
                                                       nb_pixels_edge_block * d->threshold_edges_block) /
                                                      (nb_pixels_mono_color * d->weight_mono_color +
                                                       nb_pixels_edge_block * d->threshold_edges_block + nb_pixels_normal)
                                                     );

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "CompressionDetector::detect: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "CompressionDetector::detect: Default exception from OpenCV";
    }

    return 1.0F;
}

template <typename Function>
cv::Mat CompressionDetector::checkEdgesBlock(const cv::Mat& gray_image, int blockSize, Function accessEdges) const
{
    try
    {
        cv::Mat res            = cv::Mat::zeros(gray_image.size(), CV_8UC1);

        auto accessGrayImageAt = accessEdges(gray_image);
        auto accessResAt       = accessEdges(res);

        for (int i = 2 ; i < blockSize - 1 ; ++i)
        {
            cv::Mat a = (accessGrayImageAt(i) - accessGrayImageAt(i + 1)) - (accessGrayImageAt(i - 1) - accessGrayImageAt(i));
            cv::Mat b = (accessGrayImageAt(i) - accessGrayImageAt(i + 1)) - (accessGrayImageAt(i + 1) - accessGrayImageAt(i - 2));

            /**
             * Fix reported by cppcheck: replace bitwise operator '&' with cv::bitwise_and for logical element-wise AND operation.
             * The original code used the bitwise operator '&' on cv::Mat objects, which can lead to incorrect
             * results if the matrix values are not strictly 0 or 1. cv::bitwise_and ensures a proper logical AND
             * operation on a per-pixel basis, generating a binary mask where each pixel is 255 (true) if both
             * input pixels meet the threshold condition, and 0 (false) otherwise.
             * This change improves robustness and correctness, especially for edge detection in compression analysis.
             */
            cv::Mat maskA, maskB;
            cv::compare(a, d->threshold_edges_block, maskA, cv::CMP_GE);
            cv::compare(b, d->threshold_edges_block, maskB, cv::CMP_GE);
            cv::bitwise_and(maskA, maskB, accessResAt(i));
        }

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "CompressionDetector::checkEdgesBlock: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "CompressionDetector::checkEdgesBlock: Default exception from OpenCV";
    }

    return cv::Mat();
}

cv::Mat CompressionDetector::detectMonoColorRegion(const cv::Mat& image) const
{
    try
    {
        cv::Mat median_image    = cv::Mat();
        cv::medianBlur(image, median_image, 5);
        cv::Mat mat_subtraction = cv::abs(image - median_image);
        std::vector<cv::Mat> rgbChannels(3);

        cv::split(mat_subtraction, rgbChannels);

        cv::Mat res             = rgbChannels[0] + rgbChannels[1] + rgbChannels[2];

        cv::threshold(res, res, d->threshold_mono_color, 1, cv::THRESH_BINARY_INV);

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "CompressionDetector::detectMonoColorRegion: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "CompressionDetector::detectMonoColorRegion: Default exception from OpenCV";
    }

    return cv::Mat();
}

float CompressionDetector::normalize(const float number)
{
    return (1.0F / (1.0F + qExp(-(number - d->alpha) / d->beta)));
}

} // namespace Digikam

#include "moc_compression_detector.cpp"
