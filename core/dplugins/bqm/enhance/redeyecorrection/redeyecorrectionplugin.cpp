/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to reduce red eyes
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "redeyecorrectionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "redeyecorrection.h"

namespace DigikamBqmRedEyeCorrectionPlugin
{

RedEyeCorrectionPlugin::RedEyeCorrectionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString RedEyeCorrectionPlugin::name() const
{
    return i18nc("@title", "Red Eye Correction");
}

QString RedEyeCorrectionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RedEyeCorrectionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("redeyes"));
}

QString RedEyeCorrectionPlugin::description() const
{
    return i18nc("@info", "A tool to automatically detect and correct red eye effect");
}

QString RedEyeCorrectionPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can reduce red eye effect on images.</para>");
}

QString RedEyeCorrectionPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString RedEyeCorrectionPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString RedEyeCorrectionPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> RedEyeCorrectionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Omar Amin"),
                             QString::fromUtf8("Omar dot moh dot amin at gmail dot com"),
                             QString::fromUtf8("(C) 2016"))
            ;
}

void RedEyeCorrectionPlugin::setup(QObject* const parent)
{
    RedEyeCorrection* const tool = new RedEyeCorrection(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmRedEyeCorrectionPlugin

#include "moc_redeyecorrectionplugin.cpp"
