/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to adjust HSL
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hsltoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "hsltool.h"

namespace DigikamEditorHSLToolPlugin
{

HSLToolPlugin::HSLToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString HSLToolPlugin::name() const
{
    return i18nc("@title", "HSL Correction");
}

QString HSLToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HSLToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjusthsl"));
}

QString HSLToolPlugin::description() const
{
    return i18nc("@info", "A tool to fix Hue / Saturation / Lightness");
}

QString HSLToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust Hue / Saturation / Lightness from image.");
}

QString HSLToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString HSLToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString HSLToolPlugin::handbookReference() const
{
    return QLatin1String("color-hsl");
}

QList<DPluginAuthor> HSLToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void HSLToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Hue/Saturation/Lightness..."));
    ac->setObjectName(QLatin1String("editorwindow_color_hsl"));

    // NOTE: Photoshop 7 use CTRL+U.

    ac->setShortcut(Qt::CTRL | Qt::Key_U);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotHSL()));

    addAction(ac);
}

void HSLToolPlugin::slotHSL()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        HSLTool* const tool = new HSLTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorHSLToolPlugin

#include "moc_hsltoolplugin.cpp"
