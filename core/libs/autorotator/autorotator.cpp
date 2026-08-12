/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-03-10
 * Description : Autorotator class to automatically rotate images
 *
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autorotator.h"

// Qt includes

#include <QException>
#include <QElapsedTimer>

// digiKam includes

#include "digikam_debug.h"
#include "dnnmodelmanager.h"
#include "qtopencvimg.h"
#include "previewloadthread.h"

namespace Digikam
{

AutoRotator::AutoRotator()
{
    if (!loadModel())
    {
        QException e;
        e.raise();
    }
}

bool AutoRotator::loadModel()
{
    // load the model from the DNNModelManager

    model = static_cast<DNNModelNet*>(DNNModelManager::instance()->getModel(QLatin1String("AutoRotate"),
                                                                            DNNModelUsage::DNNUsageAutoRotate));
    // check if the model was loaded

    if (model && !model->modelLoaded)
    {
        try
        {
            // verify we can get the underlying neural net

            model->getNet();
        }
        catch (cv::Exception& e)
        {
            qCWarning(DIGIKAM_AUTOROTATE_LOG) << "AutoRotator::loadModel: cv::Exception:" << e.what();

            return false;
        }
        catch (...)
        {
            qCWarning(DIGIKAM_AUTOROTATE_LOG) << "AutoRotator::loadModel: Default exception from OpenCV";

            return false;
        }
    }

    if (model && model->modelLoaded)
    {
        qCDebug(DIGIKAM_AUTOROTATE_LOG) << "AutoRotate model ready";
    }
    else
    {
        qCWarning(DIGIKAM_AUTOROTATE_LOG) << "AutoRotate model not loaded";

        return false;
    }

    return true;
}

cv::Mat AutoRotator::Preprocess(const DImg& image)
{
    // load the image to a cvMat

    cv::Mat cvImage = QtOpenCVImg::image2Mat(image,
                                             CV_8UC3,
                                             QtOpenCVImg::MatColorOrder::MCO_BGR);

    // resize the image to the correct input size

    if (std::max(cvImage.cols, cvImage.rows) > std::max(model->info.imageSize, model->info.imageSize))
    {
        // Image should be resized.

        float resizeFactor      = std::min(static_cast<float>(model->info.imageSize) / static_cast<float>(cvImage.cols),
                                           static_cast<float>(model->info.imageSize) / static_cast<float>(cvImage.rows));

        int newWidth            = (int)(resizeFactor * cvImage.cols);
        int newHeight           = (int)(resizeFactor * cvImage.rows);
        cv::resize(cvImage, cvImage, cv::Size(newWidth, newHeight));
    }

    // pad the image to the correct input size

    int padX                = (model->info.imageSize - cvImage.cols) / 2;
    int padY                = (model->info.imageSize - cvImage.rows) / 2;
    cv::copyMakeBorder(cvImage, cvImage, padY, padY, padX, padX,
                       cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    // Convert image to the correct input format for blob

    cvImage.convertTo(cvImage, CV_32F, 1.0 / 255.0);
    cv::Mat blob = cv::dnn::blobFromImage(cvImage, 1.0, cv::Size(model->info.imageSize,
                                                                 model->info.imageSize),
                                          cv::Scalar(0, 0, 0), false, false);

    return blob;
}

bool AutoRotator::shouldRotate(int degrees, int sensitivity, int angle)
{
    // check if the angle is within the range of the degrees +/- sensitivity

    if (
        (angle > (degrees - ((sensitivity * 2) + (baseArc / 2)))) &&
        (angle < (degrees + ((sensitivity * 2) + (baseArc / 2))))
       )
    {
        return true;
    }
    else
    {
        return false;
    }
}

float AutoRotator::rotationAngle(const DImg& img, bool copyDImg)
{
    if (copyDImg)
    {
        // make a copy of the image to avoid modifying the original

        DImg copy = img.copy();

        return privateRotationAngle(copy);
    }
    else
    {
        // use the original image

        return privateRotationAngle(img);
    }
}

float AutoRotator::privateRotationAngle(const DImg& img)
{
    QElapsedTimer timer;
    timer.start();

    // get a cv::Mat blob from the DImg

    cv::Mat blob     = Preprocess(img);

    // lock the model mutex

    QMutexLocker locker(&model->mutex);

    // get the underlying cv::net from the model

    cv::dnn::Net net = model->getNet();

    // process the image

    net.setInput(blob);
    cv::Mat result   = net.forward();
    float angle      = result.at<float>(0, 0);

    qCDebug(DIGIKAM_AUTOROTATE_LOG) << "AutoRotator::rotationAngle Rotation angle:"
                                    << angle << "Elapsed time:"
                                    << timer.elapsed() << "ms";

    return angle;
}

MetaEngineRotation::TransformationAction AutoRotator::rotationOrientation(const DImg& image,
                                                                          int sensitivity,
                                                                          bool copyDImg)
{
    // check for corrupted images that cannot be loaded

    if (image.isNull())
    {
        qCWarning(DIGIKAM_AUTOROTATE_LOG) << "AutoRotator::rotationOrientation: image is null";

        return MetaEngineRotation::NoTransformation;
    }

    // get the free rotation angle from the model

    int angle = rotationAngle(image, copyDImg);

    // convert the angle to a positive value

    if (angle < 0)
    {
        angle = 360 + angle;
    }

    // rotate in 90 degree increments

    MetaEngineRotation::TransformationAction rotation = MetaEngineRotation::NoTransformation;

    if      (shouldRotate(90, sensitivity, angle))
    {
        rotation = MetaEngineRotation::Rotate90;
    } 
    else if (shouldRotate(180, sensitivity, angle))
    {
        rotation = MetaEngineRotation::Rotate180;
    }
    else if (shouldRotate(270, sensitivity, angle))
    {
        rotation = MetaEngineRotation::Rotate270;
    }

    return rotation;
}

MetaEngineRotation::TransformationAction AutoRotator::rotationOrientation(const QString& filePath, int sensitivity)
{
    DImg image = PreviewLoadThread::loadSynchronously(filePath,
                                                      PreviewSettings(),
                                                      model->info.imageSize);

    return rotationOrientation(image, sensitivity, false);
}

} // namespace Digikam
