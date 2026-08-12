/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to apply film grain
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filmgrainplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "filmgrain.h"

namespace DigikamBqmFilmGrainPlugin
{

FilmGrainPlugin::FilmGrainPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString FilmGrainPlugin::name() const
{
    return i18nc("@title", "Film Grain");
}

QString FilmGrainPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FilmGrainPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("filmgrain"));
}

QString FilmGrainPlugin::description() const
{
    return i18nc("@info", "A tool to add film grain");
}

QString FilmGrainPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can add film grain over images.</para>");
}

QString FilmGrainPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString FilmGrainPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString FilmGrainPlugin::handbookReference() const
{
    return QLatin1String("bqm-effecttools");
}

QList<DPluginAuthor> FilmGrainPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void FilmGrainPlugin::setup(QObject* const parent)
{
    FilmGrain* const tool = new FilmGrain(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmFilmGrainPlugin

#include "moc_filmgrainplugin.cpp"
