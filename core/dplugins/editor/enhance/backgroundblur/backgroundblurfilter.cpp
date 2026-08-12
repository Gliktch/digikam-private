/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-08-24
 * Description : image editor plugin to blur the background of an image
 *
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// cppcheck-suppress-file knownConditionTrueFalse

#include "backgroundblurfilter.h"

// Qt includes

#include <QtMath>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "digikam_opencv.h"
#include "qtopencvimg.h"

namespace DigikamEditorBackgroundBlurToolPlugin
{

class Q_DECL_HIDDEN BackgroundBlurFilter::Private
{
public:

    Private() = default;

public:

    int    radius     = 3;   ///< Blur effect radius.
    int    transition = 0;   ///< Number of blur transitions.
    int    iterations = 10;  ///< GrabCut iterations to isolate more and less the subject.
    QRectF selection;        ///< suject area previously selected by the user.
};

BackgroundBlurFilter::BackgroundBlurFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d                 (new Private)
{
    initFilter();
}

BackgroundBlurFilter::BackgroundBlurFilter(DImg* const orgImage,
                                           const QRectF& selection,
                                           int radius,
                                           int transition,
                                           int iterations,
                                           QObject* const parent)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("BackgroundBlur")),
      d                 (new Private)
{
    d->selection  = selection;
    d->radius     = radius;
    d->transition = transition;
    d->iterations = iterations;

    initFilter();
}

BackgroundBlurFilter::BackgroundBlurFilter(DImgThreadedFilter* const parentFilter,
                                           const DImg& orgImage,
                                           const DImg& destImage,
                                           const QRectF& selection,
                                           int radius,
                                           int transition,
                                           int iterations,
                                           int progressBegin,
                                           int progressEnd)
    : DImgThreadedFilter(parentFilter,
                         orgImage,
                         destImage,
                         progressBegin,
                         progressEnd,
                         parentFilter->filterName() + QLatin1String(": BackgroundBlur")),
      d(new Private)
{
    d->selection  = selection;
    d->radius     = radius;
    d->transition = transition;
    d->iterations = iterations;

    this->filterImage();
}

BackgroundBlurFilter::~BackgroundBlurFilter()
{
    cancelFilter();

    delete d;
}

QString BackgroundBlurFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Background Blur Filter"));
}

