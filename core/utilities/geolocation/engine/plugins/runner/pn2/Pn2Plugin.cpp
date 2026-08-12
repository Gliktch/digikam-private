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

#include "Pn2Plugin.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "Pn2Runner.h"

namespace Marble
{

Pn2Plugin::Pn2Plugin(QObject* const parent)
    : ParseRunnerPlugin(parent)
{
}

QString Pn2Plugin::name() const
{
    return i18n("Pn2 File Parser");
}

QString Pn2Plugin::nameId() const
{
    return QStringLiteral("Pn2");
}

QString Pn2Plugin::version() const
{
    return QStringLiteral("1.0");
}

QString Pn2Plugin::description() const
{
    return i18n("A plugin to load Geolocation Data from the compressed Pnt data files format");
}

QString Pn2Plugin::copyrightYears() const
{
    return QStringLiteral("2012");
}

QVector<PluginAuthor> Pn2Plugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
           << PluginAuthor(QStringLiteral("Cezar Mocan"), QStringLiteral("mocancezar@gmail.com"));
}

QString Pn2Plugin::fileFormatDescription() const
{
    return i18n("Compressed Pnt");
}

QStringList Pn2Plugin::fileExtensions() const
{
    return QStringList(QStringLiteral("pn2"));
}

ParsingRunner* Pn2Plugin::newRunner() const
{
    return new Pn2Runner;
}

} // namespace Marble

#include "moc_Pn2Plugin.cpp"
