/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : Manager for creating and starting IO jobs threads
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2018 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iojobsmanager.h"

// Local includes

#include "iojobdata.h"

namespace Digikam
{

class Q_DECL_HIDDEN IOJobsManagerCreator
{
public:

    IOJobsManager object;
};

Q_GLOBAL_STATIC(IOJobsManagerCreator, creator)

// ----------------------------------------------

IOJobsManager::IOJobsManager()
    : QObject()
{
}

IOJobsManager* IOJobsManager::instance()
{
    return &creator->object;
}

IOJobsThread* IOJobsManager::startIOJobs(IOJobData* const data)
{
    IOJobsThread* const thread = new IOJobsThread(this);

    switch (data->operation())
    {
        case IOJobData::CopyAlbum:
        case IOJobData::CopyImage:
        case IOJobData::CopyFiles:
        case IOJobData::CopyToExt:
        case IOJobData::MoveAlbum:
        case IOJobData::MoveImage:
        case IOJobData::MoveFiles:
        {
            thread->copyOrMove(data);
            break;
        }

        case IOJobData::Trash:
        case IOJobData::Delete:
        {
            thread->deleteFiles(data);
            break;
        }

        case IOJobData::Rename:
        {
            thread->renameFile(data);
            break;
        }

        case IOJobData::Restore:
        {
            thread->restoreDTrashItems(data);
            break;
        }

        case IOJobData::Empty:
        {
            thread->emptyDTrashItems(data);
            break;
        }

        default:
        {
            break;
        }
    }

    connect(thread, SIGNAL(signalFinished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread* IOJobsManager::startDTrashItemsListingForCollection(const QString& collectionPath)
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->listDTrashItems(collectionPath);

    connect(thread, SIGNAL(signalFinished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

IOJobsThread* IOJobsManager::buildCollectionTrashCounters()
{
    IOJobsThread* const thread = new IOJobsThread(this);
    thread->buildTrashCounters();

    connect(thread, SIGNAL(signalFinished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

} // namespace Digikam

#include "moc_iojobsmanager.cpp"
