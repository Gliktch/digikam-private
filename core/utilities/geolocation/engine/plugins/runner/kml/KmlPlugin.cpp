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

#include "KmlPlugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "KmlRunner.h"

namespace Marble
{

KmlPlugin::KmlPlugin(QObject* const parent)
    : ParseRunnerPlugin(parent)
{
}

QString KmlPlugin::name() const
{
    return i18n("KML File Parser");
}

QString KmlPlugin::nameId() const
{
    return QStringLiteral("Kml");
}

QString KmlPlugin::version() const
{
    return QStringLiteral("1.2");
}

QString KmlPlugin::description() const
{
    return i18n("A plugin to load Geolocation Data from the KML and KMZ data file format.");
}

QString KmlPlugin::copyrightYears() const
{
    return QStringLiteral("2011, 2013, 2015");
}

QVector<PluginAuthor> KmlPlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Thibaut Gridel"), QStringLiteral("tgridel@free.fr"))
           << PluginAuthor(QStringLiteral("Dennis Nienhüser"), QStringLiteral("nienhueser@kde.org"));
}

QString KmlPlugin::fileFormatDescription() const
{
    return i18n("Google Earth KML");
}

QStringList KmlPlugin::fileExtensions() const
{
    return QStringList() << QStringLiteral("kml")
           << QStringLiteral("kmz");
}

ParsingRunner* KmlPlugin::newRunner() const
{
    return new KmlRunner;
}

} // namespace Marble

#include "moc_KmlPlugin.cpp"
