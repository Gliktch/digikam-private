/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to mix color channels
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "channelmixerplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "channelmixer.h"

namespace DigikamBqmChannelMixerPlugin
{

ChannelMixerPlugin::ChannelMixerPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ChannelMixerPlugin::name() const
{
    return i18nc("@title", "Channel Mixer");
}

QString ChannelMixerPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ChannelMixerPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString ChannelMixerPlugin::description() const
{
    return i18nc("@info", "A tool to mix color channel");
}

QString ChannelMixerPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can mix color channels from images.</para>");
}

QString ChannelMixerPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ChannelMixerPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ChannelMixerPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> ChannelMixerPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void ChannelMixerPlugin::setup(QObject* const parent)
{
    ChannelMixer* const tool = new ChannelMixer(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmChannelMixerPlugin

#include "moc_channelmixerplugin.cpp"
