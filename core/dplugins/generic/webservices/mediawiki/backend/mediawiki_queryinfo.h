/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QString>
#include <QUrl>
#include <QDateTime>

// Local includes

#include "mediawiki_page.h"
#include "mediawiki_protection.h"
#include "mediawiki_job.h"


namespace MediaWiki
{

class Iface;
class QueryInfoPrivate;

/**
 * @brief QueryInfo job.
 *
 * Uses to send a request to get basic page information.
 */
class QueryInfo : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QueryInfo)

public:

    /**
     * @brief Constructs a QueryInfo job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QueryInfo(Iface& MediaWiki, QObject* const parent = nullptr);

    /**
     * @brief Destroys the QuerySiteInfoGeneral job.
     */
    ~QueryInfo() override = default;

    /**
     * @brief Starts the job asynchronously.
     */
    void start() override;

    /**
     * @brief Set the page name.
     * @param title the page name
     */
    void setPageName(const QString& title);

    /**
     * @brief Set the token to perform a data-modifying action on a page
     * @param token the token
     */
    void setToken(const QString& token);

    /**
     * @brief Set the page id.
     * @param id the page id
     */
    void setPageId(unsigned int id);

    /**
     * @brief Set the page revision
     * @param id the page revision
     */
    void setRevisionId(unsigned int id);

Q_SIGNALS:

    /**
     * @brief Provides a page
     * @param
     */
    void page(const Page& p);
    void protection(const QVector<Protection>& protect);

private Q_SLOTS:

    /**
     * @brief Send a request.
     */
    void doWorkSendRequest();

    void doWorkProcessReply();
};

} // namespace MediaWiki
