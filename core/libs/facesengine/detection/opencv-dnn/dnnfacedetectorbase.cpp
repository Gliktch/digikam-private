/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-08-08
 * Description : Base class to perform low-level neural network inference
 *               for face detection
 *
 * SPDX-FileCopyrightText: 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnfacedetectorbase.h"

// Qt includes

#include <QtGlobal>
#include <QRect>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

// TODO: re-verify these threshold.

int   DNNFaceDetectorBase::uiConfidenceThreshold  = DNN_MODEL_THRESHOLD_NOT_SET;
float DNNFaceDetectorBase::nmsThreshold           = 0.4F;

// --------------------------------------------------------------------------------------------------------

DNNFaceDetectorBase::DNNFaceDetectorBase(float scale,
                                         const cv::Scalar& val,
                                         const cv::Size& inputImgSize)
    : scaleFactor      (scale),
      meanValToSubtract(val),
      inputImageSize   (inputImgSize)
{
}

cv::Size DNNFaceDetectorBase::nnInputSizeRequired() const
{
    return inputImageSize;
}

// TODO: these confidence boxes usually go together --> use a class container to encapsulate them?

void DNNFaceDetectorBase::selectBbox(const cv::Size& paddedSize,
                                     float confidence,
                                     int left,
                                     int right,
                                     int top,
                                     int bottom,
                                     std::vector<float>& goodConfidences,
                                     std::vector<cv::Rect>& goodBoxes,
                                     std::vector<float>& doubtConfidences,
                                     std::vector<cv::Rect>& doubtBoxes) const
{
    int width        = right  - left;
    int height       = bottom - top;

    cv::Rect bbox(left, top, width, height);

    // Take the net size of image.

    int borderLeft   = paddedSize.width;
    int borderRight  = inputImageSize.width  - paddedSize.width;
    int borderTop    = paddedSize.height;
    int borderBottom = inputImageSize.height - paddedSize.height;

    /**
     * Classify bounding boxes detected.
     * Good bounding boxes are defined as boxes that reside within the non-padded zone or
     * those that are out only for min of (10% of padded range, 10% of bbox dim).
     *
     * Bad bounding boxes are defined as boxes that have at maximum 25% of each dimension
     * out of non-padded zone.
     */

    if (
        (left   >= (int)cv::min(borderLeft * 0.9,                       borderLeft   - 0.1 * width))      &&
        (right  <= (int)cv::max(borderRight  + 0.1 * paddedSize.width,  borderRight  + 0.1 * width))      &&
        (top    >= (int)cv::min(borderTop  * 0.9,                       borderTop    - 0.1 * height))     &&
        (bottom <= (int)cv::max(borderBottom + 0.1 * paddedSize.height, borderBottom + 0.1 * height))
       )
    {
        goodBoxes.push_back(bbox);
        goodConfidences.push_back(confidence);

        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Good rect = " << QRect(bbox.x, bbox.y, bbox.width, bbox.height)
                                         << ", conf = " << confidence;
    }

    else if (
             (right  >  left)                             &&
             (right  >= (borderLeft   + 0.75 * width))    &&
             (left   <= (borderRight  - 0.75 * width))    &&
             (bottom >  top)                              &&
             (bottom >= (borderRight  + 0.75 * height))   &&
             (top    <= (borderBottom - 0.75 * height))
            )
    {
        doubtBoxes.push_back(bbox);
        doubtConfidences.push_back(confidence);

        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Doubt rect = " << QRect(bbox.x, bbox.y, bbox.width, bbox.height)
                                         << ", conf = "     << confidence;
    }
}

void DNNFaceDetectorBase::correctBbox(cv::Rect& bbox, const cv::Size& paddedSize) const
{
    // TODO: Should the box be cropped to square or not?

    int left    = cv::max(bbox.x - paddedSize.width,  0);
    int right   = cv::min(left   + bbox.width,        inputImageSize.width  - 2 * paddedSize.width);
    int top     = cv::max(bbox.y - paddedSize.height, 0);
    int bottom  = cv::min(top    + bbox.height,       inputImageSize.height - 2 * paddedSize.height);

    bbox.x      = left;
    bbox.y      = top;
    bbox.width  = right  - left;
    bbox.height = bottom - top;
}

} // namespace Digikam
