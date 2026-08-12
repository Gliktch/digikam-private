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

#include "JsonPlugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "JsonRunner.h"

namespace Marble
{

JsonPlugin::JsonPlugin(QObject* const parent)
    : ParseRunnerPlugin(parent)
{
}

QString JsonPlugin::name() const
{
    return i18n("GeoJSON File Parser");
}

QString JsonPlugin::nameId() const
{
    return QStringLiteral("GeoJSON");
}

QString JsonPlugin::version() const
{
    return QStringLiteral("2.0");
}

QString JsonPlugin::description() const
{
    return i18n("A plugin to load Geolocation Data from the GeoJSON data file format.");
}

QString JsonPlugin::copyrightYears() const
{
    return QStringLiteral("2012, 2019");
}

QVector<PluginAuthor> JsonPlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Ander Pijoan"), QStringLiteral("ander.pijoan@deusto.es"))
           << PluginAuthor(QStringLiteral("John Zaitseff"), QStringLiteral("J.Zaitseff@zap.org.au"));
}

QString JsonPlugin::fileFormatDescription() const
{
    return i18n("GeoJSON");
}

QStringList JsonPlugin::fileExtensions() const
{
    return QStringList() << QStringLiteral("json")
           << QStringLiteral("geojson");
}

ParsingRunner* JsonPlugin::newRunner() const
{
    return new JsonRunner;
}

} // namespace Marble

#include "moc_JsonPlugin.cpp"
