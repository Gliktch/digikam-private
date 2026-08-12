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

#include "PluginManager.h"

// Qt includes

#include <QPluginLoader>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QLibraryInfo>

// Local includes

#include "MarbleDirs.h"
#include "RenderPlugin.h"
#include "ParseRunnerPlugin.h"
#include "ReverseGeocodingRunnerPlugin.h"
#include "SearchRunnerPlugin.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN PluginManagerPrivate
{
public:

    explicit PluginManagerPrivate(PluginManager* const parent)
        : m_parent(parent)
    {
    }

    ~PluginManagerPrivate() = default;

public:

    void loadPlugins();
    bool addPlugin(QObject* const obj, const QPluginLoader* const loader);

public:

    bool                                        m_pluginsLoaded                 = false;
    QList<const RenderPlugin*>                  m_renderPluginTemplates;
    QList<const SearchRunnerPlugin*>            m_searchRunnerPlugins;
    QList<const ReverseGeocodingRunnerPlugin*>  m_reverseGeocodingRunnerPlugins;
    QList<const ParseRunnerPlugin*>             m_parsingRunnerPlugins;
    PluginManager*                              m_parent                        = nullptr;
    static QStringList                          m_blacklist;
    static QStringList                          m_whitelist;

#ifdef Q_OS_ANDROID

    QStringList                                 m_pluginPaths;

#endif

};

QStringList PluginManagerPrivate::m_blacklist;
QStringList PluginManagerPrivate::m_whitelist;

PluginManager::PluginManager(QObject* parent)
    : QObject(parent),
      d      (new PluginManagerPrivate(this))
{
    // Checking assets:/plugins for uninstalled plugins

#ifdef Q_OS_ANDROID

    installPluginsFromAssets();

#endif

    d->loadPlugins();
}

PluginManager::~PluginManager()
{
    delete d;
}

QList<const RenderPlugin*> PluginManager::renderPlugins() const
{
    d->loadPlugins();

    return d->m_renderPluginTemplates;
}

void PluginManager::addRenderPlugin(const RenderPlugin* plugin)
{
    d->loadPlugins();
    d->m_renderPluginTemplates << plugin;

    Q_EMIT renderPluginsChanged();
}

QList<const SearchRunnerPlugin*> PluginManager::searchRunnerPlugins() const
{
    d->loadPlugins();

    return d->m_searchRunnerPlugins;
}

void PluginManager::addSearchRunnerPlugin(const SearchRunnerPlugin* plugin)
{
    d->loadPlugins();
    d->m_searchRunnerPlugins << plugin;

    Q_EMIT searchRunnerPluginsChanged();
}

QList<const ReverseGeocodingRunnerPlugin*> PluginManager::reverseGeocodingRunnerPlugins() const
{
    d->loadPlugins();

    return d->m_reverseGeocodingRunnerPlugins;
}

void PluginManager::addReverseGeocodingRunnerPlugin(const ReverseGeocodingRunnerPlugin* plugin)
{
    d->loadPlugins();
    d->m_reverseGeocodingRunnerPlugins << plugin;

    Q_EMIT reverseGeocodingRunnerPluginsChanged();
}

QList<const ParseRunnerPlugin*> PluginManager::parsingRunnerPlugins() const
{
    d->loadPlugins();

    return d->m_parsingRunnerPlugins;
}

void PluginManager::addParseRunnerPlugin(const ParseRunnerPlugin* plugin)
{
    d->loadPlugins();
    d->m_parsingRunnerPlugins << plugin;

    Q_EMIT parseRunnerPluginsChanged();
}

void PluginManager::blacklistPlugin(const QString& filename)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    PluginManagerPrivate::m_blacklist << QLibraryInfo::path(QLibraryInfo::PluginsPath) +
                                         QString::fromUtf8("/marble/") + filename;

#else

    PluginManagerPrivate::m_blacklist << QLibraryInfo::location(QLibraryInfo::PluginsPath) +
                                         QString::fromUtf8("/marble/") + filename;

#endif

}

void PluginManager::whitelistPlugin(const QString& filename)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    PluginManagerPrivate::m_whitelist << QLibraryInfo::path(QLibraryInfo::PluginsPath) +
                                         QString::fromUtf8("/marble/") + filename;

#else

    PluginManagerPrivate::m_whitelist << QLibraryInfo::location(QLibraryInfo::PluginsPath) +
                                         QString::fromUtf8("/marble/") + filename;

#endif

}

/**
 * Append obj to the given plugins list if it inherits both T and U
 */
template<class Iface, class Plugin>
bool appendPlugin(QObject* const obj, const QPluginLoader* const loader, QList<Plugin>& plugins)
{
    if (qobject_cast<Iface*>(obj) && qobject_cast<Plugin>(obj))
    {
        Q_ASSERT(obj->metaObject()->superClass());   // all our plugins have a super class

        qCDebug(DIGIKAM_GEOENGINE_LOG) << obj->metaObject()->superClass()->className()
                                       << "plugin loaded from"
                                       << (loader ? loader->fileName() : QString::fromUtf8("<static>"));

        auto plugin = qobject_cast<Plugin>(obj);

        Q_ASSERT(plugin);   // checked above

        plugins << plugin;

        return true;
    }

    return false;
}

