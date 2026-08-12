/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - progress operations.
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

void ScanController::slotTotalFilesToScan(int count)
{
    if (d->progressDialog)
    {
        d->progressDialog->incrementMaximum(count);
    }

    d->totalFilesToScan += count;

    Q_EMIT totalFilesToScan(d->totalFilesToScan);
}

void ScanController::slotScannedFiles(int scanned)
{
    if (d->progressDialog)
    {
        d->progressDialog->advance(scanned);
    }

    if (d->totalFilesToScan)
    {
        Q_EMIT filesScanned(scanned);
        Q_EMIT scanningProgress(double(scanned) / double(d->totalFilesToScan));
    }
}

/**
 * Implementing InitializationObserver.
 */
void ScanController::moreSchemaUpdateSteps(int numberOfSteps)
{
    // not from main thread

    Q_EMIT triggerShowProgressDialog();
    Q_EMIT incrementProgressDialog(numberOfSteps);
}

/**
 * Implementing InitializationObserver.
 */
void ScanController::schemaUpdateProgress(const QString& message, int numberOfSteps)
{
    // not from main thread

    Q_EMIT progressFromInitialization(message, numberOfSteps);
}

void ScanController::slotProgressFromInitialization(const QString& message, int numberOfSteps)
{
    // main thread

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->actionPixmap(), message);
        d->progressDialog->advance(numberOfSteps);
    }
}

/**
 * Implementing InitializationObserver.
 */
void ScanController::error(const QString& errorMessage)
{
    // not from main thread

    Q_EMIT errorFromInitialization(errorMessage);
}

void ScanController::slotErrorFromInitialization(const QString& errorMessage)
{
    // main thread

    QString message = i18n("Error");

    if (d->progressDialog)
    {
        d->progressDialog->addedAction(d->errorPixmap(), message);
    }

    QMessageBox::critical(d->progressDialog, qApp->applicationName(), errorMessage);
}

void ScanController::slotShowProgressDialog()
{
    if (d->progressDialog)
    {
/*
        if (!CollectionScanner::databaseInitialScanDone())
*/
        {
            d->progressDialog->show();
        }
    }
}

void ScanController::slotTriggerShowProgressDialog()
{
    if (d->progressDialog && !d->showTimer->isActive() && !d->progressDialog->isVisible())
    {
        d->showTimer->start(300);
    }
}

} // namespace Digikam
