/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to fix white balance
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "whitebalanceplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "whitebalance.h"

namespace DigikamBqmWhiteBalancePlugin
{

WhiteBalancePlugin::WhiteBalancePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString WhiteBalancePlugin::name() const
{
    return i18nc("@title", "White Balance");
}

QString WhiteBalancePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WhiteBalancePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("whitebalance"));
}

QString WhiteBalancePlugin::description() const
{
    return i18nc("@info", "A tool to adjust white balance");
}

QString WhiteBalancePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can adjust the white balance from images.</para>");
}

QString WhiteBalancePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString WhiteBalancePlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString WhiteBalancePlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> WhiteBalancePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void WhiteBalancePlugin::setup(QObject* const parent)
{
    WhiteBalance* const tool = new WhiteBalance(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmWhiteBalancePlugin

#include "moc_whitebalanceplugin.cpp"
