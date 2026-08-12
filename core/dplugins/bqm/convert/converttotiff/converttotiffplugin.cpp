/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to TIFF.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttotiffplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttotiff.h"

namespace DigikamBqmConvertToTiffPlugin
{

ConvertToTiffPlugin::ConvertToTiffPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToTiffPlugin::name() const
{
    return i18nc("@title", "Convert To TIFF");
}

QString ConvertToTiffPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToTiffPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-tiff"));
}

QString ConvertToTiffPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to TIFF format");
}

QString ConvertToTiffPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to TIFF format.</para>"
                  "<para>The Tagged Image File Format is a computer file format for storing raster graphics images, popular among graphic artists, the publishing industry, and photographers.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/TIFF'>this page</link>.</para>");
}

QString ConvertToTiffPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToTiffPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToTiffPlugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToTiffPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            ;
}

void ConvertToTiffPlugin::setup(QObject* const parent)
{
    ConvertToTIFF* const tool = new ConvertToTIFF(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToTiffPlugin

#include "moc_converttotiffplugin.cpp"
