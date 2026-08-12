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

#include "mediawiki_iface_p.h"

// Qt includes


namespace MediaWiki
{

Iface::Iface(const QUrl& url, const QString& customUserAgent)
    : d(new Private(url, customUserAgent, new QNetworkAccessManager()))
{
}

Iface::~Iface()
{
    delete d;
}

QUrl Iface::url() const
{
    return d->url;
}

QString Iface::userAgent() const
{
    return d->userAgent;
}

QNetworkAccessManager* Iface::manager() const
{
    return d->manager;
}

} // namespace MediaWiki
