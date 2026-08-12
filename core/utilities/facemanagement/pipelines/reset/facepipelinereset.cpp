/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Performs face detection and recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facepipelinereset.h"

// Qt includes

#include <QSet>
#include <QList>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_opencv.h"
#include "sharedqueue.h"
#include "collectionmanager.h"
#include "album.h"
#include "coredb.h"
#include "iteminfo.h"
#include "faceutils.h"
#include "facescansettings.h"
#include "facepipelinepackagebase.h"
#include "identityprovider.h"
#include "identity.h"
#include "dnnsfaceextractor.h"

namespace Digikam
{

FacePipelineReset::FacePipelineReset(const FaceScanSettings& _settings)
    : FacePipelineBase(_settings)
{
}

bool FacePipelineReset::start()
{
    {
        // use the mutex to synchronize the start of the threads

        QMutexLocker lock(&mutex);

        // add the worker threads for this pipeline

        addWorker(MLPipelineStage::Finder);
/*
        addWorker(MLPipelineStage::Loader);
        addWorker(MLPipelineStage::Extractor);
        addWorker(MLPipelineStage::Classifier);
*/
        addWorker(MLPipelineStage::Writer);
    }

    return FacePipelineBase::start();
}

bool FacePipelineReset::finder()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceReset%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_FINDER_START(settings.workerThreadPriority, MLPipelineStage::Writer);

    /* =========================================================================================
     * Pipeline finder specific initialization code
     *
     * Use the block from here to MLPIPELINE_FINDER_END to find the IDs images to process.
     * The code in this block is run once per stage initialization. The number of instances
     * is alaways 1.
     */

    // get the IDs to process

    FaceUtils utils;
    QSet<qlonglong> filter;
    int serialNumber = 0;

    for (const Album* const album : std::as_const(settings.albums))
    {
        if (cancelled)
        {
            break;
        }

        if (!album->isTrashAlbum())
        {
            QList<qlonglong> imageIds = CoreDbAccess().db()->getImageIds(album->id(),
                                                                         DatabaseItem::Status::Visible,
                                                                         true);

            if (!moreCpu)
            {
                moreCpu = checkMoreWorkers(totalItemCount, imageIds.size(), settings.useFullCpu);
            }

            for (qlonglong imageId : std::as_const(imageIds))
            {
                // filter out duplicate image IDs

                if (!filter.contains(imageId))
                {
                    ++totalItemCount;
                    enqueue(nextQueue, new FacePipelinePackageBase(imageId, ++serialNumber));
                }
            }
        }
    }

    /* =========================================================================================
     * Pipeline finder specific cleanup
     *
     * Use the block from here to MLPIPELINE_FINDER_END to clean up any resources used by the stage.
     */

    MLPIPELINE_FINDER_END(MLPipelineStage::Writer);
}

bool FacePipelineReset::writer()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceReset%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Writer, MLPipelineStage::None);
    FacePipelinePackageBase* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    FaceUtils utils;
    FaceTagsEditor editor;
    IdentityProvider* const idProvider = IdentityProvider::instance();
    QIcon icon                         = QIcon::fromTheme(QLatin1String("person"));

    // clear all identities and training from recognition DB before
    // we start looping through the results from the pipeline

    idProvider->clearAllTraining();

    MLPIPELINE_LOOP_START(MLPipelineStage::Writer, thisQueue);
    package = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        // remove the face tags from the image

        QList<FaceTagsIface> databaseFaces = editor.unconfirmedFaceTagsIfaces(package->info.id());

        editor.removeFaces(databaseFaces);

        databaseFaces                      = editor.unconfirmedNameFaceTagsIfaces(package->info.id());

        editor.removeFaces(databaseFaces);

        editor.removeAllFaces(package->info.id());

        // mark the image as unscanned

        utils.markAsScanned(package->info, false);

        QString albumName = CollectionManager::instance()->albumRootLabel(package->info.albumRootId());

        // send a notification that the image was processed

        notify(MLPipelineNotification::notifyProcessed,
               package->info.name(),
               albumName + package->info.relativePath(),
               QString(),
               package->faceRects.size(),
               icon);

        // delete the package

        delete package;

        package = nullptr;
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Writer, "FacePipelineReset::writer");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    // retrain the classifier after all results have been processed

    FaceClassifier::instance()->retrain();

    MLPIPELINE_STAGE_END(MLPipelineStage::Writer, MLPipelineStage::None);
}

void FacePipelineReset::addMoreWorkers()
{
    // reset pipeline is always single thread per stage
}

} // namespace Digikam

#include "moc_facepipelinereset.cpp"