void BackgroundBlurFilter::filterImage()
{
    if (d->radius < 1)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Radius out of range...";
        m_destImage = m_orgImage;

        return;
    }

    if (d->selection.isEmpty())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Selection is empty...";
        m_destImage = m_orgImage;

        return;
    }

    bool is16Bit = m_orgImage.sixteenBit();
    cv::Mat input;

    try
    {
        if (is16Bit)
        {
            // Convert image from 16 bits to 8 bits for grabCut as this one do not support 16 bits.

            DImg img8Bit = m_orgImage;
            img8Bit.convertToEightBit();
            input        = QtOpenCVImg::image2Mat(img8Bit);
        }
        else
        {
            input        = QtOpenCVImg::image2Mat(m_orgImage);
        }

        if (input.empty())
        {
            qCWarning(DIGIKAM_DIMG_LOG) << "Failed to convert image to cv::Mat";
            m_destImage = m_orgImage;

            return;
        }

        cv::Mat output;
        postProgress(10);

        if (!runningFlag())
        {
            return;
        }

        // Convert image to CV_8UC3 (BGR) if necessary.

        cv::Mat inputBGR;

        if (input.type() != CV_8UC3)
        {
            if      (input.channels() == 1)
            {
                cv::cvtColor(input, inputBGR, cv::COLOR_GRAY2BGR);
            }
            else if (input.channels() == 4)
            {
                cv::cvtColor(input, inputBGR, cv::COLOR_BGRA2BGR);
            }
            else
            {
                qCWarning(DIGIKAM_DIMG_LOG) << "Unsupported image format";
                m_destImage = m_orgImage;

                return;
            }
        }
        else
        {
            inputBGR = input;
        }

        postProgress(20);

        if (!runningFlag())
        {
            return;
        }

        // Init the mask for GrabCut.

        cv::Rect roi(d->selection.x(), d->selection.y(), d->selection.width(), d->selection.height());
        cv::Mat mask(input.rows, input.cols, CV_8UC1, cv::GC_PR_BGD);
        mask(roi) = cv::GC_PR_FGD;
        postProgress(30);

        if (!runningFlag())
        {
            return;
        }

        // Apply GrabCut with more iterations for better accuracy.

        cv::Mat bgModel, fgModel;
        cv::grabCut(inputBGR, mask, roi, bgModel, fgModel, d->iterations, cv::GC_INIT_WITH_RECT);

        if (!runningFlag())
        {
            return;
        }

        // Refine the mask: GC_PR_FGD and GC_FGD are considered foreground.

        cv::compare(mask, cv::GC_PR_FGD, mask, cv::CMP_GE);
        postProgress(40);

        if (!runningFlag())
        {
            return;
        }

        // Smooth the mask edges.

        cv::Mat kernelClose = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernelClose);

        if (!runningFlag())
        {
            return;
        }

        // Dilate the mask slightly to include more pixels near the subject.

        cv::Mat kernelDilate = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2, 2));
        cv::dilate(mask, mask, kernelDilate);

        if (!runningFlag())
        {
            return;
        }

        // Convert the mask to BGR to be suitable for the preview.

        cv::Mat maskDisplay;
        mask.convertTo(maskDisplay, CV_8U, 255);

        if (!runningFlag())
        {
            return;
        }

        // Create a copy of original for preview.

        cv::Mat overlay = inputBGR.clone();

        if (!runningFlag())
        {
            return;
        }

        // Create an image from the mask using semi-transparent green.

        cv::Mat coloredMask;
        cv::cvtColor(maskDisplay, coloredMask, cv::COLOR_GRAY2BGR);
        coloredMask.setTo(cv::Scalar(0, 255, 0), maskDisplay);

        if (!runningFlag())
        {
            return;
        }

        // Apply a transparency to the mask.

        cv::Mat alphaMask;
        maskDisplay.convertTo(alphaMask, CV_32F, 1.0/255.0);

        if (!runningFlag())
        {
            return;
        }

        // Merge the semi-transparent mask over original image.

        for (int y = 0 ; runningFlag() && (y < inputBGR.rows) ; y++)
        {
            for (int x = 0 ; runningFlag() && (x < inputBGR.cols) ; x++)
            {
                float alpha                = alphaMask.at<float>(y, x);
                cv::Vec3b& pixel           = overlay.at<cv::Vec3b>(y, x);
                const cv::Vec3b& maskPixel = coloredMask.at<cv::Vec3b>(y, x);

                pixel[0] = static_cast<uchar>(alpha * maskPixel[0] + (1.0f - alpha) * pixel[0]);
                pixel[1] = static_cast<uchar>(alpha * maskPixel[1] + (1.0f - alpha) * pixel[1]);
                pixel[2] = static_cast<uchar>(alpha * maskPixel[2] + (1.0f - alpha) * pixel[2]);
            }
        }

        // Send the signal to render the mask preview in the GUI.

        QImage maskQImage = QtOpenCVImg::mat2Image(overlay);

        Q_EMIT signalSegmentedMask(maskQImage.convertToFormat(QImage::Format_ARGB32));

        postProgress(50);

        if (!runningFlag())
        {
            return;
        }

        // Blur the background.

        cv::Mat blurred;
        cv::GaussianBlur(inputBGR, blurred, cv::Size(0, 0), d->radius);
        postProgress(60);

        if (!runningFlag())
        {
            return;
        }

        if (d->transition == 0)
        {
            // Uniform blur.

            inputBGR.copyTo(output, mask);
            blurred.copyTo(output, ~mask);
        }
        else
        {
            // Normalize transition parameter between 0.1 and 2.0 for better control.

            float transition = 0.1F + (d->transition / 100.0F) * 1.9F;

            // Progressive blur.

            cv::Mat distanceMap;
            cv::distanceTransform(~mask, distanceMap, cv::DIST_L2, cv::DIST_MASK_5);
            cv::normalize(distanceMap, distanceMap, 0, 1, cv::NORM_MINMAX);

            // Apply a non-linear transformation to the distance map based on transition parameter.

            for (int y = 0 ; runningFlag() && (y < distanceMap.rows) ; y++)
            {
                for (int x = 0 ; runningFlag() && (x < distanceMap.cols) ; x++)
                {
                    float dist = distanceMap.at<float>(y, x);
                    distanceMap.at<float>(y, x) = std::pow(dist, 1.0F / transition);
                }
            }

            // Create the result with the progressive blur.

            output = inputBGR.clone();
            postProgress(70);

            for (int y = 0 ; runningFlag() && (y < inputBGR.rows) ; y++)
            {
                for (int x = 0 ; runningFlag() && (x < inputBGR.cols) ; x++)
                {
                    float alpha                = distanceMap.at<float>(y, x);
                    output.at<cv::Vec3b>(y, x) = alpha * blurred.at<cv::Vec3b>(y, x) +
                                                 (1.0F - alpha) * inputBGR.at<cv::Vec3b>(y, x);
                }
            }
        }

        postProgress(80);

        if (!runningFlag())
        {
            return;
        }

        // Convert back to the original format if necessary.

        if (is16Bit)
        {
            DImg output8Bit(QtOpenCVImg::mat2Image(output));
            m_destImage = output8Bit;
            m_destImage.convertToSixteenBit();
        }
        else
        {
            m_destImage = DImg(QtOpenCVImg::mat2Image(output));
        }

        if (!m_orgImage.hasAlpha())
        {
            m_destImage.removeAlphaChannel();
        }

        postProgress(90);

        if (!runningFlag())
        {
            return;
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "BackgroundBlurFilter::filterImage: cv::Exception:" << e.what();

        Q_EMIT signalSegmentedMask(QImage());

        m_destImage = m_orgImage;
    }
    catch (...)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "BackgroundBlurFilter::filterImage: Default exception from OpenCV";

        Q_EMIT signalSegmentedMask(QImage());

        m_destImage = m_orgImage;
    }
}

FilterAction BackgroundBlurFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("radius"),     d->radius);
    action.addParameter(QLatin1String("transition"), d->transition);

    return action;
}

void BackgroundBlurFilter::readParameters(const FilterAction& action)
{
    d->radius     = action.parameter(QLatin1String("radius")).toInt();
    d->transition = action.parameter(QLatin1String("transition")).toInt();
}

} // namespace DigikamEditorBackgroundBlurToolPlugin

#include "moc_backgroundblurfilter.cpp"
