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

#include "facepipelineedit.h"

// Qt includes

#include <QList>
#include <QSet>
#include <QElapsedTimer>
#include <QRectF>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_opencv.h"
#include "sharedqueue.h"
#include "collectionmanager.h"
#include "album.h"
#include "iteminfo.h"
#include "coredb.h"
#include "facescansettings.h"
#include "dimg.h"
#include "facepipelinepackagebase.h"
#include "identityprovider.h"
#include "identity.h"
#include "dnnsfaceextractor.h"
#include "metadatahub.h"
#include "scancontroller.h"
#include "facedetector.h"
#include "faceclassifier.h"

namespace Digikam
{

class Q_DECL_HIDDEN FacePipelineEditCreator
{
public:

    FacePipelineEdit object;
};

Q_GLOBAL_STATIC(FacePipelineEditCreator, creator)

// ----------------------------------------------

FacePipelineEdit::FacePipelineEdit()
    : FacePipelineBase(FaceScanSettings())
{
    debugConfirmTimer.start();
}

FacePipelineEdit* FacePipelineEdit::instance()
{
    return &creator->object;
}

FaceTagsIface FacePipelineEdit::confirmFace(const ItemInfo& info,
                                            const FaceTagsIface& face,
                                            const TagRegion& region,
                                            int   tagId,
                                            bool  retrain)
{
    if (debugConfirmTimer.elapsed() < 250)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FacePipelineEdit::confirmFace(): INFO: more than 1 "
                                            "face confirmed in less than 0.25 seconds";
    }

    debugConfirmTimer.restart();

    FaceTagsIface newFace                  = getRejectedFaceTagList(face);

    // CR #1680458 optimization: Check if the queue exists before allocating 'package'.
    // This avoids unnecessary memory allocation and eliminates the need
    // for manual 'delete' in error cases, preventing potential memory leaks.
    // The 'package' object is only created if the queue is available.

    if (!queues.contains(MLPipelineStage::Writer))
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No next queue found for MLPipelineStage::Writer";

        return (FaceTagsEditor::confirmedEntry(newFace, tagId, region));
    }

    MLPipelineQueue* const nextQueue       = queues.value(MLPipelineStage::Loader);
    FacePipelinePackageBase* const package = new FacePipelinePackageBase(info,
                                                                         newFace,
                                                                         tagId,
                                                                         region,
                                                                         DImg(),
                                                                         FacePipelinePackageBase::EditPipelineAction::Confirm,
                                                                         retrain);

    ++totalItemCount;
    Q_EMIT scheduled();
    Q_EMIT started(i18n("Confirming face"));

    emitSignalUpdateItemCount(nextQueue->size()+1);

    enqueue(nextQueue, package);

    return (FaceTagsEditor::confirmedEntry(newFace, tagId, region));
}

void FacePipelineEdit::removeFace(const ItemInfo& info,
                                  const FaceTagsIface& face)
{
    FaceTagsIface newFace                  = getRejectedFaceTagList(face);

    // Optimization: Check if the queue exists before allocating 'package'.
    // This avoids unnecessary memory allocation and eliminates the need
    // for manual 'delete' in error cases, preventing potential memory leaks.
    // The 'package' object is only created if the queue is available.

    if (!queues.contains(MLPipelineStage::Writer))
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No next queue found for MLPipelineStage::Writer";

        return;
    }

    MLPipelineQueue* const nextQueue       = queues.value(MLPipelineStage::Writer);
    FacePipelinePackageBase* const package = new FacePipelinePackageBase(info,
                                                                         newFace,
                                                                         newFace.tagId(),
                                                                         newFace.region(),
                                                                         DImg(),
                                                                         FacePipelinePackageBase::EditPipelineAction::Remove,
                                                                         newFace.isConfirmedName());

    ++totalItemCount;

    Q_EMIT scheduled();
    Q_EMIT started(i18n("Removing face"));

    emitSignalUpdateItemCount(nextQueue->size()+1);

    enqueue(nextQueue, package);
}

