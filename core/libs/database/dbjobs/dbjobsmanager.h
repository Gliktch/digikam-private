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

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "dbjobinfo.h"
#include "dbjobsthread.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DBJobsManager : public QObject
{
    Q_OBJECT

public:

    DBJobsManager();

    /**
     * @brief instance: returns DBJobsManager singleton
     * @return DBJobsManager global instance
     */
    static DBJobsManager* instance();

    /**
     * @brief startGPSJobThread: creates and starts GPS Job Thread
     * @param jInfo: holds job info about the DB job
     * @return GPSDBJobsThread instance for signal/slot connection
     */
    GPSDBJobsThread* startGPSJobThread(const GPSDBJobInfo& jInfo);

    /**
     * @brief startTagsJobThread: creates and starts Tag Job Thread
     * @param jInfo: holds job info about the DB job
     * @return TagsDBJobsThread instance for signal/slot connection
     */
    TagsDBJobsThread* startTagsJobThread(const TagsDBJobInfo& jInfo);

    /**
     * @brief startDatesJobThread: creates and starts Dates Job Thread
     * @param jInfo: holds job info about the DB job
     * @return DatesDBJobsThread instance for signal/slot connection
     */
    DatesDBJobsThread* startDatesJobThread(const DatesDBJobInfo& jInfo);

    /**
     * @brief startAlbumsJobThread: creates and starts Albums Job Thread
     * @param jInfo: holds job info about the DB job
     * @return AlbumsDBJobsThread instance for signal/slot connection
     */
    AlbumsDBJobsThread* startAlbumsJobThread(const AlbumsDBJobInfo& jInfo);

    /**
     * @brief startSearchesJobThread: creates and starts Searches Job Thread
     * @param jInfo: holds job info about the DB job
     * @return SearchesDBJobsThread instance for signal/slot connection
     */
    SearchesDBJobsThread* startSearchesJobThread(const SearchesDBJobInfo& jInfo);

private:

    explicit DBJobsManager(QObject*) = delete;
};

} // namespace Digikam
