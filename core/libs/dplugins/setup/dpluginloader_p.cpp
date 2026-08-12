/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time: private container
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dpluginloader_p.h"

// Qt includes

#include <QStringList>
#include <QElapsedTimer>
#include <QDirIterator>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLibraryInfo>
#include <QMetaProperty>
#include <QFileInfo>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_version.h"

namespace Digikam
{

DPluginLoader::Private::Private()
{
    // Do not load these plugins as they are not currently working.

    blacklist   << QLatin1String("Generic_FaceBook_Plugin");
    blacklist   << QLatin1String("Generic_IpFs_Plugin");
    blacklist   << QLatin1String("Generic_Rajce_Plugin");
    DKBlacklist << QLatin1String("Generic_DNGConverter_Plugin");
    blacklist   << QLatin1String("Generic_YandexFotki_Plugin");
    blacklist   << QLatin1String("Generic_VKontakte_Plugin");
    blacklist   << QLatin1String("Generic_Google_Plugin");

    // Ignore Geolocation plugin which uses a dedicated loader.

    blacklist   << QLatin1String("AtmospherePlugin");
    blacklist   << QLatin1String("CompassFloatItem");
    blacklist   << QLatin1String("CrosshairsPlugin");
    blacklist   << QLatin1String("CachePlugin");
    blacklist   << QLatin1String("GpsbabelPlugin");
    blacklist   << QLatin1String("GpxPlugin");
    blacklist   << QLatin1String("GraticulePlugin");
    blacklist   << QLatin1String("JsonPlugin");
    blacklist   << QLatin1String("KmlPlugin");
    blacklist   << QLatin1String("MapScaleFloatItem");
    blacklist   << QLatin1String("MeasureTool");
    blacklist   << QLatin1String("NavigationFloatItem");
    blacklist   << QLatin1String("NominatimReverseGeocodingPlugin");
    blacklist   << QLatin1String("OverviewMap");
    blacklist   << QLatin1String("OsmPlugin");
    blacklist   << QLatin1String("Pn2Plugin");
    blacklist   << QLatin1String("PntPlugin");
    blacklist   << QLatin1String("ProgressFloatItem");
}

QFileInfoList DPluginLoader::Private::pluginEntriesList() const
{
    QStringList pathList;

    // First we try to load in first the local plugin if DK_PLUG_PATH variable is declared.
    // Else, we will load plugins from the system using the standard Qt plugin path.

    QByteArray  dkenv = qgetenv("DK_PLUGIN_PATH");

    if (dkenv.isEmpty())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        pathList << QLibraryInfo::path(QLibraryInfo::PluginsPath) + QLatin1String("/digikam/");

#else

        pathList << QLibraryInfo::location(QLibraryInfo::PluginsPath) + QLatin1String("/digikam/");

#endif

    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "DK_PLUGIN_PATH env.variable detected. "
                                          "We will use it to load plugin...";

        pathList << QString::fromUtf8(dkenv).split(QLatin1Char(';'),
                                                   Qt::SkipEmptyParts);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Parsing plugins from" << pathList;

#ifdef Q_OS_MACOS

    QString filter(QLatin1String("*.dylib *.so"));

#elif defined Q_OS_WIN

    QString filter(QLatin1String("*.dll"));

#else

    QString filter(QLatin1String("*.so"));

#endif

    QFileInfoList allFiles;
    QStringList   dupFiles;

    for (const QString& path : std::as_const(pathList))
    {
        QDir dir(path, filter, QDir::Unsorted,
                 QDir::Files | QDir::NoDotAndDotDot);

        QDirIterator it(dir, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            it.next();

            if (
                !it.filePath().contains(QLatin1String("dSYM"))   &&  // Ignore debug binary extensions under macOS
                !it.filePath().contains(QLatin1String("marble")) &&  // Ignore Marble plugins.
                !dupFiles.contains(it.fileInfo().baseName())
               )
            {
                dupFiles << it.fileInfo().baseName();

                if (it.fileInfo().baseName().startsWith(QLatin1String("DImg_")))
                {
                    allFiles.prepend(it.fileInfo());
                }
                else
                {
                    allFiles.append(it.fileInfo());
                }
            }
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Plugins found:" << allFiles.count();

    return allFiles;
}

bool DPluginLoader::Private::appendPlugin(QObject* const obj,
                                          QPluginLoader* const loader)
{
    DPlugin* const plugin = qobject_cast<DPlugin*>(obj);

    if (plugin)
    {
        Q_ASSERT(obj->metaObject()->superClass()); // all our plugins have a super class

        if (plugin->version() == QLatin1String(digikam_version_short))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Plugin of type" << obj->metaObject()->superClass()->className()
                                         << "loaded from"    << loader->fileName();

            KSharedConfigPtr config = KSharedConfig::openConfig();
            KConfigGroup group      = config->group(DPluginLoader::instance()->configGroupName());

            plugin->setShouldLoaded(group.readEntry(plugin->iid(), true));
            plugin->setLibraryFileName(loader->fileName());

            allPlugins << plugin;

            return true;
        }
    }

    return false;
}

void DPluginLoader::Private::loadPlugins()
{
    if (pluginsLoaded)
    {
        return;
    }

    QElapsedTimer t;
    t.start();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Starting to load external tools.";

    Q_ASSERT(allPlugins.isEmpty());

    const auto plist = pluginEntriesList();

    for (const QFileInfo& info : plist)
    {
        if (!whitelist.isEmpty() && !whitelist.contains(info.baseName()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Ignoring non-whitelisted plugin" << info.filePath();
            continue;
        }

        if (blacklist.contains(info.baseName()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Ignoring blacklisted plugin" << info.filePath();
            continue;
        }

        if (
            (qApp->applicationName() == QLatin1String("showfoto")) &&
            (info.baseName().startsWith(QLatin1String("Bqm_")))
           )
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Ignoring specific digiKam BQM plugin in Showfoto" << info.filePath();
            continue;
        }

        if (
            (qApp->applicationName() == QLatin1String("digikam")) &&
            (DKBlacklist.contains(info.baseName()))
           )
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Ignoring specific Showfoto plugin in digiKam" << info.filePath();

            continue;
        }
/*
        qCDebug(DIGIKAM_GENERAL_LOG) << info.baseName() << "-" << info.canonicalPath();
*/
        const QString path          = info.canonicalFilePath();
        QPluginLoader* const loader = new QPluginLoader(path, DPluginLoader::instance());
        QObject* const obj          = loader->instance();

        if (obj)
        {
            bool isPlugin = appendPlugin(obj, loader);

            if (!isPlugin)
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Ignoring the following plugin since it could not be loaded:"
                                               << path;

                qCDebug(DIGIKAM_GENERAL_LOG) << "External plugin failure:" << path
                                             << "is a plugin, but it does not implement the"
                                             << "right interface or it was compiled against"
                                             << "an old version of digiKam. Ignoring it.";
                delete loader;
            }
        }
        else
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Ignoring to load the following file since it does not look like "
                                              "a valid digiKam external plugin:" << path << Qt::endl
                                           << "Reason:" << loader->errorString();
            delete loader;
        }
    }

    if (allPlugins.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "No plugins loaded. Please check if the plugins were installed in the correct path,"
                                       << "or if any errors occurred while loading plugins.";
    }

    pluginsLoaded = true;

    qCDebug(DIGIKAM_GENERAL_LOG) << Q_FUNC_INFO << "Time elapsed:" << t.elapsed() << "ms";
}

} // namespace Digikam
