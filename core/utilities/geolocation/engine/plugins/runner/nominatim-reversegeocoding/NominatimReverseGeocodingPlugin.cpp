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

#include "NominatimReverseGeocodingPlugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "OsmNominatimReverseGeocodingRunner.h"

namespace Marble
{

NominatimPlugin::NominatimPlugin(QObject* const parent)
    : ReverseGeocodingRunnerPlugin(parent)
{
    setSupportedCelestialBodies(QStringList(QStringLiteral("earth")));
    setCanWorkOffline(false);
}

QString NominatimPlugin::name() const
{
    return i18n("OpenStreetMap Nominatim Reverse Geocoding");
}

QString NominatimPlugin::guiString() const
{
    return i18n("OpenStreetMap Nominatim");
}

QString NominatimPlugin::nameId() const
{
    return QStringLiteral("nominatim-reverse");
}

QString NominatimPlugin::version() const
{
    return QStringLiteral("1.0");
}

QString NominatimPlugin::description() const
{
    return i18n("A plugin to process Online reverse geocoding using the OpenStreetMap Nominatim web-service");
}

QString NominatimPlugin::copyrightYears() const
{
    return QStringLiteral("2010, 2012");
}

QVector<PluginAuthor> NominatimPlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Dennis Nienhüser"), QStringLiteral("nienhueser@kde.org"))
           << PluginAuthor(QStringLiteral("Bernhard Beschow"), QStringLiteral("bbeschow@cs.tu-berlin.de"));
}

ReverseGeocodingRunner* NominatimPlugin::newRunner() const
{
    return new OsmNominatimRunner;
}

} // namespace Marble

#include "moc_NominatimReverseGeocodingPlugin.cpp"
