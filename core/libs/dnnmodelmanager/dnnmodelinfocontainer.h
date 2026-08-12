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

#pragma once

// Qt includes

#include <QMutex>
#include <QVersionNumber>

// Local includes

#include "digikam_opencv.h"
#include "digikam_export.h"
#include "filesdownloader.h"
#include "dnnmodeldefinitions.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNNModelInfoContainer
{

public:

    // ---------- public methods ----------

    DNNModelInfoContainer()                                         = default;
    DNNModelInfoContainer(const DNNModelInfoContainer&);
    explicit DNNModelInfoContainer(
                                   const QString&           _displayName,
                                   const QString&           _toolTip,
                                   const QString&           _fileName,
                                   const DNNModelUsageList& _usage,
                                   const QVersionNumber&    _minVersion,
                                   const QString&           _downloadPath,
                                   const QString&           _sha256,
                                   const qint64&            _fileSize,
                                   int                      _defaultThreshold,
                                   int                      _minUsableThreshold,
                                   int                      _maxUsableThreshold,
                                   DNNLoaderType            _loaderType,
                                   const QString&           _preprocessor,
                                   const QString&           _classifier,
                                   const QString&           _classList,
                                   const QString&           _configName,
                                   const cv::Scalar&        _meanValToSubtract,
                                   int                      _imageSize
                                  );
    ~DNNModelInfoContainer()                                        = default;

    DNNModelInfoContainer& operator=(const DNNModelInfoContainer&);
    DNNModelInfoContainer& operator=(DNNModelInfoContainer&&);

    bool operator==(const DNNModelInfoContainer& t) const;

public:

    QString            displayName;                                     ///< Name used for display in UI (QComboBox).
    QString            toolTip;                                         ///< Tooltip for the model.
    QString            fileName;                                        ///< Used by the downloader and model loader.
    DNNModelUsageList  usage;                                           ///< How the model can be used. | for more than one use. face_detection, face_recognition, weight, object_detection, etc...
    QVersionNumber     minVersion;                                      ///< Minimum version of digiKam needed to use this model.
    QString            downloadPath;                                    ///< Used by the downloader for the download path.
    QString            sha256;                                          ///< SHA265 hash of the file for download.
    QString            preprocessor;                                    ///< Name of preprocessor type.
    QString            classifier;                                      ///< Name of classifier type.
    QString            classList;                                       ///< Name of model containing list of class names for classification.
    QString            configName;

    qint64             fileSize            = 0;                         ///< Used by the downloader to verify size.
    int                defaultThreshold    = 0;                         ///< Threshold used for models that aren't configured by the UI.
    int                minUsableThreshold  = 0;                         ///< Used to convert UI 1-10 slider to float for processing.
    int                maxUsableThreshold  = 0;                         ///< Used to convert UI 1-10 slider to float for processing.
    DNNLoaderType      loaderType          = DNNLoaderNet;              ///< Model loader type custom (YuNet/SFace), Caffe, Darknet, Torch, Tensorflow.
    cv::Scalar         meanValToSubtract   = cv::Scalar(0.0, 0.0, 0.0);
    int                imageSize           = 0;                         ///< Max dimension of a side of an image.
};

} // namespace Digikam
