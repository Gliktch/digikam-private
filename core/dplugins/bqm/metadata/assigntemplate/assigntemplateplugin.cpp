/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to assign template
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assigntemplateplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "assigntemplate.h"

namespace DigikamBqmAssignTemplatePlugin
{

AssignTemplatePlugin::AssignTemplatePlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString AssignTemplatePlugin::name() const
{
    return i18nc("@title", "Apply Template");
}

QString AssignTemplatePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AssignTemplatePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("text-xml"));
}

QString AssignTemplatePlugin::description() const
{
    return i18nc("@info", "A tool to apply metadata template to images");
}

QString AssignTemplatePlugin::details() const
{
    return xi18nc("@info", "<para>This Batch Queue Manager tool can apply metadata template over images.</para>");
}

QString AssignTemplatePlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString AssignTemplatePlugin::handbookChapter() const
{
    return QLatin1String("metadata_tools");
}

QString AssignTemplatePlugin::handbookReference() const
{
    return QLatin1String("bqm-assigntemplate");
}

QList<DPluginAuthor> AssignTemplatePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2026"))
            ;
}

void AssignTemplatePlugin::setup(QObject* const parent)
{
    AssignTemplate* const tool = new AssignTemplate(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmAssignTemplatePlugin

#include "moc_assigntemplateplugin.cpp"
