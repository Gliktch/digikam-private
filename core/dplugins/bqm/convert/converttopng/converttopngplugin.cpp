/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to PNG.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttopngplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttopng.h"

namespace DigikamBqmConvertToPngPlugin
{

ConvertToPngPlugin::ConvertToPngPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToPngPlugin::name() const
{
    return i18nc("@title", "Convert To PNG");
}

QString ConvertToPngPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToPngPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-png"));
}

QString ConvertToPngPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to PNG format");
}

QString ConvertToPngPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to PNG format.</para>"
                  "<para>The Portable Network Graphics is a raster-graphics file-format that supports lossless data compression.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/Portable_Network_Graphics'>this page</link>.</para>");
}

QString ConvertToPngPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToPngPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToPngPlugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToPngPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            ;
}

void ConvertToPngPlugin::setup(QObject* const parent)
{
    ConvertToPNG* const tool = new ConvertToPNG(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToPngPlugin

#include "moc_converttopngplugin.cpp"
