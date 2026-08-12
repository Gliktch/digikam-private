/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Pankaj Kumar <me at panks dot me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "actionthread.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "collectionscanner.h"
#include "scancontroller.h"
#include "metadatahub.h"
#include "actiontask.h"

namespace Digikam
{

class Q_DECL_HIDDEN ActionThread::Private
{
public:

    Private() = default;

    QueueSettings settings;
};

// --------------------------------------------------------------------------------------

ActionThread::ActionThread(QObject* const parent)
    : ActionThreadBase(parent),
      d               (new Private)
{
    setObjectName(QLatin1String("QueueMngrThread"));
    qRegisterMetaType<ActionData>("ActionData");

    connect(this, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));
}

ActionThread::~ActionThread()
{
    this->cancel();

    wait();

    delete d;
}

void ActionThread::setSettings(const QueueSettings& settings)
{
    d->settings = settings;

    if (!d->settings.useMultiCoreCPU)
    {
        setMaximumNumberOfThreads(1);
    }
    else
    {
        setDefaultMaximumNumberOfThreads();
    }
}

void ActionThread::processQueueItems(const QList<AssignedBatchTools>& items)
{
    ActionJobCollection collection;

    for (int i = 0 ; i < items.size() ; ++i)
    {
        ActionTask* const t = new ActionTask();
        t->setSettings(d->settings);
        t->setItem(items.at(i));

        connect(t, SIGNAL(signalStarting(Digikam::ActionData)),
                this, SIGNAL(signalStarting(Digikam::ActionData)));

        connect(t, SIGNAL(signalFinished(Digikam::ActionData)),
                this, SLOT(slotUpdateItemInfo(Digikam::ActionData)),
                Qt::QueuedConnection);

        connect(this, SIGNAL(signalCancelActionTask()),
                t, SLOT(slotCancel()),
                Qt::QueuedConnection);

        collection.insert(t, 0);
    }

    appendJobs(collection);
}

void ActionThread::cancel()
{
    if (isRunning())
    {
        Q_EMIT signalCancelActionTask();
    }

    ActionThreadBase::cancel();
}

void ActionThread::slotUpdateItemInfo(const Digikam::ActionData& ad)
{
    if (ad.status == ActionData::BatchDone)
    {
        ItemInfo info;
        CollectionScanner scanner;
        ItemInfo source = ItemInfo::fromUrl(ad.fileUrl);

        if (ad.fileUrl == ad.destUrl)
        {
            info = ItemInfo(scanner.scanFile(ad.destUrl.toLocalFile(),
                                             CollectionScanner::Rescan));
        }
        else
        {
            info = ItemInfo(scanner.scanFile(ad.destUrl.toLocalFile(),
                                             CollectionScanner::NormalScan));
        }

        // Copy the digiKam attributes from original file to the new file

        CollectionScanner::copyFileProperties(source, info);

        if (!ad.noWrite)
        {
            // Write digiKam metadata to the new file

            MetadataHub hub;
            hub.load(info);

            ScanController::FileMetadataWrite writeScope(info);
            writeScope.changed(hub.writeToMetadata(info, MetadataHub::WRITE_ALL, true));
        }
        else
        {
            scanner.scanFile(info, CollectionScanner::CleanScan);
        }
    }

    Q_EMIT signalFinished(ad);
}

void ActionThread::slotThreadFinished()
{
    if (isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "List of Pending Jobs is empty";

        Q_EMIT signalQueueProcessed();
    }
}

} // namespace Digikam

#include "moc_actionthread.cpp"
