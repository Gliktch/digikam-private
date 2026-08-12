/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to JPEG2000.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttojp2plugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttojp2.h"

namespace DigikamBqmConvertToJp2Plugin
{

ConvertToJp2Plugin::ConvertToJp2Plugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToJp2Plugin::name() const
{
    return i18nc("@title", "Convert To JP2");
}

QString ConvertToJp2Plugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToJp2Plugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-jpeg"));
}

QString ConvertToJp2Plugin::description() const
{
    return i18nc("@info", "A tool to convert images to JPEG-2000 format");
}

QString ConvertToJp2Plugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to JPEG-2000 format.</para>"
                  "<para>This is an image compression standard and coding system. It was created by the Joint Photographic Experts Group committee in 2000 with the intention of superseding their original discrete cosine transform-based JPEG standard (created in 1992) with a newly designed, wavelet-based method.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/JPEG_2000'>this page</link>.</para>");
}

QString ConvertToJp2Plugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToJp2Plugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToJp2Plugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToJp2Plugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            ;
}

void ConvertToJp2Plugin::setup(QObject* const parent)
{
    ConvertToJP2* const tool = new ConvertToJP2(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToJp2Plugin

#include "moc_converttojp2plugin.cpp"
