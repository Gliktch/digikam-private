/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-05-02
 * Description : AI tools pipeline class
 *
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "aitoolspipeline.h"

// Qt includes

#include <QApplication>
#include <QFileInfo>
#include <QString>
#include <QList>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "systemsettings.h"
#include "aitoolspipelinepackage.h"
#include "dnnmodelmanager.h"
#include "previewloadthread.h"
#include "autorotator.h"
#include "actionthreadbase.h"
#include "collectionmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN AIToolsPipelineCreator
{
public:

    AIToolsPipeline object;
};

Q_GLOBAL_STATIC(AIToolsPipelineCreator, creator)

// -----------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN AIToolsPipeline::Private
{
public:

    Private() = default;

public:

    bool            aiToolsEnabled  = false;
    bool            isStarted       = false;
    bool            batchCancelled  = false;
    AutoRotator*    rotator         = nullptr;
    QRecursiveMutex progressMutex;
};

AIToolsPipeline::AIToolsPipeline(QObject* const parent)
    : MLPipelineFoundation(),
      d                   (new Private)
{
    setParent(parent);

    // For the signal and slot usages.

    qRegisterMetaType<MLPipelinePackageNotify>("MLPipelinePackageNotify");
    qRegisterMetaType<MLPipelinePackageNotify::Ptr>("MLPipelinePackageNotifyPtr");

    d->aiToolsEnabled = SystemSettings(qApp->applicationName()).enableAIAutoTools;
}

AIToolsPipeline::~AIToolsPipeline()
{
    delete d;
}

AIToolsPipeline* AIToolsPipeline::instance()
{
    return &creator->object;
}

bool AIToolsPipeline::start()
{
    // Check if AI tools are enabled

    if (!d->aiToolsEnabled)
    {
        return false;
    }

    // Check if the pipeline is already started

    if (!d->isStarted)
    {
        d->isStarted = true;

        try
        {
            // Load the model for auto-rotation

            d->rotator = new AutoRotator();
        }
        catch (const QException& e)
        {
            qCWarning(DIGIKAM_AUTOROTATE_LOG) << "Failed to load the AutoRotator model:" << e.what();
            d->isStarted = false;

            return false;
        }

        // this section is a block to release the mutex when done

        {
            // use the mutex to synchronize the start of the threads

            QMutexLocker lock(&mutex);

            // add the worker threads for this pipeline
            // Skip the finder and extractor stages in this pipeline

            addWorker(MLPipelineStage::Loader);
            addWorker(MLPipelineStage::Classifier);
            addWorker(MLPipelineStage::Writer);
        }

        connect(this, SIGNAL(scheduled()),
                this, SLOT(slotScheduled()));

        connect(this, SIGNAL(started(QString)),
                this, SLOT(slotStarted(QString)));

        connect(this, SIGNAL(processed(MLPipelinePackageNotify::Ptr)),
                this, SLOT(slotProcessed(MLPipelinePackageNotify::Ptr)));

        connect(this, SIGNAL(skipped(MLPipelinePackageNotify::Ptr)),
                this, SLOT(slotSkipped(MLPipelinePackageNotify::Ptr)));

        connect(this, SIGNAL(finished()),
                this, SLOT(slotFinished()));

        return MLPipelineFoundation::start();
    }
    else
    {
        return true;
    }
}

bool AIToolsPipeline::autoRotate(const ItemInfo& info, ProgressItem* const progress)
{
    return process(info, AIToolsPipelinePackage::AITool::AutoRotate, progress);
}

bool AIToolsPipeline::process(const ItemInfo& info,
                              AIToolsPipelinePackage::AITool tool,
                              ProgressItem* const progress)
{
    // Make sure the pipeline is started

    if (!d->isStarted)
    {
        return false;
    }

    // Create a new package for processing

    AIToolsPipelinePackage* const package = new AIToolsPipelinePackage(tool, info, progress);

    // Wait for the Loader queue to be ready

    while (nullptr == queues[MLPipelineStage::Loader])
    {
        QThread::msleep(10);
    }

    Q_EMIT scheduled();
    Q_EMIT started(info.filePath());

    // Submit the package to the loader stage

    return enqueue(queues[MLPipelineStage::Loader], package);
}

