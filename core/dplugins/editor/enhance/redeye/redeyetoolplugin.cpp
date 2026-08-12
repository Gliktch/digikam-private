/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to restore an image
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "redeyetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "redeyetool.h"

namespace DigikamEditorRedEyeToolPlugin
{

RedEyeToolPlugin::RedEyeToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString RedEyeToolPlugin::name() const
{
    return i18nc("@title", "Red Eye");
}

QString RedEyeToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RedEyeToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("redeyes"));
}

QString RedEyeToolPlugin::description() const
{
    return i18nc("@info", "A tool to automatically detect and correct red eye effect");
}

QString RedEyeToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can reduce red eye effect on image.");
}

QString RedEyeToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString RedEyeToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString RedEyeToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-redeyes");
}

QList<DPluginAuthor> RedEyeToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2005"))

            << DPluginAuthor(QString::fromUtf8("Omar Amin"),
                             QString::fromUtf8("Omar dot moh dot amin at gmail dot com"),
                             QString::fromUtf8("(C) 2016"))

            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void RedEyeToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Red Eye..."));
    ac->setWhatsThis(i18nc("@info", "This filter can be used to correct red eyes in a photo. "
                           "Select a region including the eyes to use this option."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_redeye"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRedEye()));

    addAction(ac);
}

void RedEyeToolPlugin::slotRedEye()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RedEyeTool* const tool = new RedEyeTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorRedEyeToolPlugin

#include "moc_redeyetoolplugin.cpp"
