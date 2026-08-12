/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to correct vignetting
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "antivignettingplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "antivignetting.h"

namespace DigikamBqmAntiVignettingPlugin
{

AntiVignettingPlugin::AntiVignettingPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString AntiVignettingPlugin::name() const
{
    return i18nc("@title", "Anti-Vignetting");
}

QString AntiVignettingPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AntiVignettingPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("antivignetting"));
}

QString AntiVignettingPlugin::description() const
{
    return i18nc("@info", "A tool to adjust vignetting to photograph");
}

QString AntiVignettingPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool remove or add vignettings to images.</para>");
}

QString AntiVignettingPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString AntiVignettingPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString AntiVignettingPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> AntiVignettingPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void AntiVignettingPlugin::setup(QObject* const parent)
{
    AntiVignetting* const tool = new AntiVignetting(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmAntiVignettingPlugin

#include "moc_antivignettingplugin.cpp"
