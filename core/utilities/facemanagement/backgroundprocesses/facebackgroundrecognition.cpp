/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-03-30
 * Description : Face Background Recognition Controller
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facebackgroundrecognition.h"

// Qt includes

#include <QTimer>
#include <QThread>          // For QThread::msleep
#include <QException>
#include <QCoreApplication> // For QCoreApplication::processEvents

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "facescansettings.h"
#include "facesengine.h"
#include "albummanager.h"
#include "faceclassifier.h"

namespace Digikam
{

class Q_DECL_HIDDEN FaceRecognitionBackgroundController::Private
{
public:

    FacesEngine*        facesEngine             = nullptr;
    bool                firstRun                = true;
    bool                runAgain                = false;
    FaceScanSettings    settings;
};

class Q_DECL_HIDDEN FaceRecognitionBackgroundControllerCreator
{
public:

    FaceRecognitionBackgroundController object;
};

Q_GLOBAL_STATIC(FaceRecognitionBackgroundControllerCreator, creator)

// -----------------------------------------------------------------

FaceRecognitionBackgroundController::FaceRecognitionBackgroundController()
    : QObject(),
      d      (new Private)
{
    bool enabled = ApplicationSettings::instance()->getFaceRecognitionBackgroundScan();

    // set the connection to the FaceClassifier

    slotSetEnabled(enabled);

    if (!enabled)
    {
        // If the background scan is disabled, we need to set the firstRun flag to false
        // The firstRun flag is used to avoid running the scan when the FaceClassifier starts
        // but we want to run it when the user enables the background scan and not skip the
        // first notification

        d->firstRun = false;
    }
}

FaceRecognitionBackgroundController::~FaceRecognitionBackgroundController()
{
    if (d->facesEngine)
    {
        d->facesEngine->cancel();
        waitForDone();
    }

    delete d;
}

FaceRecognitionBackgroundController* FaceRecognitionBackgroundController::instance()
{
    return &creator->object;
}

void FaceRecognitionBackgroundController::slotSetEnabled(bool enabled)
{
    if (enabled)
    {
        connect(FaceClassifier::instance(), &FaceClassifier::signalTrainingComplete,
                this, &FaceRecognitionBackgroundController::slotRescan);
    }
    else
    {
        disconnect(FaceClassifier::instance(), &FaceClassifier::signalTrainingComplete,
                   this, &FaceRecognitionBackgroundController::slotRescan);
    }
}

bool FaceRecognitionBackgroundController::slotRescan()
{
    // check if background scan already running

    if (d->facesEngine)
    {
        d->runAgain = true;
        return false;
    }

    d->settings.readFromConfig();

    // the background process is recognizing faces only

    d->settings.task                   = FaceScanSettings::RecognizeMarkedFaces;
    d->settings.alreadyScannedHandling = FaceScanSettings::RecognizeOnly;

    // use only a single thread to avoid overloading the CPU

    d->settings.useFullCpu             = false;

    // scan all albums

    d->settings.wholeAlbums            = true;
    d->settings.albums                 = AlbumManager::instance()->allPAlbums();

    d->settings.infos.clear();

    // set the thread priority to idle to avoid slowing down the UI

    d->settings.workerThreadPriority   = QThread::IdlePriority;

    // set the source to background recognition

    d->settings.source                 = FaceScanSettings::BackgroundRecognition;

    // don't run the first time this function is called
    // this is to avoid running the scan when the application starts

    if (d->firstRun)
    {
        d->firstRun = false;
        return false;
    }

    // clear the runAgain flag

    d->runAgain = false;

    try
    {
        // create the faces engine

        d->facesEngine = new FacesEngine(d->settings, nullptr);

        // connect signals and slots

        connect(d->facesEngine, &FacesEngine::signalComplete,
                this, &FaceRecognitionBackgroundController::slotScanDone);

/*
        connect(facesDetector, SIGNAL(signalComplete()),
                d->parentInstance, SLOT(slotScanComplete()));

        connect(facesDetector, SIGNAL(signalCanceled()),
                d->parentInstance, SLOT(slotScanComplete()));

        connect(facesDetector, SIGNAL(signalScanNotification(QString,int)),
                d->parentInstance, SIGNAL(signalNotificationError(QString,int)));
*/
        // start the faces engine

        d->facesEngine->start();

        return true;
    }

    catch(const QException& e)
    {
        Q_UNUSED(e);
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceRecognitionBackgroundController::start: exception";
    }

    catch(const std::exception& e)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceRecognitionBackgroundController::start: exception: " << e.what();
    }

    catch(...)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "FaceRecognitionBackgroundController::start: exception";
    }

    d->facesEngine = nullptr;

    return false;
}

void FaceRecognitionBackgroundController::stop()
{
    // stop the faces engine if it is running

    d->runAgain = false;

    if (d->facesEngine)
    {
        d->facesEngine->cancel();
    }
}

void FaceRecognitionBackgroundController::slotScanDone()
{
    // clean up the faces engine

    if (d->facesEngine)
    {
        // d->faceEngine is deleted automatically, just set the pointer to null

        d->facesEngine = nullptr;
    }

    // if we need to run again, start the faces engine again

    if (d->runAgain)
    {
        d->runAgain = false;
        QTimer::singleShot(100, this, &FaceRecognitionBackgroundController::slotRescan);
    }
}

bool FaceRecognitionBackgroundController::waitForDone()
{
    // wait for the faces engine to finish

    while (d->facesEngine)
    {
        // Allow Qt to process events to keep the application responsive

        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        QThread::msleep(10); // Avoid busy-waiting by adding a small delay
    }

    return true;
}

} // namespace Digikam

#include "moc_facebackgroundrecognition.cpp"
