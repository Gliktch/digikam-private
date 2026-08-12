/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to resize an image.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "resizetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "resizetool.h"

namespace DigikamEditorResizeToolPlugin
{

ResizeToolPlugin::ResizeToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString ResizeToolPlugin::name() const
{
    return i18nc("@title", "Resize Image");
}

QString ResizeToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ResizeToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-scale"));
}

QString ResizeToolPlugin::description() const
{
    return i18nc("@action", "A tool to resize an image");
}

QString ResizeToolPlugin::details() const
{
    return i18nc("@action", "This Image Editor tool can resize an image.");
}

QString ResizeToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString ResizeToolPlugin::handbookChapter() const
{
    return QLatin1String("transform_tools");
}

QString ResizeToolPlugin::handbookReference() const
{
    return QLatin1String("transform-resize");
}

QList<DPluginAuthor> ResizeToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2025"))
            ;
}

void ResizeToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "&Resize..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_resize"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotResize()));

    addAction(ac);
}

void ResizeToolPlugin::slotResize()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ResizeTool* const tool = new ResizeTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorResizeToolPlugin

#include "moc_resizetoolplugin.cpp"
