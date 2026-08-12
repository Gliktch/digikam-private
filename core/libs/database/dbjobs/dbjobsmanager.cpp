/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-05
 * Description : Manager for creating and starting DB jobs threads
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbjobsmanager.h"

// Local includes


namespace Digikam
{

class Q_DECL_HIDDEN DBJobsManagerCreator
{
public:

    DBJobsManager object;
};

Q_GLOBAL_STATIC(DBJobsManagerCreator, creator)

// -----------------------------------------------

DBJobsManager::DBJobsManager()
    : QObject()
{
}

DBJobsManager* DBJobsManager::instance()
{
    return &creator->object;
}

AlbumsDBJobsThread* DBJobsManager::startAlbumsJobThread(const AlbumsDBJobInfo& jInfo)
{
    AlbumsDBJobsThread* const thread = new AlbumsDBJobsThread(this);
    thread->albumsListing(jInfo);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

DatesDBJobsThread* DBJobsManager::startDatesJobThread(const DatesDBJobInfo& jInfo)
{
    DatesDBJobsThread* const thread = new DatesDBJobsThread(this);
    thread->datesListing(jInfo);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

TagsDBJobsThread* DBJobsManager::startTagsJobThread(const TagsDBJobInfo& jInfo)
{
    TagsDBJobsThread* const thread = new TagsDBJobsThread(this);
    thread->tagsListing(jInfo);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

SearchesDBJobsThread* DBJobsManager::startSearchesJobThread(const SearchesDBJobInfo& jInfo)
{
    SearchesDBJobsThread* const thread = new SearchesDBJobsThread(this);
    thread->searchesListing(jInfo);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

GPSDBJobsThread* DBJobsManager::startGPSJobThread(const GPSDBJobInfo& jInfo)
{
    GPSDBJobsThread* const thread = new GPSDBJobsThread(this);
    thread->GPSListing(jInfo);

    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()),
            Qt::QueuedConnection);

    thread->start();

    return thread;
}

} // namespace Digikam

#include "moc_dbjobsmanager.cpp"