void FacePipelineEdit::removeAllFaces(const ItemInfo& info)
{
    // Optimization: Check if the queue exists before allocating 'package'.
    // This avoids unnecessary memory allocation and eliminates the need
    // for manual 'delete' in error cases, preventing potential memory leaks.
    // The 'package' object is only created if the queue is available.

    if (!queues.contains(MLPipelineStage::Writer))
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No next queue found for MLPipelineStage::Writer";

        return;
    }

    MLPipelineQueue* const nextQueue       = queues.value(MLPipelineStage::Writer);
    FacePipelinePackageBase* const package = new FacePipelinePackageBase(info,
                                                                         FacePipelinePackageBase::EditPipelineAction::RemoveAll);

    ++totalItemCount;

    Q_EMIT scheduled();
    Q_EMIT started(i18n("Remove all faces"));

    emitSignalUpdateItemCount(nextQueue->size()+1);

    enqueue(nextQueue, package);
}

FaceTagsIface FacePipelineEdit::editTag(const ItemInfo& info,
                                        const FaceTagsIface& face,
                                        int newTagId)
{
    FaceTagsIface newFace                  = getRejectedFaceTagList(face);

    // Optimization: Check if the queue exists before allocating 'package'.
    // This avoids unnecessary memory allocation and eliminates the need
    // for manual 'delete' in error cases, preventing potential memory leaks.
    // The 'package' object is only created if the queue is available.

    if (!queues.contains(MLPipelineStage::Writer))
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No next queue found for MLPipelineStage::Writer";

        return newFace;
    }

    MLPipelineQueue* const nextQueue       = queues.value(MLPipelineStage::Writer);
    FacePipelinePackageBase* const package = new FacePipelinePackageBase(info,
                                                                         newFace,
                                                                         newTagId,
                                                                         newFace.region(),
                                                                         DImg(),
                                                                         FacePipelinePackageBase::EditPipelineAction::EditTag,
                                                                         face.isConfirmedName());

    ++totalItemCount;

    Q_EMIT scheduled();
    Q_EMIT started(i18n("Editing face tag"));

    emitSignalUpdateItemCount(nextQueue->size()+1);

    enqueue(nextQueue, package);

    // FaceTagsIface newFace(package->face);
    newFace.setTagId(newTagId);

    return newFace;
}

FaceTagsIface FacePipelineEdit::editRegion(const ItemInfo& info,
                                           const FaceTagsIface& face,
                                           const TagRegion& region,
                                           int tagId,
                                           bool retrain)
{
    FaceTagsIface newFace                  = getRejectedFaceTagList(face);

    // Optimization: Check if the queue exists before allocating 'package'.
    // This avoids unnecessary memory allocation and eliminates the need
    // for manual 'delete' in error cases, preventing potential memory leaks.
    // The 'package' object is only created if the queue is available.

    if (!queues.contains(MLPipelineStage::Writer))
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No next queue found for MLPipelineStage::Writer";

        return newFace;
    }

    MLPipelineQueue* const nextQueue       = queues.value(MLPipelineStage::Writer);
    FacePipelinePackageBase* const package = new FacePipelinePackageBase(info,
                                                                         newFace,
                                                                         tagId,
                                                                         region,
                                                                         DImg(),
                                                                         FacePipelinePackageBase::EditPipelineAction::EditRegion,
                                                                         retrain);

    ++totalItemCount;

    Q_EMIT scheduled();
    Q_EMIT started(i18n("Editing face region"));

    emitSignalUpdateItemCount(nextQueue->size()+1);

    enqueue(nextQueue, package);

    newFace.setRegion(region);

    if (tagId != FaceClassifierBase::UNKNOWN_LABEL_ID)
    {
        newFace.setTagId(tagId);
        newFace.setType(FaceTagsIface::typeForId(tagId));
    }

    return newFace;
}

FaceTagsIface FacePipelineEdit::addManually(const ItemInfo& info,
                                            const DImg& image,
                                            const TagRegion& region,
                                            bool retrain)
{
    FaceTagsIface newFace                  = FaceTagsEditor::unconfirmedEntry(info.id(),
                                                                              FaceClassifier::UNKNOWN_LABEL_ID,
                                                                              region, QList<int>());

    // Optimization: Check if the queue exists before allocating 'package'.
    // This avoids unnecessary memory allocation and eliminates the need
    // for manual 'delete' in error cases, preventing potential memory leaks.
    // The 'package' object is only created if the queue is available.

    if (!queues.contains(MLPipelineStage::Writer))
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No next queue found for MLPipelineStage::Writer";

        return newFace;
    }

    MLPipelineQueue* const nextQueue       = queues.value(MLPipelineStage::Writer);
    FacePipelinePackageBase* const package = new FacePipelinePackageBase(info,
                                                                         newFace,
                                                                         newFace.tagId(),
                                                                         newFace.region(),
                                                                         image,
                                                                         FacePipelinePackageBase::EditPipelineAction::AddManually,
                                                                         retrain);

    ++totalItemCount;

    Q_EMIT scheduled();
    Q_EMIT started(i18n("Manually adding face"));

    emitSignalUpdateItemCount(nextQueue->size()+1);

    enqueue(nextQueue, package);

    return newFace;
}

