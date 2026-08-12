/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to invert colors
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "invertplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "invert.h"

namespace DigikamBqmInvertPlugin
{

InvertPlugin::InvertPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString InvertPlugin::name() const
{
    return i18nc("@title", "Invert Colors");
}

QString InvertPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon InvertPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("edit-select-invert"));
}

QString InvertPlugin::description() const
{
    return i18nc("@info", "A tool to invert image colors");
}

QString InvertPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can invert colors from images.</para>");
}

QString InvertPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString InvertPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString InvertPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> InvertPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void InvertPlugin::setup(QObject* const parent)
{
    Invert* const tool = new Invert(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmInvertPlugin

#include "moc_invertplugin.cpp"
