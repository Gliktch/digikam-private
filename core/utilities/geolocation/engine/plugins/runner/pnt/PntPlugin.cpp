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

#include "PntPlugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "PntRunner.h"

namespace Marble
{

PntPlugin::PntPlugin(QObject* const parent)
    : ParseRunnerPlugin(parent)
{
}

QString PntPlugin::name() const
{
    return i18n("Pnt File Parser");
}

QString PntPlugin::nameId() const
{
    return QStringLiteral("Pnt");
}

QString PntPlugin::version() const
{
    return QStringLiteral("1.0");
}

QString PntPlugin::description() const
{
    return i18n("A plugin to load Geolocation Data from the Micro World Database II Files format");
}

QString PntPlugin::copyrightYears() const
{
    return QStringLiteral("2011");
}

QVector<PluginAuthor> PntPlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Thibaut Gridel"), QStringLiteral("tgridel@free.fr"));
}

QString PntPlugin::fileFormatDescription() const
{
    return i18n("Micro World Database II");
}

QStringList PntPlugin::fileExtensions() const
{
    return QStringList(QStringLiteral("pnt"));
}

ParsingRunner* PntPlugin::newRunner() const
{
    return new PntRunner;
}

} // namespace Marble

#include "moc_PntPlugin.cpp"
