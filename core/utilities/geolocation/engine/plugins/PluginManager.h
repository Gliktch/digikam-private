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

#pragma once

// Qt includes

#include <QObject>
#include <QList>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class RenderPlugin;
class PluginManagerPrivate;
class SearchRunnerPlugin;
class ReverseGeocodingRunnerPlugin;
class ParseRunnerPlugin;

/**
 * @short The class that handles Marble's plugins.
 *
 * Ownership policy for plugins:
 *
 * On every invocation of createNetworkPlugins and
 * createFloatItems the PluginManager creates new objects and transfers
 * ownership to the calling site. In order to create
 * the objects, the PluginManager internally has a list of the plugins
 * which are owned by the PluginManager and destroyed by it.
 *
 */
class DIGIKAM_EXPORT PluginManager : public QObject
{
    Q_OBJECT

public:

    explicit PluginManager(QObject* parent = nullptr);

    ~PluginManager()                                                                  override;

    /**
     * @brief Returns all available RenderPlugins.
     *
     * Ownership of the items remains in PluginManager.
     * In order to use the RenderPlugins, first create new instances using
     * RenderPlugin::newInstance().
     */
    QList<const RenderPlugin*> renderPlugins()                                  const;

    /**
     * @brief Add a RenderPlugin manually to the list of known plugins. Normally you
     * don't need to call this method since all plugins are loaded automatically.
     * @param plugin The plugin to add. Ownership retains with the caller.
     */
    void addRenderPlugin(const RenderPlugin* plugin);

    /**
     * Returns all search runner plugins.
     * @note: Runner plugins are owned by the PluginManager, do not delete them.
     */
    QList<const SearchRunnerPlugin*> searchRunnerPlugins()                      const;

    /**
     * @brief Add a SearchRunnerPlugin manually to the list of known plugins. Normally you
     * don't need to call this method since all plugins are loaded automatically.
     * @param plugin The plugin to add. Ownership retains with the caller.
     */
    void addSearchRunnerPlugin(const SearchRunnerPlugin* plugin);

    /**
     * Returns all reverse geocoding runner plugins.
     * @note: The runner plugins are owned by the PluginManager, do not delete them.
     */
    QList<const ReverseGeocodingRunnerPlugin*> reverseGeocodingRunnerPlugins()  const;

    /**
     * @brief Add a ReverseGeocodingRunnerPlugin manually to the list of known plugins. Normally you
     * don't need to call this method since all plugins are loaded automatically.
     * @param plugin The plugin to add. Ownership retains with the caller.
     */
    void addReverseGeocodingRunnerPlugin(const ReverseGeocodingRunnerPlugin* plugin);

    /**
     * Returns all parse runner plugins.
     * @note: The runner plugins are owned by the PluginManager, do not delete them.
     */
    QList<const ParseRunnerPlugin*> parsingRunnerPlugins()                      const;

    /**
     * @brief Add a ParseRunnerPlugin manually to the list of known plugins. Normally you
     * don't need to call this method since all plugins are loaded automatically.
     * @param plugin The plugin to add. Ownership retains with the caller.
     */
    void addParseRunnerPlugin(const ParseRunnerPlugin* plugin);

    /**
     * @brief blacklistPlugin Prevent that a plugin is loaded from the given filename
     * @param filename The name of the file (excluding prefix and file extension) to blacklist. E.g.
     * to ignore "libWikipedia.so" on Linux and "Wikipedia.dll" on Windows, pass "Wikipedia"
     */
    static void blacklistPlugin(const QString& filename);

    /**
     * @brief whitelistPlugin Add a plugin to the whitelist of plugins. If the whitelist is not
     * empty, only whitelisted plugins are loaded. If a plugin is both whitelisted and blacklisted,
     * it will not be loaded
     * @param filename The name of the file (excluding prefix and file extension) to whitelist. E.g.
     * to ignore "libWikipedia.so" on Linux and "Wikipedia.dll" on Windows, pass "Wikipedia"
     */
    static void whitelistPlugin(const QString& filename);

Q_SIGNALS:

    void renderPluginsChanged();

    void positionProviderPluginsChanged();

    void searchRunnerPluginsChanged();

    void reverseGeocodingRunnerPluginsChanged();

    void parseRunnerPluginsChanged();

private:

    Q_DISABLE_COPY(PluginManager)

#ifdef Q_OS_ANDROID

    void installPluginsFromAssets()                                             const;

#endif

private:

    PluginManagerPrivate* const d = nullptr;
};

} // namespace Marble
