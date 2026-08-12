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

// Local includes

#include "mediawiki_job.h"

namespace MediaWiki
{

class Iface;
class LogoutPrivate;

/**
 * @brief Logout job.
 *
 * Uses for log out a user.
 */
class Logout : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Logout)

public:

    /**
     * @brief Constructs a Logout job.
     * @param parent the QObject parent
     */
    explicit Logout(Iface& MediaWiki, QObject* const parent = nullptr);

    /**
     * @brief Destroys the Logout job.
     */
    ~Logout()    override = default;

    /**
     * @brief Starts the job asynchronously.
     */
    void start() override;

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki
