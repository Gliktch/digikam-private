/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-16
 * Description : Face Recognition CLI tool
 *               NOTE: This tool is able to use ORL database which are
 *               freely available set of images to test face recognition.
 *               It contain 10 photos of 20 different peoples from slightly
 *               different angles. See here for details:
 *               www.cl.cam.ac.uk/research/dtg/attarchive/facedatabase.html
 *
 * SPDX-FileCopyrightText: 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QElapsedTimer>
#include <QPair>

// Local includes

#include "digikam_debug.h"
#include "facialrecognition_wrapper.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;

QStringList toPaths(char** argv, int startIndex, int argc)
{
    QStringList files;

    for (int i = startIndex ; i < argc ; ++i)
    {
        files << QString::fromLocal8Bit(argv[i]);
    }

    return files;
}

QList<QImage*> toImages(const QStringList& paths)
{
    QList<QImage*> images;

    for (const QString& path : std::as_const(paths))
    {
        images << new QImage(path);
    }

    return images;
}

// --------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if ((argc < 2) || ((QString::fromLatin1(argv[1]) == QString::fromLatin1("train")) && (argc < 3)))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Bad Arguments!!!\nUsage: " << argv[0]
                                   << "identify <image1> <image2> ... | train name <image1> <image2> ..."
                                   "| ORL <path to orl_faces>";
        return 0;
    }

    QCoreApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.
    DbEngineParameters prm    = DbEngineParameters::parametersFromConfig();
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);
    FacialRecognitionWrapper recognizer;

    if (QString::fromLatin1(argv[1]) == QString::fromLatin1("identify"))
    {
        QStringList paths     = toPaths(argv, 2, argc);
        QList<QImage*> images = toImages(paths);

        QElapsedTimer timer;
        timer.start();
        QList<Identity> identities = recognizer.recognizeFaces(images);
        int elapsed                = timer.elapsed();

        qCDebug(DIGIKAM_TESTS_LOG) << "Recognition took" << elapsed
                                   << " for " << images.size() << ", "
                                   << ((float)elapsed / images.size()) << " per image";

        for (int i = 0 ; i < paths.size() ; ++i)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Identified " << identities[i].attribute(QString::fromLatin1("name"))
                                       << " in " << paths[i];
        }
    }

    else if (QString::fromLatin1(argv[1]) == QString::fromLatin1("train"))
    {
        QString name = QString::fromLocal8Bit(argv[2]);
        qCDebug(DIGIKAM_TESTS_LOG) << "Training " << name;

        QStringList paths     = toPaths(argv, 3, argc);
        QList<QImage*> images = toImages(paths);
        Identity identity     = recognizer.findIdentity(QString::fromLatin1("name"), name);

        if (identity.isNull())
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Adding new identity to database for name" << name;
            QMultiMap<QString, QString> attributes;
            attributes.insert(QString::fromLatin1("name"), name);
            identity                                = recognizer.addIdentity(attributes);
        }

        QElapsedTimer timer;
        timer.start();

        QList<QPair<QImage*, QString> > imageSet;
        QList<QImage*>::iterator isi;

        for (isi = images.begin() ; isi != images.end() ; ++isi)
        {
            imageSet << QPair<QImage*, QString>(*isi, QLatin1String("f00d"));
        }

        recognizer.train(identity, imageSet);

        int elapsed = timer.elapsed();

        qCDebug(DIGIKAM_TESTS_LOG) << "Training took" << elapsed << "for"
                                   << images.size() << ","
                                   << ((float)elapsed / images.size()) << "per image";
    }

    else if (QString::fromLatin1(argv[1]) == QString::fromLatin1("orl"))
    {
        QString orlPath = QString::fromLocal8Bit(argv[2]);

        if (orlPath.isEmpty())
        {
            orlPath = QString::fromLatin1("orl_faces"); // Relative to current dir.
        }

        QDir orlDir(orlPath);

        if (!orlDir.exists())
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Cannot find orl_faces directory";

            return 0;
        }

        const int OrlIdentities       = 40;
        const int OrlSamples          = 10;
        const QString trainingContext = QString::fromLatin1("test application");

        QMap<int, Identity> idMap;
        QList<Identity> trainingToBeCleared;

        for (int i = 1 ; i <= OrlIdentities ; ++i)
        {
            QMultiMap<QString, QString> attributes;
            attributes.insert(QString::fromLatin1("name"), QString::number(i));
            Identity identity                       = recognizer.findIdentity(attributes);

            if (identity.isNull())
            {
                Identity identity2 = recognizer.addIdentity(attributes);
                idMap[i]           = identity2;
                qCDebug(DIGIKAM_TESTS_LOG) << "Created identity" << identity2.id() << "for ORL directory" << i;
            }

            else
            {
                qCDebug(DIGIKAM_TESTS_LOG) << "Already have identity for ORL directory" << i << ", clearing training data";
                idMap[i] = identity;
                trainingToBeCleared << identity;
            }
        }

        recognizer.clearTraining(trainingToBeCleared);
        QMap<int, QStringList> trainingImages, recognitionImages;

        for (int i = 1 ; i <= OrlIdentities ; ++i)
        {
            for (int j = 1 ; j <= OrlSamples ; ++j)
            {
                QString path = orlDir.path() + QString::fromLatin1("/s%1/%2.pgm").arg(i).arg(j);

                if (j <= OrlSamples / 2)
                {
                    trainingImages[i] << path;
                }

                else
                {
                    recognitionImages[i] << path;
                }
            }
        }

        if (!QFileInfo::exists(trainingImages.value(1).constFirst()))
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Could not find files of ORL database";

            return 0;
        }

        QElapsedTimer timer;
        timer.start();

        int correct         = 0;
        int notRecognized   = 0;
        int falsePositive   = 0;
        int totalTrained    = 0;
        int totalRecognized = 0;
        int elapsed         = 0;

        for (QMap<int, QStringList>::const_iterator it = trainingImages.constBegin() ;
             it != trainingImages.constEnd() ; ++it)
        {
            Identity identity = recognizer.findIdentity(QString::fromLatin1("name"), QString::number(it.key()));

            if (identity.isNull())
            {
                qCDebug(DIGIKAM_TESTS_LOG) << "Identity management failed for ORL person" << it.key();
            }

            QList<QImage*> images = toImages(it.value());
            qCDebug(DIGIKAM_TESTS_LOG) << "Training ORL directory" << it.key();

            QList<QPair<QImage*, QString> > imageSet;
            QList<QImage*>::iterator isi;

            for (isi = images.begin() ; isi != images.end() ; ++isi)
            {
                imageSet << QPair<QImage*, QString>(*isi, QLatin1String("f00d"));
            }

            recognizer.train(identity, imageSet);
            totalTrained        += images.size();
        }

        elapsed = timer.restart();

        if (totalTrained)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Training 5/10 or ORL took" << elapsed
                                       << "ms," << ((float)elapsed / totalTrained)
                                       << "ms per image";
        }

        for (QMap<int, QStringList>::const_iterator it = recognitionImages.constBegin() ;
             it != recognitionImages.constEnd() ; ++it)
        {
            Identity identity        = idMap.value(it.key());
            QList<QImage*> images    = toImages(it.value());
            QList<Identity> results  = recognizer.recognizeFaces(images);

            qCDebug(DIGIKAM_TESTS_LOG) << "Result for" << it.value().first()
                                       << "is identity" << results.first().id();

            for (const Identity& foundId : std::as_const(results))
            {
                if (foundId.isNull())
                {
                    notRecognized++;
                }

                else if (foundId == identity)
                {
                    correct++;
                }

                else
                {
                    falsePositive++;
                }
            }

            totalRecognized += images.size();
        }

        elapsed = timer.elapsed();

        if (totalRecognized)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Recognition of 5/10 or ORL took" << elapsed << "ms," << ((float)elapsed / totalRecognized) << "ms per image";
            qCDebug(DIGIKAM_TESTS_LOG) << correct       << "of 200 (" << (float(correct)       / totalRecognized * 100) << "%) were correctly recognized";
            qCDebug(DIGIKAM_TESTS_LOG) << falsePositive << "of 200 (" << (float(falsePositive) / totalRecognized * 100) << "%) were falsely assigned to an identity";
            qCDebug(DIGIKAM_TESTS_LOG) << notRecognized << "of 200 (" << (float(notRecognized) / totalRecognized * 100) << "%) were not recognized";
        }

        else
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "No face recognized";
        }
    }

    return 0;
}
