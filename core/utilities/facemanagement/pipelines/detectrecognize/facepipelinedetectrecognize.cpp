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

#include "facepipelinedetectrecognize.h"

// C++ includes

#include <climits>
#include <cmath>

// Qt includes

#include <QSet>
#include <QList>
#include <QRectF>
#include <QElapsedTimer>

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
#include "previewloadthread.h"
#include "faceutils.h"
#include "facescansettings.h"
#include "facepipelinepackagebase.h"
#include "thumbnailloadthread.h"
#include "faceclassifier.h"
#include "identityprovider.h"
#include "qtopencvimg.h"
#include "ocvocldnnsetter.h"

namespace Digikam
{

FacePipelineDetectRecognize::FacePipelineDetectRecognize(const FaceScanSettings& _settings)
    : FacePipelineBase(_settings)
{
}

FacePipelineDetectRecognize::~FacePipelineDetectRecognize()
{
    if (faceDetector)
    {
        delete faceDetector;
    }

    // extractor is singleton, so no need to delete it
}

bool FacePipelineDetectRecognize::start()
{
    // create the face detector and extractor

    try
    {
        faceDetector  = new DNNFaceDetectorYuNet;
        faceExtractor = static_cast<DNNModelSFace*>(DNNModelManager::instance()->getModel(QStringLiteral("sface"),
                                                                                          DNNModelUsage::DNNUsageFaceRecognition));
        faceExtractor->getNet();
    }

    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        if (faceDetector)
        {
            delete faceDetector;
            faceDetector = nullptr;
        }

        // extractor is singleton, so no need to delete it
    }

    catch (...)
    {
        if (faceDetector)
        {
            delete faceDetector;
            faceDetector = nullptr;
        }
    }

    // check if the detector and extractor were created

    if ((nullptr == faceDetector) || (nullptr == faceExtractor) || !faceExtractor->modelLoaded)
    {
        return false;
    }

    // set the face detection size and accuracy

    faceDetector->setFaceDetectionSize(settings.detectSize);
    faceDetector->uiConfidenceThreshold = settings.detectAccuracy;

    {
        // use the mutex to synchronize the start of the threads

        QMutexLocker lock(&mutex);

        // add the worker threads for this pipeline

        addWorker(MLPipelineStage::Finder);
        addWorker(MLPipelineStage::Loader);
        addWorker(MLPipelineStage::Extractor);
        addWorker(MLPipelineStage::Classifier);
        addWorker(MLPipelineStage::Writer);
    }

    return FacePipelineBase::start();
}

bool FacePipelineDetectRecognize::finder()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceDetReco%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_FINDER_START(settings.workerThreadPriority, MLPipelineStage::Loader);

    /* =========================================================================================
     * Pipeline finder specific initialization code
     *
     * Use the block from here to MLPIPELINE_FINDER_END to find the IDs images to process.
     * The code in this block is run once per stage initialization. The number of instances
     * is always 1.
     */

    // get the IDs to process

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
            QList<qlonglong> imageIds;

            // get the image IDs for the album

            if      (album->type() == Album::PHYSICAL)
            {
                imageIds = CoreDbAccess().db()->getImageIds(album->id(),
                                                            DatabaseItem::Status::Visible,
                                                            (FaceScanSettings::AlreadyScannedHandling::Skip !=
                                                             settings.alreadyScannedHandling));
            }
            else if (album->type() == Album::TAG)
            {
                imageIds = CoreDbAccess().db()->getItemIDsInTag(album->id());
            }

            // iterate over the image IDs and add unique IDs to the queue for processing

            for (qlonglong imageId : std::as_const(imageIds))
            {
                // filter out duplicate image IDs

                if (!filter.contains(imageId))
                {
                    // quick check if we should add threads.

                    if (!moreCpu)
                    {
                        moreCpu = checkMoreWorkers(totalItemCount, imageIds.size(), settings.useFullCpu);
                    }

                    ++totalItemCount;
                    filter << imageId;

                    // add the image ID to the queue for processing

                    enqueue(nextQueue, new FacePipelinePackageBase(imageId, ++serialNumber));
                }
            }
        }
    }

    for (const ItemInfo& info : std::as_const(settings.infos))
    {
        // filter out duplicate image IDs

        qlonglong imageId = info.id();

        if (!filter.contains(imageId))
        {
            // quick check if we should add threads.

            if (!moreCpu)
            {
                moreCpu = checkMoreWorkers(totalItemCount, settings.infos.size(), settings.useFullCpu);
            }

            ++totalItemCount;
            filter << imageId;

            // add the image ID to the queue for processing

            enqueue(nextQueue, new FacePipelinePackageBase(imageId, ++serialNumber));
        }
    }

    /* =========================================================================================
     * Pipeline finder specific cleanup
     *
     * Use the block from here to MLPIPELINE_FINDER_END to clean up any resources used by the stage.
     */

    MLPIPELINE_FINDER_END(MLPipelineStage::Loader);
}

