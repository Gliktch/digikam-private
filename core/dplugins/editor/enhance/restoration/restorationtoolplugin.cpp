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

#include "restorationtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "restorationtool.h"

namespace DigikamEditorRestorationToolPlugin
{

RestoreToolPlugin::RestoreToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString RestoreToolPlugin::name() const
{
    return i18nc("@title", "Restoration");
}

QString RestoreToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RestoreToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("restoration"));
}

QString RestoreToolPlugin::description() const
{
    return i18nc("@info", "A tool to restore an image using Greystoration algorithm");
}

QString RestoreToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can restore an image using Greystoration algorithm.");
}

QString RestoreToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString RestoreToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString RestoreToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-restoration");
}

QList<DPluginAuthor> RestoreToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2025"))
            ;
}

void RestoreToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Restoration..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_restoration"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRestore()));

    addAction(ac);
}

void RestoreToolPlugin::slotRestore()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RestorationTool* const tool = new RestorationTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorRestorationToolPlugin

#include "moc_restorationtoolplugin.cpp"
