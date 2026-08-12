/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to adjust perspective.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "perspectivetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "perspectivetool.h"

namespace DigikamEditorPerspectiveToolPlugin
{

PerspectiveToolPlugin::PerspectiveToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString PerspectiveToolPlugin::name() const
{
    return i18nc("@title", "Perspective");
}

QString PerspectiveToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PerspectiveToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("perspective"));
}

QString PerspectiveToolPlugin::description() const
{
    return i18nc("@info", "A tool to adjust perspective");
}

QString PerspectiveToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust perspective from an image.");
}

QString PerspectiveToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString PerspectiveToolPlugin::handbookChapter() const
{
    return QLatin1String("transform_tools");
}

QString PerspectiveToolPlugin::handbookReference() const
{
    return QLatin1String("transform-perspective");
}

QList<DPluginAuthor> PerspectiveToolPlugin::authors() const
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

void PerspectiveToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Perspective Adjustment..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_perspective"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPerspective()));

    addAction(ac);
}

void PerspectiveToolPlugin::slotPerspective()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        PerspectiveTool* const tool = new PerspectiveTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorPerspectiveToolPlugin

#include "moc_perspectivetoolplugin.cpp"
