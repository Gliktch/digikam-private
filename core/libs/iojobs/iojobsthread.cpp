/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs thread for file system jobs
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2018 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iojobsthread.h"

// Qt includes

#include <QFileInfo>
#include <QDir>

// Local includes

#include "iojob.h"
#include "iojobdata.h"
#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

class Q_DECL_HIDDEN IOJobsThread::Private
{

public:

    Private() = default;

    int         jobsCount   = 0;
    bool        isCanceled  = false;

    IOJobData*  jobData     = nullptr;

    QStringList errorsList;
};

IOJobsThread::IOJobsThread(QObject* const parent)
    : ActionThreadBase(parent),
      d               (new Private)
{
    setObjectName(QLatin1String("IOJobsThread"));
}

IOJobsThread::~IOJobsThread()
{
    delete d->jobData;
    delete d;
}

void IOJobsThread::copyOrMove(IOJobData* const data)
{
    d->jobData  = data;

    ActionJobCollection collection;

    int threads = qMin(maximumNumberOfThreads(),
                       data->sourceUrls().count());

    for (int i = 0 ; i < threads ; ++i)
    {
        CopyOrMoveJob* const j = new CopyOrMoveJob(data);

        connectOneJob(j);

        collection.insert(j, 0);
        d->jobsCount++;
    }

    appendJobs(collection);
}

void IOJobsThread::deleteFiles(IOJobData* const data)
{
    d->jobData = data;

    ActionJobCollection collection;

    int threads = qMin(maximumNumberOfThreads(),
                       data->sourceUrls().count());

    for (int i = 0 ; i < threads ; ++i)
    {
        DeleteJob* const j = new DeleteJob(data);

        connectOneJob(j);

        collection.insert(j, 0);
        d->jobsCount++;
    }

    appendJobs(collection);
}

void IOJobsThread::renameFile(IOJobData* const data)
{
    d->jobData             = data;
    ActionJobCollection collection;

    RenameFileJob* const j = new RenameFileJob(data);

    connectOneJob(j);

    connect(j, SIGNAL(signalRenameFailed(QUrl)),
            this, SIGNAL(signalRenameFailed(QUrl)));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::listDTrashItems(const QString& collectionPath)
{
    ActionJobCollection collection;

    DTrashItemsListingJob* const j = new DTrashItemsListingJob(collectionPath);

    connect(j, SIGNAL(trashItemInfo(DTrashItemInfo)),
            this, SIGNAL(collectionTrashItemInfo(DTrashItemInfo)));

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(signalFinished()));

    collection.insert(j, 0);

    appendJobs(collection);
}

void IOJobsThread::restoreDTrashItems(IOJobData* const data)
{
    d->jobData = data;
    ActionJobCollection collection;

    RestoreDTrashItemsJob* const j = new RestoreDTrashItemsJob(data);

    connectOneJob(j);

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::emptyDTrashItems(IOJobData* const data)
{
    d->jobData = data;
    ActionJobCollection collection;

    EmptyDTrashItemsJob* const j = new EmptyDTrashItemsJob(data);

    connectOneJob(j);

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::buildTrashCounters()
{
    ActionJobCollection collection;

    BuildTrashCountersJob* const j = new BuildTrashCountersJob();

    connect(j, SIGNAL(signalTrashCountersMap(QMap<QString,int>)),
            this, SIGNAL(signalTrashCountersMap(QMap<QString,int>)));

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(signalFinished()));

    collection.insert(j, 0);

    appendJobs(collection);
}

bool IOJobsThread::isCanceled() const
{
    return d->isCanceled;
}

bool IOJobsThread::hasErrors() const
{
    return !d->errorsList.isEmpty();
}

QStringList& IOJobsThread::errorsList() const
{
    return d->errorsList;
}

IOJobData* IOJobsThread::jobData() const
{
    return d->jobData;
}

void IOJobsThread::connectOneJob(IOJob* const j)
{
    connect(j, SIGNAL(signalError(QString)),
            this, SLOT(slotError(QString)));

    connect(j, SIGNAL(signalDone()),
            this, SLOT(slotOneJobFinished()));

    connect(j, SIGNAL(signalOneProccessed(QUrl)),
            this, SIGNAL(signalOneProccessed(QUrl)),
            Qt::BlockingQueuedConnection);
}

void IOJobsThread::slotOneJobFinished()
{
    d->jobsCount--;

    if (d->jobsCount == 0)
    {
        Q_EMIT signalFinished();

        qCDebug(DIGIKAM_IOJOB_LOG) << "Thread Finished";
    }
}

void IOJobsThread::slotError(const QString& errString)
{
    d->errorsList.append(errString);
}

void IOJobsThread::slotCancel()
{
    d->isCanceled = true;
    ActionThreadBase::cancel();
}

} // namespace Digikam

#include "moc_iojobsthread.cpp"
