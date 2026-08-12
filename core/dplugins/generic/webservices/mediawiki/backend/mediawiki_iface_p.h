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

#include "mediawiki_iface.h"

// Qt includes

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>

namespace MediaWiki
{

class Q_DECL_HIDDEN Iface::Private
{

public:

    Private(const QUrl& _url, const QString& _userAgent, QNetworkAccessManager* const _manager)
        : url      (_url),
          manager  (_manager)
    {
        userAgent = (
                     _userAgent.isEmpty() ? QString()
                                          : QString(_userAgent + QLatin1String("-"))
                    ) + POSTFIX_USER_AGENT;
    }

    ~Private()
    {
        delete manager;
    }

public:

    const QString POSTFIX_USER_AGENT        = QString::fromUtf8("MediaWiki-silk");

    const QUrl                   url;
    QString                      userAgent;
    QNetworkAccessManager* const manager    = nullptr;
};

} // namespace MediaWiki
