/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to adjust HSL
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hslcorrectionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "hslcorrection.h"

namespace DigikamBqmHSLCorrectionPlugin
{

HSLCorrectionPlugin::HSLCorrectionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString HSLCorrectionPlugin::name() const
{
    return i18nc("@title", "HSL Correction");
}

QString HSLCorrectionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HSLCorrectionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjusthsl"));
}

QString HSLCorrectionPlugin::description() const
{
    return i18nc("@info", "A tool to fix Hue / Saturation / Lightness");
}

QString HSLCorrectionPlugin::details() const
{
    return i18nc("@info", "<para>This Batch Queue Manager tool can adjust Hue / Saturation / Lightness from images.</para>");
}

QString HSLCorrectionPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString HSLCorrectionPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString HSLCorrectionPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> HSLCorrectionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void HSLCorrectionPlugin::setup(QObject* const parent)
{
    HSLCorrection* const tool = new HSLCorrection(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmHSLCorrectionPlugin

#include "moc_hslcorrectionplugin.cpp"
