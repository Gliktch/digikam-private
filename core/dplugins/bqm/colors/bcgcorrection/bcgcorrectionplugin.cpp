/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to adjust BCG
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bcgcorrectionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "bcgcorrection.h"

namespace DigikamBqmBCGCorrectionPlugin
{

BCGCorrectionPlugin::BCGCorrectionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString BCGCorrectionPlugin::name() const
{
    return i18nc("@title", "BCG Correction");
}

QString BCGCorrectionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BCGCorrectionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("contrast"));
}

QString BCGCorrectionPlugin::description() const
{
    return i18nc("@info", "A tool to fix Brightness / Contrast / Gamma");
}

QString BCGCorrectionPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can adjust Brightness / Contrast / Gamma from images.</para>");
}

QString BCGCorrectionPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString BCGCorrectionPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString BCGCorrectionPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> BCGCorrectionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void BCGCorrectionPlugin::setup(QObject* const parent)
{
    BCGCorrection* const tool = new BCGCorrection(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmBCGCorrectionPlugin

#include "moc_bcgcorrectionplugin.cpp"
