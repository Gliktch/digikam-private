/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce vignetting
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "antivignettingtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "antivignettingtool.h"

namespace DigikamEditorAntivignettingToolPlugin
{

AntiVignettingToolPlugin::AntiVignettingToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString AntiVignettingToolPlugin::name() const
{
    return i18nc("@title", "Vignetting Correction");
}

QString AntiVignettingToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AntiVignettingToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("antivignetting"));
}

QString AntiVignettingToolPlugin::description() const
{
    return i18nc("@info", "A tool to correct vignetting");
}

QString AntiVignettingToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool correct vignetting from an image.");
}

QString AntiVignettingToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString AntiVignettingToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString AntiVignettingToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-vignetting");
}

QList<DPluginAuthor> AntiVignettingToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Julien Narboux"),
                             QString::fromUtf8("julien at narboux dot fr"),
                             QString::fromUtf8("(C) 2010"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void AntiVignettingToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Vignetting Correction..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_antivignetting"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAntiVignetting()));

    addAction(ac);
}

void AntiVignettingToolPlugin::slotAntiVignetting()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        AntiVignettingTool* const tool = new AntiVignettingTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorAntivignettingToolPlugin

#include "moc_antivignettingtoolplugin.cpp"
