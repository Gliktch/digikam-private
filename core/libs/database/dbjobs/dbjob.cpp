/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-01
 * Description : DB Jobs for listing and scanning
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbjob.h"

// Local includes

#include "digikam_globals.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"
#include "coredb.h"
#include "facetags.h"
#include "itemlister.h"
#include "digikam_debug.h"
#include "dbjobsthread.h"

namespace Digikam
{

DBJob::DBJob()
    : ActionJob()
{
}

// ----------------------------------------------

AlbumsJob::AlbumsJob(const AlbumsDBJobInfo& jobInfo)
    : DBJob    (),
      m_jobInfo(jobInfo)
{
}

void AlbumsJob::run()
{
    ActionJob::run();       // To customize thread name

    if (m_jobInfo.isFoldersJob())
    {
        const QHash<int, int>& albumNumberHash = CoreDbAccess().db()->getNumberOfImagesInAlbums();

        Q_EMIT foldersData(albumNumberHash);
    }
    else
    {
        ItemLister lister;
        lister.setRecursive(m_jobInfo.isRecursive());
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive

        ItemListerJobGrowingPartsSendingReceiver receiver(this, 200, 2000, 100);
        lister.listPAlbum(&receiver, m_jobInfo.albumRootId(), m_jobInfo.album());
        receiver.sendData();
    }

    Q_EMIT signalDone();
}

// ----------------------------------------------

DatesJob::DatesJob(const DatesDBJobInfo& jobInfo)
    : DBJob    (),
      m_jobInfo(jobInfo)
{
}

void DatesJob::run()
{
    ActionJob::run();       // To customize thread name

    if (m_jobInfo.isFoldersJob())
    {
        const QMap<QDateTime, int>& dateNumberMap = CoreDbAccess().db()->getAllCreationDates();

        Q_EMIT foldersData(dateNumberMap);
    }
    else
    {
        ItemLister lister;
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive

        ItemListerJobPartsSendingReceiver receiver(this, 200);
        lister.listDateRange(&receiver, m_jobInfo.startDate(), m_jobInfo.endDate());

        // Send rest

        receiver.sendData();
    }

    Q_EMIT signalDone();
}

// ----------------------------------------------

GPSJob::GPSJob(const GPSDBJobInfo& jobInfo)
    : DBJob    (),
      m_jobInfo(jobInfo)
{
}

void GPSJob::run()
{
    ActionJob::run();       // To customize thread name

    if (m_jobInfo.isDirectQuery())
    {
        QList<QVariant> imagesInfoFromArea =
                CoreDbAccess().db()->getImageIdsFromArea(m_jobInfo.lat1(),
                                                         m_jobInfo.lat2(),
                                                         m_jobInfo.lng1(),
                                                         m_jobInfo.lng2(),
                                                         0,
                                                         QLatin1String("rating"));

        Q_EMIT directQueryData(imagesInfoFromArea);
    }
    else
    {
        ItemLister lister;
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive

        ItemListerJobPartsSendingReceiver receiver(this, 200);
        lister.listAreaRange(&receiver,
                             m_jobInfo.lat1(),
                             m_jobInfo.lat2(),
                             m_jobInfo.lng1(),
                             m_jobInfo.lng2());
        // send rest

        receiver.sendData();
    }

    Q_EMIT signalDone();
}

// ----------------------------------------------

TagsJob::TagsJob(const TagsDBJobInfo& jobInfo)
    : DBJob    (),
      m_jobInfo(jobInfo)
{
}

void TagsJob::run()
{
    ActionJob::run();       // To customize thread name

    if      (m_jobInfo.isFoldersJob())
    {
        const QHash<int, int>& tagNumberHash = CoreDbAccess().db()->getNumberOfImagesInTags();

        //qCDebug(DIGIKAM_DBJOB_LOG) << tagNumberHash;

        Q_EMIT foldersData(tagNumberHash);
    }
    else if (m_jobInfo.isFaceFoldersJob())
    {
        QString property;
        QHash<int, int> counts;
        QMap<QString, QHash<int, int> > facesNumberMap;

        property = ImageTagPropertyName::autodetectedFace();
        counts   = CoreDbAccess().db()->getNumberOfImagesInTagProperties(property);

        if (!counts.contains(FaceTags::unknownPersonTagId()))
        {
            counts[FaceTags::unknownPersonTagId()] = 0;
        }

        facesNumberMap.insert(property, counts);

        property = ImageTagPropertyName::autodetectedPerson();
        counts   = CoreDbAccess().db()->getNumberOfImagesInTagProperties(property);

        facesNumberMap.insert(property, counts);

        property = ImageTagPropertyName::ignoredFace();
        counts   = CoreDbAccess().db()->getNumberOfImagesInTagProperties(property);

        facesNumberMap.insert(property, counts);

        property        = ImageTagPropertyName::tagRegion();
        counts          = CoreDbAccess().db()->getNumberOfImagesInTagProperties(property);
        const auto tags = FaceTags::allPersonTags();

        for (int tagId : tags)
        {
            if (!counts.contains(tagId))
            {
                counts[tagId] = 0;
            }
        }

        facesNumberMap.insert(property, counts);

        Q_EMIT faceFoldersData(facesNumberMap);
    }
    else
    {
        ItemLister lister;
        lister.setRecursive(m_jobInfo.isRecursive());
        lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

        // Send data every 200 images to be more responsive

        ItemListerJobPartsSendingReceiver receiver(this, 200);

        if (!m_jobInfo.specialTag().isNull())
        {
            QString searchXml = lister.tagSearchXml(m_jobInfo.tagsIds().constFirst(),
                                                    m_jobInfo.specialTag(),
                                                    m_jobInfo.isRecursive());
            lister.listImageTagPropertySearch(&receiver, searchXml);
        }
        else
        {
            lister.listTag(&receiver, m_jobInfo.tagsIds());
        }

        // Finish sending

        receiver.sendData();
    }

    Q_EMIT signalDone();
}

// ----------------------------------------------

SearchesJob::SearchesJob(const SearchesDBJobInfo& jobInfo)
    : DBJob    (),
      m_jobInfo(jobInfo),
      m_iface  (nullptr)
{
}

SearchesJob::SearchesJob(const SearchesDBJobInfo& jobInfo,
                         const QSet<qlonglong>::const_iterator& begin,
                         const QSet<qlonglong>::const_iterator& end,
                         HaarIface* iface)
    : DBJob    (),
      m_jobInfo(jobInfo),
      m_begin  (begin),
      m_end    (end),
      m_iface  (iface)
{
}

void SearchesJob::run()
{
    ActionJob::run();       // To customize thread name

    m_jobInfo.isDuplicatesJob() ? runFindDuplicates() : runSearches();
}

void SearchesJob::runSearches()
{
    QList<SearchInfo> infos;
    const auto& ids = m_jobInfo.searchIds();

    for (int id : ids)
    {
        infos << CoreDbAccess().db()->getSearchInfo(id);
    }

    ItemLister lister;
    lister.setListOnlyAvailable(m_jobInfo.isListAvailableImagesOnly());

    // Send data every 200 images to be more responsive

    ItemListerJobPartsSendingReceiver receiver(this, 200);

    for (const SearchInfo& info : std::as_const(infos))
    {
        if (info.type == DatabaseSearch::HaarSearch)
        {
            lister.listHaarSearch(&receiver, info.query);
        }
        else
        {
            bool ok;
            qlonglong referenceImageId = info.name.toLongLong(&ok);

            if (ok)
            {
                lister.listSearch(&receiver, info.query, 0, referenceImageId);
            }
            else
            {
                lister.listSearch(&receiver, info.query, 0, -1);
            }
        }

        if (!receiver.hasError)
        {
            receiver.sendData();
        }
    }

    Q_EMIT signalDone();
}

void SearchesJob::runFindDuplicates()
{
    if (m_jobInfo.imageIds().isEmpty())
    {
        qCDebug(DIGIKAM_DBJOB_LOG) << "No image ids passed for duplicates search";

        Q_EMIT signalDuplicatesResults(HaarIface::DuplicatesResultsMap());

        return;
    }

    DuplicatesProgressObserver observer(this);

    if (!m_iface)
    {
        qCDebug(DIGIKAM_DBJOB_LOG) << "Invalid HaarIface pointer";

        Q_EMIT signalDuplicatesResults(HaarIface::DuplicatesResultsMap());

        return;
    }

    auto restriction = static_cast<HaarIface::DuplicatesSearchRestrictions>(m_jobInfo.searchResultRestriction());
    auto results     = m_iface->findDuplicates(m_jobInfo.imageIds(),
                                               m_begin,
                                               m_end,
                                               m_jobInfo.refImageSelectionMethod(),
                                               m_jobInfo.refImageIds(),
                                               m_jobInfo.minThreshold(),
                                               m_jobInfo.maxThreshold(),
                                               restriction,
                                               &observer);

    Q_EMIT signalDuplicatesResults(results);
}

bool SearchesJob::isCanceled() const
{
    return m_cancel;
}

} // namespace Digikam

#include "moc_dbjob.cpp"
