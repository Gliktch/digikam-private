/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to color space
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccconvertplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "iccconvert.h"

namespace DigikamBqmIccConvertPlugin
{

IccConvertPlugin::IccConvertPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString IccConvertPlugin::name() const
{
    return i18nc("@title", "Color Profile Conversion");
}

QString IccConvertPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon IccConvertPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("preferences-desktop-display-color"));
}

QString IccConvertPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to a color space");
}

QString IccConvertPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images to a different color space.</para>");
}

QString IccConvertPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString IccConvertPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString IccConvertPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> IccConvertPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void IccConvertPlugin::setup(QObject* const parent)
{
    IccConvert* const tool = new IccConvert(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmIccConvertPlugin

#include "moc_iccconvertplugin.cpp"
