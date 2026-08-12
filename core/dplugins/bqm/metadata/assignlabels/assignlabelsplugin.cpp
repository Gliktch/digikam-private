/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-09-23
 * Description : a BQM plugin to assign captions metadata.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assignlabelsplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "assignlabels.h"

namespace DigikamBqmAssignLabelsPlugin
{

AssignLabelsPlugin::AssignLabelsPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString AssignLabelsPlugin::name() const
{
    return i18nc("@title", "Assign Labels");
}

QString AssignLabelsPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AssignLabelsPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("rating"));
}

QString AssignLabelsPlugin::description() const
{
    return i18nc("@info", "A tool to assign labels metadata to images");
}

QString AssignLabelsPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool will assign rating, or color, or pick label metadata values to images.</para>");
}

QString AssignLabelsPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString AssignLabelsPlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString AssignLabelsPlugin::handbookReference() const
{
    return QLatin1String("bqm-assignlabels");
}

QList<DPluginAuthor> AssignLabelsPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2021-2025"))
            ;
}

void AssignLabelsPlugin::setup(QObject* const parent)
{
    AssignLabels* const tool = new AssignLabels(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmAssignLabelsPlugin

#include "moc_assignlabelsplugin.cpp"
