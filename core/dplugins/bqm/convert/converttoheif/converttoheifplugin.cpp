/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-02
 * Description : a BQM plugin to convert to HEIF.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttoheifplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttoheif.h"

namespace DigikamBqmConvertToHeifPlugin
{

ConvertToHeifPlugin::ConvertToHeifPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToHeifPlugin::name() const
{
    return i18nc("@title", "Convert To HEIF");
}

QString ConvertToHeifPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToHeifPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString ConvertToHeifPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to HEIF format");
}

QString ConvertToHeifPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to HEIF format.</para>"
                  "<para>High Efficiency Image File Format (HEIF), also known as High Efficiency Image Coding (HEIC), "
                  "is a file format for individual images and image sequences. It was developed by the "
                  "Moving Picture Experts Group (MPEG) and it claims that twice as much information can be "
                  "stored in a HEIF image as in a JPEG image of the same size, resulting in a better quality image. "
                  "HEIF also supports animation, and is capable of storing more information than an animated GIF "
                  "at a small fraction of the size.</para>"
                  "<para>See <link url='https://en.wikipedia.org/wiki/High_Efficiency_Image_File_Format'>"
                  "High Efficiency Image File Format</link> for details.</para>");
}

QString ConvertToHeifPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToHeifPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToHeifPlugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToHeifPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2024"))
            ;
}

void ConvertToHeifPlugin::setup(QObject* const parent)
{
    ConvertToHEIF* const tool = new ConvertToHEIF(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToHeifPlugin

#include "moc_converttoheifplugin.cpp"
