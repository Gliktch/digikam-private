/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix BCG.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bcgtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "bcgtool.h"

namespace DigikamEditorBCGToolPlugin
{

BCGToolPlugin::BCGToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString BCGToolPlugin::name() const
{
    return i18nc("@title", "BCG Correction");
}

QString BCGToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BCGToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("contrast"));
}

QString BCGToolPlugin::description() const
{
    return i18nc("@info", "A tool to fix Brightness / Contrast / Gamma");
}

QString BCGToolPlugin::details() const
{
    return i18nc("@title", "This Image Editor tool can adjust Brightness / Contrast / Gamma from image.");
}

QString BCGToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString BCGToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString BCGToolPlugin::handbookReference() const
{
    return QLatin1String("color-bcg");
}

QList<DPluginAuthor> BCGToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2004"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void BCGToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Brightness/Contrast/Gamma..."));
    ac->setObjectName(QLatin1String("editorwindow_color_bcg"));
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBCGTool()));

    addAction(ac);
}

void BCGToolPlugin::slotBCGTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BCGTool* const tool = new BCGTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorBCGToolPlugin

#include "moc_bcgtoolplugin.cpp"