bool PluginManagerPrivate::addPlugin(QObject* const obj, const QPluginLoader* const loader)
{
    if (!obj || !loader)
    {
        return false;
    }

    bool isPlugin =             appendPlugin<RenderPluginInterface>       (obj, loader, m_renderPluginTemplates);
    isPlugin      = isPlugin || appendPlugin<SearchRunnerPlugin>          (obj, loader, m_searchRunnerPlugins);
    isPlugin      = isPlugin || appendPlugin<ReverseGeocodingRunnerPlugin>(obj, loader, m_reverseGeocodingRunnerPlugins);
    isPlugin      = isPlugin || appendPlugin<ParseRunnerPlugin>           (obj, loader, m_parsingRunnerPlugins);

    if (!isPlugin)
    {
        qCWarning(DIGIKAM_GEOENGINE_LOG) << "Ignoring the following plugin since it couldn't be loaded:"
                                         << loader->fileName();

        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Plugin failure:"
                                       << loader->fileName()
                                       << "is a plugin, but it does not implement the "
                                          "right interfaces or it was compiled against "
                                          "an old version of Marble. Ignoring it.";
    }

    return isPlugin;
}

void PluginManagerPrivate::loadPlugins()
{
    if (m_pluginsLoaded)
    {
        return;
    }

    QElapsedTimer t;
    t.start();
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Starting to load Plugins.";

    QStringList pluginFileNameList = MarbleDirs::pluginEntryList(QString::fromUtf8(""), QDir::Files);

    MarbleDirs::debug();

    Q_ASSERT(m_renderPluginTemplates.isEmpty());
    Q_ASSERT(m_searchRunnerPlugins.isEmpty());
    Q_ASSERT(m_reverseGeocodingRunnerPlugins.isEmpty());
    Q_ASSERT(m_parsingRunnerPlugins.isEmpty());

    bool foundPlugin = false;

    for (const QString& fileName : std::as_const(pluginFileNameList))
    {
        QString const baseName    = QFileInfo(fileName).baseName();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QString const libBaseName = QLibraryInfo::path(QLibraryInfo::PluginsPath) +
                                    QString::fromUtf8("/marble/")                 +
                                    QFileInfo(fileName).baseName();

#else

        QString const libBaseName = QLibraryInfo::location(QLibraryInfo::PluginsPath) +
                                    QString::fromUtf8("/marble/") +
                                    QFileInfo(fileName).baseName();

#endif

        if (!m_whitelist.isEmpty() && !m_whitelist.contains(baseName) && !m_whitelist.contains(libBaseName))
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Ignoring non-whitelisted plugin " << fileName;

            continue;
        }

        if (m_blacklist.contains(baseName) || m_blacklist.contains(libBaseName))
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Ignoring blacklisted plugin " << fileName;

            continue;
        }

        // qCDebug(DIGIKAM_GEOENGINE_LOG) << fileName << " - " << MarbleDirs::pluginPath( fileName );

        QString const path = MarbleDirs::pluginPath(fileName);

#ifdef Q_OS_ANDROID

        QFileInfo targetFile(path);

        if (!m_pluginPaths.contains(targetFile.canonicalFilePath()))
        {
            // @todo Delete the file here?

            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Ignoring file " << path
                                           << " which is not among the currently installed plugins";

            continue;
        }

#endif

        QPluginLoader* loader = new QPluginLoader(path, m_parent);
        QObject* obj          = loader->instance();

        if (obj)
        {
            bool isPlugin = addPlugin(obj, loader);

            if (!isPlugin)
            {
                delete loader;
            }
            else
            {
                foundPlugin = true;
            }
        }

        else
        {
            qCWarning(DIGIKAM_GEOENGINE_LOG) << "Ignoring to load the following file since it "
                                                "doesn't look like a valid Marble plugin:"
                                             << path << Qt::endl
                                             << "Reason:" << loader->errorString();
            delete loader;
        }
    }

    const auto staticPlugins = QPluginLoader::staticInstances();

    for (auto obj : staticPlugins)
    {
        // cppcheck-suppress knownConditionTrueFalse
        if (addPlugin(obj, nullptr))
        {
            foundPlugin = true;
        }
    }

    if (!foundPlugin)
    {
        qCWarning(DIGIKAM_GEOENGINE_LOG) << "No plugins loaded. Please check if the plugins "
                                            "were installed in the correct path, "
                                            "or if any errors occurred while loading plugins.";
    }

    m_pluginsLoaded = true;

    qCDebug(DIGIKAM_GEOENGINE_LOG) << Q_FUNC_INFO << "Time elapsed:" << t.elapsed() << "ms";
}

#ifdef Q_OS_ANDROID

void PluginManager::installPluginsFromAssets() const
{
    d->m_pluginPaths.clear();
    QStringList copyList              = MarbleDirs::pluginEntryList(QString());
    QDir pluginHome(MarbleDirs::localPath());
    pluginHome.mkpath(MarbleDirs::pluginLocalPath());
    pluginHome.setCurrent(MarbleDirs::pluginLocalPath());

    QStringList pluginNameFilter      = QStringList() << "lib*.so";
    QStringList const existingPlugins = QDir(MarbleDirs::pluginLocalPath()).entryList(pluginNameFilter, QDir::Files);

    for (const QString& existingPlugin : std::as_const(existingPlugins))
    {
        QFile::remove(existingPlugin);
    }

    for (const QString& file : std::as_const(copyList))
    {
        QString const target = MarbleDirs::pluginLocalPath() + QLatin1Char('/') + file;

        if (QFileInfo(MarbleDirs::pluginSystemPath() + QLatin1Char('/') + file).isDir())
        {
            pluginHome.mkpath(target);
        }
        else
        {
            QFile temporaryFile(MarbleDirs::pluginSystemPath() + QLatin1Char('/') + file);
            temporaryFile.copy(target);
            QFileInfo targetFile(target);
            d->m_pluginPaths << targetFile.canonicalFilePath();
        }
    }
}

#endif

} // namespace Marble

#include "moc_PluginManager.cpp"
