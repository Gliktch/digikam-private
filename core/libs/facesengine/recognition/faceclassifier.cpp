/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Face classifier
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "faceclassifier.h"

// Qt includes

#include <QException>
#include <QReadWriteLock>
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QFile>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "identityprovider.h"
#include "dnnmodelmanager.h"
#include "facebackgroundrecognition.h"
#include "systemsettings.h"
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN FaceClassifier::Private
{
public:

    Private() = default;

public:

    bool                                    ready                   = false;
    bool                                    trainingWaiting         = false;
    bool                                    exiting                 = false;
    bool                                    useFullSearch           = true;
    bool                                    initialLoad             = true;
    bool                                    debugLog                = SystemSettings(qApp->applicationName()).enableLogging;

    FaceScanSettings::FaceRecognitionModel  recognizeModel          = FaceScanSettings::FaceRecognitionModel::SFace;
    IdentityProvider*                       identityProvider        = nullptr;

    /// Qt variables

    QReadWriteLock                          trainingLock;
    QFutureWatcher<bool>                    trainingFuture;
    QThread*                                trainingThread           = nullptr;
    QMutex                                  trainingThreadMutex;
    QMutex                                  trainingMutex;
    QHash<int, QList<cv::Mat> >             identityFeatures;

    /// OpenCV variables

    cv::Ptr<cv::ml::KNearest>               knnClassifier;
    cv::Ptr<cv::ml::SVM>                    svmClassifier;

    /**
     * Classifier hyper-parameters.
     */

    /// KNN hyper-parameters

    const int                               knn_defaultK            = 8;
    const cv::ml::KNearest::Types           knn_algorithm           = cv::ml::KNearest::Types::BRUTE_FORCE;

    /// SVM hyper-parameters

    const cv::ml::SVM::Types                svm_type                = cv::ml::SVM::Types::C_SVC;
    const cv::ml::SVM::KernelTypes          svm_kernel              = cv::ml::SVM::KernelTypes::RBF;
    const double                            svm_gamma               = 0.01;
    const double                            svm_C                   = 150;
    const double                            svm_epsilon             = 1e-5;
    const int                               svm_minIterations       = 50;
    const int                               svm_maxIterations       = 500;
};

class Q_DECL_HIDDEN FaceClassifierCreator
{
public:

    FaceClassifier object;
};

Q_GLOBAL_STATIC(FaceClassifierCreator, creator)

// -----------------------------------------------------------

FaceClassifier::FaceClassifier()
    : FaceClassifierBase(),
      d                 (new Private)
{
    d->identityProvider = IdentityProvider::instance();

    // Use dynamic binding as retrain() function is virtual and called in constructor.

    this->retrain();
}

FaceClassifier::~FaceClassifier()
{
    delete d;
}

FaceClassifier* FaceClassifier::instance()
{
    return &creator->object;
}

bool FaceClassifier::ready() const
{
    return d->ready;
}

void FaceClassifier::cancel()
{
    // Don't restart the training thread if we are exiting.

    d->exiting         = true;
    d->trainingWaiting = false;

    // stop the training thread

    QMutexLocker lock(&d->trainingThreadMutex);

    if (d->trainingThread)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::cancel: forcing termination of training thread";
        d->trainingThread->terminate();

        QThread::msleep(100);
    }

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::cancel: cancel complete";
}

void FaceClassifier::setParameters(const FaceScanSettings& parameters)
{
    d->recognizeModel = parameters.recognizeModel;

    // set the threshold for the model in case we cannot get it from the model

    threshold         = 0.5F;

    try
    {
        // TODO: add other models if we decide to keep OpenFace

        if (d->recognizeModel == FaceScanSettings::FaceRecognitionModel::SFace)
        {
            // get a pointer to the model

            const DNNModelBase* const model = DNNModelManager::instance()->getModel(QLatin1String("sface"),
                                                                                    DNNModelUsage::DNNUsageFaceRecognition);

            // check if the model is loaded

            if (model)
            {
                // get the threshold from the model

                threshold = model->getThreshold(parameters.recognizeAccuracy);
            }
        }
    }
    catch (const std::exception& e)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::setParameters: exception: " << e.what();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::setParameters: Default exception from OpenCV";
    }
}

cv::Ptr<cv::ml::KNearest> FaceClassifier::createKNearest() const
{
    // Create the KNN classifier.

    cv::Ptr<cv::ml::KNearest> cvClassifier = cv::ml::KNearest::create();
    cvClassifier->setIsClassifier(true);
    cvClassifier->setAlgorithmType(d->knn_algorithm);
    cvClassifier->setDefaultK(d->knn_defaultK);

    return cvClassifier;
}

