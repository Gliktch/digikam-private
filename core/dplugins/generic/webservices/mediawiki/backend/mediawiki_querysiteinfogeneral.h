/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "mediawiki_job.h"
#include "mediawiki_generalinfo.h"

namespace MediaWiki
{

class Iface;
class QuerySiteInfoGeneralPrivate;

/**
 * @brief QuerySiteInfoGeneral job.
 *
 * Uses for fetch a generals information about the wiki.
 */
class QuerySiteInfoGeneral : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuerySiteInfoGeneral)

public:

    enum
    {
        IncludeAllDenied = Job::UserDefinedError + 1
    };

public:

    /**
     * @brief Constructs a QuerySiteInfoGeneral job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QuerySiteInfoGeneral(Iface& MediaWiki, QObject* const parent = nullptr);

    /**
     * @brief Destroys the QuerySiteInfoGeneral job.
     */
    ~QuerySiteInfoGeneral() override = default;

    /**
     * @brief Starts the job asynchronously.
     */
    void start() override;

Q_SIGNALS:

    /**
     * @brief Provide general info.
     * @param generalinfo the general info
     */
    void signalResult(const Generalinfo& generalinfo);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki
