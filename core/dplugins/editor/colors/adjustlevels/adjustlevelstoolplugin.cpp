/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to adjust color levels.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "adjustlevelstoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "adjustlevelstool.h"

namespace DigikamEditorAdjustLevelsToolPlugin
{

AdjustLevelsToolPlugin::AdjustLevelsToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString AdjustLevelsToolPlugin::name() const
{
    return i18nc("@title", "Adjust Levels");
}

QString AdjustLevelsToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AdjustLevelsToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjustlevels"));
}

QString AdjustLevelsToolPlugin::description() const
{
    return i18nc("@info", "A tool to adjust color levels");
}

QString AdjustLevelsToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust the color levels from image.");
}

QString AdjustLevelsToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString AdjustLevelsToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString AdjustLevelsToolPlugin::handbookReference() const
{
    return QLatin1String("color-levels");
}

QList<DPluginAuthor> AdjustLevelsToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2025"))
            ;
}

void AdjustLevelsToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Levels Adjust..."));
    ac->setObjectName(QLatin1String("editorwindow_color_adjustlevels"));
    // NOTE: Photoshop 7 use CTRL+M (but it's used in KDE to toggle menu bar).
    ac->setShortcut(Qt::CTRL | Qt::Key_L);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAdjustCurvesTool()));

    addAction(ac);
}

void AdjustLevelsToolPlugin::slotAdjustCurvesTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        AdjustLevelsTool* const tool = new AdjustLevelsTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorAdjustLevelsToolPlugin

#include "moc_adjustlevelstoolplugin.cpp"
