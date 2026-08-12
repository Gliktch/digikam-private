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

#include "GpsbabelPlugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "GpsbabelRunner.h"

namespace Marble
{

GpsbabelPlugin::GpsbabelPlugin(QObject* const parent)
    : ParseRunnerPlugin(parent)
{
}

QString GpsbabelPlugin::name() const
{
    return i18n("GPSBabel NMEA File Parser");
}

QString GpsbabelPlugin::nameId() const
{
    return QStringLiteral("GPSBabel");
}

QString GpsbabelPlugin::version() const
{
    return QStringLiteral("1.0");
}

QString GpsbabelPlugin::description() const
{
    return i18n("A plugin to load Geolocation Data from NMEA files through GPSBabel command line tool.");
}

QString GpsbabelPlugin::copyrightYears() const
{
    return QStringLiteral("2013");
}

QVector<PluginAuthor> GpsbabelPlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Mohammed Nafees"), QStringLiteral("nafees.technocool@gmail.com"));
}

QString GpsbabelPlugin::fileFormatDescription() const
{
    return i18n("GPSBabel files");
}

QStringList GpsbabelPlugin::fileExtensions() const
{
    return QStringList()
           << QStringLiteral("nmea")
           << QStringLiteral("igc")
           << QStringLiteral("tiger")
           << QStringLiteral("ov2")
           << QStringLiteral("garmin")
           << QStringLiteral("csv")
           << QStringLiteral("magellan");
}

ParsingRunner* GpsbabelPlugin::newRunner() const
{
    return new GpsbabelRunner;
}

} // namespace Marble

#include "moc_GpsbabelPlugin.cpp"