bool AIToolsPipeline::process(const DImg& image,
                              AIToolsPipelinePackage::AITool tool,
                              ProgressItem* const progress)
{
    // Make sure the pipeline is started

    if (!d->isStarted)
    {
        return false;
    }

    // Create a new package for processing with the pre-loaded image

    AIToolsPipelinePackage* const package = new AIToolsPipelinePackage(tool, image, progress);

    // Create a thumbnail from the DImg

    if (!package->image.isNull())
    {
        // Create an icon from the thumbnail

        package->thumbnailIcon = QIcon(package->image.smoothScale(48, 48, Qt::KeepAspectRatio).convertToPixmap());
    }
    else
    {
        // Set a default icon if the image is null

        package->thumbnailIcon = QIcon::fromTheme(QLatin1String("image-x-generic"));
    }

    // Wait for the Classifier queue to be ready

    while (nullptr == queues[MLPipelineStage::Classifier])
    {
        QThread::msleep(10);
    }

    Q_EMIT scheduled();
    Q_EMIT started(image.originalFilePath());

    // Submit the package directly to the classifier stage, bypassing the loader

    return enqueue(queues[MLPipelineStage::Classifier], package);
}

bool AIToolsPipeline::loader()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("AITools%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Loader, MLPipelineStage::Classifier);
    AIToolsPipelinePackage* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    const DNNModelBase* const model = DNNModelManager::instance()->getModel(QLatin1String("AutoRotate"),
                                                                            DNNModelUsage::DNNUsageAutoRotate);

    MLPIPELINE_LOOP_START(MLPipelineStage::Loader, thisQueue);
    package = static_cast<AIToolsPipelinePackage*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        // Load the image for processing

        package->image = PreviewLoadThread::loadSynchronously(package->info.filePath(),
                                                              PreviewSettings(),
                                                              model->info.imageSize);

        // Create a thumbnail from the loaded image

        if (!package->image.isNull())
        {
            // Create an icon from the thumbnail

            package->thumbnailIcon = QIcon(package->image.smoothScale(48, 48, Qt::KeepAspectRatio).convertToPixmap());
        }

        // Pass the package directly to the classifier stage, skipping extractor

        enqueue(nextQueue, package);
        package = nullptr;
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Loader, "AIToolsPipeline::loader");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Loader, MLPipelineStage::Classifier);
}

bool AIToolsPipeline::classifier()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("AITools%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Classifier, MLPipelineStage::Writer);
    AIToolsPipelinePackage* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    MLPIPELINE_LOOP_START(MLPipelineStage::Classifier, thisQueue);
    package = static_cast<AIToolsPipelinePackage*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        // Use autoRotator to detect rotation and set the rotationTransformation in the package

        if (!package->image.isNull())
        {
            // Detect rotation angle using AutoRotator

            package->rotationTransformation = d->rotator->rotationOrientation(package->image, 10);

            // Pass the package to the next stage

            enqueue(nextQueue, package);
        }
        else
        {
            // If the image is null, notify that the image could not be loaded

            QString albumName = CollectionManager::instance()->albumRootLabel(package->info.albumRootId());

            notify(MLPipelineNotification::notifySkipped,
                    package->info.name(),
                    albumName + package->info.relativePath(),
                    i18n("Unable to load image"),
                    0,
                    QIcon::fromTheme(QLatin1String("image-missing")));
        }

        package = nullptr;
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Classifier, "AIToolsPipeline::classifier");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Classifier, MLPipelineStage::Writer);
}

