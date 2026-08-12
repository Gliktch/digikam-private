/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert 8 bits color depth to 16
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "convert8to16plugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "convert8to16.h"

namespace DigikamBqmConvert8To16Plugin
{

Convert8To16Plugin::Convert8To16Plugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString Convert8To16Plugin::name() const
{
    return i18nc("@title", "Convert to 16 bits");
}

QString Convert8To16Plugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon Convert8To16Plugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("depth8to16"));
}

QString Convert8To16Plugin::description() const
{
    return i18nc("@info", "A tool to convert color depth to 16 bits");
}

QString Convert8To16Plugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images color depth to 16 bits.</para>");
}

QString Convert8To16Plugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString Convert8To16Plugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString Convert8To16Plugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> Convert8To16Plugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void Convert8To16Plugin::setup(QObject* const parent)
{
    Convert8to16* const tool = new Convert8to16(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvert8To16Plugin

#include "moc_convert8to16plugin.cpp"
