/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "piwigosession.h"

// Qt includes

#include <QApplication>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericPiwigoPlugin
{

class Q_DECL_HIDDEN PiwigoSession::Private
{
public:

    Private() = default;

    QString url;
    QString username;
    QString password;
};

PiwigoSession::PiwigoSession()
    : d(new Private)
{
    load();
}

PiwigoSession::~PiwigoSession()
{
    delete d;
}

QString PiwigoSession::url() const
{
    return d->url;
}

QString PiwigoSession::username() const
{
    return d->username;
}

QString PiwigoSession::password() const
{
    return d->password;
}

void PiwigoSession::setUrl(const QString& url)
{
    d->url = url;
}

void PiwigoSession::setUsername(const QString& username)
{
    d->username = username;
}

void PiwigoSession::setPassword(const QString& password)
{
    d->password = password;
}

void PiwigoSession::load()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Piwigo Settings"));

    d->url                  = group.readEntry("URL",      QString());
    d->username             = group.readEntry("Username", QString());
    d->password             = group.readEntry("Password", QString());
}

void PiwigoSession::save()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Piwigo Settings"));

    group.writeEntry(QString::fromUtf8("URL"),      url());
    group.writeEntry(QString::fromUtf8("Username"), username());
    group.writeEntry(QString::fromUtf8("Password"), password());

    config->sync();
}

} // namespace DigikamGenericPiwigoPlugin
