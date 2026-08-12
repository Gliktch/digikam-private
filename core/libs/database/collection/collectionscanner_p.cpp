/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database - private containers.
 *
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

bool s_modificationDateEquals(const QDateTime& a, const QDateTime& b)
{
    if (!a.isValid() || !b.isValid())
    {
        return false;
    }

    if (a != b)
    {
        // allow a "modify window" of one second.
        // FAT filesystems store the modify date in 2-second resolution.

        int diff = a.secsTo(b);

        if (abs(diff) > 1)
        {
            return false;
        }
    }

    return true;
}

// --------------------------------------------------------------------

bool CollectionScannerHintContainerImplementation::hasMetadataHint(qlonglong id)
{
    QReadLocker locker(&lock);

    return (
            metadataAboutToAdjustHints.contains(id) ||
            metadataAdjustedHints.contains(id)
           );
}

bool CollectionScannerHintContainerImplementation::hasMetadataAboutToAdjustHint(qlonglong id)
{
    QReadLocker locker(&lock);

    return metadataAboutToAdjustHints.contains(id);
}

bool CollectionScannerHintContainerImplementation::hasMetadataAdjustedHint(qlonglong id)
{
    QReadLocker locker(&lock);

    return metadataAdjustedHints.contains(id);
}

void CollectionScannerHintContainerImplementation::recordHint(const ItemMetadataAdjustmentHint& hint)
{
    if (hint.isAboutToEdit())
    {
        ItemInfo info(hint.id());

        if (!
            (s_modificationDateEquals(hint.modificationDate(), info.modDateTime()) &&
             (hint.fileSize() == info.fileSize())
            )
           )
        {
            // refuse to create a hint as a rescan is required already before any metadata edit
            // or, in case of multiple edits, there is already a hint with an older date, then all is fine.

            return;
        }

        QWriteLocker locker(&lock);
        metadataAboutToAdjustHints[hint.id()] = hint.modificationDate();
    }
    else if (hint.isEditingFinished())
    {
        QWriteLocker locker(&lock);
        QHash<qlonglong, QDateTime>::iterator it = metadataAboutToAdjustHints.find(hint.id());

        if (it == metadataAboutToAdjustHints.end())
        {
            return;
        }

        metadataAboutToAdjustHints.erase(it);

        metadataAdjustedHints[hint.id()] = hint.modificationDate();
    }
    else // Aborted
    {
         QWriteLocker locker(&lock);
         QDateTime formerDate = metadataAboutToAdjustHints.take(hint.id());
         Q_UNUSED(formerDate);
    }
}

void CollectionScannerHintContainerImplementation::clear()
{
    QWriteLocker locker(&lock);

    metadataAdjustedHints.clear();
    metadataAboutToAdjustHints.clear();
}

// --------------------------------------------------------------------

void CollectionScanner::Private::resetRemovedItemsTime()
{
    removedItemsTime = QDateTime();
}

void CollectionScanner::Private::removedItems()
{
    removedItemsTime = QDateTime::currentDateTime();
}

bool CollectionScanner::Private::checkObserver() const
{
    if (observer)
    {
        return observer->continueQuery();
    }

    return true;
}

bool CollectionScanner::Private::checkDeferred(const QFileInfo& info)
{
    if (deferredFileScanning)
    {
        deferredAlbumPaths << info.path();

        return true;
    }

    return false;
}

bool CollectionScanner::Private::checkIgnoreDirectory(const QString& dirName) const
{
    for (const QString& ignore : std::as_const(ignoreDirectory))
    {
        QString wildcard = QRegularExpression::wildcardToRegularExpression(ignore);
        QRegularExpression regExp(QRegularExpression::anchoredPattern(wildcard));

        if (regExp.match(dirName).hasMatch())
        {
            return true;
        }
    }

    return false;
}

void CollectionScanner::Private::finishScanner(ItemScanner& scanner)
{
    /**
     * Perform the actual write operation to the database
     */
    {
        CoreDbOperationGroup group;
        scanner.commit();
    }

    if (recordHistoryIds && scanner.hasHistoryToResolve())
    {
        needResolveHistorySet << scanner.id();
    }
}

} // namespace Digikam
