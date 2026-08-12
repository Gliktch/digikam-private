/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to adjust time
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjustplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "timeadjust.h"

namespace DigikamBqmTimeAdjustPlugin
{

TimeAdjustPlugin::TimeAdjustPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString TimeAdjustPlugin::name() const
{
    return i18nc("@title", "Time Adjust");
}

QString TimeAdjustPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TimeAdjustPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("appointment-new"));
}

QString TimeAdjustPlugin::description() const
{
    return i18nc("@info", "A tool to adjust date and time-stamp from images");
}

QString TimeAdjustPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can adjust time in images.</para>");
}

QString TimeAdjustPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString TimeAdjustPlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString TimeAdjustPlugin::handbookReference() const
{
    return QLatin1String("time-adjust");
}

QList<DPluginAuthor> TimeAdjustPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2024-2025"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2025"))
            ;
}

void TimeAdjustPlugin::setup(QObject* const parent)
{
    TimeAdjust* const tool = new TimeAdjust(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmTimeAdjustPlugin

#include "moc_timeadjustplugin.cpp"
