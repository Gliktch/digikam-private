/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to fix colors automatically
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autocorrectionplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "autocorrection.h"

namespace DigikamBqmAutoCorrectionPlugin
{

AutoCorrectionPlugin::AutoCorrectionPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString AutoCorrectionPlugin::name() const
{
    return i18nc("@title", "Color Auto-Correction");
}

QString AutoCorrectionPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AutoCorrectionPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("autocorrection"));
}

QString AutoCorrectionPlugin::description() const
{
    return i18nc("@info", "A tool to fix colors automatically");
}

QString AutoCorrectionPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can adjust colors automatically from images.</para>");
}

QString AutoCorrectionPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString AutoCorrectionPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString AutoCorrectionPlugin::handbookReference() const
{
    return QLatin1String("bqm-colortools");
}

QList<DPluginAuthor> AutoCorrectionPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            ;
}

void AutoCorrectionPlugin::setup(QObject* const parent)
{
    AutoCorrection* const tool = new AutoCorrection(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmAutoCorrectionPlugin

#include "moc_autocorrectionplugin.cpp"
