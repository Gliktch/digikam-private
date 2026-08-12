/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "SearchRunnerPlugin.h"

// Qt includes

#include <QIcon>

namespace Marble
{

class Q_DECL_HIDDEN SearchRunnerPlugin::Private
{
public:

    QStringList m_supportedCelestialBodies;

    bool        m_canWorkOffline;

public:

    Private();
};

SearchRunnerPlugin::Private::Private()
    : m_canWorkOffline(true)
{
    // nothing to do
}

SearchRunnerPlugin::SearchRunnerPlugin(QObject* parent) :
    QObject(parent),
    d(new Private)
{
}

SearchRunnerPlugin::~SearchRunnerPlugin()
{
    delete d;
}

QIcon SearchRunnerPlugin::icon() const
{
    return QIcon();
}

bool SearchRunnerPlugin::supportsCelestialBody(const QString& celestialBodyId) const
{
    if (d->m_supportedCelestialBodies.isEmpty())
    {
        return true;
    }

    return d->m_supportedCelestialBodies.contains(celestialBodyId);
}

void SearchRunnerPlugin::setSupportedCelestialBodies(const QStringList& celestialBodies)
{
    d->m_supportedCelestialBodies = celestialBodies;
}

void SearchRunnerPlugin::setCanWorkOffline(bool canWorkOffline)
{
    d->m_canWorkOffline = canWorkOffline;
}

bool SearchRunnerPlugin::canWorkOffline() const
{
    return d->m_canWorkOffline;
}

bool SearchRunnerPlugin::canWork() const
{
    return true;
}

} // namespace Marble

#include "moc_SearchRunnerPlugin.cpp"