cv::Ptr<cv::ml::SVM> FaceClassifier::createSVM(int iterations) const
{
    // Create the SVM classifier.

    cv::Ptr<cv::ml::SVM> cvClassifier = cv::ml::SVM::create();
    cvClassifier->setType(d->svm_type);
    cvClassifier->setKernel(d->svm_kernel);
    cvClassifier->setGamma(d->svm_gamma);
    cvClassifier->setC(d->svm_C);
    cvClassifier->setTermCriteria(cv::TermCriteria(
                                                 cv::TermCriteria::Type::MAX_ITER + cv::TermCriteria::Type::EPS,
                                                 iterations,
                                                 d->svm_epsilon
                                                ));

    return cvClassifier;
}

bool FaceClassifier::retrain()
{
    // Called to retrain the classifier.

    if (d->exiting)
    {
        // don't retrain if we are exiting

        return false;
    }

    // lock the mutex

    QMutexLocker lock(&d->trainingMutex);

    // tell the training thread there is training to do

    d->trainingWaiting = true;

    // check if the training thread is already running

    if (d->trainingFuture.future().isRunning())
    {
        // training thread is already running

        return false;
    }

    // start the training thread

    d->trainingFuture.setFuture(QtConcurrent::run(
                                                  QThreadPool::globalInstance(),

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                                  &FaceClassifier::loadTrainingData,
                                                  this

#else

                                                  this,
                                                  &FaceClassifier::loadTrainingData

#endif

                                                 )
    );

    return true;
}

bool FaceClassifier::loadTrainingData()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    // Training thread to load the training data and should have a higher priority.

    QMutexLocker threadLock(&d->trainingThreadMutex);
    d->trainingThread = QThread::currentThread();
    d->trainingThread->setPriority(QThread::Priority::HighPriority);
    threadLock.unlock();

    qint64 ceElapsed  = 0;

    QElapsedTimer timer;
    timer.start();

    // Loop to rebuild the classifier components if there is training waiting
    // d->trainingWaiting can be modified outside the loop.

    do
    {
        // set default full search flag

        bool useFullSearch = true;

        // clear the training waiting flag

        d->trainingWaiting = false;

        try
        {
            // create new KNN and SVM classifiers.  We'll swap them in at the end

            QHash<int, QList<cv::Mat> > identityFeatures;

            // get the training data from the identity provider

            cv::Ptr<cv::ml::TrainData> trainData = d->identityProvider->getTrainingData();

            // if we have training data, split it into samples and labels

            if (nullptr != trainData)
            {
                cv::Mat samples = trainData->getSamples();
                cv::Mat labels  = trainData->getResponses();

                for (int i = 0 ; i < labels.rows ; ++i)
                {
                    int label = labels.at<int>(i);
                    identityFeatures[label].append(samples.row(i));
                }

                int svm_iterations = qMin(
                                          qMax(
                                               20,
                                               identityFeatures.count()
                                              ),
                                          d->svm_maxIterations
                                         );

                cv::Ptr<cv::ml::KNearest> knn = createKNearest();
                cv::Ptr<cv::ml::SVM>      svm = createSVM(svm_iterations);

                if (d->initialLoad)
                {
                    // clear initial load flag

                    d->initialLoad   = false;

                    // If this is the first time we are loading the training data,
                    // we can use the FaceClassifier with just the identity features if we use full search
                    // See bug 502219.

                    d->useFullSearch = true;

                    // lock the classifiers and identity list

                    d->trainingLock.lockForWrite();

                    // swap the new identity list with the currently running one

                    d->identityFeatures = identityFeatures;

                    // classifier is ready to use

                    d->ready         = true;

                    // unlock the identity list

                    d->trainingLock.unlock();

                    qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: face classifier is ready to use with full search in "
                                                     << timer.elapsed() << "ms";
                }

                // if we have enough samples, train the classifiers

                if (identityFeatures.count() > d->knn_defaultK)
                {
                    knn->train(trainData);
                    svm->train(trainData);

                    if (knn->isTrained() && svm->isTrained())
                    {
                        if (d->debugLog)
                        {
                            QElapsedTimer ceTimer;
                            ceTimer.start();
                            float currentError = svm->calcError(trainData, false, cv::noArray());
                            qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: SVM calcError is: "
                                                             << QString::number(currentError, 'f', 6).toLatin1().constData();
                            ceElapsed = ceTimer.elapsed();
                        }

                        // we have enough samples to use the classifiers

                        useFullSearch = false;
                    }
                    else
                    {
                        // Something went wrong with training KNN or SVM classifiers, so revert to full search

                        useFullSearch = true;

                        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: KNN or SVM training failed, reverting to full search";
                    }
                }
                else
                {
                    // we don't have enough samples to use the classifiers so use full search

                    useFullSearch = true;
                }

                // lock the classifiers and identity list

                d->trainingLock.lockForWrite();

                // swap the new classifiers and identity list with the currently running ones

                d->knnClassifier    = knn;
                d->svmClassifier    = svm;
                d->identityFeatures = identityFeatures;
                d->useFullSearch    = useFullSearch;
                d->ready            = true;

                // unlock the classifiers and identity list

                d->trainingLock.unlock();
            }
            else
            {
                // no training data, so we cannot train the classifiers

                qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: no training data available";
            }
        }
        catch (cv::Exception& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: exception: " << e.what();
        }
        catch (const QException& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: exception: " << e.what();
        }
        catch (const std::exception& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: exception: " << e.what();
        }
        catch (...)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: Default exception from OpenCV";
        }
    }
    while (d->trainingWaiting);

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::loadTrainingData: training completed in "
                                     << timer.elapsed() - ceElapsed << "ms";

    // emit the training complete signal

    Q_EMIT signalTrainingComplete();

    threadLock.relock();
    d->trainingThread = nullptr;

    return true;
}

