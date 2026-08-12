/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-03
 * Description : a BQM plugin to convert to AVIF.
 *
 * SPDX-FileCopyrightText: 2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttoavifplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttoavif.h"

namespace DigikamBqmConvertToAvifPlugin
{

ConvertToAvifPlugin::ConvertToAvifPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToAvifPlugin::name() const
{
    return i18nc("@title", "Convert To AVIF");
}

QString ConvertToAvifPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToAvifPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString ConvertToAvifPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to AVIF format");
}

QString ConvertToAvifPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to AVIF format.</para>"
                  "<para>AV1 Image File Format is an image file format specification for storing images or image sequences compressed with AV1 "
                  "in the ISOBMFF container format.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/AVIF'>this page</link>.</para>");
}

QString ConvertToAvifPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToAvifPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToAvifPlugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToAvifPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2022-2024"))
            ;
}

void ConvertToAvifPlugin::setup(QObject* const parent)
{
    ConvertToAVIF* const tool = new ConvertToAVIF(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToAvifPlugin

#include "moc_converttoavifplugin.cpp"