bool FacePipelineDetectRecognize::loader()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceDetReco%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Loader, MLPipelineStage::Extractor);
    FacePipelinePackageBase* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    MLPIPELINE_LOOP_START(MLPipelineStage::Loader, thisQueue);
    package = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */
    {
        // check if the ID is for an image (not video or other file type)

        bool sendNotification = true;

        if (DatabaseItem::Category::Image == package->info.category())
        {
            PreviewSettings::RawLoading rawLoadingMode = PreviewSettings::RawPreviewAutomatic;

            if (QStringLiteral("RAW-ARW") == package->info.format())
            {
                // fix for #447767: ARW preview is different aspect than the image

                bool loadFullSize =
                   (
                    qMin(package->info.dimensions().width()         / 2, package->info.dimensions().height()       / 2) <
                    qMin(faceDetector->nnInputSizeRequired().height / 2, faceDetector->nnInputSizeRequired().width / 2)
                   );

                rawLoadingMode = (loadFullSize ? PreviewSettings::RawPreviewFromRawFullSize
                                               : PreviewSettings::RawPreviewFromRawHalfSize);
            }

            // load high quality image for detection.

            package->image = PreviewLoadThread::loadHighQualitySynchronously(package->info.filePath(), rawLoadingMode);

            // check for corrupted images that cannot be loaded

            if (!package->image.isNull())
            {
                // create a thumbnail for the notification

                package->thumbnailIcon = QIcon(package->image.smoothScale(48, 48, Qt::KeepAspectRatio).convertToPixmap());

                // send to the next stage

                enqueue(nextQueue, package);

                package = nullptr;

                sendNotification = false;
            }
        }

        if (sendNotification)
        {
            // send a notification that the file was skipped

            notify(MLPipelineNotification::notifySkipped,
                   package->info.name(),
                   package->info.relativePath(),
                   QString(),
                   0,
                   package->thumbnailIcon);

            // delete the package since it is not needed

            delete package;

            package = nullptr;
        }
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Loader, "FacePipelineDetectRecognize::loader");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Loader, MLPipelineStage::Extractor);
}

