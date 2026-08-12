/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-01
 * Description : DB Jobs thread for listing and scanning
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbjobsthread.h"

// Local includes

#include "coredbaccess.h"
#include "duplicatesprogressobserver.h"
#include "digikam_debug.h"

namespace Digikam
{

DBJobsThread::DBJobsThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("DBJobsThread"));
}

bool DBJobsThread::hasErrors()
{
    return !m_errorsList.isEmpty();
}

QList<QString>& DBJobsThread::errorsList()
{
    return m_errorsList;
}

void DBJobsThread::connectFinishAndErrorSignals(DBJob* const j)
{
    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(signalFinished()));

    connect(j, SIGNAL(error(QString)),
            this, SLOT(error(QString)));
}

void DBJobsThread::error(const QString& errString)
{
    m_errorsList.append(errString);
}

// -------------------------------------------------

AlbumsDBJobsThread::AlbumsDBJobsThread(QObject* const parent)
    : DBJobsThread(parent)
{
}

void AlbumsDBJobsThread::albumsListing(const AlbumsDBJobInfo& info)
{
    AlbumsJob* const j = new AlbumsJob(info);

    connectFinishAndErrorSignals(j);

    if (info.isFoldersJob())
    {
        connect(j, SIGNAL(foldersData(QHash<int,int>)),
                this, SIGNAL(foldersData(QHash<int,int>)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ItemListerRecord>)),
                this, SIGNAL(data(QList<ItemListerRecord>)));
    }

    ActionJobCollection collection;
    collection.insert(j, 0);

    appendJobs(collection);
}

// -------------------------------------------------

TagsDBJobsThread::TagsDBJobsThread(QObject* const parent)
    : DBJobsThread(parent)
{
}

void TagsDBJobsThread::tagsListing(const TagsDBJobInfo& info)
{
    TagsJob* const j = new TagsJob(info);

    connectFinishAndErrorSignals(j);

    if (info.isFoldersJob())
    {
        connect(j, SIGNAL(foldersData(QHash<int,int>)),
                this, SIGNAL(foldersData(QHash<int,int>)));
    }
    else if (info.isFaceFoldersJob())
    {
        connect(j, SIGNAL(faceFoldersData(QMap<QString,QHash<int,int> >)),       // krazy:exclude=normalize
                this, SIGNAL(faceFoldersData(QMap<QString,QHash<int,int> >)));   // krazy:exclude=normalize
    }
    else
    {
        connect(j, SIGNAL(data(QList<ItemListerRecord>)),
                this, SIGNAL(data(QList<ItemListerRecord>)));
    }

    ActionJobCollection collection;
    collection.insert(j, 0);

    appendJobs(collection);
}

// -------------------------------------------------

DatesDBJobsThread::DatesDBJobsThread(QObject* const parent)
    : DBJobsThread(parent)
{
}

void DatesDBJobsThread::datesListing(const DatesDBJobInfo& info)
{
    DatesJob* const j = new DatesJob(info);

    connectFinishAndErrorSignals(j);

    if (info.isFoldersJob())
    {
        connect(j, SIGNAL(foldersData(QMap<QDateTime,int>)),
                this, SIGNAL(foldersData(QMap<QDateTime,int>)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ItemListerRecord>)),
                this, SIGNAL(data(QList<ItemListerRecord>)));
    }

    ActionJobCollection collection;
    collection.insert(j, 0);

    appendJobs(collection);
}

// -------------------------------------------------

GPSDBJobsThread::GPSDBJobsThread(QObject* const parent)
    : DBJobsThread(parent)
{
}

void GPSDBJobsThread::GPSListing(const GPSDBJobInfo& info)
{
    GPSJob* const j = new GPSJob(info);

    connectFinishAndErrorSignals(j);

    if (info.isDirectQuery())
    {
        connect(j, SIGNAL(directQueryData(QList<QVariant>)),
                this, SIGNAL(directQueryData(QList<QVariant>)));
    }
    else
    {
        connect(j, SIGNAL(data(QList<ItemListerRecord>)),
                this, SIGNAL(data(QList<ItemListerRecord>)));
    }

    ActionJobCollection collection;
    collection.insert(j, 0);

    appendJobs(collection);
}

// -------------------------------------------------

SearchesDBJobsThread::SearchesDBJobsThread(QObject* const parent)
    : DBJobsThread(parent)
{
}

void SearchesDBJobsThread::searchesListing(const SearchesDBJobInfo& info)
{
    ActionJobCollection collection;

    if (info.isDuplicatesJob())
    {
        m_results.clear();
        m_haarIface.reset(new HaarIface(info.imageIds()));
        m_isAlbumUpdate    = info.isAlbumUpdate();
        m_processedImages  = 0;
        m_totalImages2Scan = info.imageIds().count();

        const int threadsCount         = (m_totalImages2Scan < 200) ? 1 : qMax(1, maximumNumberOfThreads());
        const int images2ScanPerThread = m_totalImages2Scan / threadsCount;

        QSet<qlonglong>::const_iterator begin = info.imageIds().constBegin();
        QSet<qlonglong>::const_iterator end   = info.imageIds().constBegin();

        // Split job on multiple threads

        for (int i = 0 ; i < threadsCount ; ++i)
        {
            // The last thread should read until the end of the list.

            if (i == threadsCount - 1)
            {
                end = info.imageIds().constEnd();
            }
            else
            {
                // TODO: port to std::advance https://en.cppreference.com/w/cpp/iterator/advance

                for (int j = 0 ; ((end != info.imageIds().constEnd()) && (j < images2ScanPerThread)) ; ++j, ++end);
            }

            SearchesJob* const job = new SearchesJob(info, begin, end, m_haarIface.data());

            begin = end;

            connect(job, &SearchesJob::signalDuplicatesResults,
                    this, &SearchesDBJobsThread::slotDuplicatesResults);

            connect(job, &SearchesJob::signalImageProcessed,
                    this, &SearchesDBJobsThread::slotImageProcessed);

            collection.insert(job, 0);
        }
    }
    else
    {
        SearchesJob* const job = new SearchesJob(info);
        connectFinishAndErrorSignals(job);

        connect(job, SIGNAL(data(QList<ItemListerRecord>)),
                this, SIGNAL(data(QList<ItemListerRecord>)));

        collection.insert(job, 0);
    }

    appendJobs(collection);
}

void SearchesDBJobsThread::slotImageProcessed(const ItemInfo& inf, const QImage& img, int dup)
{
    Q_EMIT signalProgress((++m_processedImages * 100) / m_totalImages2Scan, inf, img, dup);
}

void SearchesDBJobsThread::slotDuplicatesResults(const HaarIface::DuplicatesResultsMap& incoming)
{
    auto searchResults = [&](qlonglong imageId) -> HaarIface::DuplicatesResultsMap::iterator
    {
        for (auto it = m_results.begin() ; it != m_results.end() ; ++it)
        {
            if ((imageId == it.key()) || (it->second.contains(imageId)))
            {
                return it;
            }
        }

        return m_results.end();
    };

    const auto keys = incoming.keys();

    for (const auto& referenceImage : keys)
    {
        if (searchResults(referenceImage) == m_results.end())
        {
            m_results.insert(referenceImage, incoming.value(referenceImage));
        }
    }

    if (m_processedImages != m_totalImages2Scan)
    {
        return;
    }

    HaarIface::rebuildDuplicatesAlbums(m_results, m_isAlbumUpdate);

    Q_EMIT signalFinished();
}

} // namespace Digikam

#include "moc_dbjobsthread.cpp"
