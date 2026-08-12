/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Integrated, multithread face detection / recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>
#include <QSemaphore>
#include <QAtomicInteger>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadPool>
#include <QFuture>
#include <QFutureWatcher>

// Local includes

#include "mlpipelinefoundation.h"
#include "facescansettings.h"
#include "dnnmodelbase.h"

namespace Digikam
{

class FacePipelineBase : public MLPipelineFoundation
{
    Q_OBJECT

public:

    enum FilterMode
    {
        /// Will read any given image.
        ScanAll,

        /// Scan new images, will skip any image that is already marked as scanned.
        ScanNew,

        /// Adds new face(s) to training
        TrainNew,

        /// Retrains the face DB
        TrainAll,

        /// Removes the face(s) from training
        TrainRemove,

        /// Removes all face training, sets all images to not scanned
        TrainReset
    };

    enum WriteMode
    {
        /// Write results. Merge with existing entries.
        NormalWrite,

        /// Add new results. Previous all results will be cleared.
        OverwriteAllFaces,

        /// Add new results. Previous unconfirmed results will be cleared.
        OverwriteUnconfirmed
    };

public:

    explicit FacePipelineBase(const FaceScanSettings& _settings);
    virtual ~FacePipelineBase()                                                     override;

protected:

    DNNModelBase*               detectorModel       = nullptr;
    double                      blurThreshold       = 4.87F;
    double                      noiseThreshold1     = 3.7;
    double                      noiseThreshold2     = 1.89;
    const float                 minThumbnailSize    = 0.4F;

    FaceScanSettings            settings;

protected:

    double detectNoise1(const cv::Mat& cvGrayImage) const;
    double detectNoise2(const cv::Mat& cvGrayImage) const;
    double detectBlur(const cv::Mat& cvGrayImage) const;
    bool useForTraining(const QSize& faceSize, const cv::Mat& image);

    bool commonFaceThumbnailLoader(const QString& pipelineName,
                                   QThread::Priority stagePriority,
                                   MLPipelineFoundation::MLPipelineStage thisStage,
                                   MLPipelineFoundation::MLPipelineStage nextStage);

    bool commonFaceThumbnailExtractor(const QString& pipelineName,
                                      QThread::Priority stagePriority,
                                      MLPipelineFoundation::MLPipelineStage thisStage,
                                      MLPipelineFoundation::MLPipelineStage nextStage,
                                      bool trainingQualityCheck = false);

    // queue helper functions

    bool enqueue(MLPipelineQueue* thisQueue, MLPipelinePackageFoundation* package)  override;

private:

    /// @note disabled

    FacePipelineBase()                                   = delete;
    FacePipelineBase(QObject* const)                     = delete;
    FacePipelineBase(const FacePipelineBase&)            = delete;
    FacePipelineBase& operator=(const FacePipelineBase&) = delete;

private:

    static const int BLOCK = 20;
};

} // namespace Digikam