int FaceClassifier::predict(const cv::Mat& target, const QList<int>& exclusionLabelList) const
{
    int label = UNKNOWN_LABEL_ID;
    Q_UNUSED(label);

    if (!d->ready)
    {
        return UNKNOWN_LABEL_ID;
    }

    d->trainingLock.lockForRead();

    if (d->useFullSearch)
    {
        // we don't have enough identities and samples yet to use the knn and svm classifiers
        // so we perform a full brute-force search on all the known faces

        label = predictFullSearch(target, exclusionLabelList);
    }
    else
    {
        // use the classifier algorithm

        label = predictClassifier(target, exclusionLabelList);
    }

    d->trainingLock.unlock();

    return label;
}

int FaceClassifier::predict(const cv::UMat& target, const QList<int>& exclusionLabelList) const
{
    cv::Mat matTarget = target.getMat(cv::ACCESS_READ);

    return predict(matTarget, exclusionLabelList);
}

int FaceClassifier::predictFullSearch(const cv::Mat& target, const QList<int>& exclusionLabelList)const
{
    QElapsedTimer timer;
    timer.start();

    int label = listSearch(target, d->identityFeatures, exclusionLabelList);

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::predictFullSearch: classifier prediction is: "
                                     << label << " completed in " << timer.elapsed();

    return label;
}