bool AIToolsPipeline::writer()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("AITools%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Writer, MLPipelineStage::None);
    AIToolsPipelinePackage* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    MLPIPELINE_LOOP_START(MLPipelineStage::Writer, thisQueue);
    package = static_cast<AIToolsPipelinePackage*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        switch (package->tool)
        {
            case AIToolsPipelinePackage::AITool::AutoRotate:
            {
                // Only emit the signal if a rotation is actually needed

                if (MetaEngineRotation::NoTransformation != package->rotationTransformation)
                {
                    QList<ItemInfo> infos;
                    infos << package->info;

                    Q_EMIT signalTransform(infos, package->rotationTransformation);
                }

                break;
            }

            default:
            {
                break;
            }
        }

        QString albumName = CollectionManager::instance()->albumRootLabel(package->info.albumRootId());

        // send a notification that the image was processed

        notify(MLPipelineNotification::notifyProcessed,
               package->info.name(),
               albumName + package->info.relativePath(),
               MetaEngineRotation::transformationActionToString(package->rotationTransformation),
               1,
               package->thumbnailIcon.isNull() ? QIcon::fromTheme(QLatin1String("applications-science"))
                                               : package->thumbnailIcon);

        // Delete the package

        delete package;
        package = nullptr;
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Writer, "AIToolsPipeline::writer");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Writer, MLPipelineStage::None);
}

void AIToolsPipeline::addMoreWorkers()
{
    // Add workers to each stage as needed
}

ProgressItem* AIToolsPipeline::getProgressItem()
{
    QMutexLocker lock(&d->progressMutex);

    ProgressItem* item = ProgressManager::instance()->findItembyId(QLatin1String("AIToolPipeline"));

    if (!item)
    {
        // Create a new progress item if one doesn't exist

        item = ProgressManager::createProgressItem(QLatin1String("AIToolPipeline"),
                                                  i18n("AI Tools Processing"),
                                                  QString(),
                                                  true,  // can be canceled
                                                  true); // has thumbnail

        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                this, SLOT(slotBatchCancel()));
    }

    return item;
}

void AIToolsPipeline::slotScheduled()
{
    // Getting the progress item will initialize it if it does not exist

    d->batchCancelled = false;

    getProgressItem();
}

void AIToolsPipeline::slotStarted(const QString& message)
{
    if (d->batchCancelled)
    {
        return;
    }

    getProgressItem()->setLabel(message);
    getProgressItem()->incTotalItems(1);
}

void AIToolsPipeline::slotProcessed(const MLPipelinePackageNotify::Ptr& package)
{
    if (d->batchCancelled)
    {
        return;
    }

    ProgressItem* const item = getProgressItem();
    item->setThumbnail(package->thumbnail);

    QString lbl              = i18n("Auto-rotating: %1\n", package->name);
    lbl.append(i18n("Album: %1\n", package->path));
    lbl.append(i18n("Rotation: %1\n", package->displayData));

    item->setLabel(lbl);
    item->incCompletedItems(1);
    item->updateProgress();

    if (item->totalCompleted())
    {
        Q_EMIT finished();
    }
}

void AIToolsPipeline::slotSkipped(const MLPipelinePackageNotify::Ptr& package)
{
    if (d->batchCancelled)
    {
        return;
    }

    MLPipelinePackageNotify::Ptr notify;
    notify = new MLPipelinePackageNotify(package->name,
                                         package->path,
                                         i18n("Skipped"),
                                         0,
                                         package->thumbnail);
    slotProcessed(notify);
}

void AIToolsPipeline::slotFinished()
{
    if (d->batchCancelled)
    {
        return;
    }

    ProgressItem* const item = getProgressItem();

    item->setComplete();
}

void AIToolsPipeline::slotBatchCancel()
{
    if (d->batchCancelled)
    {
        return;
    }

    // Only empty the Loader and Classifier queues

    const QList<MLPipelineStage> stagesToClear =
    {
        MLPipelineStage::Loader,
        MLPipelineStage::Classifier,
        MLPipelineStage::Writer
    };

    d->batchCancelled = true;

    QMutexLocker lock(&d->progressMutex);

    ProgressItem* const item = getProgressItem();

    // Clear only the specified stages

    for (const MLPipelineStage& stage : stagesToClear)
    {
        // Check if this stage has a queue

        if (queues.contains(stage))
        {
            MLPipelineQueue* const queue = queues[stage];

            // Clear the queue

            while (!queue->isEmpty())
            {
                MLPipelinePackageFoundation* const package = queue->pop_front();

                // Make sure we don't delete the queue end signal

                if (queueEndSignal() != package)
                {
                    item->incCompletedItems(1);

                    delete package;
                }
            }
        }
    }

    item->setComplete();
}

} // namespace Digikam

#include "moc_aitoolspipeline.cpp"
