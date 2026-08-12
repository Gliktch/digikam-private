/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to jpeg.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttojpegplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttojpeg.h"

namespace DigikamBqmConvertToJpegPlugin
{

ConvertToJpegPlugin::ConvertToJpegPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToJpegPlugin::name() const
{
    return i18nc("@title", "Convert To JPEG");
}

QString ConvertToJpegPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToJpegPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-jpeg"));
}

QString ConvertToJpegPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to JPEG format");
}

QString ConvertToJpegPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to JPEG format.</para>"
                  "<para>The Joint Photographic Experts Group format is a commonly used method of lossy compression for digital images, particularly for those images produced by digital photography.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/JPEG'>this page</link>.</para>");
}

QString ConvertToJpegPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToJpegPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToJpegPlugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToJpegPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            ;
}

void ConvertToJpegPlugin::setup(QObject* const parent)
{
    ConvertToJPEG* const tool = new ConvertToJPEG(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToJpegPlugin

#include "moc_converttojpegplugin.cpp"