bool FacePipelineDetectRecognize::extractor()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceDetReco%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Extractor, MLPipelineStage::Classifier);
    FacePipelinePackageBase* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    FaceUtils utils;

    MLPIPELINE_LOOP_START(MLPipelineStage::Extractor, thisQueue);
    package = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        // preprocess the image

        // copy the image to a cv::UMat

        cv::UMat cvUResizedImage;
        cv::UMat cvUImage   = QtOpenCVImg::image2Mat(
                                                     package->image,
                                                     CV_8UC3,
                                                     QtOpenCVImg::MatColorOrder::MCO_BGR
                                                    )
                                                    .getUMat(cv::ACCESS_RW);

        // resize the image if needed. Only resize if the image is larger than the input size of the detector

        cv::Size inputImageSize = faceDetector->nnInputSizeRequired();
        float resizeFactor      = 1.0F;

        if (std::max(cvUImage.cols, cvUImage.rows) > std::max(inputImageSize.width, inputImageSize.height))
        {
            // Image should be resized. YuNet image sizes are much more flexible than SSD and YOLO
            // so we just need to make sure no one bound exceeds the max. No padding needed.

            resizeFactor            = std::min(static_cast<float>(inputImageSize.width)  / static_cast<float>(cvUImage.cols),
                                               static_cast<float>(inputImageSize.height) / static_cast<float>(cvUImage.rows));

            int newWidth            = (int)(resizeFactor * cvUImage.cols);
            int newHeight           = (int)(resizeFactor * cvUImage.rows);
            cv::resize(cvUImage, cvUResizedImage, cv::Size(newWidth, newHeight));
        }
        else
        {
            cvUResizedImage = cvUImage;
        }

        // get reciprocal factor for resizing the face to back the original image size

        float reciprocalFactor      = 1.0F / resizeFactor;

        // lock OpenCV OpenCL DNN settings while using UMat

        OpenCVOpenCLDNNSetter openCLDNNSetter;;

        // detect any faces in the image

        cv::UMat udetectionResults  = faceDetector->callModel(cvUResizedImage);

        // process detected faces

        if (udetectionResults.rows > 0)
        {
            cv::Mat detectionResults   = udetectionResults.getMat(cv::ACCESS_READ);

            // get list of all previously detected faces

            QList<FaceTagsIface> faces = utils.databaseFaces(package->info.id());

            QList<QRectF> faceFRects;

            // Loop through the faces found.

            for (int i = 0 ; i < detectionResults.rows ; ++i)
            {
                // Create the rect of the face.

                const float x      = detectionResults.at<float>(i, 0);
                const float y      = detectionResults.at<float>(i, 1);
                const float width  = detectionResults.at<float>(i, 2);
                const float height = detectionResults.at<float>(i, 3);

                if (!std::isfinite(x)      ||
                    !std::isfinite(y)      ||
                    !std::isfinite(width)  ||
                    !std::isfinite(height) ||
                    (width  <= 0.0F)       ||
                    (height <= 0.0F))
                {
                    continue;
                }

                const int X       = static_cast<int>(x);
                const int Y       = static_cast<int>(y);
                const int iwidth  = static_cast<int>(width);
                const int iheight = static_cast<int>(height);

                if (
                    (X      == INT_MIN) ||
                    (Y      == INT_MIN) ||
                    (iwidth  == INT_MIN) ||
                    (iheight == INT_MIN)
                   )
                {
                    qCWarning(DIGIKAM_FACESENGINE_LOG) << "Invalid detection rectangle"
                                                       << "(" << X<< ", " << Y
                                                       << "[" << iwidth << ", " << iheight << "]"
                                                       << "for image"
                                                       << package->info.filePath();

                    continue;
                }

                // Add the rect to result list.

                QRectF currentFaceFRect(qreal(X)      / qreal(cvUResizedImage.cols),
                                        qreal(Y)      / qreal(cvUResizedImage.rows),
                                        qreal(iwidth) / qreal(cvUResizedImage.cols),
                                        qreal(iheight)/ qreal(cvUResizedImage.rows));

                faceFRects << currentFaceFRect;

                // compute current image relative rect

                QRect rect = QRect(package->image.width()  * currentFaceFRect.x(),
                                   package->image.height() * currentFaceFRect.y(),
                                   package->image.width()  * currentFaceFRect.width(),
                                   package->image.height() * currentFaceFRect.height());

                // check if rect is already assigned to a face to filter out confirmed and ignored faces

                bool found = false;
                FaceTagsIface face;

                if (faces.size() > 0)
                {
                    for (int j = 0; j < faces.size(); ++j)
                    {
                        if (faces[j].region().intersects(TagRegion(rect), 0.5))
                        {
                            if (faces[j].isConfirmedName() || faces[j].isIgnoredName())
                            {
                                // face is already confirmed or ignored

                                found = true;
                                break;
                            }
                            else
                            {
                                // face is not confirmed or ignored, but already detected

                                face = faces[j];
                                break;
                            }
                        }
                    }
                }

                // if face is not found (confirmed), then extract for classification

                if (!found)
                {
                    cv::UMat ualignedFace;
                    cv::UMat uface_features;
                    cv::Mat  face_features;

                    // extract the face vectors (features) for classification

                    // get cvMat of the face landmarks

                    cv::Mat row = detectionResults.row(i);

                    // convert the face landmarks to the full size image

                    for (int j = 0 ; j < row.cols ; ++j)
                    {
                        row.at<float>(j) = (int)(row.at<float>(j) * reciprocalFactor);
                    }

                    // convert the face landmarks to a UMat

                    cv::UMat urow = row.getUMat(cv::ACCESS_READ);

                    // align and crop the face

                    QMutexLocker lock(&(faceExtractor->mutex));

                    faceExtractor->getNet()->alignCrop(cvUImage, urow, ualignedFace);

                    cv::UMat paddedFace;

                    if (std::min(ualignedFace.cols, ualignedFace.rows) > 112)
                    {
                        // Image should be resized. YuNet image sizes are much more flexible than SSD and YOLO.
                        // So we just need to make sure no one bound exceeds the max. No padding needed.

                        float resizeFactor2     = std::min(static_cast<float>(112) / static_cast<float>(ualignedFace.cols),
                                                           static_cast<float>(112) / static_cast<float>(ualignedFace.rows));

                        int newWidth            = (int)(resizeFactor2 * ualignedFace.cols);
                        int newHeight           = (int)(resizeFactor2 * ualignedFace.rows);
                        cv::resize(ualignedFace, paddedFace, cv::Size(newWidth, newHeight));
                    }
                    else
                    {
                        paddedFace = ualignedFace.clone();
                    }

                    ualignedFace.release();

                    // get the face features

                    faceExtractor->getNet()->feature(paddedFace, uface_features);

                    face_features = uface_features.getMat(cv::ACCESS_READ);

                    // normalize the face features if we have any

                    if (0 < face_features.rows)
                    {
                        // normalize the face features

                        cv::Mat normalized_features;
                        normalize(face_features, normalized_features);

                        // add the face features and face rect to the package

                        package->featuresList << normalized_features;
                        package->faceRects << currentFaceFRect;
                        package->faceList << face;
                    }
                }
            }
        }

        // send the package to the next stage

        enqueue(nextQueue, package);

        package = nullptr;
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Extractor, "FacePipelineDetectRecognize::extractor");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Extractor, MLPipelineStage::Classifier);
}

