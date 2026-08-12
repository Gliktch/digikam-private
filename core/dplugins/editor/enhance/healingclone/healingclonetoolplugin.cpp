/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce image artifacts
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019      by Ahmed Fathi <ahmed dot fathi dot abdelmageed at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "healingclonetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "healingclonetool.h"

namespace DigikamEditorHealingCloneToolPlugin
{

HealingCloneToolPlugin::HealingCloneToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString HealingCloneToolPlugin::name() const
{
    return i18nc("@title", "Healing Clone Tool");
}

QString HealingCloneToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HealingCloneToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("edit-clone"));
}

QString HealingCloneToolPlugin::description() const
{
    return i18nc("@info", "A tool to fix image artifacts");
}

QString HealingCloneToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can fix image artifacts by cloning area.");
}

QString HealingCloneToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString HealingCloneToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString HealingCloneToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-clone");
}

QList<DPluginAuthor> HealingCloneToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Shaza Ismail Kaoud"),
                             QString::fromUtf8("shaza dot ismail dot k at gmail dot com"),
                             QString::fromUtf8("(C) 2017"))
            << DPluginAuthor(QString::fromUtf8("Ahmed Fathi"),
                             QString::fromUtf8("ahmed dot fathi dot abdelmageed at gmail dot com"),
                             QString::fromUtf8("(C) 2019"))
            ;
}

void HealingCloneToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Healing Clone..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_healingclone"));
    ac->setWhatsThis(i18nc("@info", "This filter can be used to clone a part in a photo to erase unwanted region."));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotHealingClone()));

    addAction(ac);
}

void HealingCloneToolPlugin::slotHealingClone()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        HealingCloneTool* const tool = new HealingCloneTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorHealingCloneToolPlugin

#include "moc_healingclonetoolplugin.cpp"
