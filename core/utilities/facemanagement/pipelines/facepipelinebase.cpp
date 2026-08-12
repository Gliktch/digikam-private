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

#include "facepipelinebase.h"

// Qt includes

#include <QList>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_opencv.h"
#include "sharedqueue.h"
#include "facepipelinepackagebase.h"
#include "thumbnailloadthread.h"
#include "dnnsfaceextractor.h"
#include "dnnmodelmanager.h"
#include "identityprovider.h"

namespace Digikam
{

FacePipelineBase::FacePipelineBase(const FaceScanSettings& _settings)
    : MLPipelineFoundation(),
      settings            (_settings)
{
}

FacePipelineBase::~FacePipelineBase()
{
}

double FacePipelineBase::detectNoise1(const cv::Mat& cvGrayImage) const
{
    // Use a Gaussian filter to detect noise

    // apply Gaussian blur to the image

    cv::Mat blurred;
    cv::GaussianBlur(cvGrayImage, blurred, cv::Size(5, 5), 0);

    // calculate the difference between the original and blurred image

    cv::Mat noise     = cvGrayImage - blurred;

    // calculate the standard deviation of the noise

    cv::Mat mean, stddev;
    cv::meanStdDev(noise, mean, stddev);

    double noiseLevel = stddev.at<double>(0, 0);

    return noiseLevel;
}

double FacePipelineBase::detectNoise2(const cv::Mat& cvGrayImage) const
{
    int H        = cvGrayImage.rows;
    int W        = cvGrayImage.cols;
    cv::Mat M    = (cv::Mat_<double>(3, 3) <<  1, -2,  1,
                                              -2,  4, -2,
                                               1, -2,  1);

    cv::Mat convolved;
    cv::filter2D(cvGrayImage, convolved, CV_64F, M);

    double sigma = cv::sum(cv::abs(convolved))[0];
    sigma        = sigma * std::sqrt(0.5 * M_PI) / (6 * (W - 2) * (H - 2));

    return sigma;
}

double FacePipelineBase::detectBlur(const cv::Mat& cvGrayImage) const
{
    // Use a Fast Fourier Transform to detect blurriness

    int cx = cvGrayImage.cols / 2;
    int cy = cvGrayImage.rows / 2;

    // Convert the image to a flat float

    cv::Mat fImage;
    cvGrayImage.convertTo(fImage, CV_32F);

    // FFT

    cv::Mat fourierTransform;
    cv::dft(fImage, fourierTransform, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);

    // center low frequencies in the middle
    // by shuffling the quadrants.

    cv::Mat q0(fourierTransform, cv::Rect(0,  0,  cx, cy));     // Top-Left - Create a ROI per quadrant
    cv::Mat q1(fourierTransform, cv::Rect(cx, 0,  cx, cy));     // Top-Right
    cv::Mat q2(fourierTransform, cv::Rect(0,  cy, cx, cy));     // Bottom-Left
    cv::Mat q3(fourierTransform, cv::Rect(cx, cy, cx, cy));     // Bottom-Right

    // swap quadrants (Top-Left with Bottom-Right)

    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    // swap quadrant (Top-Right with Bottom-Left)

    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    // Block the low frequencies
    // #define BLOCK could also be a argument on the command line of course

    fourierTransform(cv::Rect(cx - BLOCK, cy - BLOCK, 2 * BLOCK, 2 * BLOCK)).setTo(0);

    //shuffle the quadrants to their original position

    cv::Mat orgFFT;
    fourierTransform.copyTo(orgFFT);
    cv::Mat p0(orgFFT, cv::Rect(0,  0,  cx, cy));     // Top-Left - Create a ROI per quadrant
    cv::Mat p1(orgFFT, cv::Rect(cx, 0,  cx, cy));     // Top-Right
    cv::Mat p2(orgFFT, cv::Rect(0,  cy, cx, cy));     // Bottom-Left
    cv::Mat p3(orgFFT, cv::Rect(cx, cy, cx, cy));     // Bottom-Right

    // swap quadrant (Top-Left with Bottom-Right)

    p0.copyTo(tmp);
    p3.copyTo(p0);
    tmp.copyTo(p3);

    // swap quadrant (Top-Right with Bottom-Left)

    p1.copyTo(tmp);
    p2.copyTo(p1);
    tmp.copyTo(p2);

    // IFFT

    cv::Mat invFFT;
    cv::Mat logFFT;
    double minVal     = 0.0;
    double maxVal     = 0.0;

    cv::dft(orgFFT, invFFT, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);

    invFFT            = cv::abs(invFFT);
    cv::minMaxLoc(invFFT, &minVal, &maxVal);

    // check for impossible values

    if (maxVal <= 0.0)
    {
        return 1;
    }

    cv::log(invFFT, logFFT);
    logFFT           *= 20;

    cv::Scalar result = cv::mean(logFFT);

    return result.val[0];
}

bool FacePipelineBase::useForTraining(const QSize& faceSize, const cv::Mat& cvImage)
{
    if (!detectorModel)
    {
        detectorModel = DNNModelManager::instance()->getModel(QStringLiteral("yunet"),
                                                              DNNModelUsage::DNNUsageFaceDetection);
    }

    // thumbnail must be at least minThumbnailSize of the size the detector expects

    if (
        ((detectorModel->info.imageSize * minThumbnailSize) > faceSize.width()) ||
        ((detectorModel->info.imageSize * minThumbnailSize) > faceSize.height())
       )
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FacePipelineBase::useForTraining Image is too small for face detection"
                                         << faceSize.width() << "x" << faceSize.height()
                                         << "required:" << detectorModel->info.imageSize * minThumbnailSize;
        return false;
    }

