/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions task for metadata synchronizer.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatasynctask.h"

// Local includes

#include "collectionscanner.h"
#include "scancontroller.h"
#include "metadatahub.h"
#include "digikam_debug.h"
#include "maintenancedata.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetadataSyncTask::Private
{
public:

    Private() = default;

public:

    bool                                tagsOnly    = false;

    MetadataSynchronizer::SyncDirection direction   = MetadataSynchronizer::WriteFromDatabaseToFile;

    MaintenanceData*                    data        = nullptr;
};

// -------------------------------------------------------

MetadataSyncTask::MetadataSyncTask()
    : ActionJob(),
      d        (new Private)
{
}

MetadataSyncTask::~MetadataSyncTask()
{
    cancel();

    delete d;
}

void MetadataSyncTask::setTagsOnly(bool value)
{
    d->tagsOnly = value;
}

void MetadataSyncTask::setDirection(MetadataSynchronizer::SyncDirection dir)
{
    d->direction = dir;
}

void MetadataSyncTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void MetadataSyncTask::run()
{
    ActionJob::run();       // To customize thread name

    // While we have data (using this as check for non-null).

    while (d->data)
    {
        if (m_cancel)
        {
            return;
        }

        ItemInfo item = d->data->getItemInfo();

        // If the item is null, we are done.

        if (item.isNull())
        {
            break;
        }

        if (d->direction == MetadataSynchronizer::WriteFromDatabaseToFile)
        {
            MetadataHub hub;

            // Read in from database.

            hub.load(item);

            // Write out to file DMetadata.

            if (d->tagsOnly)
            {
                ScanController::FileMetadataWrite writeScope(item);
                writeScope.changed(hub.writeToMetadata(item, MetadataHub::WRITE_TAGS, true));
            }
            else
            {
                Q_EMIT signalRemovePending(item);

                ScanController::FileMetadataWrite writeScope(item);
                writeScope.changed(hub.writeToMetadata(item, MetadataHub::WRITE_ALL, true));
            }
        }
        else // MetadataSynchronizer::ReadFromFileToDatabase
        {
            CollectionScanner scanner;
            scanner.scanFile(item, CollectionScanner::CleanScan);
        }

        Q_EMIT signalFinished(item, QImage());
    }

    Q_EMIT signalDone();
}

} // namespace Digikam

#include "moc_metadatasynctask.cpp"
