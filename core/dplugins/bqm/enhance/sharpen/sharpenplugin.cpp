/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to sharp images
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sharpenplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "sharpen.h"

namespace DigikamBqmSharpenPlugin
{

SharpenPlugin::SharpenPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString SharpenPlugin::name() const
{
    return i18nc("@title", "Sharpen Image");
}

QString SharpenPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SharpenPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("sharpenimage"));
}

QString SharpenPlugin::description() const
{
    return i18nc("@info", "A tool to sharp images");
}

QString SharpenPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can sharp images.</para>");
}

QString SharpenPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString SharpenPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString SharpenPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> SharpenPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Matthias Welwarsky"),
                             QString::fromUtf8("matze at welwarsky dot de"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void SharpenPlugin::setup(QObject* const parent)
{
    Sharpen* const tool = new Sharpen(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmSharpenPlugin

#include "moc_sharpenplugin.cpp"
