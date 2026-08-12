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

#include "ParsingRunnerManager.h"

// Qt includes

#include <QFileInfo>
#include <QList>
#include <QThreadPool>
#include <QTimer>
#include <QMutex>

// Local includes

#include "PluginManager.h"
#include "ParseRunnerPlugin.h"
#include "RunnerTask.h"
#include "digikam_debug.h"

namespace Marble
{

class MarbleModel;

class Q_DECL_HIDDEN ParsingRunnerManager::Private
{
public:

    Private(ParsingRunnerManager* parent, const PluginManager* pluginManager);

    ~Private();

    void cleanupParsingTask();
    void addParsingResult(GeoDataDocument* document, const QString& error);

public:

    ParsingRunnerManager* const q                       = nullptr;
    const PluginManager* const  m_pluginManager         = nullptr;
    QMutex                      m_parsingTasksMutex;
    int                         m_parsingTasks;
    GeoDataDocument*            m_fileResult            = nullptr;
};

ParsingRunnerManager::Private::Private(ParsingRunnerManager* parent, const PluginManager* pluginManager) :
    q(parent),
    m_pluginManager(pluginManager),
    m_parsingTasks(0),
    m_fileResult(nullptr)
{
    qRegisterMetaType<GeoDataDocument*>("GeoDataDocument*");
}

ParsingRunnerManager::Private::~Private()
{
    // nothing to do
}

void ParsingRunnerManager::Private::cleanupParsingTask()
{
    QMutexLocker locker(&m_parsingTasksMutex);
    m_parsingTasks = qMax(0, m_parsingTasks - 1);

    if (m_parsingTasks == 0)
    {
        Q_EMIT q->parsingFinished();
    }
}

ParsingRunnerManager::ParsingRunnerManager(const PluginManager* pluginManager, QObject* parent) :
    QObject(parent),
    d(new Private(this, pluginManager))
{
    if (QThreadPool::globalInstance()->maxThreadCount() < 4)
    {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    }
}

ParsingRunnerManager::~ParsingRunnerManager()
{
    delete d;
}

void ParsingRunnerManager::parseFile(const QString& fileName, DocumentRole role)
{
    QList<const ParseRunnerPlugin*> plugins = d->m_pluginManager->parsingRunnerPlugins();
    const QFileInfo fileInfo(fileName);
    const QString suffix = fileInfo.suffix().toLower();
    const QString completeSuffix = fileInfo.completeSuffix().toLower();

    d->m_parsingTasks = 0;

    for (const ParseRunnerPlugin* plugin : plugins)
    {
        QStringList const extensions = plugin->fileExtensions();

        if (extensions.isEmpty() || extensions.contains(suffix) || extensions.contains(completeSuffix))
        {
            ParsingTask* task = new ParsingTask(plugin->newRunner(), this, fileName, role);
            connect(task, SIGNAL(finished()), this, SLOT(cleanupParsingTask()));
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "parse task " << plugin->nameId() << " " << (quintptr)task;
            ++d->m_parsingTasks;
            QThreadPool::globalInstance()->start(task);
        }
    }

    if (d->m_parsingTasks == 0)
    {
        Q_EMIT parsingFinished();
    }
}

GeoDataDocument* ParsingRunnerManager::openFile(const QString& fileName, DocumentRole role, int timeout)
{
    d->m_fileResult = nullptr;
    QEventLoop localEventLoop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    connect(&watchdog, SIGNAL(timeout()),
            &localEventLoop, SLOT(quit()));
    connect(this, SIGNAL(parsingFinished()),
            &localEventLoop, SLOT(quit()), Qt::QueuedConnection);

    watchdog.start(timeout);
    parseFile(fileName, role);
    localEventLoop.exec();
    return d->m_fileResult;
}

void ParsingRunnerManager::Private::addParsingResult(GeoDataDocument* document, const QString& error)
{
    if (document || !error.isEmpty())
    {
        if (document)
        {
            m_fileResult = document;
        }

        Q_EMIT q->parsingFinished(document, error);
    }
}

} // namespace Marble

#include "moc_ParsingRunnerManager.cpp"
