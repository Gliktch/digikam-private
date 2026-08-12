/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-05-15
 * Description : CLI tool to test and verify Face Recognition
 *               NOTE: This tool integrates the whole Face Management
 *               work flow, especially designed to verify and benchmark
 *               Face Recognition algorithm. It is adapted from recognize.cpp
 *               developed by Aditya Bhatt.
 *
 * SPDX-FileCopyrightText: 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QElapsedTimer>
#include <QCommandLineParser>
#include <QRectF>
#include <QList>
#include <QUuid>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "facescansettings.h"
#include "facedetector.h"
#include "facialrecognition_wrapper.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;

// --------------------------------------------------------------------------------------------------

QStringList toPaths(char** argv, int startIndex, int argc)
{
    QStringList files;

    for (int i = startIndex ; i < argc ; ++i)
    {
        files << QString::fromLatin1(argv[i]);
    }

    return files;
}

QList<QImage> toImages(const QStringList& paths)
{
    QList<QImage> images;

    for (const QString& path : std::as_const(paths))
    {
        images << QImage(path);
    }

    return images;
}

void prepareForTrain(const QString& testSetPath,
                     QMap<unsigned, QStringList>& testset,
                     QMap<unsigned, QStringList>& trainingset,
                     double ratio,
                     unsigned int nbOfSamples,
                     unsigned int& nbOfIdentities)
{
    QDir testSet(testSetPath);
    QStringList subjects = testSet.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    qCDebug(DIGIKAM_TESTS_LOG) << nbOfSamples << "," << nbOfIdentities;

    if (nbOfIdentities == 0)
    {
        nbOfIdentities = subjects.size();
    }

    for (unsigned i = 1 ; i <= nbOfIdentities ; ++i)
    {
        QString subjectPath = QString::fromLatin1("%1%2").arg(testSetPath)
                              .arg(subjects.takeFirst());
        QDir subjectDir(subjectPath);

        QStringList files                  = subjectDir.entryList(QDir::Files);
        unsigned int nbOfSamplePerIdentity = (nbOfSamples == 0) ? files.size() : nbOfSamples;

        for (unsigned j = 1 ; j <= nbOfSamplePerIdentity ; ++j)
        {
            QString path = QString::fromLatin1("%1/%2").arg(subjectPath)
                           .arg(files.takeFirst());

            if (j <= static_cast<unsigned int>(qRound(nbOfSamplePerIdentity * ratio)))
            {
                trainingset[i] << path;
                qCDebug(DIGIKAM_TESTS_LOG) << "training" << path;
            }

            else
            {
                testset[i] << path;
                qCDebug(DIGIKAM_TESTS_LOG) << "test" << path;
            }
        }
    }
}

QImage scaleForDetection(const DImg& image, const FaceDetector& detector)
{
    int recommendedSize = detector.recommendedImageSize(image.size());

    if (qMax(image.width(), image.height()) > (uint)recommendedSize)
    {
        return image.smoothScale(recommendedSize, recommendedSize, Qt::KeepAspectRatio).copyQImage();
    }

    return image.copyQImage();
}

QList<QRectF> processFaceDetection(const QImage& image, FaceDetector& detector)
{
    DImg img(image);
    QImage detectionImage       = scaleForDetection(img, detector);
    QList<QRectF> detectedFaces = detector.detectFaces(detectionImage, img.originalSize());

    qCDebug(DIGIKAM_TESTS_LOG) << "Found" << detectedFaces.size() << "faces";

    return detectedFaces;
}

QList<QRectF> processFaceDetection(const QString& imagePath, FaceDetector& detector)
{
    QList<QRectF> detectedFaces = detector.detectFaces(imagePath);

    qCDebug(DIGIKAM_TESTS_LOG) << "(Input CV) Found" << detectedFaces.size() << "faces";

    return detectedFaces;
}

QImage retrieveFace(const DImg& image, const QList<QRectF>& rects)
{
    if (rects.size() > 1)
    {
        qFatal("More than 1 face found in image, strange for our test set!!!");
    }

    QRectF rect = rects.first();
    QImage face = image.copyQImage(rect);

    return face;
}

