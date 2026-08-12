/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert 16 bits color depth to 8
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "convert16to8plugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert16to8.h"

namespace DigikamBqmConvert16To8Plugin
{

Convert16To8Plugin::Convert16To8Plugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString Convert16To8Plugin::name() const
{
    return i18nc("@info", "Convert to 8 bits");
}

QString Convert16To8Plugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon Convert16To8Plugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("depth16to8"));
}

QString Convert16To8Plugin::description() const
{
    return i18nc("@info", "A tool to convert color depth to 8 bits");
}

QString Convert16To8Plugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert image color depth to 8 bits.</para>");
}

QString Convert16To8Plugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString Convert16To8Plugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString Convert16To8Plugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> Convert16To8Plugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void Convert16To8Plugin::setup(QObject* const parent)
{
    Convert16to8* const tool = new Convert16to8(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvert16To8Plugin

#include "moc_convert16to8plugin.cpp"
