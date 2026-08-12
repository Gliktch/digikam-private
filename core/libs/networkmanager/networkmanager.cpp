/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-31
 * Description : digiKam global static QNetworkAccessManager
 *
 * SPDX-FileCopyrightText: 2022 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "networkmanager.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN NetworkManager::Private
{
public:

    Private() = default;

    QNetworkAccessManager* networkManager = nullptr;
};

// -----------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN NetworkManagerCreator
{
public:

    NetworkManager object;
};

Q_GLOBAL_STATIC(NetworkManagerCreator, creator)

// -----------------------------------------------------------------------------------------------

NetworkManager::NetworkManager()
    : QObject(),
      d      (new Private)
{
    d->networkManager = new QNetworkAccessManager(this);
}

NetworkManager::~NetworkManager()
{
    delete d;
}

NetworkManager* NetworkManager::instance()
{
    return &creator->object;
}

QNetworkAccessManager* NetworkManager::getNetworkManager(QObject* const object) const
{
    if (thread() == object->thread())
    {
        return d->networkManager;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "New QNetworkAccessManager for" << object;

    return (new QNetworkAccessManager(object));
}

} // namespace Digikam

#include "moc_networkmanager.cpp"