bool FacePipelineDetectRecognize::classifier()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceDetReco%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Classifier, MLPipelineStage::Writer);
    FacePipelinePackageBase* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    FaceClassifier* const classifier = FaceClassifier::instance();
    classifier->setParameters(settings);

    MLPIPELINE_LOOP_START(MLPipelineStage::Classifier, thisQueue);
    package = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        for (int i = 0 ; i < package->featuresList.size() ; ++i)
        {
            // verify the feature mat is not empty

            if (0 < package->featuresList[i].rows)
            {
                QList<int> exclusionIdentityIds;

                for (const auto tagId : package->faceList[i].rejectedFaceTagList())
                {
                    // add the Identity ID for the rejected face tag to the exclusion list

                    QMultiMap<QString, QString> attributes = FaceTags::identityAttributes(tagId);
                    Identity identity                      = IdentityProvider::instance()->findIdentity(attributes);

                    if (!identity.isNull())
                    {
                        exclusionIdentityIds << identity.id();
                    }
                }

                // classify the features

                package->labelList << classifier->predict(package->featuresList[i], exclusionIdentityIds);
            }
            else
            {
                package->labelList << FaceClassifier::UNKNOWN_LABEL_ID;
            }
        }

        enqueue(nextQueue, package);

        package = nullptr;
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Classifier, "FacePipelineDetectRecognize::classifier");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Classifier, MLPipelineStage::Writer);
}

