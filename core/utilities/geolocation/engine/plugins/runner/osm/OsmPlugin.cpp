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

#include "OsmPlugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "OsmRunner.h"

namespace Marble
{

OsmPlugin::OsmPlugin(QObject* const parent)
    : ParseRunnerPlugin(parent)
{
}

QString OsmPlugin::name() const
{
    return i18n("Osm File Parser");
}

QString OsmPlugin::nameId() const
{
    return QStringLiteral("Osm");
}

QString OsmPlugin::version() const
{
    return QStringLiteral("1.0");
}

QString OsmPlugin::description() const
{
    return i18n("A plugin to load Geolocation Data from the OpenStreetMap data file format.");
}

QString OsmPlugin::copyrightYears() const
{
    return QStringLiteral("2011, 2016");
}

QVector<PluginAuthor> OsmPlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Thibaut Gridel"), QStringLiteral("tgridel@free.fr"))
           << PluginAuthor(QStringLiteral("Dennis Nienhüser"), QStringLiteral("nienhueser@kde.org"));
}

QString OsmPlugin::fileFormatDescription() const
{
    return i18n("OpenStreetMap Data");
}

QStringList OsmPlugin::fileExtensions() const
{
    return QStringList() << QStringLiteral("osm") << QStringLiteral("osm.zip") << QStringLiteral("o5m") << QStringLiteral("osm.pbf");
}

ParsingRunner* OsmPlugin::newRunner() const
{
    return new OsmRunner;
}

} // namespace Marble

#include "moc_OsmPlugin.cpp"
