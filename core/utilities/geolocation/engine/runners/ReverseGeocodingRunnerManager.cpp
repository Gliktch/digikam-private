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

#include "ReverseGeocodingRunnerManager.h"

// Qt includes

#include <QList>
#include <QThreadPool>
#include <QTimer>

// Local includes

#include "MarbleModel.h"
#include "GeoDataCoordinates.h"
#include "GeoDataPlacemark.h"
#include "Planet.h"
#include "PluginManager.h"
#include "ReverseGeocodingRunnerPlugin.h"
#include "RunnerTask.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN ReverseGeocodingRunnerManager::Private
{
public:

    Private(ReverseGeocodingRunnerManager* parent, const MarbleModel* marbleModel);

    QList<const ReverseGeocodingRunnerPlugin*> plugins(const QList<const ReverseGeocodingRunnerPlugin*>& plugins) const;

    void addReverseGeocodingResult(const GeoDataCoordinates& coordinates, const GeoDataPlacemark& placemark);
    void cleanupReverseGeocodingTask(ReverseGeocodingTask* task);

public:

    ReverseGeocodingRunnerManager* const q               = nullptr;
    const MarbleModel* const             m_marbleModel   = nullptr;
    const PluginManager*                 m_pluginManager = nullptr;
    QList<ReverseGeocodingTask*>         m_reverseTasks;
    QVector<GeoDataCoordinates>          m_reverseGeocodingResults;
    QString                              m_reverseGeocodingResult;
};

ReverseGeocodingRunnerManager::Private::Private(ReverseGeocodingRunnerManager* parent, const MarbleModel* marbleModel) :
    q(parent),
    m_marbleModel(marbleModel),
    m_pluginManager(marbleModel->pluginManager())
{
    qRegisterMetaType<GeoDataPlacemark>("GeoDataPlacemark");
    qRegisterMetaType<GeoDataCoordinates>("GeoDataCoordinates");
}

QList<const ReverseGeocodingRunnerPlugin*> ReverseGeocodingRunnerManager::Private::plugins(const QList<const ReverseGeocodingRunnerPlugin*>& plugins) const
{
    QList<const ReverseGeocodingRunnerPlugin*> result;

    for (const ReverseGeocodingRunnerPlugin* plugin : plugins)
    {
        if ((m_marbleModel && m_marbleModel->workOffline() && !plugin->canWorkOffline()))
        {
            continue;
        }

        if (!plugin->canWork())
        {
            continue;
        }

        if (m_marbleModel && !plugin->supportsCelestialBody(m_marbleModel->planet()->id()))
        {
            continue;
        }

        result << plugin;
    }

    return result;
}

void ReverseGeocodingRunnerManager::Private::addReverseGeocodingResult(const GeoDataCoordinates& coordinates, const GeoDataPlacemark& placemark)
{
    if (!m_reverseGeocodingResults.contains(coordinates) && !placemark.address().isEmpty())
    {
        m_reverseGeocodingResults.push_back(coordinates);
        m_reverseGeocodingResult = placemark.address();
        Q_EMIT q->reverseGeocodingFinished(coordinates, placemark);
    }

    if (m_reverseTasks.isEmpty())
    {
        Q_EMIT q->reverseGeocodingFinished();
    }
}

void ReverseGeocodingRunnerManager::Private::cleanupReverseGeocodingTask(ReverseGeocodingTask* task)
{
    m_reverseTasks.removeAll(task);
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "removing task " << m_reverseTasks.size() << " " << (quintptr)task;

    if (m_reverseTasks.isEmpty())
    {
        Q_EMIT q->reverseGeocodingFinished();
    }
}

ReverseGeocodingRunnerManager::ReverseGeocodingRunnerManager(const MarbleModel* marbleModel, QObject* parent) :
    QObject(parent),
    d(new Private(this, marbleModel))
{
    if (QThreadPool::globalInstance()->maxThreadCount() < 4)
    {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    }
}

ReverseGeocodingRunnerManager::~ReverseGeocodingRunnerManager()
{
    delete d;
}

void ReverseGeocodingRunnerManager::reverseGeocoding(const GeoDataCoordinates& coordinates)
{
    d->m_reverseTasks.clear();
    d->m_reverseGeocodingResult.clear();
    d->m_reverseGeocodingResults.removeAll(coordinates);

    QList<const ReverseGeocodingRunnerPlugin*> plugins = d->plugins(d->m_pluginManager->reverseGeocodingRunnerPlugins());

    for (const ReverseGeocodingRunnerPlugin* plugin : plugins)
    {
        ReverseGeocodingTask* task = new ReverseGeocodingTask(plugin->newRunner(), this, d->m_marbleModel, coordinates);
        connect(task, SIGNAL(finished(ReverseGeocodingTask*)), this, SLOT(cleanupReverseGeocodingTask(ReverseGeocodingTask*)));
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "reverse task " << plugin->nameId() << " " << (quintptr)task;
        d->m_reverseTasks << task;
    }

    for (ReverseGeocodingTask* task : d->m_reverseTasks)
    {
        QThreadPool::globalInstance()->start(task);
    }

    if (plugins.isEmpty())
    {
        GeoDataPlacemark anonymous;
        anonymous.setCoordinate(coordinates);
        Q_EMIT reverseGeocodingFinished(coordinates, anonymous);
        d->cleanupReverseGeocodingTask(nullptr);
    }
}

QString ReverseGeocodingRunnerManager::searchReverseGeocoding(const GeoDataCoordinates& coordinates, int timeout)
{
    QEventLoop localEventLoop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    connect(&watchdog, SIGNAL(timeout()),
            &localEventLoop, SLOT(quit()));
    connect(this, SIGNAL(reverseGeocodingFinished()),
            &localEventLoop, SLOT(quit()), Qt::QueuedConnection);

    watchdog.start(timeout);
    reverseGeocoding(coordinates);
    localEventLoop.exec();
    return d->m_reverseGeocodingResult;
}

} // namespace Marble

#include "moc_ReverseGeocodingRunnerManager.cpp"
