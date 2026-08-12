/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to reduce noise
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "noisereductionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "noisereduction.h"

namespace DigikamBqmNoiseReductionPlugin
{

NoiseReductionPlugin::NoiseReductionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString NoiseReductionPlugin::name() const
{
    return i18nc("@title", "Noise Reduction");
}

QString NoiseReductionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon NoiseReductionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("noisereduction"));
}

QString NoiseReductionPlugin::description() const
{
    return i18nc("@info", "A tool to remove photograph noise using wavelets");
}

QString NoiseReductionPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can reduce noise in images.</para>");
}

QString NoiseReductionPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString NoiseReductionPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString NoiseReductionPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> NoiseReductionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void NoiseReductionPlugin::setup(QObject* const parent)
{
    NoiseReduction* const tool = new NoiseReduction(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmNoiseReductionPlugin

#include "moc_noisereductionplugin.cpp"
