/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Aesthetic detection based on deep learning
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "aesthetic_detector.h"

// Qt includes

#include <QStandardPaths>
#include <QMutexLocker>

// Local includes

#include "digikam_debug.h"
#include "systemsettings.h"

namespace Digikam
{

cv::dnn::Net AestheticDetector::s_model      = cv::dnn::Net();
QMutex       AestheticDetector::s_modelMutex = QMutex();

AestheticDetector::AestheticDetector()
    : AbstractDetector()
{
}

float AestheticDetector::detect(const cv::Mat& image) const
{
    try
    {
        QMutexLocker locker(&s_modelMutex);

        cv::Mat input = preprocess(image);

        if (!s_model.empty())
        {
            s_model.setInput(input);
            cv::Mat out = s_model.forward();

            return postProcess(out);
        }
        else
        {
            qCCritical(DIGIKAM_DETECTOR_LOG) << "Cannot load Aesthetic DNN model";

            return (-1.0F);
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::detect: cv::Exception:" << e.what();

        return (-1.0F);
    }
    catch (...)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::detect: Default exception from OpenCV";

        return (-1.0F);
    }
}

cv::Mat AestheticDetector::preprocess(const cv::Mat& image) const
{
    try
    {
        cv::Mat img_rgb;
        cv::cvtColor(image, img_rgb, cv::COLOR_BGR2RGB);
        cv::Mat cv_resized;
        cv::resize(img_rgb, cv_resized, cv::Size(299, 299), 0, 0, cv::INTER_NEAREST_EXACT);
        cv_resized.convertTo(cv_resized, CV_32FC3);
        cv_resized   = cv_resized.mul(1.0F / 127.5F);
        subtract(cv_resized, cv::Scalar(1, 1, 1), cv_resized);

        cv::Mat blob = cv::dnn::blobFromImage(cv_resized, 1, cv::Size(299, 299), cv::Scalar(0, 0, 0), false, false);

        return blob;
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::preprocess: cv::Exception:" << e.what();

        return cv::Mat();
    }
    catch (...)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::preprocess: Default exception from OpenCV";

        return cv::Mat();
    }
}

float AestheticDetector::postProcess(const cv::Mat& modelOutput) const
{
    try
    {
        cv::Point maxLoc;
        cv::minMaxLoc(modelOutput, nullptr, nullptr, nullptr, &maxLoc);
        qCDebug(DIGIKAM_DETECTOR_LOG) << "class : " << maxLoc.x;

        return float(maxLoc.x);
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::postProcess: cv::Exception:" << e.what();

        return float(cv::Point().x);
    }
    catch (...)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::postProcess: Default exception from OpenCV";

        return float(cv::Point().x);
    }
}

bool AestheticDetector::s_loadModel()
{
    try
    {
        QMutexLocker locker(&s_modelMutex);

        if (!s_model.empty())
        {
            return true;
        }

        SystemSettings system(QLatin1String("digikam"));
        QString appPath = system.getFacesEnginePath();

        QString model   = QLatin1String("weights_inceptionv3_299.pb");
        QString nnmodel = appPath + QLatin1Char('/') + model;

        if (QFileInfo::exists(nnmodel))
        {
            qCDebug(DIGIKAM_DETECTOR_LOG) << "Aesthetic detector model:" << model;

#ifdef Q_OS_WIN

            s_model = cv::dnn::readNetFromTensorflow(nnmodel.toLocal8Bit().constData());

#else

            s_model = cv::dnn::readNetFromTensorflow(nnmodel.toStdString());

#endif
            s_model.setPreferableBackend(cv::dnn::DNN_TARGET_CPU);
            s_model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

#if (OPENCV_VERSION == QT_VERSION_CHECK(4, 7, 0))

            s_model.enableWinograd(false);

#endif

            return true;
        }
        else
        {
            qCCritical(DIGIKAM_DETECTOR_LOG) << "Cannot found Aesthetic DNN model" << nnmodel;
            qCCritical(DIGIKAM_DETECTOR_LOG) << "Aesthetic detection feature cannot be used!";

            return false;
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::s_loadModel: cv::Exception:" << e.what();

        return false;
    }
    catch (...)
    {
       qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::s_loadModel: Default exception from OpenCV";

       return false;
    }
}

void AestheticDetector::s_unloadModel()
{
    try
    {
        QMutexLocker locker(&s_modelMutex);

        if (!s_model.empty())
        {
            s_model = cv::dnn::Net();
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::s_unloadModel: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::s_unloadModel: Default exception from OpenCV";
    }
}

bool AestheticDetector::s_isEmptyModel()
{
    try
    {
        QMutexLocker locker(&s_modelMutex);

        return (s_model.empty());
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::s_isEmptyModel: cv::Exception:" << e.what();

        return true;
    }
    catch (...)
    {
        qCWarning(DIGIKAM_DETECTOR_LOG) << "AestheticDetector::s_isEmptyModel: Default exception from OpenCV";

        return true;
    }
}

} // namespace Digikam

#include "moc_aesthetic_detector.cpp"
