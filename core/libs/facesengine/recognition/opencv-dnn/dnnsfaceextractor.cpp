/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-06-01
 * Description : Face recognition using deep learning.
 *               The internal DNN library interface.
 *
 * SPDX-FileCopyrightText: 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnsfaceextractor.h"

// Qt includes

#include <QMutex>
#include <QString>
#include <QFileInfo>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QUnhandledException>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dnnmodelmanager.h"
#include "dnnmodelsface.h"
#include "dnnmodelyunet.h"
#include "ocvocldnnsetter.h"

namespace Digikam
{

class Q_DECL_HIDDEN DNNSFaceExtractor::Private
{
public:

    Private()  = default;
    ~Private() = default;

public:

    int                             ref             = 1;

    // TODO: move d private to base. Move DNNModelBase* to d private. YuNet stays in SFace.

    DNNModelBase*                   model           = nullptr;
    DNNModelYuNet*                  detectorModel   = nullptr;
};

DNNSFaceExtractor::DNNSFaceExtractor()
    : DNNFaceExtractorBase(),
      d                   (new Private)
{
    // Virtual call in constructor: use dynamic binding.

    if (!this->loadModels())
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "Failed to load SFace model";
        std::runtime_error e("Failed to load SFace model");

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        std::exception_ptr p = std::make_exception_ptr(e);
        QUnhandledException(p).raise();

#else

        QUnhandledException().raise();

#endif

    }
}

DNNSFaceExtractor::~DNNSFaceExtractor()
{
    --(d->ref);

    if (d->ref == 0)
    {
        delete d;
    }
}

bool DNNSFaceExtractor::loadModels()
{
    d->model         = DNNModelManager::instance()->getModel(QLatin1String("SFace"), DNNModelUsage::DNNUsageFaceRecognition);
    d->detectorModel = static_cast<DNNModelYuNet*>(DNNModelManager::instance()->getModel(QLatin1String("YuNet"), DNNModelUsage::DNNUsageFaceDetection));

    try
    {
        if (d->model && d->detectorModel)
        {
            // Load the SFace model for feature extraction.

            if (!d->model->modelLoaded)
            {
                cv::Ptr<cv::FaceRecognizerSF> net = static_cast<DNNModelSFace*>(d->model)->getNet();

                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Extractor model:" << d->model->info.displayName;
            }

            // Load the YuNet model for 5-point face alignment.

            if (!d->detectorModel->modelLoaded)
            {
                cv::Ptr<cv::FaceDetectorYN> detNet = static_cast<DNNModelYuNet*>(d->detectorModel)->getNet();

                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Recognition model:" << d->detectorModel->info.displayName;
            }
        }

        else
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "SFace cannot load faces engine DNN models";
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "Faces recognition feature cannot be used!";

            return false;
        }
    }
    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "cv::Exception:" << e.what();

        return false;
    }
    catch (...)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "SFace cannot find faces engine DNN models";
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "Faces recognition feature cannot be used!";

        return false;
    }

    return true;
}

float DNNSFaceExtractor::getThreshold(int uiThreshold) const
{
    if (d->model)
    {
        return d->model->getThreshold(uiThreshold);
    }

    return 0.0F;
}

cv::Mat DNNSFaceExtractor::alignFace(const cv::Mat& inputImage) const
{
    cv::Mat alignedFace;
    static_cast<DNNModelSFace*>(d->model)->getNet()->alignCrop(inputImage, inputImage, alignedFace);

    return alignedFace;
}

cv::UMat DNNSFaceExtractor::alignFace(const cv::UMat& inputImage) const
{
    cv::UMat alignedFace;
    static_cast<DNNModelSFace*>(d->model)->getNet()->alignCrop(inputImage, inputImage, alignedFace);

    return alignedFace;
}

