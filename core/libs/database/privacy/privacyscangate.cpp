/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyscangate.h"

// Qt includes

#include <QGlobalStatic>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QWriteLocker>

namespace Digikam
{

namespace
{

class PrivacyScanGateData
{
public:

    QReadWriteLock                                  lock;
    QSharedPointer<const PrivacyScanGateProvider>   provider;
    quint64                                         generation = 0;
};

Q_GLOBAL_STATIC(PrivacyScanGateData, scanGateData)

} // namespace

void PrivacyScanGate::setProvider(const QSharedPointer<const PrivacyScanGateProvider>& provider)
{
    QWriteLocker locker(&scanGateData->lock);
    scanGateData->provider = provider;

    if (++scanGateData->generation == 0)
    {
        ++scanGateData->generation;
    }
}

void PrivacyScanGate::resetProvider()
{
    setProvider(QSharedPointer<const PrivacyScanGateProvider>());
}

PrivacyScanDisposition PrivacyScanGate::evaluate(const PrivacyScanRequest& request)
{
    QSharedPointer<const PrivacyScanGateProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&scanGateData->lock);
        provider   = scanGateData->provider;
        generation = scanGateData->generation;
    }

    if (!provider)
    {
        return PrivacyScanDisposition::RootRecovering;
    }

    const PrivacyScanDisposition result = provider->evaluate(request);

    {
        QReadLocker locker(&scanGateData->lock);

        if ((generation != scanGateData->generation) || (provider != scanGateData->provider))
        {
            return PrivacyScanDisposition::RootRecovering;
        }
    }

    return result;
}

bool PrivacyScanGate::hasDeferredRoots()
{
    QSharedPointer<const PrivacyScanGateProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&scanGateData->lock);
        provider   = scanGateData->provider;
        generation = scanGateData->generation;
    }

    if (!provider)
    {
        return true;
    }

    const bool deferred = provider->hasDeferredRoots();

    {
        QReadLocker locker(&scanGateData->lock);

        if ((generation != scanGateData->generation) || (provider != scanGateData->provider))
        {
            return true;
        }
    }

    return deferred;
}

bool PrivacyScanGate::rootContainsProtectedItems(int albumRootId)
{
    QSharedPointer<const PrivacyScanGateProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&scanGateData->lock);
        provider   = scanGateData->provider;
        generation = scanGateData->generation;
    }

    if (!provider)
    {
        return true;
    }

    const bool contains = provider->rootContainsProtectedItems(albumRootId);

    {
        QReadLocker locker(&scanGateData->lock);

        if ((generation != scanGateData->generation) || (provider != scanGateData->provider))
        {
            return true;
        }
    }

    return contains;
}

} // namespace Digikam