    // convert to grayscale for use in noise and blur detection

    cv::Mat cvGrayImage;
    cv::cvtColor(cvImage, cvGrayImage, cv::COLOR_RGB2GRAY);

    // use a Gaussian filter to check for noisy images

    double noise1 = detectNoise1(cvGrayImage);
    double noise2 = detectNoise2(cvGrayImage);

    // check if image is too noisy

    if ((noise1 > noiseThreshold1) || (noise2 > noiseThreshold2))
    {
        return false;
    }

    // use a FFT filter to check for blurred images

    double blur = detectBlur(cvGrayImage);

    // check if image is too blurry

    if (blur < blurThreshold)
    {
        return false;
    }

    return true;
}

bool FacePipelineBase::commonFaceThumbnailLoader(const QString& pipelineName,
                                                 QThread::Priority stagePriority,
                                                 MLPipelineStage thisStage,
                                                 MLPipelineStage nextStage)
{
    MLPIPELINE_STAGE_START(stagePriority, thisStage, nextStage);
    FacePipelinePackageBase* package              = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    ThumbnailLoadThread* const thumbnailLoadThread = new ThumbnailLoadThread;
/*
    ThumbnailLoadThread* thumbnailLoadThread = ThumbnailLoadThread::defaultThread();
*/
    thumbnailLoadThread->setPixmapRequested(false);
    thumbnailLoadThread->setThumbnailSize(ThumbnailLoadThread::maximumThumbnailSize());
    thumbnailLoadThread->setPriority(QThread::NormalPriority);

    ThumbnailImageCatcher* const catcher           = new ThumbnailImageCatcher(thumbnailLoadThread);

    catcher->setActive(true);

    MLPIPELINE_LOOP_START(thisStage, thisQueue);
    package                                        = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        catcher->thread()->find(ItemInfo::thumbnailIdentifier(package->face.imageId()),
                                                              package->face.region().toRect());
        catcher->enqueue();
        QList<QImage> images = catcher->waitForThumbnails();

        if (images.size() && !images[0].isNull())
        {
            // store the thumbnail image in the package

            package->thumbnail     = images[0];

            // create a thumbnail icon

            package->thumbnailIcon = QIcon(DImg(package->thumbnail).smoothScale(48,
                                                                                48,
                                                                                Qt::KeepAspectRatio).convertToPixmap());

            // send the package to the next stage

            enqueue(nextQueue, package);

            package = nullptr;
        }
        else
        {
            // send a notification that the file was skipped

            notify(MLPipelineNotification::notifySkipped,
                   package->info.name(),
                   package->info.filePath(),
                   QString(),
                   0,
                   QIcon::fromTheme(QLatin1String("image-missing")));

            // delete the package since it is not needed

            delete package;

            package = nullptr;
        }
    }

    /* =========================================================================================
     * End pipeline stage specific loop
     */

    MLPIPELINE_LOOP_END(thisStage, QString(pipelineName + QLatin1String("::commonFaceThumbnailLoader")).toLocal8Bit().data());

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    catcher->setActive(false);
    catcher->thread()->stopAllTasks();
    catcher->cancel();

    delete catcher->thread();
    delete catcher;

    MLPIPELINE_STAGE_END(thisStage, nextStage);
}

