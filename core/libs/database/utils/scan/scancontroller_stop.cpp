/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - stop operations.
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

void ScanController::slotCancelPressed()
{
    abortInitialization();
    cancelCompleteScan();
}

void ScanController::abortInitialization()
{
    QMutexLocker lock(&d->mutex);
    d->needsInitialization    = false;
    d->continueInitialization = false;
}

void ScanController::cancelCompleteScan()
{
    QMutexLocker lock(&d->mutex);
    d->needsCompleteScan = false;
    d->continueScan      = false;
    Q_EMIT completeScanCanceled();
}

void ScanController::cancelAllAndSuspendCollectionScan()
{
    QMutexLocker lock(&d->mutex);

    d->needsInitialization    = false;
    d->continueInitialization = false;

    d->needsCompleteScan      = false;
    d->continueScan           = false;

    d->scanTasks.clear();
    d->continuePartialScan    = false;

    d->relaxedTimer->stop();

    // like suspendCollectionScan

    d->scanSuspended++;

    while (!d->idle)
    {
        d->condVar.wait(&d->mutex, 20);
    }
}

void ScanController::suspendCollectionScan()
{
    QMutexLocker lock(&d->mutex);
    d->scanSuspended++;
}

/// implementing InitializationObserver
void ScanController::finishedSchemaUpdate(UpdateResult result)
{
    // not from main thread

    switch (result)
    {
        case InitializationObserver::UpdateSuccess:
        {
            d->advice = Success;
            break;
        }

        case InitializationObserver::UpdateError:
        {
            d->advice = ContinueWithoutDatabase;
            break;
        }

        case InitializationObserver::UpdateErrorMustAbort:
        {
            d->advice = AbortImmediately;
            break;
        }
    }
}

void ScanController::finishFileMetadataWrite(const ItemInfo& info, bool changed)
{
    QFileInfo fi(info.filePath());
    d->hints->recordHint(ItemMetadataAdjustmentHint(info.id(),
                                                    changed ? ItemMetadataAdjustmentHint::MetadataEditingFinished :
                                                              ItemMetadataAdjustmentHint::MetadataEditingAborted,
                                                    fi.lastModified(),
                                                    fi.size()));

    scanFileDirectlyNormal(info);
}

void ScanController::shutDown()
{
    if (!isRunning())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);

        d->running                = false;
        d->continueInitialization = false;
        d->continueScan           = false;
        d->continuePartialScan    = false;

        d->condVar.wakeAll();
    }

    wait();
}

void ScanController::slotRelaxedScanning()
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Starting scan!";
    d->externalTimer->stop();
    d->relaxedTimer->stop();

    QMutexLocker lock(&d->mutex);
    d->condVar.wakeAll();
}

} // namespace Digikam
