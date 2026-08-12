/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to apply local contrast
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localcontrastplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "localcontrast.h"

namespace DigikamBqmLocalContrastPlugin
{

LocalContrastPlugin::LocalContrastPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString LocalContrastPlugin::name() const
{
    return i18nc("@title", "Local Contrast");
}

QString LocalContrastPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon LocalContrastPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("contrast"));
}

QString LocalContrastPlugin::description() const
{
    return i18nc("@info", "A tool to emulate tone mapping");
}

QString LocalContrastPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can emulate tone mapping over images.</para>");
}

QString LocalContrastPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString LocalContrastPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString LocalContrastPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> LocalContrastPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void LocalContrastPlugin::setup(QObject* const parent)
{
    LocalContrast* const tool = new LocalContrast(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmLocalContrastPlugin

#include "moc_localcontrastplugin.cpp"
