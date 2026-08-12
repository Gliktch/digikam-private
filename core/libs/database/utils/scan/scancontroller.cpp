/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "scancontroller_p.h"

namespace Digikam
{

ScanController::FileMetadataWrite::FileMetadataWrite(const ItemInfo& info)
    : m_info(info)
{
    ScanController::instance()->beginFileMetadataWrite(info);
}

void ScanController::FileMetadataWrite::changed(bool wasChanged)
{
    m_changed = (m_changed || wasChanged);
}

ScanController::FileMetadataWrite::~FileMetadataWrite()
{
    ScanController::instance()->finishFileMetadataWrite(m_info, m_changed);
}

// ----------------------------------------------------------------------------

class Q_DECL_HIDDEN ScanControllerCreator
{
public:

    ScanController object;
};

Q_GLOBAL_STATIC(ScanControllerCreator, creator)

// ----------------------------------------------------------------------------

ScanController::ScanController()
    : d(new Private)
{
    // create event loop

    d->eventLoop = new QEventLoop(this);

    connect(this, SIGNAL(databaseInitialized(bool)),
            d->eventLoop, SLOT(quit()));

    connect(this, SIGNAL(completeScanDone()),
            d->eventLoop, SLOT(quit()));

    connect(this, SIGNAL(completeScanCanceled()),
            d->eventLoop, SLOT(quit()));

    // create timer to show progress dialog

    d->showTimer = new QTimer(this);
    d->showTimer->setSingleShot(true);

    connect(d->showTimer, &QTimer::timeout,
            this, &ScanController::slotShowProgressDialog);

    connect(this, &ScanController::triggerShowProgressDialog,
            this, &ScanController::slotTriggerShowProgressDialog);

    // create timer for relaxed scheduling

    d->relaxedTimer = new QTimer(this);
    d->relaxedTimer->setSingleShot(true);
    d->relaxedTimer->setInterval(250);

    connect(d->relaxedTimer, &QTimer::timeout,
            this, &ScanController::slotRelaxedScanning);

    // create timer for external scheduling

    d->externalTimer = new QTimer(this);
    d->externalTimer->setSingleShot(true);
    d->externalTimer->setInterval(1500);

    connect(d->externalTimer, &QTimer::timeout,
            this, &ScanController::slotRelaxedScanning);

    // inter-thread connections

    connect(this, &ScanController::errorFromInitialization,
            this, &ScanController::slotErrorFromInitialization);

    connect(this, &ScanController::progressFromInitialization,
            this, &ScanController::slotProgressFromInitialization);

    // start thread

    d->running = true;
    start();
}

ScanController::~ScanController()
{
    shutDown();

    delete d->progressDialog;
    delete d->hints;

    delete d;
}

ScanController* ScanController::instance()
{
    return &creator->object;
}

void ScanController::setInitializationMessage()
{
    QString message = i18nc("@info", "Initializing database...");

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->restartPixmap(), message);
    }
}

/**
 * Implementing InitializationObserver
 */
bool ScanController::continueQuery()
{
    // not from main thread

    return d->continueInitialization;
}

void ScanController::createProgressDialog()
{
    if (d->progressDialog)
    {
        return;
    }

    d->progressDialog = new DProgressDlg(nullptr);
    d->progressDialog->setLabel(i18nc("@label", "Scanning collections, please wait..."));
    d->progressDialog->setWhatsThis(i18nc("@info",
                                          "This shows the progress of the scan. "
                                          "During the scan, all files on disk "
                                          "are registered in a database. "
                                          "Note: this dialog can appear automatically "
                                          "if a previous scan of collections have not been fully completed."));

    d->progressDialog->setMaximum(1);
    d->progressDialog->setValue(0);

    connect(this, SIGNAL(incrementProgressDialog(int)),
            d->progressDialog, SLOT(incrementMaximum(int)));

    connect(d->progressDialog, SIGNAL(signalCancelPressed()),
            this, SLOT(slotCancelPressed()));
}

