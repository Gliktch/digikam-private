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

#include "assigncaptionsplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "assigncaptions.h"

namespace DigikamBqmAssignCaptionsPlugin
{

AssignCaptionsPlugin::AssignCaptionsPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString AssignCaptionsPlugin::name() const
{
    return i18nc("@title", "Assign Captions");
}

QString AssignCaptionsPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AssignCaptionsPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("edit-text-frame-update"));
}

QString AssignCaptionsPlugin::description() const
{
    return i18nc("@info", "A tool to assign captions metadata to images");
}

QString AssignCaptionsPlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool will assign title "
                  "and description metadata alternative values to images.</para>");
}

QString AssignCaptionsPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString AssignCaptionsPlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString AssignCaptionsPlugin::handbookReference() const
{
    return QLatin1String("bqm-assigncaptions");
}

QList<DPluginAuthor> AssignCaptionsPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2021-2025"))
            ;
}

void AssignCaptionsPlugin::setup(QObject* const parent)
{
    AssignCaptions* const tool = new AssignCaptions(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmAssignCaptionsPlugin

#include "moc_assigncaptionsplugin.cpp"
