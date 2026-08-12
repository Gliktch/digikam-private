/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to add border
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "borderplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "border.h"

namespace DigikamBqmBorderPlugin
{

BorderPlugin::BorderPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString BorderPlugin::name() const
{
    return i18nc("@title", "Add Border");
}

QString BorderPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BorderPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bordertool"));
}

QString BorderPlugin::description() const
{
    return i18nc("@info", "A tool to add a border around images");
}

QString BorderPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can add decorative border around images.</para>");
}

QString BorderPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString BorderPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString BorderPlugin::handbookReference() const
{
    return QLatin1String("bqm-decoratetools");
}

QList<DPluginAuthor> BorderPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void BorderPlugin::setup(QObject* const parent)
{
    Border* const tool = new Border(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmBorderPlugin

#include "moc_borderplugin.cpp"