bool FacePipelineBase::commonFaceThumbnailExtractor(const QString& pipelineName,
                                                    QThread::Priority stagePriority,
                                                    MLPipelineStage thisStage,
                                                    MLPipelineStage nextStage,
                                                    bool trainingQualityCheck)
{
    MLPIPELINE_STAGE_START(stagePriority, thisStage, nextStage);
    FacePipelinePackageBase* package = nullptr;

    /* =========================================================================================
     * Pipeline stage specific initialization code
     *
     * Use the block from here to MLPIPELINE_LOOP_START to initialize the stage.
     * The code in this block is run once per stage initialization. The number of instances
     * is at least 1. More instances are created by addMoreWorkers if needed.
     */

    DNNSFaceExtractor extractor;
    FaceUtils utils;

    MLPIPELINE_LOOP_START(thisStage, thisQueue);
    package                          = static_cast<FacePipelinePackageBase*>(mlpackage);

    /* =========================================================================================
     * Start pipeline stage specific loop
     *
     * All code from here to MLPIPELINE_LOOP_END is in a try/catch block and loop.
     * This loop is run once per image.
     */

    {
        QImage inputImage(package->thumbnail.copy());

        // preprocess image to be in the correct format

        if (inputImage.format() != QImage::Format_BGR888)
        {
            inputImage = inputImage.convertToFormat(QImage::Format_BGR888);
        }

        // create a cv::Mat image from the QImage

        cv::Mat  cvImage    = cv::Mat(
                                      inputImage.height(),
                                      inputImage.width(),
                                      CV_8UC3,
                                      inputImage.scanLine(0),
                                      inputImage.bytesPerLine()
                                     );

        if (!cvImage.empty())
        {
            cv::UMat cvUImage                     = cvImage.getUMat(cv::ACCESS_READ);

            // Extract the face features

            QPair<cv::Mat, cv::Mat> faceEmbedding = extractor.getFaceEmbedding(cvUImage);
            package->features                     = faceEmbedding.second;

            // Check for a valid feature set

            if (!package->features.empty() && trainingQualityCheck)
            {
                /**
                 * Compute the size of the face region in pixels as shown in the original image
                 * This is used to check if the image is suitable for training.
                 */

                // Prevent divide by zero - bug 513760.

                float width       = static_cast<float>(package->info.dimensions().width());
                float height      = static_cast<float>(package->info.dimensions().height());
                float thumbWidth  = static_cast<float>(package->thumbnail.width());
                float thumbHeight = static_cast<float>(package->thumbnail.height());

                if ((width <= 0.0) || (height <= 0.0) || (thumbWidth <= 0.0) || (thumbHeight <= 0.0))
                {
                    qCWarning(DIGIKAM_FACESENGINE_LOG) << "Invalid dimensions or thumbnail size, skipping training check.";
                    package->useForTraining = false;
                }
                else
                {
                    // Percentage of the region compared to the image

                    float xThumbnailPercent = static_cast<float>(package->face.region().toRect().width())  / width;
                    float yThumbnailPercent = static_cast<float>(package->face.region().toRect().height()) / height;

                    // Prevent infinite or not a number values - bug 513760.

                    float xFacePos          = faceEmbedding.first.at<float>(0, 2);
                    float yFacePos          = faceEmbedding.first.at<float>(0, 3);

                    if (
                        std::isnan(xFacePos) || std::isnan(yFacePos) ||
                        std::isinf(xFacePos) || std::isinf(yFacePos)
                       )
                    {
                        qCWarning(DIGIKAM_FACESENGINE_LOG) << "Invalid face embedding values, skipping training check.";
                        package->useForTraining = false;
                    }
                    else
                    {
                        // Percentage of the face compared to the thumbnail

                        float xFaceAreaPercent = xFacePos / thumbWidth;
                        float yFaceAreaPercent = yFacePos / thumbHeight;

                        // Compute the size of the face area in pixels

                        QSize faceSize(qRound(package->info.dimensions().width()  * xThumbnailPercent * xFaceAreaPercent),
                                       qRound(package->info.dimensions().height() * yThumbnailPercent * yFaceAreaPercent));

                        // Check if the image is suitable for training

                        package->useForTraining = useForTraining(faceSize, cvImage);
                    }
                }
            }

            for (const auto tagId : package->face.rejectedFaceTagList())
            {
                // add the Identity ID for the rejected face tag to the exclusion list

                QMultiMap<QString, QString> attributes = FaceTags::identityAttributes(tagId);
                Identity identity                      = IdentityProvider::instance()->findIdentity(attributes);

                if (!identity.isNull())
                {
                    package->exclusionIdentityIds << identity.id();
                }
            }

            // send the package to the next stage

            enqueue(nextQueue, package);

            package = nullptr;
        }
        else
        {
            // send a notification that the file was skipped

            notify(MLPipelineNotification::notifySkipped,
                   package->info.name(),
                   package->info.filePath(),
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

    MLPIPELINE_LOOP_END(thisStage, QString(pipelineName + QLatin1String("::commonFaceThumbnailExtractor")).toLocal8Bit().data());

    /* =========================================================================================
     * Pipeline stage specific cleanup
     *
     * Use the block from here to MLPIPELINE_STAGE_END to clean up any resources used by the stage.
     */

    MLPIPELINE_STAGE_END(thisStage, nextStage);
}

bool FacePipelineBase::enqueue(MLPipelineQueue* thisQueue, MLPipelinePackageFoundation* package)
{
    if (nullptr != package)
    {
        // calculate the package size.  Only big items need to be checked

        package->size = static_cast<FacePipelinePackageBase*>(package)->image.size().width()  *
                        static_cast<FacePipelinePackageBase*>(package)->image.size().height() * 4;
    }

    return MLPipelineFoundation::enqueue(thisQueue, package);
}

} // namespace Digikam

#include "moc_facepipelinebase.cpp"
