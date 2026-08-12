/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-07-22
 * Description : Class to perform faces detection using OpenCV DNN module
 *
 * SPDX-FileCopyrightText: 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>
#include <QList>
#include <QRect>

// Local includes

#include "digikam_opencv.h"
#include "dimg.h"
#include "dnnfacedetectorbase.h"

namespace Digikam
{

enum DetectorNNModel
{
    DNNDetectorSSD = 0,   ///< SSD MobileNet neural network inference [https://github.com/arunponnusamy/cvlib]
    DNNDetectorYOLOv3,    ///< YOLO neural network inference          [https://github.com/sthanhng/yoloface]
    DNNDetectorYuNet      ///< YuNet neural network inference         [https://github.com/opencv/opencv_zoo/tree/main]
};

class DIGIKAM_EXPORT OpenCVDNNFaceDetector
{

public:

    explicit OpenCVDNNFaceDetector(DetectorNNModel model = DetectorNNModel::DNNDetectorYuNet);
    ~OpenCVDNNFaceDetector();

    cv::Mat prepareForDetection(const DImg& inputImage, cv::Size& paddedSize)           const;
    cv::Mat prepareForDetection(const QImage& inputImage, cv::Size& paddedSize)         const;
    cv::Mat prepareForDetection(const QString& inputImagePath, cv::Size& paddedSize)    const;
    cv::Mat prepareForDetectionYuNet(cv::Mat& cvImage, cv::Size& paddedSize)            const;

    void setAccuracy(const int accuracy);
    void setFaceDetectionSize(FaceScanSettings::FaceDetectionSize size);

    QList<QRect> detectFaces(const cv::Mat& inputImage, const cv::Size& paddedSize);
    std::vector<cv::Rect> cvDetectFaces(const cv::Mat& inputImage, const cv::Size& paddedSize);

    /**
     * @return the image size (one dimension).
     * recommended for face detection. If the image is considerably larger, it will be rescaled automatically.
     */
    static int recommendedImageSizeForDetection();

private:

    cv::Mat prepareForDetection(cv::Mat& cvImage, cv::Size& paddedSize)                 const;

private:

    /// @note disabled
    OpenCVDNNFaceDetector(const OpenCVDNNFaceDetector&)            = delete;
    OpenCVDNNFaceDetector& operator=(const OpenCVDNNFaceDetector&) = delete;

private:

    DetectorNNModel      m_modelType       = DNNDetectorYuNet;
    DNNFaceDetectorBase* m_inferenceEngine = nullptr;
};

} // namespace Digikam
