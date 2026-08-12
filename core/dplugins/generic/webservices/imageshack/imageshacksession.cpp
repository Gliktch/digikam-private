/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShackSession web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imageshacksession.h"

// Qt includes

#include <QApplication>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericImageShackPlugin
{

class Q_DECL_HIDDEN ImageShackSession::Private
{

public:

    Private() = default;

public:

    bool    loggedIn = false;

    QString authToken;
    QString username;
    QString email;
    QString password;
    QString credits;
};

ImageShackSession::ImageShackSession()
    : d(new Private)
{
    readSettings();
}

ImageShackSession::~ImageShackSession()
{
    delete d;
}

bool ImageShackSession::loggedIn() const
{
    return d->loggedIn;
}

QString ImageShackSession::username() const
{
    return d->username;
}

QString ImageShackSession::email() const
{
    return d->email;
}

QString ImageShackSession::password() const
{
    return d->password;
}

QString ImageShackSession::authToken() const
{
    return d->authToken;
}

QString ImageShackSession::credits() const
{
    return d->credits;
}

void ImageShackSession::setLoggedIn(bool b)
{
    d->loggedIn = b;
}

void ImageShackSession::setUsername(const QString& username)
{
    d->username = username;
}

void ImageShackSession::setEmail(const QString& email)
{
    d->email = email;
}

void ImageShackSession::setAuthToken(const QString& token)
{
    d->authToken = token;
}

void ImageShackSession::setPassword(const QString& pass)
{
    d->password = pass;
}

void ImageShackSession::setCredits(const QString& credits)
{
    d->credits = credits;
}

void ImageShackSession::logOut()
{
    d->loggedIn = false;
    d->username.clear();
    d->email.clear();
    d->credits.clear();
    saveSettings();
}

void ImageShackSession::readSettings()
{
    static bool bLoaded = false;

    if (bLoaded)
    {
        return;
    }

    bLoaded = true;

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("ImageShack Settings"));
}

void ImageShackSession::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("ImageShack Settings"));

    config->sync();
}

} // namespace DigikamGenericImageShackPlugin
