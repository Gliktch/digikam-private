/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix colors balance
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cbtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "cbtool.h"

namespace DigikamEditorColorBalanceToolPlugin
{

CBToolPlugin::CBToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString CBToolPlugin::name() const
{
    return i18nc("@title", "Color Balance");
}

QString CBToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CBToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjustrgb"));
}

QString CBToolPlugin::description() const
{
    return i18nc("@info", "A tool to adjust color balance");
}

QString CBToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust color balance from image.");
}

QString CBToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString CBToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString CBToolPlugin::handbookReference() const
{
    return QLatin1String("color-balance");
}

QList<DPluginAuthor> CBToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void CBToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Color Balance..."));
    ac->setObjectName(QLatin1String("editorwindow_color_rgb"));
    // NOTE: Photoshop 7 use CTRL+B.
    ac->setShortcut(Qt::CTRL | Qt::Key_B);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotColorBalance()));

    addAction(ac);
}

void CBToolPlugin::slotColorBalance()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        CBTool* const tool = new CBTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorColorBalanceToolPlugin

#include "moc_cbtoolplugin.cpp"
