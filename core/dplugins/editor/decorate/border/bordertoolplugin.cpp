/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to add border
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bordertoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "bordertool.h"

namespace DigikamEditorBorderToolPlugin
{

BorderToolPlugin::BorderToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString BorderToolPlugin::name() const
{
    return i18nc("@title", "Add Border");
}

QString BorderToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BorderToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bordertool"));
}

QString BorderToolPlugin::description() const
{
    return i18nc("@info", "A tool to add a border around image");
}

QString BorderToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can add decorative border around image.");
}

QString BorderToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString BorderToolPlugin::handbookChapter() const
{
    return QLatin1String("decorate_tools");
}

QString BorderToolPlugin::handbookReference() const
{
    return QLatin1String("decorate-border");
}

QList<DPluginAuthor> BorderToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void BorderToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Add Border..."));
    ac->setObjectName(QLatin1String("editorwindow_decorate_border"));
    ac->setActionCategory(DPluginAction::EditorDecorate);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBorder()));

    addAction(ac);
}

void BorderToolPlugin::slotBorder()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BorderTool* const tool = new BorderTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorBorderToolPlugin

#include "moc_bordertoolplugin.cpp"
