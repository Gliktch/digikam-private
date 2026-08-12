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

#pragma once

// Qt includes

#include <QObject>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dtrashiteminfo.h"
#include "iojobsthread.h"

namespace Digikam
{

class IOJobData;

class DIGIKAM_GUI_EXPORT IOJobsManager : public QObject
{
    Q_OBJECT

public:

    IOJobsManager();

    /**
     * @brief instance: returns the singleton of IO Jobs Manager
     * @return IOJobsManager global instance
     */
    static IOJobsManager* instance();

    /**
     * @brief startIOJobs: Starts a thread to copy, move, delete or rename items
     * @param data: IOJobData container with source and destination url
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startIOJobs(IOJobData* const data);

    /**
     * @brief Starts a thread for listing items inside trash for specific collection
     * @param collectionPath: the path for collection to list items for it's trash
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* startDTrashItemsListingForCollection(const QString& collectionPath);

    /**
     * @brief Starts a thread for count trash items for all collections
     * @return IOJobsThread pointer for signal/slot connection
     */
    IOJobsThread* buildCollectionTrashCounters();

private:

    /// @note disabled
    explicit IOJobsManager(QObject*);
};

} // namespace Digikam
