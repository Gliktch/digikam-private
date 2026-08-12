/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "mediawiki_job.h"
#include "mediawiki_usergroup.h"

namespace MediaWiki
{

class Iface;
class QuerySiteinfoUsergroupsPrivate;

/**
 * @brief UserGroups job.
 *
 * Uses for fetch a list of all user groups and their permissions.
 */
class QuerySiteinfoUsergroups : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuerySiteinfoUsergroups)

public:

    /**
     * @brief Constructs a UserGroups job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QuerySiteinfoUsergroups(Iface& MediaWiki, QObject* const parent = nullptr);

    /**
     * @brief Destroys the UserGroups job.
     */
    ~QuerySiteinfoUsergroups() override = default;

    /**
     * @brief If true number of users of each user group is included
     * @param includeNumber if true number of users of each user group is included
     */
    void setIncludeNumber(bool includeNumber);

    /**
     * @brief Starts the job asynchronously.
     */
    void start() override;

Q_SIGNALS:

    /**
     * @brief Provides a list of all user groups.
     * @param usergroups list of all user groups
     * @see QuerySiteinfoUsergroups::Result
     */
    void usergroups(const QList<UserGroup>& usergroups);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki
