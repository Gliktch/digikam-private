/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Noise detection basic factor
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * References  : https://cse.buffalo.edu/~siweilyu/papers/ijcv14.pdf
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "noise_detector.h"

// Qt includes

#include <QtMath>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

const int SIZE_FILTER = 4;

NoiseDetector::Mat3D initFiltersHaar()
{
    try
    {
        NoiseDetector::Mat3D res;

        res.reserve(SIZE_FILTER * SIZE_FILTER);

        float mat_base[SIZE_FILTER][SIZE_FILTER] =
        {
            {   0.5F,            0.5F,           0.5F,            0.5F           },
            {   0.5F,            0.5F,          -0.5F,           -0.5F           },
            {   0.7071F,        -0.7071F,        0.0F,            0.0F           },
            {   0.0F,            0.0F,           0.7071F,        -0.7071F        }
        };

        cv::Mat mat_base_opencv = cv::Mat(SIZE_FILTER, SIZE_FILTER, CV_32FC1, &mat_base);

        for (int i = 0 ; i < SIZE_FILTER ; i++)
        {
            for (int j = 0 ; j < SIZE_FILTER ; j++)
            {
                res.push_back(mat_base_opencv.row(i).t() * mat_base_opencv.row(j));
            }
        }

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }

    return NoiseDetector::Mat3D();
}

const NoiseDetector::Mat3D NoiseDetector::filtersHaar = initFiltersHaar();

class Q_DECL_HIDDEN NoiseDetector::Private
{
public:

    Private() = default;

public:

    int   size_filter = 4;

    float alpha       = 18.0F;
    float beta        = 7.0F;
};

/**
 * @brief Main noise detection
 */
NoiseDetector::NoiseDetector()
    :  AbstractDetector(),
       d               (new Private)
{
}

NoiseDetector::~NoiseDetector()
{
    delete d;
}

float NoiseDetector::detect(const cv::Mat& image) const
{
    try
    {
        cv::Mat image_float = image;

        image_float.convertTo(image_float, CV_32F);

        // Decompose to channels

        Mat3D channels      = decompose_by_filter(image_float, filtersHaar);

        // Calculate variance and kurtosis

        cv::Mat variance, kurtosis;

        calculate_variance_kurtosis(channels, variance, kurtosis);

        // Calculate variance of noise

        float V             = noise_variance(variance, kurtosis);

        return normalize(V);
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }

    return 1.0F;
}

NoiseDetector::Mat3D NoiseDetector::decompose_by_filter(const cv::Mat& image, const Mat3D& filters) const
{
    try
    {
        Mat3D filtersUsed = filters.mid(1); // do not use first filter
        Mat3D channels;

        channels.reserve(filtersUsed.size());

        for (const auto& filter : std::as_const(filtersUsed))
        {
            cv::Mat tmp = cv::Mat(image.size().width, image.size().height, CV_32FC1);

            cv::filter2D(image, tmp, -1, filter, cv::Point(-1, -1), 0.0, cv::BORDER_REPLICATE);

            channels.push_back(tmp);
        }

        return channels;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }

    return Mat3D();
}

void NoiseDetector::calculate_variance_kurtosis(const Mat3D& channels, cv::Mat& variance, cv::Mat& kurtosis) const
{
    try
    {
        // Get raw moments

        cv::Mat mu1 = raw_moment(channels, 1);
        cv::Mat mu2 = raw_moment(channels, 2);
        cv::Mat mu3 = raw_moment(channels, 3);
        cv::Mat mu4 = raw_moment(channels, 4);

        // Calculate variance and kurtosis projection

        variance    = mu2 - pow_mat(mu1, 2);
        kurtosis    = (mu4 - 4.0 * mu1.mul(mu3) + 6.0 * pow_mat(mu1,2).mul(mu2) - 3.0 * pow_mat(mu1,4)) / pow_mat(variance, 2) - 3.0;

        cv::threshold(kurtosis, kurtosis, 0, 0, cv::THRESH_TOZERO);
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }
}

float NoiseDetector::noise_variance(const cv::Mat& variance, const cv::Mat& kurtosis) const
{
    try
    {
        cv::Mat sqrt_kurtosis;

        cv::sqrt(kurtosis, sqrt_kurtosis);

        float aa    = mean_mat(sqrt_kurtosis);

        float bb    = mean_mat(pow_mat(variance, -1));

        float cc    = mean_mat(pow_mat(variance, -2));

        float dd    = mean_mat(sqrt_kurtosis.mul(pow_mat(variance, -1)));

        float sqrtK = (aa*cc - bb*dd) / (cc-bb * bb);

        return ((1.0F - aa / sqrtK) / bb);
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }

    return 1.0F;
}

cv::Mat NoiseDetector::raw_moment(const NoiseDetector::Mat3D& mat, int order) const
{
    try
    {
        float taille_image = mat[0].size().width * mat[0].size().height;

        std::vector<float> vec;
        vec.reserve(mat.size());

        for (const auto& mat2d : std::as_const(mat))
        {
            vec.push_back(cv::sum(pow_mat(mat2d, order))[0] / taille_image);
        }

        return cv::Mat(vec, true);
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }

    return cv::Mat();
}

cv::Mat NoiseDetector::pow_mat(const cv::Mat& mat, float order) const
{
    try
    {
        cv::Mat res = cv::Mat(mat.size().width, mat.size().height, CV_32FC1);
        cv::pow(mat, order, res);

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }

    return cv::Mat();
}

float NoiseDetector::mean_mat(const cv::Mat& mat) const
{
    try
    {
        cv::Scalar mean, std;
        cv::meanStdDev(mat,mean,std);

        return mean[0];
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "Default exception from OpenCV";
    }

    return 1.0F;
}

/**
 * @brief Normalize result to interval [0 - 1]
 */
float NoiseDetector::normalize(const float number) const
{
    return (1.0F / (1.0F + qExp(-(number - d->alpha) / d->beta)));
}

} // namespace Digikam

#include "moc_noise_detector.cpp"
