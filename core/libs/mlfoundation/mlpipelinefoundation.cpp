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

#include "mlpipelinefoundation.h"

// Qt includes

#include <QtConcurrentRun>
#include <QThread>
#include <QIcon>

// KDE includes

#include <kmemoryinfo.h>

// Local includes

#include "digikam_debug.h"
#include "sharedqueue.h"

namespace Digikam
{

MLPipelineFoundation::MLPipelineFoundation()
    : QObject()
{
    threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(qMax(8, QThread::idealThreadCount() * 2));

    connect(this, &MLPipelineFoundation::signalAddMoreWorkers,
            this, &MLPipelineFoundation::slotAddMoreWorkers);
}

MLPipelineFoundation::~MLPipelineFoundation()
{
    cancelled = true;

    threadPool->waitForDone();

    for (QFutureWatcher<bool>* const watcher : std::as_const(watchList))
    {
        watcher->waitForFinished();

        delete watcher;
    }
}

bool MLPipelineFoundation::start()
{
    KMemoryInfo memInfo;

    if (!memInfo.isNull())
    {
        quint64 available = memInfo.totalPhysical();
        maxBufferSize     = available / 4;
    }

    pipelineTimer.start();

    return true;
}

void MLPipelineFoundation::cancel()
{
    if (!cancelled)
    {
        /**
         * @note worker threads can be in 1 of 3 states when cancel is called
         *   1. waiting for a new package
         *   2. processing a package
         *   3. waiting to push a package
         *
         * handle all 3 cases so the worker thread sees the cancel signal
         */

        // set the cancel flag (case 2 above)

        cancelled = true;

        for (auto queue : std::as_const(queues))
        {
            // update the max queue size to something big

            // queue->setMaxDepth(queue->maxDepthLimit());
            queue->cancel(queueEndSignal());
        }

        // quick pause to let the other threads see the cancel signal

        QThread::msleep(100);

        for (auto queue : std::as_const(queues))
        {
            // pop the front of the queue to free up any threads waiting on the queue (case 3 above)

            if (1 < queue->size())
            {
                MLPipelinePackageFoundation* const package = queue->pop_front();

                if (queueEndSignal() != package)
                {
                    delete package;
                }
            }

            // send end of queue signal (case 1 above)

            queue->push_back(queueEndSignal());
        }

        // wait for all threads to finish

        while (!hasFinished())
        {
            QThread::msleep(100);
        }

        // clear the queues of any unprocessed packages

        clearAllQueues();
    }
}

bool MLPipelineFoundation::hasFinished() const
{
    bool result = true;

    for (const QFutureWatcher<bool>* watcher : std::as_const(watchList))
    {
        result &= watcher->future().isFinished();
    }

    if (result)
    {
        showPipelinePerformance();
    }

    return result;
}

bool MLPipelineFoundation::addWorker(const MLPipelineStage& stage)
{
    switch (stage)
    {
        case MLPipelineStage::Finder:
        {
            // always 1 finder thread, no incoming queue
            // start a new thread

            QFutureWatcher<bool>* const watcher = new QFutureWatcher<bool>(this);
            watcher->setFuture(QtConcurrent::run(threadPool,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                                 &MLPipelineFoundation::finder,
                                                 this

#else

                                                 this,
                                                 &MLPipelineFoundation::finder

#endif
                                                ));

            watchList.append(watcher);

            connect(watcher, &QFutureWatcher<bool>::finished,
                    this, &MLPipelineFoundation::slotFinished);

            break;
        }

        case MLPipelineStage::Loader:
        {
            // create a queue if one doesn't exist

            if (!queues.contains(MLPipelineStage::Loader))
            {
                queues.insert(MLPipelineStage::Loader, new MLPipelineQueue());
            }

            // start a new thread

            QFutureWatcher<bool>* const watcher = new QFutureWatcher<bool>(this);
            watcher->setFuture(QtConcurrent::run(threadPool,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                                 &MLPipelineFoundation::loader,
                                                 this

#else

                                                 this,
                                                 &MLPipelineFoundation::loader

#endif
                                                ));

            watchList.append(watcher);

            connect(watcher, &QFutureWatcher<bool>::finished,
                    this, &MLPipelineFoundation::slotFinished);

            break;
        }

        case MLPipelineStage::Extractor:
        {
            // create a queue if one doesn't exist

            if (!queues.contains(MLPipelineStage::Extractor))
            {
                queues.insert(MLPipelineStage::Extractor, new MLPipelineQueue());
            }

            // start a new thread

            QFutureWatcher<bool>* const watcher = new QFutureWatcher<bool>(this);
            watcher->setFuture(QtConcurrent::run(threadPool,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                                 &MLPipelineFoundation::extractor,
                                                 this

#else

                                                 this,
                                                 &MLPipelineFoundation::extractor

#endif
                                                ));

            watchList.append(watcher);

            connect(watcher, &QFutureWatcher<bool>::finished,
                    this, &MLPipelineFoundation::slotFinished);

            break;
        }

        case MLPipelineStage::Classifier:
        {
            // create a queue if one doesn't exist

            if (!queues.contains(MLPipelineStage::Classifier))
            {
                queues.insert(MLPipelineStage::Classifier, new MLPipelineQueue());
            }

            // start a new thread

            QFutureWatcher<bool>* const watcher = new QFutureWatcher<bool>(this);
            watcher->setFuture(QtConcurrent::run(threadPool,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                                 &MLPipelineFoundation::classifier,
                                                 this

#else

                                                 this,
                                                 &MLPipelineFoundation::classifier

#endif
                                                ));

            watchList.append(watcher);

            connect(watcher, &QFutureWatcher<bool>::finished,
                    this, &MLPipelineFoundation::slotFinished);

            break;
        }

        case MLPipelineStage::Trainer:
        {
            // create a queue if one doesn't exist

            if (!queues.contains(MLPipelineStage::Trainer))
            {
                queues.insert(MLPipelineStage::Trainer, new MLPipelineQueue());
            }

            // start a new thread

            QFutureWatcher<bool>* const watcher = new QFutureWatcher<bool>(this);
            watcher->setFuture(QtConcurrent::run(threadPool,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                                 &MLPipelineFoundation::trainer,
                                                 this

#else

                                                 this,
                                                 &MLPipelineFoundation::trainer

#endif
                                                ));

            watchList.append(watcher);

            connect(watcher, &QFutureWatcher<bool>::finished,
                    this, &MLPipelineFoundation::slotFinished);

            break;
        }

        case MLPipelineStage::Writer:
        {
            // create a queue if one doesn't exist

            if (!queues.contains(MLPipelineStage::Writer))
            {
                queues.insert(MLPipelineStage::Writer, new MLPipelineQueue());
            }

            // start a new thread

            QFutureWatcher<bool>* const watcher = new QFutureWatcher<bool>(this);
            watcher->setFuture(QtConcurrent::run(threadPool,

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                                 &MLPipelineFoundation::writer,
                                                 this

#else

                                                 this,
                                                 &MLPipelineFoundation::writer

#endif
                                                ));

            watchList.append(watcher);

            connect(watcher, &QFutureWatcher<bool>::finished,
                    this, &MLPipelineFoundation::slotFinished);

            break;
        }

        case MLPipelineStage::None:
        {
            // do nothing

            break;
        }
    }

    return true;
}

void MLPipelineFoundation::slotFinished()
{
    if (hasFinished())
    {
        Q_EMIT finished();
    }
}

bool MLPipelineFoundation::checkMoreWorkers(int _totalItemCount, int currentItemCount, bool useFullCpu)
{
    if (
        useFullCpu                                 &&
        ((_totalItemCount + currentItemCount) > 25) &&
        (QThread::idealThreadCount() > 4)
       )
    {
        int newInstances = qMin(3, (QThread::idealThreadCount() / 4) - 1);

        for (int i = 0 ; i < newInstances ; ++i)
        {
            qCDebug(DIGIKAM_MLPIPELINEFOUNDATION_LOG) << "MLPipelineFoundation::checkMoreWorkers: Sending signal to more workers";

            Q_EMIT signalAddMoreWorkers();
        }

        return true;
    }

    return false;
}

void MLPipelineFoundation::slotAddMoreWorkers()
{
    qCDebug(DIGIKAM_MLPIPELINEFOUNDATION_LOG) << "MLPipelineFoundation::slotAddMoreWorkers: Adding more workers";

    // call the derived class to add more workers

    addMoreWorkers();
}

void MLPipelineFoundation::clearAllQueues()
{
    for (MLPipelineQueue* const queue : std::as_const(queues))
    {
        // remove any incoming items

        while (!queue->isEmpty())
        {
            MLPipelinePackageFoundation* const package = queue->pop_front();

            if (queueEndSignal() != package)
            {
                delete package;
            }
        }
    }
}

bool MLPipelineFoundation::enqueue(MLPipelineQueue* thisQueue, MLPipelinePackageFoundation* package)
{
    if (!cancelled && (queueEndSignal() != package))
    {
        // check if buffer memory is full

        if ((package->size + usedBufferSize) > maxBufferSize)
        {
            // slow things down, but only if the cancelled flag is false

            thisQueue->setMaxDepth(throttledQueueDepth);
        }

        // check for 50% free buffer space

        if (
            (throttledQueueDepth == thisQueue->maxDepth()) &&
            ((package->size + usedBufferSize) < (maxBufferSize / 2))
           )
        {
            // speed things up

            thisQueue->setMaxDepth(QThread::idealThreadCount());
        }

        // add the package to the queue

        usedBufferSize += package->size;
        thisQueue->push_back(package);

        return true;
    }
    else
    {
        if (queueEndSignal() != package)
        {
            delete package;
        }

        return false;
    }
}

MLPipelinePackageFoundation* MLPipelineFoundation::dequeue(MLPipelineQueue* thisQueue)
{
    MLPipelinePackageFoundation* package = queueEndSignal();

    if (!cancelled)
    {
        package = thisQueue->pop_front();

        if (queueEndSignal() != package)
        {
            usedBufferSize -= package->size;
        }
    }

    return package;
}

void MLPipelineFoundation::stageStart(QThread::Priority threadPriority,
                                      MLPipelineStage thisStage,
                                      MLPipelineStage nextStage,
                                      MLPipelineQueue*& thisQueue,
                                      MLPipelineQueue*& nextQueue)
{
    QMutexLocker lock(&threadStageMutex);

    if (!performanceProfileList.contains(thisStage))
    {
        MLPipelinePerformanceProfile profile;
        profile.itemCount           = 0;
        profile.maxQueueCount       = 0;
        profile.elapsedTime         = 0;
        profile.maxElapsedTime      = 0;
        profile.currentThreadCount  = 1;
        profile.maxThreadCount      = 1;
        performanceProfileList.insert(thisStage, profile);
    }
    else
    {
        performanceProfileList[thisStage].currentThreadCount++;
        performanceProfileList[thisStage].maxThreadCount = qMax(performanceProfileList[thisStage].maxThreadCount,
                                                                performanceProfileList[thisStage].currentThreadCount);
    }

    waitForStart();
    QThread::currentThread()->setPriority(threadPriority);

    if ((MLPipelineStage::None != thisStage) && (MLPipelineStage::Finder != thisStage))
    {
        thisQueue = queues.value(thisStage);

        // no throttle on loader queue since it's only IDs and doesn't take up much memory
        // otherwise throttle to the ideal thread count

        if (MLPipelineStage::Loader != thisStage)
        {
            thisQueue->setMaxDepth(QThread::idealThreadCount());
        }
    }

    if (MLPipelineStage::None != nextStage)
    {
        nextQueue = queues.value(nextStage);
    }
}

void MLPipelineFoundation::stageEnd(MLPipelineStage thisStage, MLPipelineStage nextStage)
{
    QMutexLocker lock(&threadStageMutex);

    if (queues.contains(thisStage))
    {
        queues[thisStage]->setMaxDepth(queues[thisStage]->maxDepthLimit());

        // tell other threads to exit

        queues[thisStage]->push_back(queueEndSignal());
    }

    performanceProfileList[thisStage].currentThreadCount--;

    // last one out turns off the lights

    if (
        queues.contains(nextStage) &&
        (performanceProfileList[thisStage].currentThreadCount == 0)
       )
    {
        queues[nextStage]->setMaxDepth(queues[nextStage]->maxDepthLimit());

        // tell the next stage to exit

        queues[nextStage]->push_back(queueEndSignal());
    }
}

void MLPipelineFoundation::notify(MLPipelineNotification notification,
                                  const QString& _name,
                                  const QString& _path,
                                  const QString& _displayData,
                                  int _processed,
                                  const QImage& _thumbnail)
{
    if (!_thumbnail.isNull())
    {
        notify(notification,
               _name,
               _path,
               _displayData,
               _processed,
               DImg(_thumbnail));
    }
    else
    {
        notify(notification,
               _name,
               _path,
               _displayData,
               _processed,
               missingIcon);
    }
}

void MLPipelineFoundation::notify(MLPipelineNotification notification,
                                  const QString& _name,
                                  const QString& _path,
                                  const QString& _displayData,
                                  int _processed,
                                  const DImg& _thumbnail)
{
    if (!_thumbnail.isNull())
    {
        notify(notification,
               _name,
               _path,
               _displayData,
               _processed,
               QIcon(_thumbnail.smoothScale(48, 48, Qt::KeepAspectRatio).convertToPixmap()));
    }
    else
    {
        notify(notification,
               _name,
               _path,
               _displayData,
               _processed,
               missingIcon);
    }
}

void MLPipelineFoundation::notify(MLPipelineNotification notification,
                                  const QString& _name,
                                  const QString& _path,
                                  const QString& _displayData,
                                  int _processed,
                                  const QIcon& _thumbnail)
{
    MLPipelinePackageNotify::Ptr notify;

    if (_thumbnail.isNull())
    {
        notify = new MLPipelinePackageNotify(_name, _path, _displayData, _processed, missingIcon);
    }
    else
    {
        notify = new MLPipelinePackageNotify(_name, _path, _displayData, _processed, _thumbnail);
    }

    switch (notification)
    {
        case MLPipelineNotification::notifyProcessed:
        {
            Q_EMIT processed(notify);

            break;
        }

        case MLPipelineNotification::notifySkipped:
        {
            Q_EMIT skipped(notify);

            break;
        }
    }
}

void MLPipelineFoundation::pipelinePerformanceStart(const MLPipelineStage& stage, QElapsedTimer& timer)
{
    if (queues.contains(stage))
    {
        performanceProfileList[stage].maxQueueCount = qMax(performanceProfileList[stage].maxQueueCount,
                                                           queues[stage]->size());
    }

    ++performanceProfileList[stage].itemCount;

    timer.start();
}

void MLPipelineFoundation::pipelinePerformanceEnd(const MLPipelineStage& stage, QElapsedTimer& timer)
{
    qint64 elapsedTime                           = timer.elapsed();
    performanceProfileList[stage].elapsedTime   += elapsedTime;
    performanceProfileList[stage].maxElapsedTime = qMax((qint64)performanceProfileList[stage].maxElapsedTime, elapsedTime);
}

void MLPipelineFoundation::pipelinePerformanceEnd(const MLPipelineStage& stage, int _totalItemCount, QElapsedTimer& timer)
{
    performanceProfileList[stage].itemCount      = _totalItemCount;
    performanceProfileList[stage].elapsedTime    = timer.elapsed();
    performanceProfileList[stage].maxElapsedTime = performanceProfileList[stage].elapsedTime;
}

void MLPipelineFoundation::showPipelinePerformance() const
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    for (auto [stage, profile] : performanceProfileList.asKeyValueRange())
    {

#else

    for (const auto& stage : performanceProfileList.keys())
    {

        const auto& profile = performanceProfileList[stage];

#endif

        if (profile.itemCount > 0)
        {
            qCDebug(DIGIKAM_MLPIPELINEFOUNDATION_LOG) << "Stage:"               << stage
                                                      << " Items Processed:"    << profile.itemCount
                                                      << " Max Thread Count:"   << profile.maxThreadCount
                                                      << " Max Queue Depth:"    << profile.maxQueueCount
                                                      << " Total Elapsed:"      << profile.elapsedTime
                                                      << " Max Elapsed:"        << profile.maxElapsedTime
                                                      << " Avg Elapsed:"        << profile.elapsedTime / profile.itemCount;
        }
    }

    qCDebug(DIGIKAM_MLPIPELINEFOUNDATION_LOG) << "Total Elapsed:" << pipelineTimer.elapsed();
}

void MLPipelineFoundation::emitSignalUpdateItemCount(const qlonglong itemCount)
{
    Q_EMIT signalUpdateItemCount(itemCount);
}

} // namespace Digikam

#include "moc_mlpipelinefoundation.cpp"
