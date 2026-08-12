/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Foundation for all machine learning pipelines
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
#include <QIcon>
#include <QAtomicInteger>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadPool>
#include <QFuture>
#include <QFutureWatcher>

// Local includes

#include "digikam_export.h"
#include "digikam_opencv.h"
#include "mlpipelinepackagenotify.h"
#include "mlpipelinepackagefoundation.h"
#include "sharedqueue.h"
#include "mlpipelinemacros.h"
#include "actionthreadbase.h"

namespace Digikam
{

class MLPipelinePackage;

class DIGIKAM_EXPORT MLPipelineFoundation : public QObject
{
    Q_OBJECT

public:

    enum MLPipelineStage
    {
        /// Finder stage finds the data for the pipeline
        Finder,

        /// Loader stage loads and prepares the data for extraction
        Loader,

        /// Extractor stage pulls the features from the data
        Extractor,

        /// Classifier stage adds a label (face, autotag, etc) to an extracted object
        Classifier,

        /// Classifier stage adds a label (face, autotag, etc) to an extracted object
        Trainer,

        /// Writer stage saves the data to the DB
        Writer,

        /// Empty stage
        None
    };

    enum MLPipelineNotification
    {
        notifySkipped,
        notifyProcessed
    };

    typedef struct _MLPipelinePerformanceProfile
    {
        int elapsedTime                         = 0;
        int itemCount                           = 0;
        int maxQueueCount                       = 0;
        int maxElapsedTime                      = 0;

        QAtomicInteger<int> currentThreadCount;
        QAtomicInteger<int> maxThreadCount;
    }
    MLPipelinePerformanceProfile;

    typedef SharedQueue<MLPipelinePackageFoundation*> MLPipelineQueue;

public:

    MLPipelineFoundation();
    virtual ~MLPipelineFoundation();

    virtual bool start();
    virtual void cancel();

    bool hasFinished()      const;

Q_SIGNALS:

    /**
     * @brief Emitted when processing is scheduled.
     */
    void scheduled();

    /**
     * @brief Emitted when processing has started.
     */
    void started(const QString& message);

    /**
     * @brief Emitted when one package begins processing.
     */
    void processing(const MLPipelinePackageNotify::Ptr& package);

    /**
     * @brief Emitted when one package has finished processing.
     */
    void processed(const MLPipelinePackageNotify::Ptr& package);
    void progressValueChanged(float progress);

    /**
     * @brief Emitted when the last package has finished processing.
     */
    void finished();

    /**
     * @brief Emitted when one or several packages were skipped, usually because they have already been scanned.
     */
    void skipped(const MLPipelinePackageNotify::Ptr& package);

    void signalAddMoreWorkers();

    void signalUpdateItemCount(const qlonglong itemCount);

private Q_SLOTS:

    void slotFinished();
    void slotAddMoreWorkers();
    void slotCancel()                                   { cancel(); }

protected:

    QMap<MLPipelineStage, MLPipelineQueue*>             queues;
    const int                                           throttledQueueDepth = 1;
    QMutex                                              mutex;
    QMutex                                              threadStageMutex;
    QAtomicInteger<int>                                 itemsProcessed      = 0;
    QAtomicInteger<int>                                 totalItemCount      = 0;
    bool                                                cancelled           = false;
    QThreadPool*                                        threadPool          = nullptr;
    QList<QFutureWatcher<bool>* >                       watchList;
    QMap<MLPipelineStage, MLPipelinePerformanceProfile> performanceProfileList;
    quint64                                             maxBufferSize       = 2147483648;           ///< 2 GB default
    quint64                                             usedBufferSize      = 0;
    QIcon                                               missingIcon         = QIcon::fromTheme(QLatin1String("image-missing"));
    QElapsedTimer                                       pipelineTimer;

protected:

    virtual bool finder()                                        = 0;
    virtual bool loader()                                        = 0;
    virtual bool extractor()                                     = 0;
    virtual bool classifier()                                    = 0;       ///< @todo rename to postprocessor
    virtual bool trainer()                                       = 0;
    virtual bool writer()                                        = 0;

    virtual void addMoreWorkers()                                = 0;

    bool checkMoreWorkers(int totalItemCount, int currentItemCount, bool useFullCpu);

    /**
     * @brief Queue helper functions
     */

    MLPipelinePackageFoundation* queueEndSignal() const
    {
        return nullptr;
    }

    void clearAllQueues();

    virtual bool enqueue(MLPipelineQueue* thisQueue, MLPipelinePackageFoundation* package);
    virtual MLPipelinePackageFoundation* dequeue(MLPipelineQueue* thisQueue);

    bool addWorker(const MLPipelineStage& stage);

    void waitForStart()
    {
        QMutexLocker lock(&mutex);
    }

    void stageStart(QThread::Priority threadPriority,
                    MLPipelineStage thisStage,
                    MLPipelineStage nextStage,
                    MLPipelineQueue*& thisQueue,
                    MLPipelineQueue*& nextQueue);

    void stageEnd(MLPipelineStage thisStage, MLPipelineStage nextStage);

    virtual void notify(MLPipelineNotification notification,
                        const QString& _name,
                        const QString& _path,
                        const QString& _displayData,
                        int _processed,
                        const QImage& _thumbnail);

    virtual void notify(MLPipelineNotification notification,
                        const QString& _name,
                        const QString& _path,
                        const QString& _displayData,
                        int _processed,
                        const DImg& _thumbnail);

    virtual void notify(MLPipelineNotification notification,
                        const QString& _name,
                        const QString& _path,
                        const QString& _displayData,
                        int _processed,
                        const QIcon& _thumbnail);

    /**
     * @brief Helper methods to perform signal emitting through the MLPIPELINE_FINDER_END mocro with the moc processor.
     */
    void emitSignalUpdateItemCount(const qlonglong itemCount);

    /**
     * @brief Methods to profile the Pipeline performance.
     */
    void pipelinePerformanceStart(const MLPipelineStage& stage, QElapsedTimer& timer);
    void pipelinePerformanceEnd(const MLPipelineStage& stage, QElapsedTimer& timer);
    void pipelinePerformanceEnd(const MLPipelineStage& stage, int totalItemCount, QElapsedTimer& timer);
    void showPipelinePerformance() const;

private:

    /// @note disabled
    explicit MLPipelineFoundation(QObject* const)                = delete;
    MLPipelineFoundation(const MLPipelineFoundation&)            = delete;
    MLPipelineFoundation& operator=(const MLPipelineFoundation&) = delete;
};

} // namespace Digikam
