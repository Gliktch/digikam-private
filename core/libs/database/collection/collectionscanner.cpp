/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionscanner_p.h"

namespace Digikam
{

CollectionScanner::CollectionScanner()
    : d(new Private)
{
}

CollectionScanner::~CollectionScanner()
{
    delete d;
}

void CollectionScanner::setSignalsEnabled(bool on)
{
    d->wantSignals = on;
}

void CollectionScanner::setNeedFileCount(bool on)
{
    d->needTotalFiles = on;
}

void CollectionScanner::setPerformFastScan(bool on)
{
    d->performFastScan = on;
}

CollectionScannerHintContainer* CollectionScanner::createHintContainer()
{
    return (new CollectionScannerHintContainerImplementation);
}

void CollectionScanner::setHintContainer(CollectionScannerHintContainer* const container)
{
    // the API specs require the object given here to be created by createContainer, so we can cast.

    d->hints = static_cast<CollectionScannerHintContainerImplementation*>(container);
}

void CollectionScanner::setUpdateHashHint(bool hint)
{
    d->updatingHashHint = hint;
}

void CollectionScanner::setObserver(CollectionScannerObserver* const observer)
{
    d->observer = observer;
}

void CollectionScanner::setDeferredFileScanning(bool defer)
{
    d->deferredFileScanning = defer;
}

QStringList CollectionScanner::deferredAlbumPaths() const
{
    return d->deferredAlbumPaths.values();
}

QList<qlonglong> CollectionScanner::getNewIdsList() const
{
    return d->newIdsList;
}

} // namespace Digikam

#include "moc_collectionscanner.cpp"
