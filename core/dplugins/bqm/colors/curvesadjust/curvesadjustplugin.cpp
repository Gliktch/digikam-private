/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to adjust color curves
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "curvesadjustplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "curvesadjust.h"

namespace DigikamBqmCurvesAdjustPlugin
{

CurvesAdjustPlugin::CurvesAdjustPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString CurvesAdjustPlugin::name() const
{
    return i18nc("@title", "Curves Adjust");
}

QString CurvesAdjustPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CurvesAdjustPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString CurvesAdjustPlugin::description() const
{
    return i18nc("@info", "A tool to adjust color curves");
}

QString CurvesAdjustPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can adjust the color curves from images.</para>");
}

QString CurvesAdjustPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString CurvesAdjustPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString CurvesAdjustPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> CurvesAdjustPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2024"))
            ;
}

void CurvesAdjustPlugin::setup(QObject* const parent)
{
    CurvesAdjust* const tool = new CurvesAdjust(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmCurvesAdjustPlugin

#include "moc_curvesadjustplugin.cpp"
