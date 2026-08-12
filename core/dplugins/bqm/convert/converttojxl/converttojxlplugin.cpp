/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-03
 * Description : a BQM plugin to convert to JXL.
 *
 * SPDX-FileCopyrightText: 2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttojxlplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttojxl.h"

namespace DigikamBqmConvertToJxlPlugin
{

ConvertToJxlPlugin::ConvertToJxlPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToJxlPlugin::name() const
{
    return i18nc("@title", "Convert To JXL");
}

QString ConvertToJxlPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToJxlPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString ConvertToJxlPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to JXL format");
}

QString ConvertToJxlPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to JPEG-XL format.</para>"
                  "<para>JPEG-XL is a royalty-free raster-graphics file format that supports both lossy and lossless compression. It is designed to outperform existing raster formats and thus to become their universal replacement.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/JPEG_XL'>this page</link>.</para>");
}

QString ConvertToJxlPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToJxlPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToJxlPlugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToJxlPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2022-2024"))
            ;
}

void ConvertToJxlPlugin::setup(QObject* const parent)
{
    ConvertToJXL* const tool = new ConvertToJXL(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToJxlPlugin

#include "moc_converttojxlplugin.cpp"