QList<FaceTagsIface> FacePipelineEdit::deleteRejectedFaceTagLists(const ItemInfo& info)
{
    QList<FaceTagsIface> result;
    QList<FaceTagsIface> faces = FaceUtils().databaseFaces(info.id());

    for (const FaceTagsIface& face : std::as_const(faces))
    {
        result << deleteRejectedFaceTagList(face);
    }

    return result;
}

FaceTagsIface FacePipelineEdit::deleteRejectedFaceTagList(const FaceTagsIface& face)
{
    FaceTagsIface newFace(face);
    newFace.clearRejectedFaceTagList();

    // Optimization: Check if the queue exists before allocating 'package'.
    // This avoids unnecessary memory allocation and eliminates the need
    // for manual 'delete' in error cases, preventing potential memory leaks.
    // The 'package' object is only created if the queue is available.

    if (!queues.contains(MLPipelineStage::Writer))
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "No next queue found for MLPipelineStage::Writer";

        return newFace;
    }

    MLPipelineQueue* const nextQueue       = queues.value(MLPipelineStage::Writer);
    FacePipelinePackageBase* const package = new FacePipelinePackageBase(ItemInfo(face.imageId()),
                                                                         face,
                                                                         face.tagId(),
                                                                         face.region(),
                                                                         DImg(),
                                                                         FacePipelinePackageBase::EditPipelineAction::DeleteRejectedFaceTagList,
                                                                         false);

    ++totalItemCount;

    Q_EMIT scheduled();
    Q_EMIT started(i18n("Clearing rejected face tag list."));

    emitSignalUpdateItemCount(nextQueue->size()+1);

    enqueue(nextQueue, package);

    return newFace;
}

bool FacePipelineEdit::start()
{
    if (!isStarted)
    {
        isStarted = true;

        {
            // use the mutex to synchronize the start of the threads

            QMutexLocker lock(&mutex);

            // add the worker threads for this pipeline
/*
            addWorker(MLPipelineStage::Finder);
*/
            addWorker(MLPipelineStage::Loader);
            addWorker(MLPipelineStage::Extractor);
/*
            addWorker(MLPipelineStage::Classifier);
*/
            addWorker(MLPipelineStage::Writer);
        }

        return FacePipelineBase::start();
    }
    else
    {
        return true;
    }
}

bool FacePipelineEdit::loader()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceEdit%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    return commonFaceThumbnailLoader(QStringLiteral("FacePipelineEdit"),
                                     QThread::NormalPriority,
                                     MLPipelineStage::Loader,
                                     MLPipelineStage::Extractor);
}

bool FacePipelineEdit::extractor()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceEdit%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    return commonFaceThumbnailExtractor(QStringLiteral("FacePipelineEdit"),
                                        QThread::NormalPriority,
                                        MLPipelineStage::Extractor,
                                        MLPipelineStage::Writer,
                                        true);
}