int FaceClassifier::predictClassifier(const cv::Mat& target, const QList<int>& exclusionLabelList) const
{
    int     label                      = UNKNOWN_LABEL_ID;
    int     badLabel1                  = UNKNOWN_LABEL_ID;
    int     badLabel2                  = UNKNOWN_LABEL_ID;
    cv::Mat knn_resultMat;
    cv::Mat knn_neighbors;
    cv::Mat knn_distances;
    cv::Mat svm_prediction;
    const IdentityProvider* const idProvider = IdentityProvider::instance();

    QElapsedTimer timer;
    timer.start();

    // stage 1 - compare SVM and KNN classifier results

    d->svmClassifier->predict(target, svm_prediction);
    int svm_result = int(svm_prediction.at<float>(0, 0));

    d->knnClassifier->findNearest(target,
                                  std::min<int>(d->identityFeatures.count(), d->knn_defaultK),
                                  knn_resultMat,
                                  knn_neighbors,
                                  knn_distances);

    int knn_result = int(knn_resultMat.at<float>(0, 0));

    // check if results exist in recognition DB

    if (!idProvider->isValidId(svm_result) || exclusionLabelList.contains(svm_result))
    {
        svm_result = UNKNOWN_LABEL_ID;
    }

    if (!idProvider->isValidId(knn_result) || exclusionLabelList.contains(knn_result))
    {
        knn_result = UNKNOWN_LABEL_ID;
    }

    // If the SVM and KNN classifiers agree it's usually over 98% correct unless
    // we have massively unbalanced data so we need to validate the results.

    if ((svm_result == knn_result) && (svm_result != UNKNOWN_LABEL_ID))
    {
        if (validateKNNSVMResult(target, svm_result))
        {
            // we have a match

            qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::predictClassifier: classifier stage 1 prediction is: "
                                             << svm_result << " completed in " << timer.elapsed();

            return svm_result;
        }

        else
        {
            // save the bad label so we don't check it again

            badLabel1 = svm_result;
        }
    }

    // Stage 2 - check the absolute nearest neighbor for a match to the SVM result

    if ((UNKNOWN_LABEL_ID != svm_result) && (UNKNOWN_LABEL_ID != knn_result))
    {
        // find the absolute nearest neighbor

        float distance = MAX_DISTANCE;

        for (int i = 0 ; i < knn_neighbors.cols ; ++i)
        {
            if (distance > abs(knn_distances.at<float>(0, i)))
            {
                label    = int(knn_neighbors.at<float>(0, i));
                distance = abs(knn_distances.at<float>(0, i));
            }
        }

        // verify we haven't checked this label before

        if ((label == svm_result) && (label != badLabel1))
        {
            // check the distance of the target to the samples for the label

            if (validateKNNSVMResult(target, label))
            {
                // we have a match

                qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::predictClassifier: classifier stage 2 prediction is: "
                                                 << label << " completed in " << timer.elapsed();

                return label;
            }

            else
            {
                // save the bad label so we don't check it again

                badLabel2 = label;
            }
        }
    }

    // Stage 3 - do brute-force search on distinct set of SVM and KNN labels returned
    // from the internal classifiers.  Ignore labels we've already checked.

    {
        d->knnClassifier->findNearest(target,
                                      std::min<int>(d->identityFeatures.count(), d->knn_defaultK),
                                      knn_resultMat,
                                      knn_neighbors,
                                      knn_distances);

        QHash<int, QList<cv::Mat> > featureSet;

        if ((svm_result != badLabel1) && (svm_result != badLabel2) && idProvider->isValidId(svm_result))
        {
            featureSet[svm_result] = d->identityFeatures[svm_result];
        }

        for (int i = 0 ; i < knn_neighbors.cols ; ++i)
        {
            int tmpLabel = int(knn_neighbors.at<float>(0, i));

            if ((tmpLabel != badLabel1) && (tmpLabel != badLabel2) && idProvider->isValidId(tmpLabel))
            {
                featureSet[tmpLabel] = d->identityFeatures[tmpLabel];
            }
        }

        label = listSearch(target, featureSet, exclusionLabelList);
    }

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceClassifier::predictClassifier: classifier stage 3 prediction is: "
                                     << label << " completed in " << timer.elapsed();

    return label;
}

bool FaceClassifier::validateKNNSVMResult(const cv::Mat& target, int label) const
{
    bool result = false;

    for (const cv::Mat& feature : std::as_const(d->identityFeatures[label]))
    {
        float distance = 10000.0F;

        if (featureSFaceCompare(target, feature, distance))
        {
            result = true;
            break;
        }
    }

    return result;
}

int FaceClassifier::listSearch(const cv::Mat& target,
                               const QHash<int, QList<cv::Mat> >& identityFeatures,
                               const QList<int>& exclusionLabelList) const
{
    VotingGroups votes;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    for (const auto& [key, value] : identityFeatures.asKeyValueRange())
    {

#else

    for (const auto& key : identityFeatures.keys())
    {
        const auto& value = identityFeatures[key];

#endif

        for (const cv::Mat& feature : std::as_const(value))
        {
            if (exclusionLabelList.contains(key))
            {
                continue;
            }

            float distance = 10000.0F;

            if (featureSFaceCompare(target, feature, distance))
            {
                votes.addVote(key, distance / value.count());
            }
        }
    }

    return votes.winner(VotingGroups::VotesLowScore);
}

bool FaceClassifier::featureSFaceCompare(const cv::Mat& target, const cv::Mat& sample, float& distance) const
{
    double cosDistance     = sum(target.mul(sample))[0];
    double norm_l1Distance = norm(target, sample);

    // Recompute l1Threshold for SFace based on passed in sqRange.
    // d->recognizeThreshold is controlled via the UI.
    // We use the inverse of the cos result to compare to sqRange
    // and norm_l1 distance compared to recomputed l1Threshold.
    // norm_l1 is almost always < 1 for a good match. We add .1 of d->recognizeThreshold for extra verification
    // to avoid false negatives.

    float l1Threshold      = 1.0F + (threshold / 10.0F);

    // NOTE: both cosine distance and l1 distance can help to avoid errors with similarity prediction.

    distance = (1.0 - cosDistance);

    return (
            (distance        < threshold) &&
            (norm_l1Distance < l1Threshold)
           );
}

} // namespace Digikam

#include "moc_faceclassifier.cpp"
