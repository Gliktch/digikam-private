/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to convert to PGF.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "converttopgfplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "converttopgf.h"

namespace DigikamBqmConvertToPgfPlugin
{

ConvertToPgfPlugin::ConvertToPgfPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString ConvertToPgfPlugin::name() const
{
    return i18nc("@title", "Convert To PGF");
}

QString ConvertToPgfPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ConvertToPgfPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-generic"));
}

QString ConvertToPgfPlugin::description() const
{
    return i18nc("@info", "A tool to convert images to PGF format");
}

QString ConvertToPgfPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can convert images data to PGF format.</para>"
                  "<para>The Progressive Graphics File is a wavelet-based bitmapped image format that employs lossless and lossy data compression.</para>"
                  "<para>See details about this format from <link url='https://en.wikipedia.org/wiki/Progressive_Graphics_File'>this page</link>.</para>");
}

QString ConvertToPgfPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString ConvertToPgfPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString ConvertToPgfPlugin::handbookReference() const
{
    return QLatin1String("bqm-converttools");
}

QList<DPluginAuthor> ConvertToPgfPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void ConvertToPgfPlugin::setup(QObject* const parent)
{
    ConvertToPGF* const tool = new ConvertToPGF(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmConvertToPgfPlugin

#include "moc_converttopgfplugin.cpp"
