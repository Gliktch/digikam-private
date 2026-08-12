/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to apply color effects.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorfxtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "colorfxtool.h"

namespace DigikamEditorColorFxToolPlugin
{

ColorFXToolPlugin::ColorFXToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString ColorFXToolPlugin::name() const
{
    return i18nc("@title", "Color Effects");
}

QString ColorFXToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ColorFXToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString ColorFXToolPlugin::description() const
{
    return i18nc("@info", "A tool to apply color effects to an image");
}

QString ColorFXToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can apply color effects to an image.");
}

QString ColorFXToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString ColorFXToolPlugin::handbookChapter() const
{
    return QLatin1String("effects_tools");
}

QString ColorFXToolPlugin::handbookReference() const
{
    return QLatin1String("effects-color");
}

QList<DPluginAuthor> ColorFXToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2005"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2024"))
            ;
}

void ColorFXToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Color Effects..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_colorfx"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotColorFX()));

    addAction(ac);
}

void ColorFXToolPlugin::slotColorFX()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ColorFxTool* const tool = new ColorFxTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorColorFxToolPlugin

#include "moc_colorfxtoolplugin.cpp"
