/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to add watermark
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "watermarkplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "watermark.h"

namespace DigikamBqmWatermarkPlugin
{

WaterMarkPlugin::WaterMarkPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString WaterMarkPlugin::name() const
{
    return i18nc("@title", "Add Watermark");
}

QString WaterMarkPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WaterMarkPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("insert-text"));
}

QString WaterMarkPlugin::description() const
{
    return i18nc("@info", "A tool to overlay an image or text as a visible watermark");
}

QString WaterMarkPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can add a text watermark over images.</para>");
}

QString WaterMarkPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString WaterMarkPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString WaterMarkPlugin::handbookReference() const
{
    return QLatin1String("bqm-decoratetools");
}

QList<DPluginAuthor> WaterMarkPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            << DPluginAuthor(QString::fromUtf8("Mikkel Baekhoej Christensen"),
                             QString::fromUtf8("mbc at baekhoej dot dk"),
                             QString::fromUtf8("(C) 2010"))
            << DPluginAuthor(QString::fromUtf8("Ahmed Fathi"),
                             QString::fromUtf8("ahmed dot fathi dot abdelmageed at gmail dot com"),
                             QString::fromUtf8("(C) 2017"))
            ;
}

void WaterMarkPlugin::setup(QObject* const parent)
{
    WaterMark* const tool = new WaterMark(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmWatermarkPlugin

#include "moc_watermarkplugin.cpp"
