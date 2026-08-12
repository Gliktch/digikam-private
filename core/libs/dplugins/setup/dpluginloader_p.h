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

#pragma once

#include "dpluginloader.h"

// Qt includes

#include <QDir>
#include <QPluginLoader>

namespace Digikam
{

class Q_DECL_HIDDEN DPluginLoader::Private
{
public:

    Private();
    ~Private() = default;

    /**
     * Try to find plugin files from Qt5 plugins install dir:
     */
    QFileInfoList pluginEntriesList() const;

    /**
     * Stage to load plugins in memory
     */
    void loadPlugins();

    /**
     * Append object to the given plugins list.
     */
    bool appendPlugin(QObject* const obj,
                      QPluginLoader* const loader);

public:

    bool                  pluginsLoaded = false;
    QList<DPlugin*>       allPlugins;
    QStringList           blacklist;
    QStringList           whitelist;
    QStringList           DKBlacklist;      ///< Showfoto specific plugins to ignore in digiKam.
};

} // namespace Digikam