QList<QImage*> retrieveFaces(const QList<QImage>& images, const QList<QRectF>& rects)
{
    QList<QImage*> faces;
    unsigned index = 0;

    for (const QRectF& rect : std::as_const(rects))
    {
        DImg temp(images.at(index));
        QImage* const croppedFace = new QImage();
        *croppedFace              = temp.copyQImage(rect);

        faces << croppedFace;
        ++index;
    }

    return faces;
}

// --------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.

    // Options for commandline parser.

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QLatin1String("db"), QLatin1String("Faces database"),                     QLatin1String("path to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("rs"), QLatin1String("Split ratio (test set / whole set)"), QLatin1String("decimal")));
    parser.addOption(QCommandLineOption(QLatin1String("ts"), QLatin1String("Test set folder"),                    QLatin1String("path relative to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("ds"), QLatin1String("Training set (dev set) folder"),      QLatin1String("path relative to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("ni"), QLatin1String("Number of total objects"),            QLatin1String("nbIdentities")));
    parser.addOption(QCommandLineOption(QLatin1String("ns"), QLatin1String("Number of samples per object"),       QLatin1String("nbSamples")));
    parser.addOption(QCommandLineOption(QLatin1String("as"), QLatin1String("Option to run test on the entire set")));
    parser.addHelpOption();
    parser.process(app);

    // Parse arguments.

    bool optionErrors = false;

    if (parser.optionNames().empty())
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "NO options!!!";
        optionErrors = true;
    }

    else if (!parser.isSet(QLatin1String("db")))
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "MISSING database for test!!!";
        optionErrors = true;
    }

    else if (
        !parser.isSet(QLatin1String("as")) &&
        (!parser.isSet(QLatin1String("ni")) || !parser.isSet(QLatin1String("ns")))
    )
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "UNKNOWN training set / test set separation!!!";
        optionErrors = true;
    }

    else if (parser.isSet(QLatin1String("ts")) && !parser.isSet(QLatin1String("ds")))
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "UNKNOWN Dev set!!!";
        optionErrors = true;
    }

    else if (parser.isSet(QLatin1String("ds")) && !parser.isSet(QLatin1String("ts")))
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "UNKNOWN Test set!!!";
        optionErrors = true;
    }

    if (optionErrors)
    {
        parser.showHelp();
        return 1;
    }

    QString facedb              = parser.value(QLatin1String("db"));
    unsigned int nbOfSamples    = 0;
    unsigned int nbOfIdentities = 0;

    if (!parser.isSet(QLatin1String("as")))
    {
        nbOfSamples    = parser.value(QLatin1String("ns")).toUInt();
        nbOfIdentities = parser.value(QLatin1String("ni")).toUInt();
    }

    double ratio = 0.0;

    if (parser.isSet(QLatin1String("rs")))
    {
        ratio = parser.value(QLatin1String("rs")).toDouble();
    }

    // Init config for digiKam.

    DbEngineParameters prm = DbEngineParameters::parametersFromConfig();
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);
    FacialRecognitionWrapper recognizer;

    // Construct training set, test set.

    QMap<unsigned, QStringList> testset, trainingset;

    if (ratio > 0.0)
    {
        prepareForTrain(facedb, testset, trainingset, ratio, nbOfSamples, nbOfIdentities);
    }

    else
    {
        QString testsetFolder    = parser.value(QLatin1String("ts"));
        QString trainingsetFoler = parser.value(QLatin1String("ds"));

        // TODO: Overload of prepareForTrain() to create training set and test set here.
    }

    // Create IDs.

    QMap<unsigned, Identity> idMap;

    for (unsigned i = 1 ; i <= nbOfIdentities ; ++i)
    {
        QMultiMap<QString, QString> attributes;
        attributes.insert(QLatin1String("name"), QString::number(i));
        idMap[i] = recognizer.addIdentityDebug(attributes);
    }

    // Init FaceDetector used for detecting faces and bounding box before recognizing.

    FaceDetector detector;

    // Evaluation metrics.

    unsigned int correct = 0, notRecognized = 0, falsePositive = 0, totalTrained = 0, totalRecognized = 0;
    unsigned int elapsedTraining = 0, elapsedTesting = 0;
    unsigned int detectingTime = 0;

    /*
     *  // Without using detector.

        for (QMap<unsigned, QStringList>::const_iterator it = trainingset.constBegin() ;
             it != trainingset.constEnd() ; ++it)
        {
            Identity identity = recognizer.findIdentity(QString::fromLatin1("name"), QString::number(it.key()));

            if (identity.isNull())
            {
                qCDebug(DIGIKAM_TESTS_LOG) << "Identity management failed for person " << it.key();
            }

            QList<QImage> images = toImages(it.value());
            qCDebug(DIGIKAM_TESTS_LOG) << "Training directory " << it.key();

            recognizer.train(identity, images, trainingContext);
            totalTrained        += images.size();
        }

        elapsedTraining = timer.restart();

        for (QMap<unsigned, QStringList>::const_iterator it = testset.constBegin() ;
             it != testset.constEnd() ; ++it)
        {
            Identity identity       = idMap.value(it.key());
            QList<QImage> images    = toImages(it.value());
            QList<Identity> results = recognizer.recognizeFaces(images);

            qCDebug(DIGIKAM_TESTS_LOG) << "Result for " << it.value().first() << " is identity " << results.first().id();

            for (const Identity& foundId : std::as_const(results))
            {
                if      (foundId.isNull())
                {
                    ++notRecognized;
                }
                else if (foundId == identity)
                {
                    ++correct;
                }
                else
                {
                    ++falsePositive;
                }
            }

            totalRecognized += images.size();
        }
    */

    QStringList undetectedTrainedFaces;
    QStringList undetectedTestedFaces;
    QStringList falsePositiveFaces;

    for (QMap<unsigned, QStringList>::const_iterator it = trainingset.constBegin() ;
         it != trainingset.constEnd() ; ++it)
    {
        Identity identity      = idMap.value(it.key());
        QStringList imagePaths = it.value();

        QList<QImage> detectedFaces;
        QList<QRectF> bboxes;

        QList<QImage> rawImages = toImages(imagePaths);
        qCDebug(DIGIKAM_TESTS_LOG) << "Training directory " << it.key();

        for (const QImage& image : std::as_const(rawImages))
        {
            QString imagePath = imagePaths.takeFirst();

            // Start timing for benchmark face detection.

            QElapsedTimer timer;
            timer.start();

            QList<QRectF> detectedBoundingBox = processFaceDetection(imagePath, detector);

            detectingTime += timer.elapsed();

            if (detectedBoundingBox.size())
            {
                detectedFaces << image;
                bboxes        << detectedBoundingBox.first();
                ++totalTrained;
            }

            else
            {
                undetectedTrainedFaces << imagePath;
            }
        }

        QList<QImage*> faces = retrieveFaces(detectedFaces, bboxes);

        // Start timing for benchmark training.

        QList<QPair<QImage*, QString> > faceSet;
        QList<QImage*>::iterator fsi;

        for (fsi = faces.begin() ; fsi != faces.end() ; ++fsi)
        {
            faceSet << QPair<QImage*, QString>(*fsi, QLatin1String("f00d"));
        }

        QElapsedTimer timer;
        timer.start();

        recognizer.train(identity, faceSet);

        elapsedTraining += timer.elapsed();
    }

    for (QMap<unsigned, QStringList>::const_iterator it = testset.constBegin() ;
         it != testset.constEnd() ; ++it)
    {
        Identity identity       = idMap.value(it.key());
        QList<QImage> rawImages = toImages(it.value());
        QStringList imagePaths  = it.value();

        QList<QImage> detectedFaces;
        QList<QRectF> bboxes;

        for (const QImage& image : std::as_const(rawImages))
        {
            QString imagePath = imagePaths.takeFirst();

            // Start timing for benchmark face detection.

            QElapsedTimer timer;
            timer.start();

            QList<QRectF> detectedBoundingBox = processFaceDetection(imagePath, detector);
            detectingTime                    += timer.elapsed();

            if (detectedBoundingBox.size())
            {
                detectedFaces << image;
                bboxes        << detectedBoundingBox.first();
                ++totalRecognized;
            }

            else
            {
                undetectedTestedFaces << imagePath;
            }

            imagePaths << imagePath;
        }

        QList<QImage*> faces = retrieveFaces(detectedFaces, bboxes);

        // Start timing for benchmark testing.

        QElapsedTimer timer;
        timer.start();

        QList<Identity> results = recognizer.recognizeFaces(faces);
        elapsedTesting         += timer.elapsed();

        /*
                qCDebug(DIGIKAM_TESTS_LOG) << "Result for " << it.value().first()
                                           << " is identity " << results.first().id();
        */
        for (const Identity& foundId : std::as_const(results))
        {
            QString imagePath = imagePaths.takeFirst();

            if (foundId.isNull())
            {
                ++notRecognized;
            }

            else if (foundId == identity)
            {
                ++correct;
            }

            else
            {
                ++falsePositive;
                falsePositiveFaces << QString::fromLatin1("Image at %1 with identity %2")
                                   .arg(imagePath)
                                   .arg(foundId.id());
            }
        }

        /*
                totalRecognized += images.size();
        */
    }

    unsigned nbUndetectedTrainedFaces = undetectedTrainedFaces.size();

    qCDebug(DIGIKAM_TESTS_LOG) << "\n" << nbUndetectedTrainedFaces << "/" << totalTrained + nbUndetectedTrainedFaces
                               << "(" << float(nbUndetectedTrainedFaces) / (totalTrained + nbUndetectedTrainedFaces) * 100
                               << "%)" << "faces cannot be detected for training";

    for (const QString& path : std::as_const(undetectedTrainedFaces))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << path;
    }

    if (totalTrained)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Training" << totalTrained << "of" << nbOfIdentities
                                   << "different objects took" << elapsedTraining << "ms,"
                                   << ((float)elapsedTraining / totalTrained) << "ms per image";
    }

    unsigned nbUndetectedTestedFaces = undetectedTestedFaces.size();

    qCDebug(DIGIKAM_TESTS_LOG) << "\n" << nbUndetectedTestedFaces << "/" << totalRecognized + nbUndetectedTestedFaces
                               << "(" << float(nbUndetectedTestedFaces) / (totalRecognized + nbUndetectedTestedFaces) * 100
                               << "%)" << "faces cannot be detected for testing";

    for (const QString& path : std::as_const(undetectedTestedFaces))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << path;
    }

    if (totalRecognized)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Recognition test performed on" << totalRecognized << "of" << nbOfIdentities << "different objects took " << elapsedTesting << " ms, " << ((float)elapsedTesting / totalRecognized) << " ms per image";
        qCDebug(DIGIKAM_TESTS_LOG) << correct       << "/" << totalRecognized << "(" << (float(correct) / totalRecognized * 100) << "%) were correctly recognized";
        qCDebug(DIGIKAM_TESTS_LOG) << falsePositive << "/" << totalRecognized << "(" << (float(falsePositive) / totalRecognized * 100) << "%) were falsely assigned to an identity (false positive)";
        qCDebug(DIGIKAM_TESTS_LOG) << notRecognized << "/" << totalRecognized << "(" << (float(notRecognized) / totalRecognized * 100) << "%) were not recognized";
    }

    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "No face recognized";
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "\nFalse positive faces";

    for (const QString& path : std::as_const(falsePositiveFaces))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << path;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "\n Average time of face detection"
                               << detectingTime * 1.0 / (totalTrained + nbUndetectedTrainedFaces + totalRecognized + nbUndetectedTestedFaces)
                               << "ms";

    return 0;
}
