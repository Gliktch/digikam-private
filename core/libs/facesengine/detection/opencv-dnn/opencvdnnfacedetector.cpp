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

#include "opencvdnnfacedetector.h"

// C++ includes

#include <vector>

// Qt includes

#include <QtGlobal>
#include <QStandardPaths>
#include <qmath.h>

// Local includes

#include "digikam_debug.h"
#include "dnnfacedetectoryunet.h"

namespace Digikam
{

OpenCVDNNFaceDetector::OpenCVDNNFaceDetector(DetectorNNModel model)
    : m_modelType(model)
{
    switch (m_modelType)
    {
        case DetectorNNModel::DNNDetectorYuNet:
        {
            m_inferenceEngine = new DNNFaceDetectorYuNet;
            break;
        }

        default:
        {
            qFatal("UNKNOWN neural network model");
        }
    }
}

OpenCVDNNFaceDetector::~OpenCVDNNFaceDetector()
{
    delete m_inferenceEngine;
}

int OpenCVDNNFaceDetector::recommendedImageSizeForDetection()
{
    return 800;
}

// TODO: prepareForDetection give different performances.

cv::Mat OpenCVDNNFaceDetector::prepareForDetection(const DImg& inputImage, cv::Size& paddedSize) const
{
    if (inputImage.isNull() || !inputImage.size().isValid())
    {
        return cv::Mat();
    }

    try
    {
        cv::Mat cvImage;
        int type               = (inputImage.sixteenBit() ? CV_16UC4 : CV_8UC4);
        cv::Mat cvImageWrapper = cv::Mat(inputImage.height(), inputImage.width(), type, inputImage.bits());

        // DImg is always 4 channel. Convert to 3 channel RGB

        cvtColor(cvImageWrapper, cvImage, cv::COLOR_RGBA2RGB);

        // convert to 8 bit if 16 bit

        if (type == CV_16UC4)
        {
            cvImage.convertTo(cvImage, CV_8UC3, 1 / 256.0);
        }

        if (DetectorNNModel::DNNDetectorYuNet == m_modelType)
        {
            return prepareForDetectionYuNet(cvImage, paddedSize);
        }
        else
        {
            return prepareForDetection(cvImage, paddedSize);
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: Default exception from OpenCV";

        return cv::Mat();
    }
}

cv::Mat OpenCVDNNFaceDetector::prepareForDetection(const QImage& inputImage, cv::Size& paddedSize) const
{
    if (inputImage.isNull() || !inputImage.size().isValid())
    {
        return cv::Mat();
    }

    try
    {
        cv::Mat cvImage;
        cv::Mat cvImageWrapper;
        QImage qimage(inputImage);

        switch (qimage.format())
        {
            case QImage::Format_RGB32:
            case QImage::Format_ARGB32:
            case QImage::Format_ARGB32_Premultiplied:
            {
                // I think we can ignore premultiplication when converting to grayscale.

                cvImageWrapper = cv::Mat(qimage.height(), qimage.width(), CV_8UC4,
                                         qimage.scanLine(0), qimage.bytesPerLine());
                cvtColor(cvImageWrapper, cvImage, cv::COLOR_RGBA2RGB);
                break;
            }

            default:
            {
                qimage         = qimage.convertToFormat(QImage::Format_RGB888);
                cvImageWrapper = cv::Mat(qimage.height(), qimage.width(), CV_8UC3,
                                         qimage.scanLine(0), qimage.bytesPerLine());
                // cvtColor(cvImageWrapper, cvImage, cv::COLOR_RGB2BGR);
                break;
            }
        }

        return prepareForDetection(cvImage, paddedSize);
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: Default exception from OpenCV";

        return cv::Mat();
    }
}

cv::Mat OpenCVDNNFaceDetector::prepareForDetection(const QString& inputImagePath, cv::Size& paddedSize) const
{
    try
    {
        std::vector<char> buffer;
        QFile file(inputImagePath);
        buffer.resize(file.size());

        if (!file.open(QIODevice::ReadOnly))
        {
            return cv::Mat();
        }

        file.read(buffer.data(), file.size());
        file.close();

        cv::Mat cvImage = cv::imdecode(std::vector<char>(buffer.begin(), buffer.end()), cv::IMREAD_COLOR);

        return prepareForDetection(cvImage, paddedSize);
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: Default exception from OpenCV";

        return cv::Mat();
    }
}

cv::Mat OpenCVDNNFaceDetector::prepareForDetection(cv::Mat& cvImage, cv::Size& paddedSize) const
{
    try
    {
        // Resize image before padding to fit in neural net.

        cv::Size inputImageSize = m_inferenceEngine->nnInputSizeRequired();
        float k                 = qMin(inputImageSize.width  * 1.0F / cvImage.cols,
                                       inputImageSize.height * 1.0F / cvImage.rows);

        int newWidth            = (int)(k * cvImage.cols);
        int newHeight           = (int)(k * cvImage.rows);
        cv::resize(cvImage, cvImage, cv::Size(newWidth, newHeight));

        // Pad with black pixels.

        int padX                = (inputImageSize.width  - newWidth)  / 2;
        int padY                = (inputImageSize.height - newHeight) / 2;

        cv::Mat imagePadded;

        cv::copyMakeBorder(cvImage, imagePadded,
                           padY, padY,
                           padX, padX,
                           cv::BORDER_CONSTANT,
                           cv::Scalar(0, 0, 0));

        paddedSize              = cv::Size(padX, padY);

        return imagePadded;
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "OpenCVDNNFaceDetector::prepareForDetection: Default exception from OpenCV";

        return cv::Mat();
    }
}

cv::Mat OpenCVDNNFaceDetector::prepareForDetectionYuNet(cv::Mat& cvImage, cv::Size& paddedSize) const
{
    try
    {
        cv::Size inputImageSize = m_inferenceEngine->nnInputSizeRequired();
        float resizeFactor      = 1.0F;

        if (std::max(cvImage.cols, cvImage.rows) > std::max(inputImageSize.width, inputImageSize.height))
        {
            // Image should be resized.  YuNet image sizes are much more flexible than SSD and YOLO
            // so we just need to make sure no one bound exceeds the max. No padding needed.

            resizeFactor            = std::min(static_cast<float>(inputImageSize.width)  / static_cast<float>(cvImage.cols),
                                               static_cast<float>(inputImageSize.height) / static_cast<float>(cvImage.rows));

            int newWidth            = (int)(resizeFactor * cvImage.cols);
            int newHeight           = (int)(resizeFactor * cvImage.rows);
            cv::resize(cvImage, cvImage, cv::Size(newWidth, newHeight));
        }

        paddedSize = cv::Size(0, 0); // Special case for YuNet.

        return cvImage;
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "prepareForDetectionYuNet: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "prepareForDetectionYuNet: Default exception from OpenCV";

        return cv::Mat();
    }
}

void OpenCVDNNFaceDetector::setAccuracy(const int accuracy)
{
    m_inferenceEngine->uiConfidenceThreshold = accuracy;
}

void OpenCVDNNFaceDetector::setFaceDetectionSize(FaceScanSettings::FaceDetectionSize size)
{
    m_inferenceEngine->setFaceDetectionSize(size);
}

QList<QRect> OpenCVDNNFaceDetector::detectFaces(const cv::Mat& inputImage,
                                                const cv::Size& paddedSize)
{
    std::vector<cv::Rect> detectedBboxes = cvDetectFaces(inputImage, paddedSize);

    QList<QRect> results;

/*
    cv::Mat imageTest = inputImage.clone();
*/
    for (const cv::Rect& bbox : detectedBboxes)
    {
        QRect rect(bbox.x, bbox.y, bbox.width, bbox.height);
        results << rect;
    }

    return results;
}

std::vector<cv::Rect> OpenCVDNNFaceDetector::cvDetectFaces(const cv::Mat& inputImage,
                                                           const cv::Size& paddedSize)
{
    std::vector<cv::Rect> detectedBboxes;

    m_inferenceEngine->detectFaces(inputImage, paddedSize, detectedBboxes);

    return detectedBboxes;
}

} // namespace Digikam