const QPair<cv::Mat, cv::Mat> DNNSFaceExtractor::getFaceEmbedding(const cv::Mat& faceImage) const
{
    QPair<cv::Mat, cv::Mat> result;
    cv::Mat normalized_descriptors = cv::Mat();

    QElapsedTimer timer;

    // Start the timer for profiling.

    timer.start();

    try
    {
        /*
         Note: Using an OpenCVOpenCDNNSetter is not needed when using a cv::Mat
         because cv::Mat does not use OpenCL.
        */

        // Add a border so there is room to rotate the image during alignment.

        cv::Mat borderFace;

        cv::copyMakeBorder(faceImage, borderFace,
                           60, 60,
                           60, 60,
                           cv::BORDER_CONSTANT,
                           cv::Scalar(0, 0, 0));

        if (
            d->model                        &&
            d->model->modelLoaded           &&
            d->detectorModel                &&
            d->detectorModel->modelLoaded
           )
        {
            QMutexLocker detectorLock(&d->detectorModel->mutex);

            // Redetect face using YuNet to get landmarks.

            cv::Mat faceLandmark;

            d->detectorModel->getNet()->setInputSize(borderFace.size());
            d->detectorModel->getNet()->setScoreThreshold(d->detectorModel->getThreshold(1));
            d->detectorModel->getNet()->detect(borderFace, faceLandmark);

            result.first = faceLandmark.clone();

            detectorLock.unlock();

            if (0 < faceLandmark.rows)
            {
                cv::Mat face_descriptors;
                cv::Mat alignedFace;

                QMutexLocker lock(&d->model->mutex);

                // Align and crop the face to standard size.

                static_cast<DNNModelSFace*>(d->model)->getNet()->alignCrop(borderFace, faceLandmark, alignedFace);

                borderFace.release();
                faceLandmark.release();

                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Finish aligning face in " << timer.elapsed() << " ms";
                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Start neural network";

                timer.start();

                static_cast<DNNModelSFace*>(d->model)->getNet()->feature(alignedFace, face_descriptors);

                alignedFace.release();

                if (face_descriptors.rows > 0)
                {
                    normalize(face_descriptors, normalized_descriptors);
                    result.second = normalized_descriptors;
                }
            }
            else
            {
                qCDebug(DIGIKAM_FACESENGINE_LOG) << "No face landmarks found";
            }
        }
    }
    catch (cv::Exception& e)
    {
        qCritical(DIGIKAM_FACESENGINE_LOG) << "DNNSFaceExtractor::getFaceEmbedding: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "DNNSFaceExtractor::getFaceEmbedding: Default exception from OpenCV";
    }

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Finish computing face embedding in "
                                     << timer.elapsed() << " ms";

    return result;
}

const QPair<cv::Mat, cv::Mat> DNNSFaceExtractor::getFaceEmbedding(const cv::UMat& faceImage) const
{
    QPair<cv::Mat, cv::Mat> result;
    cv::Mat normalized_descriptors = cv::Mat();

    QElapsedTimer timer;

    // Start the timer for profiling.

    timer.start();

    try
    {
        // lock OpenCV OpenCL DNN settings while using UMat

        OpenCVOpenCLDNNSetter openCLDNNSetter;

        // Add a border so there is room to rotate the image during alignment.

        cv::UMat borderFace;

        cv::copyMakeBorder(faceImage, borderFace,
                           60, 60,
                           60, 60,
                           cv::BORDER_CONSTANT,
                           cv::Scalar(0, 0, 0));

        if (
            d->model                        &&
            d->model->modelLoaded           &&
            d->detectorModel                &&
            d->detectorModel->modelLoaded
           )
        {
            QMutexLocker detectorLock(&d->detectorModel->mutex);

            // Redetect face using YuNet to get landmarks.

            cv::UMat faceLandmark;

            d->detectorModel->getNet()->setInputSize(borderFace.size());
            d->detectorModel->getNet()->setScoreThreshold(d->detectorModel->getThreshold(1));
            d->detectorModel->getNet()->detect(borderFace, faceLandmark);

            result.first = faceLandmark.getMat(cv::ACCESS_READ).clone();

            detectorLock.unlock();

            if (0 < faceLandmark.rows)
            {
                cv::Mat face_descriptors;
                cv::UMat uface_descriptors;
                cv::UMat alignedFace;

                QMutexLocker lock(&d->model->mutex);

                // Align and crop the face to standard size.

                static_cast<DNNModelSFace*>(d->model)->getNet()->alignCrop(borderFace, faceLandmark, alignedFace);

                borderFace.release();
                faceLandmark.release();

                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Finish aligning face in " << timer.elapsed() << " ms";
                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Start neural network";

                timer.start();

                static_cast<DNNModelSFace*>(d->model)->getNet()->feature(alignedFace, uface_descriptors);

                alignedFace.release();

                if (uface_descriptors.rows > 0)
                {
                    normalize(uface_descriptors.getMat(cv::ACCESS_FAST), normalized_descriptors);
                    result.second = normalized_descriptors;
                }
            }
            else
            {
                qCDebug(DIGIKAM_FACESENGINE_LOG) << "No face landmarks found";
            }
        }
    }
    catch (cv::Exception& e)
    {
        qCritical(DIGIKAM_FACESENGINE_LOG) << "DNNSFaceExtractor::getFaceEmbedding: cv::Exception:" << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "DNNSFaceExtractor::getFaceEmbedding: Default exception from OpenCV";
    }

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Finish computing face embedding in "
                                     << timer.elapsed() << " ms";

    return result;
}

} // namespace Digikam
