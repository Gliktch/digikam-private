/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix colors automatically
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autocorrectiontoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "autocorrectiontool.h"

namespace DigikamEditorAutoCorrectionToolPlugin
{

AutoCorrectionToolPlugin::AutoCorrectionToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString AutoCorrectionToolPlugin::name() const
{
    return i18nc("@title", "Color Auto-Correction");
}

QString AutoCorrectionToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AutoCorrectionToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("autocorrection"));
}

QString AutoCorrectionToolPlugin::description() const
{
    return i18nc("@info", "A tool to fix colors automatically");
}

QString AutoCorrectionToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust colors automatically from image.");
}

QString AutoCorrectionToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString AutoCorrectionToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString AutoCorrectionToolPlugin::handbookReference() const
{
    return QLatin1String("color-auto");
}

QList<DPluginAuthor> AutoCorrectionToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void AutoCorrectionToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Color Auto-Correction..."));
    ac->setObjectName(QLatin1String("editorwindow_color_autocorrection"));

    // NOTE: Photoshop 7 use CTRL+SHIFT+B

    ac->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_B);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAutoCorrection()));

    addAction(ac);
}

void AutoCorrectionToolPlugin::slotAutoCorrection()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        AutoCorrectionTool* const tool = new AutoCorrectionTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorAutoCorrectionToolPlugin

#include "moc_autocorrectiontoolplugin.cpp"
