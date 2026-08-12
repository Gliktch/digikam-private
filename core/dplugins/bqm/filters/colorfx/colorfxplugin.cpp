/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to apply color effects
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorfxplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "colorfx.h"

namespace DigikamBqmColorFXPlugin
{

ColorFXPlugin::ColorFXPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ColorFXPlugin::name() const
{
    return i18nc("@title", "Color Effects");
}

QString ColorFXPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ColorFXPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString ColorFXPlugin::description() const
{
    return i18nc("@info", "A tool to apply color effects");
}

QString ColorFXPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can apply color effects over images.</para>");
}

QString ColorFXPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ColorFXPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ColorFXPlugin::handbookReference() const
{
    return QLatin1String("bqm-effecttools");
}

QList<DPluginAuthor> ColorFXPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Alexander Dymo"),
                             QString::fromUtf8("adymo at develop dot org"),
                             QString::fromUtf8("(C) 2012"))
            ;
}

void ColorFXPlugin::setup(QObject* const parent)
{
    ColorFX* const tool = new ColorFX(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmColorFXPlugin

#include "moc_colorfxplugin.cpp"
