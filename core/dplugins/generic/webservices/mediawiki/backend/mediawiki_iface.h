/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Remi Benoit <r3m1 dot benoit at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>

namespace MediaWiki
{

/**
 * @brief Provides access to wiki powered by Iface.
 */
class Iface
{
public:

    /**
     * @brief Constructs a Iface by its url api.
     * @param url the url api of the wiki
     * @param customUserAgent you can specify the user agent to use
                              which will be concatenated with the postfix user agent
     *                        else the postfix user agent is used only
     */
    explicit Iface(const QUrl& url, const QString& customUserAgent = QString());

    /**
     * @brief Destructs the Iface.
     */
    ~Iface();

    /**
     * @brief Returns the url api of the wiki.
     * @returns the url api of the wiki
     */
    QUrl url()                          const;

    /**
     * @brief Returns the user agent of the wiki.
     * @return the user agent of the wiki
     */
    QString userAgent()                 const;

    /**
     * @brief Returns the network manager instance of the wiki.
     * @return the network manager instance of the wiki
     */
    QNetworkAccessManager* manager()    const;

private:

    /// @note disabled
    Iface(const Iface&)            = delete;
    Iface& operator=(const Iface&) = delete;

private:

    class Private;
    Private* const d = nullptr;

    friend class JobPrivate;
};

} // namespace MediaWiki
