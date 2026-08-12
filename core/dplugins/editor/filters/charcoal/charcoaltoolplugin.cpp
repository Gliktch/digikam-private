/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to emulate charcoal drawing.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "charcoaltoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "charcoaltool.h"

namespace DigikamEditorCharcoalToolPlugin
{

CharcoalToolPlugin::CharcoalToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString CharcoalToolPlugin::name() const
{
    return i18nc("@title", "Charcoal Drawing");
}

QString CharcoalToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CharcoalToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("charcoaltool"));
}

QString CharcoalToolPlugin::description() const
{
    return i18nc("@info", "A tool to emulate charcoal drawing");
}

QString CharcoalToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can emulate charcoal drawing to an image.");
}

QString CharcoalToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString CharcoalToolPlugin::handbookChapter() const
{
    return QLatin1String("effects_tools");
}

QString CharcoalToolPlugin::handbookReference() const
{
    return QLatin1String("effects-charcoal");
}

QList<DPluginAuthor> CharcoalToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void CharcoalToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Charcoal Drawing..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_charcoal"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotCharcoal()));

    addAction(ac);
}

void CharcoalToolPlugin::slotCharcoal()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        CharcoalTool* const tool = new CharcoalTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorCharcoalToolPlugin

#include "moc_charcoaltoolplugin.cpp"
