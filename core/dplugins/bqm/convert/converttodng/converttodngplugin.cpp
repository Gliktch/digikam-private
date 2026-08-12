/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to DNG.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttodngplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttodng.h"

namespace DigikamBqmConvertToDngPlugin
{

ConvertToDngPlugin::ConvertToDngPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToDngPlugin::name() const
{
    return i18nc("@title", "Convert RAW To DNG");
}

QString ConvertToDngPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToDngPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString ConvertToDngPlugin::description() const
{
    return i18nc("@info", "A tool to convert RAW images to DNG container");
}

QString ConvertToDngPlugin::details() const
{
    return xi18nc("@info", "<para>This Generic tool can convert RAW images data to DNG format.</para>"
                  "<para>The Digital Negative is a lossless RAW image format created by Adobe.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/Digital_Negative'>this page</link>.</para>");
}

QString ConvertToDngPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToDngPlugin::handbookChapter() const
{
    return QLatin1String("dng_converter");
}

QList<DPluginAuthor> ConvertToDngPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            << DPluginAuthor(QString::fromUtf8("Jens Mueller"),
                             QString::fromUtf8("tschenser at gmx dot de"),
                             QString::fromUtf8("(C) 2010-2011"))
            << DPluginAuthor(QString::fromUtf8("Smit Mehta"),
                             QString::fromUtf8("smit dot meh at gmail dot com"),
                             QString::fromUtf8("(C) 2012"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2020-2024"))
            ;
}

void ConvertToDngPlugin::setup(QObject* const parent)
{
    ConvertToDNG* const tool = new ConvertToDNG(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToDngPlugin

#include "moc_converttodngplugin.cpp"