bool FacePipelineDetectRecognize::writer()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("FaceDetReco%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    MLPIPELINE_STAGE_START(QThread::LowPriority, MLPipelineStage::Writer, MLPipelineStage::None);
    FacePipelinePackageBase* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    const IdentityProvider* const idProvider          = IdentityProvider::instance();
    FaceUtils                     utils;
    ThumbnailLoadThread* const    thumbnailLoadThread = ThumbnailLoadThread::defaultThread();
    const QList<AlbumRootInfo>    roots               = CoreDbAccess().db()->getAlbumRoots();
    Q_UNUSED(roots);

    MLPIPELINE_LOOP_START(MLPipelineStage::Writer, thisQueue);
    package                                           = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        switch (settings.alreadyScannedHandling)
        {
            case FaceScanSettings::Rescan:
            {
                // remove old unconfirmed face rects

                QList<FaceTagsIface> oldEntries = utils.unconfirmedFaceTagsIfaces(package->info.id());
                utils.removeFaces(oldEntries);
                break;
            }

            case FaceScanSettings::ClearAll:
            {
                // remove all face rects

                utils.removeAllFaces(package->info.id());
                break;
            }

            case FaceScanSettings::RecognizeOnly:
            case FaceScanSettings::Skip:
            {
                // do nothing
                // Skipped images were skipped in the finder stage
                // RecognizeOnly defines the pipeline

                break;
            }
        }

        // mark the image as scanned

        utils.markAsScanned(package->info);

        // create thumbnails and write the new face rects to the database

        QStringList names;

        if (package->faceRects.size())
        {
            QList<FaceTagsIface>    databaseFaces;
            QList<Identity>         identities;
            QList<QRectF>           paddedRectList;

            for (int i = 0 ; i < package->faceRects.size() ; ++i)
            {
                // pad the rect by FaceTagsIface::faceThumbnailResizeFactor (1/2 on each side)

                QRectF paddedRect = QRectF(qMax(0.0, package->faceRects[i].x()      - (package->faceRects[i].width()  * (FaceTagsIface::faceThumbnailResizeFactor / 2.0F))),
                                           qMax(0.0, package->faceRects[i].y()      - (package->faceRects[i].height() * (FaceTagsIface::faceThumbnailResizeFactor / 2.0F))),
                                           qMin(1.0 ,package->faceRects[i].width()  + (package->faceRects[i].width()  * FaceTagsIface::faceThumbnailResizeFactor)),
                                           qMin(1.0 ,package->faceRects[i].height() + (package->faceRects[i].height() * FaceTagsIface::faceThumbnailResizeFactor)));

                paddedRectList << paddedRect;

                QRect faceRect(std::round(package->image.width()  * paddedRect.x()),
                               std::round(package->image.height() * paddedRect.y()),
                               std::round(package->image.width()  * paddedRect.width()),
                               std::round(package->image.height() * paddedRect.height()));

                if (package->labelList[i] != FaceClassifier::UNKNOWN_LABEL_ID)
                {
                    Identity identity = idProvider->identity(package->labelList[i]);
                    names         << identity.attribute(QStringLiteral("name"));
                    identities    << identity;
                    databaseFaces << FaceTagsIface(FaceTagsIface::Type::UnconfirmedName,
                                                   package->info.id(),
                                                   FaceTags::unconfirmedPersonTagId(),
                                                   TagRegion(faceRect));
                }
                else
                {
                    identities    << Identity();
                    databaseFaces << FaceTagsIface(FaceTagsIface::Type::UnknownName,
                                                   package->info.id(),
                                                   FaceTags::unknownPersonTagId(),
                                                   TagRegion(faceRect));
                }
            }

            // store the thumbnails

            if (!package->image.isNull())
            {
                utils.storeThumbnails(thumbnailLoadThread, package->info.filePath(),
                                      databaseFaces, package->image);
            }

            // write the new face rects to the database

            utils.writeUnconfirmedResults(package->info.id(),
                                          paddedRectList,
                                          identities,
                                          package->image.originalSize());
        }

        QString albumName = CollectionManager::instance()->albumRootLabel(package->info.albumRootId());

        // send a notification that the image was processed

        notify(MLPipelineNotification::notifyProcessed,
               package->info.name(),
               albumName + package->info.relativePath(),
               names.join(QLatin1String(", ")),
               package->faceRects.size(),
               package->thumbnailIcon);

        // delete the package

        delete package;

        package = nullptr;
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(MLPipelineStage::Writer, "FacePipelineDetectRecognize::writer");

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(MLPipelineStage::Writer, MLPipelineStage::None);
}

void FacePipelineDetectRecognize::addMoreWorkers()
{
    /* =========================================================================================
     * Use the performanceProfile metrics to find the slowest stages
     * and add more workers to those stages.
     *
     * For the Face detection and recognition pipeline, the loader is the
     * slowest stage so add 3 more loaders, 2 more extractors, and 1 more classifier.
     */

    addWorker(Loader);
    addWorker(Loader);
    addWorker(Loader);
    addWorker(Extractor);
    addWorker(Extractor);
    addWorker(Classifier);
}

} // namespace Digikam

#include "moc_facepipelinedetectrecognize.cpp"
