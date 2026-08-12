/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to fix colors balance
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorbalanceplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "colorbalance.h"

namespace DigikamBqmColorBalancePlugin
{

ColorBalancePlugin::ColorBalancePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ColorBalancePlugin::name() const
{
    return i18nc("@title", "Color Balance");
}

QString ColorBalancePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ColorBalancePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjustrgb"));
}

QString ColorBalancePlugin::description() const
{
    return i18nc("@info", "A tool to adjust color balance");
}

QString ColorBalancePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can adjust color balance from images.</para>");
}

QString ColorBalancePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ColorBalancePlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ColorBalancePlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> ColorBalancePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void ColorBalancePlugin::setup(QObject* const parent)
{
    ColorBalance* const tool = new ColorBalance(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmColorBalancePlugin

#include "moc_colorbalanceplugin.cpp"
