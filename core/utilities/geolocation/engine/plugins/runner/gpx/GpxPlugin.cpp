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

#include "GpxPlugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "GpxRunner.h"

namespace Marble
{

GpxPlugin::GpxPlugin(QObject* const parent)
    : ParseRunnerPlugin(parent)
{
}

QString GpxPlugin::name() const
{
    return i18n("Gpx File Parser");
}

QString GpxPlugin::nameId() const
{
    return QStringLiteral("Gpx");
}

QString GpxPlugin::version() const
{
    return QStringLiteral("1.0");
}

QString GpxPlugin::description() const
{
    return i18n("A plugin to load Geolocation Data from the Gpx data File format.");
}

QString GpxPlugin::copyrightYears() const
{
    return QStringLiteral("2011");
}

QVector<PluginAuthor> GpxPlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Thibaut Gridel"), QStringLiteral("tgridel@free.fr"));
}

QString GpxPlugin::fileFormatDescription() const
{
    return i18n("GPS Data");
}

QStringList GpxPlugin::fileExtensions() const
{
    return QStringList(QStringLiteral("gpx"));
}

ParsingRunner* GpxPlugin::newRunner() const
{
    return new GpxRunner;
}

} // namespace Marble

#include "moc_GpxPlugin.cpp"
