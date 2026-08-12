/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-07
 * Description : a BQM plugin to fix hot pixels
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hotpixelsplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "hotpixels.h"

namespace DigikamBqmHotPixelsPlugin
{

HotPixelsPlugin::HotPixelsPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString HotPixelsPlugin::name() const
{
    return i18nc("@title", "Hot Pixels");
}

QString HotPixelsPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HotPixelsPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("hotpixels"));
}

QString HotPixelsPlugin::description() const
{
    return i18nc("@info", "A tool to fix hot pixels");
}

QString HotPixelsPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can fix hot pixels in images.</para>");
}

QString HotPixelsPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString HotPixelsPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString HotPixelsPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> HotPixelsPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Unai Garro"),
                             QString::fromUtf8("ugarro at users dot sourceforge dot net"),
                             QString::fromUtf8("(C) 2005-2006"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void HotPixelsPlugin::setup(QObject* const parent)
{
    HotPixels* const tool = new HotPixels(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmHotPixelsPlugin

#include "moc_hotpixelsplugin.cpp"