void ScanController::run()
{
    while (d->running)
    {
        bool doInit             = false;
        bool doScan             = false;
        bool doScanDeferred     = false;
        bool doFinishScan       = false;
        bool doPartialScan      = false;
        bool doUpdateUniqueHash = false;

        QString task;
        {
            QMutexLocker lock(&d->mutex);

            if      (d->needsInitialization)
            {
                d->needsInitialization = false;
                doInit                 = true;
            }
            else if (d->needsCompleteScan && !d->scanSuspended)
            {
                d->needsCompleteScan = false;
                doScan               = true;
                doScanDeferred       = d->deferFileScanning;
            }
            else if (d->needsUpdateUniqueHash && !d->scanSuspended)
            {
                d->needsUpdateUniqueHash = false;
                doUpdateUniqueHash       = true;
            }
            else if (!d->completeScanDeferredAlbums.isEmpty() && d->finishScanAllowed && !d->scanSuspended)
            {
                // d->completeScanDeferredAlbums is only accessed from the thread, no need to copy

                doFinishScan  = true;
            }
            else if (!d->scanTasks.isEmpty() && !d->scanSuspended)
            {
                doPartialScan = true;
                task          = d->scanTasks.takeFirst();
            }
            else
            {
                d->idle = true;
                d->condVar.wait(&d->mutex);
                d->idle = false;
            }
        }

        if      (doInit)
        {
            d->continueInitialization = true;

            // pass "this" as InitializationObserver

            bool success              = CoreDbAccess::checkReadyForUse(this);

            // If d->advice has not been adjusted to a value indicating failure, do this here

            if (!success && (d->advice == Success))
            {
                d->advice = ContinueWithoutDatabase;
            }

            Q_EMIT databaseInitialized(success);
        }
        else if (doScan)
        {
            CollectionScanner scanner;
            connectCollectionScanner(&scanner);

            scanner.setNeedFileCount(d->needTotalFiles);
            scanner.setPerformFastScan(d->performFastScan);
            scanner.setDeferredFileScanning(doScanDeferred);
            scanner.setHintContainer(d->hints);

            SimpleCollectionScannerObserver observer(&d->continueScan);
            scanner.setObserver(&observer);

            scanner.completeScan();

            if (doScanDeferred)
            {
                d->completeScanDeferredAlbums = scanner.deferredAlbumPaths();
                d->finishScanAllowed          = false;
            }

            d->newIdsList = scanner.getNewIdsList();

            Q_EMIT completeScanDone();

        }
        else if (doFinishScan)
        {
            if (d->completeScanDeferredAlbums.isEmpty())
            {
                continue;
            }

            CollectionScanner scanner;
            connectCollectionScanner(&scanner);

            Q_EMIT collectionScanStarted(i18nc("@info:status", "Scanning collection"));

            //TODO: reconsider performance

            scanner.setNeedFileCount(true);//d->needTotalFiles);

            scanner.setHintContainer(d->hints);

            SimpleCollectionScannerObserver observer(&d->continueScan);
            scanner.setObserver(&observer);

            scanner.finishCompleteScan(d->completeScanDeferredAlbums);

            d->completeScanDeferredAlbums.clear();

            Q_EMIT completeScanDone();
            Q_EMIT collectionScanFinished();
        }
        else if (doPartialScan)
        {
            CollectionScanner scanner;
            scanner.setHintContainer(d->hints);
            connectCollectionScanner(&scanner);

            SimpleCollectionScannerObserver observer(&d->continuePartialScan);
            scanner.setObserver(&observer);

            scanner.partialScan(task);

            Q_EMIT partialScanDone(task);
        }
        else if (doUpdateUniqueHash)
        {
            CoreDbAccess access;
            CoreDbSchemaUpdater updater(access.db(), access.backend(), access.parameters());
            updater.setCoreDbAccess(&access);
            updater.setObserver(this);
            updater.updateUniqueHash();

            Q_EMIT completeScanDone();
        }
    }
}

/**
 * Also implementing InitializationObserver.
 */
void ScanController::connectCollectionScanner(CollectionScanner* const scanner)
{
    scanner->setSignalsEnabled(true);

    connect(scanner, SIGNAL(startCompleteScan()),
            this, SLOT(slotStartCompleteScan()));

    connect(scanner, SIGNAL(totalFilesToScan(int)),
            this, SLOT(slotTotalFilesToScan(int)));

    connect(scanner, SIGNAL(partialFilesToScan(int)),
            this, SIGNAL(totalFilesToScan(int)));

    connect(scanner, SIGNAL(startScanningAlbum(QString,QString)),
            this, SLOT(slotStartScanningAlbum(QString,QString)));

    connect(scanner, SIGNAL(scannedFiles(int)),
            this, SLOT(slotScannedFiles(int)));

    connect(scanner, SIGNAL(startScanningAlbumRoot(QString)),
            this, SLOT(slotStartScanningAlbumRoot(QString)));

    connect(scanner, SIGNAL(startScanningForStaleAlbums()),
            this, SLOT(slotStartScanningForStaleAlbums()));

    connect(scanner, SIGNAL(startScanningAlbumRoots()),
            this, SLOT(slotStartScanningAlbumRoots()));
}

void ScanController::allowToScanDeferredFiles()
{
    QMutexLocker lock(&d->mutex);
    d->finishScanAllowed = true;
    d->condVar.wakeAll();
}

void ScanController::updateUniqueHash()
{
    createProgressDialog();

    // we only need to count the files in advance
    // if we show a progress percentage in progress dialog

    d->needTotalFiles = true;

    {
        QMutexLocker lock(&d->mutex);
        d->needsUpdateUniqueHash = true;
        d->condVar.wakeAll();
    }

    // NOTE: loop is quit by signal

    d->eventLoop->exec();

    delete d->progressDialog;
    d->progressDialog = nullptr;
    d->needTotalFiles = false;
}

ItemInfo ScanController::scannedInfo(const QString& filePath,
                                     CollectionScanner::FileScanMode mode)
{
    CollectionScanner scanner;
    scanner.setHintContainer(d->hints);

    ItemInfo info = ItemInfo::fromLocalFile(filePath);

    if (info.isNull() || !info.isVisible())
    {
        qlonglong id = scanner.scanFile(filePath, CollectionScanner::NormalScan);

        return ItemInfo(id);
    }
    else
    {
        scanner.scanFile(info, mode);

        return info;
    }
}

QList<qlonglong> ScanController::getNewIdsList() const
{
    return d->newIdsList;
}

} // namespace Digikam

#include "moc_scancontroller.cpp"
