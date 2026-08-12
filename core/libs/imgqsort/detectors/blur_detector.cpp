/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Blur detection basic factor
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blur_detector.h"

// Qt includes

#include <QtMath>

// Local includes

#include "digikam_debug.h"
#include "focuspoints_extractor.h"

namespace Digikam
{

class Q_DECL_HIDDEN BlurDetector::Private
{
public:

    Private() = default;

public:

    float                               min_abs                 = 1.0F;
    float                               ordre_log_filter        = 30.0F;
    float                               sigma_smooth_image      = 5.0F;
    float                               filter_defocus          = 115.0F;

    int                                 part_size_motion_blur   = 40;
    float                               edges_filter            = 8.0F;
    double                              theta_resolution        = CV_PI / 900.0;
    double                              min_line_length         = 30;
    float                               threshold_hough         = 15.0F;
    int                                 min_nb_lines            = 3;
    float                               max_stddev              = 0.65F;

    int                                 part_size_mono_color    = 40;
    float                               mono_color_threshold    = 10.0F;

    bool                                have_focus_region       = false;
    int                                 ratio_expand_af_point   = 2;
    FocusPointsExtractor::ListAFPoints  af_points;
};

BlurDetector::BlurDetector(const DImg& image)
    : AbstractDetector(),
      d               (new Private)
{
    QScopedPointer<FocusPointsExtractor> const extractor(new FocusPointsExtractor(nullptr, image.originalFilePath()));

    d->af_points         = extractor->get_af_points(FocusPoint::TypePoint::Selected);
    d->have_focus_region = !d->af_points.isEmpty();
}

BlurDetector::~BlurDetector()
{
    delete d;
}

float BlurDetector::detect(const cv::Mat& image) const
{
    try
    {
        cv::Mat edgesMap      = edgeDetection(image);

        cv::Mat defocusMap    = detectDefocusMap(edgesMap);
        defocusMap.convertTo(defocusMap, CV_8U);

        cv::Mat motionBlurMap = detectMotionBlurMap(edgesMap);
        motionBlurMap.convertTo(motionBlurMap, CV_8U);

        cv::Mat weightsMat    = getWeightMap(image);

        cv::Mat blurMap       = defocusMap + motionBlurMap;

        cv::Mat res           = weightsMat.mul(blurMap);

        int totalPixels       = cv::countNonZero(weightsMat);

        int blurPixel         = cv::countNonZero(res);

        float percentBlur     = float(blurPixel) / float(totalPixels);

        // qCDebug(DIGIKAM_DIMG_LOG) << "percentage of blur" << percentBlur;

        return percentBlur;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detect: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detect: Default exception from OpenCV";
    }

    return 0.0F;
}

cv::Mat BlurDetector::edgeDetection(const cv::Mat& image) const
{
    try
    {
        // Convert the image to grayscale

        cv::Mat image_gray;
        cvtColor(image, image_gray, cv::COLOR_BGR2GRAY);

        // Use laplacian to detect edge map

        cv::Mat dst;
        cv::Laplacian( image_gray, dst, CV_32F);

        return dst;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::edgeDetection: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::edgeDetection: Default exception from OpenCV";
    }

    return cv::Mat();
}

cv::Mat BlurDetector::detectDefocusMap(const cv::Mat& edgesMap) const
{
    try
    {
        cv::Mat abs_map = cv::abs(edgesMap);

        abs_map.setTo(d->min_abs, (abs_map < d->min_abs));

        // Log filter

        cv::log(abs_map,abs_map);

        abs_map        /= log(d->ordre_log_filter);

        // Smooth image to get blur map

        cv::blur(abs_map, abs_map, cv::Size(d->sigma_smooth_image, d->sigma_smooth_image));
        cv::medianBlur(abs_map, abs_map, d->sigma_smooth_image);

        // Mask blurred pixel as 1 and sharp pixel 0

        cv::Mat res     = abs_map * 255;

        cv::threshold(res, res, d->filter_defocus, 1, cv::THRESH_BINARY_INV);

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detectDefocusMap: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detectDefocusMap: Default exception from OpenCV";
    }

    return cv::Mat();
}

cv::Mat BlurDetector::detectMotionBlurMap(const cv::Mat& edgesMap) const
{
    try
    {
        // Divide image

        // qCDebug(DIGIKAM_DIMG_LOG) << "Divide image to small parts";

        int nb_parts_row = static_cast<int>(edgesMap.size().height / d->part_size_motion_blur);
        int nb_parts_col = static_cast<int>(edgesMap.size().width  / d->part_size_motion_blur);
        cv::Mat res      = cv::Mat::zeros(edgesMap.size(), CV_8U);

        for (int i = 0 ; i < nb_parts_row ; ++i)
        {
            for (int j = 0 ; j < nb_parts_col ; ++j)
            {
                cv::Rect rect
                {
                    j * d->part_size_motion_blur,
                    i * d->part_size_motion_blur,
                    d->part_size_motion_blur,
                    d->part_size_motion_blur
                };

                cv::Mat subImg = edgesMap(rect);

                if (isMotionBlur(subImg))
                {
                    // qCDebug(DIGIKAM_DIMG_LOG) << "Motion blurred part detected";

                    res(rect).setTo(1);
                }
            }
        }

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detectMotionBlurMap: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detectMotionBlurMap: Default exception from OpenCV";
    }

    return cv::Mat();
}

bool BlurDetector::isMotionBlur(const cv::Mat& frag) const
{
    try
    {
        // Convert to 8u

        cv::Mat tmp;

        cv::threshold(frag, tmp, d->edges_filter, 255, cv::THRESH_BINARY);
        tmp.convertTo(tmp, CV_8U);

        std::vector<cv::Vec4i> lines;
        HoughLinesP(tmp, lines, 1, d->theta_resolution, d->threshold_hough, d->min_line_length, 10);

        // Detect if region is motion blurred by number of parallel lines

        if (static_cast<int>(lines.size()) > d->min_nb_lines)
        {
            std::vector<float> list_theta;

            for (const auto& line : std::as_const(lines))
            {
                float theta = (line[2] == line[0])    ? 0.0F          : qAtan((line[3] - line[1]) / (line[2] - line[0]));
                theta       = (theta < 0.0F)          ? theta + CV_PI : theta;
                theta       = (theta < CV_PI / 20.0F) ? CV_PI - theta : theta;

                list_theta.push_back(theta);
            }

            // Calculate Standard Deviation

            cv::Scalar mean, stddev;
            cv::meanStdDev(list_theta, mean, stddev);

            if (stddev[0] < d->max_stddev)
            {
                // qCDebug(DIGIKAM_DIMG_LOG) << "Standard Deviation for group of lines " << stddev[0];
            }

            return (stddev[0] < d->max_stddev);
        }
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::isMotionBlur: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::isMotionBlur: Default exception from OpenCV";
    }

    return false;
}

bool BlurDetector::haveFocusRegion(const DImg& image) const
{
    QScopedPointer<FocusPointsExtractor> const extractor(new FocusPointsExtractor(nullptr, image.originalFilePath()));

    d->af_points = extractor->get_af_points(FocusPoint::TypePoint::Selected);

    return (!d->af_points.isEmpty());
}

cv::Mat BlurDetector::getWeightMap(const cv::Mat& image) const
{
    try
    {
        cv::Mat res = cv::Mat::zeros(image.size(), CV_8UC1);

        if (d->have_focus_region)
        {
            for (const auto& point : std::as_const(d->af_points))
            {
                QPointF pos           = point.getCenterPosition();
                QSizeF size           = point.getSize();

                int x_position_corner = std::max(static_cast<int>((pos.x() - size.width()  * 0.5 *d->ratio_expand_af_point) * image.size().width),  0);
                int y_position_corner = std::max(static_cast<int>((pos.y() - size.height() * 0.5 *d->ratio_expand_af_point) * image.size().height), 0);

                int width             = std::min(image.size().width - x_position_corner,
                                                 static_cast<int>(size.width() * image.size().width * d->ratio_expand_af_point) );
                int height            = std::min(image.size().height - y_position_corner,
                                                 static_cast<int>(size.height() * image.size().height * d->ratio_expand_af_point) );

                if ((width > 0) && (height > 0))
                {
                    cv::Rect rect
                    {
                        x_position_corner,
                        y_position_corner,
                        width,
                        height
                    };

                    res(rect).setTo(1);
                }
                else
                {
                    res = detectBackgroundRegion(image);

                    cv::threshold(res, res, 0.5, 1, cv::THRESH_BINARY_INV);

                    break;
                }
            }
        }
        else
        {
            res = detectBackgroundRegion(image);

            cv::threshold(res, res, 0.5, 1, cv::THRESH_BINARY_INV);
        }

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::getWeightMap: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::getWeightMap: Default exception from OpenCV";
    }

    return cv::Mat();
}

cv::Mat BlurDetector::detectBackgroundRegion(const cv::Mat& image)    const
{
    try
    {
        // qCDebug(DIGIKAM_DIMG_LOG) << "Divide image to small parts";

        int nb_parts_row = static_cast<int>(image.size().height / d->part_size_mono_color);
        int nb_parts_col = static_cast<int>(image.size().width  / d->part_size_mono_color);

        cv::Mat res = cv::Mat::zeros(image.size(), CV_8U);

        for (int i = 0 ; i < nb_parts_row ; ++i)
        {
            for (int j = 0 ; j < nb_parts_col ; ++j)
            {
                cv::Rect rect
                {
                    j * d->part_size_mono_color,
                    i * d->part_size_mono_color,
                    d->part_size_mono_color,
                    d->part_size_mono_color
                };

                cv::Mat subImg = image(rect);

                cv::Scalar mean, stddev;

                cv::meanStdDev(subImg, mean, stddev);

                if (stddev[0] < d->mono_color_threshold)
                {
                    res(rect).setTo(1);
                }
            }
        }

        return res;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detectBackgroundRegion: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_DETECTOR_LOG) << "BlurDetector::detectBackgroundRegion: Default exception from OpenCV";
    }

    return cv::Mat();
}

} // namespace Digikam

#include "moc_blur_detector.cpp"
