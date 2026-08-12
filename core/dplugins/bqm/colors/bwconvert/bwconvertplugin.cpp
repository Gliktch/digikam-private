/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to Black and White
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bwconvertplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "bwconvert.h"

namespace DigikamBqmBWConvertPlugin
{

BWConvertPlugin::BWConvertPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString BWConvertPlugin::name() const
{
    return i18nc("@title", "Black and White Convert");
}

QString BWConvertPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BWConvertPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bwtonal"));
}

QString BWConvertPlugin::description() const
{
    return i18nc("@info", "A tool to convert to black and white");
}

QString BWConvertPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images to black and white.</para>");
}

QString BWConvertPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString BWConvertPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString BWConvertPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> BWConvertPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void BWConvertPlugin::setup(QObject* const parent)
{
    BWConvert* const tool = new BWConvert(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmBWConvertPlugin

#include "moc_bwconvertplugin.cpp"
