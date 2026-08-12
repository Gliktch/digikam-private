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

#include "SearchRunnerManager.h"

// Qt includes

#include <QString>
#include <QThreadPool>
#include <QTimer>
#include <QMutex>

// Local includes

#include "MarblePlacemarkModel.h"
#include "MarbleModel.h"
#include "Planet.h"
#include "GeoDataPlacemark.h"
#include "PluginManager.h"
#include "ParseRunnerPlugin.h"
#include "ReverseGeocodingRunnerPlugin.h"
#include "SearchRunnerPlugin.h"
#include "RunnerTask.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN SearchRunnerManager::Private
{
public:

    Private(SearchRunnerManager* parent, const MarbleModel* marbleModel);

    template<typename T>
    QList<T*> plugins(const QList<T*>& plugins) const;

    void addSearchResult(const QVector<GeoDataPlacemark*>& result);
    void cleanupSearchTask(SearchTask* task);
    void notifySearchResultChange();
    void notifySearchFinished();

public:

    SearchRunnerManager* const  q                   = nullptr;
    const MarbleModel* const    m_marbleModel       = nullptr;
    const PluginManager*        m_pluginManager     = nullptr;
    QString                     m_lastSearchTerm;
    GeoDataLatLonBox            m_lastPreferredBox;
    QMutex                      m_modelMutex;
    MarblePlacemarkModel        m_model;
    QList<SearchTask*>         m_searchTasks;
    QVector<GeoDataPlacemark*> m_placemarkContainer;
};

SearchRunnerManager::Private::Private(SearchRunnerManager* parent, const MarbleModel* marbleModel) :
    q(parent),
    m_marbleModel(marbleModel),
    m_pluginManager(marbleModel->pluginManager()),
    m_model(new MarblePlacemarkModel(parent))
{
    m_model.setPlacemarkContainer(&m_placemarkContainer);
    qRegisterMetaType<QVector<GeoDataPlacemark*> >("QVector<GeoDataPlacemark*>");
}

template<typename T>
QList<T*> SearchRunnerManager::Private::plugins(const QList<T*>& plugins) const
{
    QList<T*> result;

    for (T* plugin : plugins)
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

void SearchRunnerManager::Private::addSearchResult(const QVector<GeoDataPlacemark*>& result)
{
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Runner reports" << result.size() << " search results";

    if (result.isEmpty())
    {
        return;
    }

    m_modelMutex.lock();
    int start = m_placemarkContainer.size();
    int count = 0;
    bool distanceCompare = m_marbleModel->planet() != nullptr;

    for (int i = 0; i < result.size(); ++i)
    {
        bool same = false;

        for (int j = 0; j < m_placemarkContainer.size(); ++j)
        {
            if (distanceCompare &&
                (result[i]->coordinate().sphericalDistanceTo(m_placemarkContainer[j]->coordinate())
                 * m_marbleModel->planet()->radius() < 1))
            {
                same = true;
            }
        }

        if (!same)
        {
            m_placemarkContainer.append(result[i]);
            ++count;
        }
    }

    m_model.addPlacemarks(start, count);
    m_modelMutex.unlock();
    notifySearchResultChange();
}

void SearchRunnerManager::Private::cleanupSearchTask(SearchTask* task)
{
    m_searchTasks.removeAll(task);
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "removing search task" << m_searchTasks.size() << (quintptr)task;

    if (m_searchTasks.isEmpty())
    {
        if (m_placemarkContainer.isEmpty())
        {
            notifySearchResultChange();
        }

        notifySearchFinished();
    }
}

void SearchRunnerManager::Private::notifySearchResultChange()
{
    Q_EMIT q->searchResultChanged(&m_model);
    Q_EMIT q->searchResultChanged(m_placemarkContainer);
}

void SearchRunnerManager::Private::notifySearchFinished()
{
    Q_EMIT q->searchFinished(m_lastSearchTerm);
    Q_EMIT q->placemarkSearchFinished();
}

SearchRunnerManager::SearchRunnerManager(const MarbleModel* marbleModel, QObject* parent) :
    QObject(parent),
    d(new Private(this, marbleModel))
{
    if (QThreadPool::globalInstance()->maxThreadCount() < 4)
    {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    }
}

SearchRunnerManager::~SearchRunnerManager()
{
    delete d;
}

void SearchRunnerManager::findPlacemarks(const QString& searchTerm, const GeoDataLatLonBox& preferred)
{
    if (searchTerm == d->m_lastSearchTerm && preferred == d->m_lastPreferredBox)
    {
        d->notifySearchResultChange();
        d->notifySearchFinished();
        return;
    }

    d->m_lastSearchTerm = searchTerm;
    d->m_lastPreferredBox = preferred;

    d->m_searchTasks.clear();

    d->m_modelMutex.lock();
    bool placemarkContainerChanged = false;

    if (!d->m_placemarkContainer.isEmpty())
    {
        d->m_model.removePlacemarks(QString::fromUtf8("PlacemarkRunnerManager"), 0, d->m_placemarkContainer.size());
        qDeleteAll(d->m_placemarkContainer);
        d->m_placemarkContainer.clear();
        placemarkContainerChanged = true;
    }

    d->m_modelMutex.unlock();

    if (placemarkContainerChanged)
    {
        d->notifySearchResultChange();
    }

    if (searchTerm.trimmed().isEmpty())
    {
        d->notifySearchFinished();
        return;
    }

    QList<const SearchRunnerPlugin*> plugins = d->plugins(d->m_pluginManager->searchRunnerPlugins());

    for (const SearchRunnerPlugin* plugin : plugins)
    {
        SearchTask* task = new SearchTask(plugin->newRunner(), this, d->m_marbleModel, searchTerm, preferred);
        connect(task, SIGNAL(finished(SearchTask*)), this, SLOT(cleanupSearchTask(SearchTask*)));
        d->m_searchTasks << task;
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "search task " << plugin->nameId() << " " << (quintptr)task;
    }

    for (SearchTask* task : d->m_searchTasks)
    {
        QThreadPool::globalInstance()->start(task);
    }

    if (plugins.isEmpty())
    {
        d->cleanupSearchTask(nullptr);
    }
}

QVector<GeoDataPlacemark*> SearchRunnerManager::searchPlacemarks(const QString& searchTerm, const GeoDataLatLonBox& preferred, int timeout)
{
    QEventLoop localEventLoop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    connect(&watchdog, SIGNAL(timeout()),
            &localEventLoop, SLOT(quit()));
    connect(this, SIGNAL(placemarkSearchFinished()),
            &localEventLoop, SLOT(quit()), Qt::QueuedConnection);

    watchdog.start(timeout);
    findPlacemarks(searchTerm, preferred);
    localEventLoop.exec();
    return d->m_placemarkContainer;
}

} // namespace Marble

#include "moc_SearchRunnerManager.cpp"
