/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNNModelNet base class
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnmodelbase.h"

// Qt Includes

#include <QFileInfo>
#include <QStandardPaths>
#include <QApplication>

// Local includes

#include "digikam_debug.h"
#include "dnnmodelmanager.h"
#include "dnnmodelinfocontainer.h"
#include "systemsettings.h"

namespace Digikam
{

DNNModelBase::DNNModelBase(const DNNModelInfoContainer& _info)
    : info(_info)
{
}

DownloadInfo DNNModelBase::getDownloadInformation() const
{
    return DownloadInfo(
                        info.downloadPath,
                        info.fileName,
                        info.sha256,
                        info.fileSize
                       );
}

bool DNNModelBase::checkFilename() const
{
    QFileInfo fileInfo(getModelPath());

    return (fileInfo.exists() && (fileInfo.size() == info.fileSize));
}

const QString DNNModelBase::getModelPath() const
{
    SystemSettings system(QLatin1String("digikam"));
    QString appPath = system.getFacesEnginePath();

    if (!appPath.isEmpty())
    {
        QString modelPath = appPath + QLatin1Char('/') + info.fileName;

        return modelPath;
    }

    qCCritical(DIGIKAM_DNNMODELMNGR_LOG) << "Cannot find DNN models path";

    return QString();
}

const QPair<int, int> DNNModelBase::getBackendAndTarget() const
{
    int backend_id = cv::dnn::DNN_BACKEND_DEFAULT;
    int target_id  = cv::dnn::DNN_TARGET_CPU;

    // query OpenCV for capabilities. Return best match.

    try
    {
        SystemSettings system(qApp->applicationName());

        if (
            system.enableOpenCL    &&
            system.enableDnnOpenCL &&
            cv::ocl::haveOpenCL()  &&
            cv::ocl::useOpenCL()
           )
        {
            // use OpenCL if available

            backend_id = cv::dnn::DNN_BACKEND_OPENCV;
            target_id  = cv::dnn::DNN_TARGET_OPENCL;

            qCDebug(DIGIKAM_DNNMODELMNGR_LOG) << "Using OpenCV backend and OpenCL target";
        }
        else
        {
            // use CPU if OpenCL is not available

            qCDebug(DIGIKAM_DNNMODELMNGR_LOG) << "Using default backend and CPU target";
        }
    }
    catch(const std::exception& e)
    {
        qCDebug(DIGIKAM_DNNMODELMNGR_LOG) << "DNNModelBase::getBackendAndTarget: error checking OpenCL capabilities. " << e.what();
    }
    catch(...)
    {
        qCDebug(DIGIKAM_DNNMODELMNGR_LOG) << "DNNModelBase::getBackendAndTarget: error checking OpenCL capabilities. ";
    }

    const std::map<std::string, int> str2backend
    {
        { "default",     cv::dnn::DNN_BACKEND_DEFAULT          },
#if OPENCV_TEST_VERSION(5,0,0)
        { "halide",      cv::dnn::DNN_BACKEND_HALIDE           },
#endif
        { "ie",          cv::dnn::DNN_BACKEND_INFERENCE_ENGINE },
        { "opencv",      cv::dnn::DNN_BACKEND_OPENCV           },
        { "cuda",        cv::dnn::DNN_BACKEND_CUDA             }
    };

    const std::map<std::string, int> str2target
    {
        { "cpu",         cv::dnn::DNN_TARGET_CPU               },
        { "opencl",      cv::dnn::DNN_TARGET_OPENCL            },
        { "myriad",      cv::dnn::DNN_TARGET_MYRIAD            },
        { "vulkan",      cv::dnn::DNN_TARGET_VULKAN            },
        { "opencl_fp16", cv::dnn::DNN_TARGET_OPENCL_FP16       },
        { "cuda",        cv::dnn::DNN_TARGET_CUDA              },
        { "cuda_fp16",   cv::dnn::DNN_TARGET_CUDA_FP16         }
    };

    // Env vars for testing combinations.

    QString cvBackend    = QString::fromLocal8Bit(qgetenv("DIGIKAM_DNN_BACKEND"));
    QString cvTarget     = QString::fromLocal8Bit(qgetenv("DIGIKAM_DNN_TARGET"));

    if (cvBackend.length() > 0)
    {
        try
        {
            backend_id = str2backend.at(cvBackend.toLower().toUtf8().data());
            qCDebug(DIGIKAM_DNNMODELMNGR_LOG) << "Using OpenCV backend:" << cvBackend;
        }
        catch (...)
        {
            qCWarning(DIGIKAM_DNNMODELMNGR_LOG) << "Invalid OpenCV backend:" << cvBackend;
        }
    }

    if (cvTarget.length() > 0)
    {
        try
        {
            target_id = str2target.at(cvTarget.toLower().toUtf8().data());
            qCDebug(DIGIKAM_DNNMODELMNGR_LOG) << "Using OpenCV target:" << cvTarget;
        }
        catch (...)
        {
            qCWarning(DIGIKAM_DNNMODELMNGR_LOG) << "Invalid OpenCV target:" << cvTarget;
        }
    }

    // Return the result.

    return QPair<int, int>(backend_id, target_id);
}

float DNNModelBase::getThreshold(int uiThreshold) const
{
    float threshold = 0.5F;

    if (DNN_MODEL_THRESHOLD_NOT_SET == uiThreshold)
    {
        if (info.defaultThreshold > 100)
        {
            threshold = (float)info.defaultThreshold / 1000.0F;
        }
        else
        {
            threshold = (float)info.defaultThreshold / 100.0F;
        }
    }
    else
    {
        float increment = (float)(info.maxUsableThreshold - info.minUsableThreshold) / 9.0F;
        threshold       = (((float)(uiThreshold - 1.0F) * increment) + (float)info.minUsableThreshold) / 100.F;
    }

    if (info.usage.contains(DNNModelUsage::DNNUsageFaceRecognition))
    {
        // The threshold is the inverse for face recognition.

        threshold = 1.0F - threshold;
    }

    return threshold;
}

} // namespace Digikam