bool FacePipelineEdit::writer()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceEdit%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::NormalPriority, MLPipelineStage::Writer, MLPipelineStage::None);
    FacePipelinePackageBase* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    //--------------------------------------------------------------------------------

    IdentityProvider* const idProvider = IdentityProvider::instance();

    // override the default queue depth

    thisQueue->setMaxDepth(thisQueue->maxDepthLimit());

    MLPIPELINE_LOOP_START(MLPipelineStage::Writer, thisQueue);
    package                            = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        FaceUtils utils;

        switch (package->action)
        {
            case FacePipelinePackageBase::EditPipelineAction::Confirm:
            {
                TagRegion confirmedRegion   = package->region.isValid() ? package->region
                                                                        : package->face.region();
                FaceTagsIface confirmedFace = utils.confirmName(package->face, package->tagId, confirmedRegion);
                Identity identity           = utils.identityForTag(confirmedFace.tagId());

                if (0 != package->features.rows)
                {
                    if (package->useForTraining)
                    {
                        idProvider->addTraining(identity, confirmedFace.hash(), package->features);
                    }
                    else
                    {
                        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FacePipelineEdit::writer(): not using for training: "
                                                         << package->info.filePath();
                    }
                }
                else
                {
                    qCDebug(DIGIKAM_FACESENGINE_LOG) << "FacePipelineEdit::writer(): bad mat";
                }

                break;
            }

            case FacePipelinePackageBase::EditPipelineAction::Remove:
            {
                utils.removeFace(package->face);
                break;
            }

            case FacePipelinePackageBase::EditPipelineAction::RemoveAll:
            {
                utils.removeAllFaces(package->info.id());
                break;
            }

            case FacePipelinePackageBase::EditPipelineAction::EditTag:
            {
                // Change Tag operation.

                if (
                    (package->face.isUnconfirmedName())                       &&
                    (FaceTags::unknownPersonTagId() != package->face.tagId()) &&
                    (FaceTags::unknownPersonTagId() == package->tagId)
                   )
                {
                    // The face is unconfirmed and the tag is not the unknown person tag.

                    utils.rejectSuggestedTag(package->face);
                }
                else
                {
                    utils.changeTag(package->face, package->tagId);
                }

                break;
            }

            case FacePipelinePackageBase::EditPipelineAction::EditRegion:
            {
                if (package->face.region() != package->region)
                {
                    package->face = utils.changeRegion(package->face, package->region);
                }

                if (package->tagId != FaceClassifier::UNKNOWN_LABEL_ID)
                {
                   utils.changeTag(package->face, package->tagId);
                }

                break;
            }

            case FacePipelinePackageBase::EditPipelineAction::AddManually:
            {
                utils.addManually(utils.unconfirmedEntry(package->info.id(), package->tagId,
                                                         package->region, package->face.rejectedFaceTagList()));
                break;
            }

            case FacePipelinePackageBase::EditPipelineAction::DeleteRejectedFaceTagList:
            {
                utils.removeRejectedFaceTagList(package->face);
                break;
            }
        }

        // update the tags

        if (utils.normalTagChanged())
        {
            MetadataHub hub;
            hub.load(package->info);

            ScanController::FileMetadataWrite writeScope(package->info);
            writeScope.changed(hub.writeToMetadata(package->info, MetadataHub::WRITE_TAGS));
        }

        // retrain the face classifier if the retrain flag is set

        if (package->retrain)
        {
            FaceClassifier::instance()->retrain();
        }

        // send a notification that the image was processed

        notify(MLPipelineNotification::notifyProcessed,
               package->info.name(),
               package->info.relativePath(),
               QString(),
               package->faceRects.size(),
               package->thumbnail);

        // delete the package

        delete package;

        package = nullptr;

        if (
            (0 == queues[MLPipelineStage::Loader]->size())    &&
            (0 == queues[MLPipelineStage::Extractor]->size()) &&
            (0 == queues[MLPipelineStage::Writer]->size())
           )
        {
            Q_EMIT progressValueChanged((float)1.0);

            totalItemCount = 0;

            Q_EMIT finished();
        }
        else
        {
            ++itemsProcessed;

            Q_EMIT progressValueChanged((float)itemsProcessed/(float)totalItemCount);
        }
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Writer, "FacePipelineEdit::writer");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Writer, MLPipelineStage::None);
}

void FacePipelineEdit::addMoreWorkers()
{
    // edit pipeline is always single thread per stage. Do nothing.
}

FaceTagsIface FacePipelineEdit::getRejectedFaceTagList(const FaceTagsIface& face) const
{
    /**
     * some FaceTagsIface objects don't have the rejectedFaceTagList set
     * because of the way they were loaded.
     * This function will set it to the value from the database.
     */

    FaceTagsIface newFace(face);

    QList<FaceTagsIface> faces = FaceUtils().databaseFaces(face.imageId());

    for (const FaceTagsIface& f : std::as_const(faces))
    {
        if (f.region() == newFace.region())
        {
            newFace.setRejectedFaceTagList(f.rejectedFaceTagList());
            break;
        }
    }

    return newFace;
}

} // namespace Digikam

#include "moc_facepipelineedit.cpp"
