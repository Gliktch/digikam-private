/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to add texture
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "texturetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "texturetool.h"

namespace DigikamEditorTextureToolPlugin
{

TextureToolPlugin::TextureToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString TextureToolPlugin::name() const
{
    return i18nc("@title", "Texture");
}

QString TextureToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TextureToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("texture"));
}

QString TextureToolPlugin::description() const
{
    return i18nc("@info", "A tool to apply a texture over an image");
}

QString TextureToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can apply a texture over an image.");
}

QString TextureToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString TextureToolPlugin::handbookChapter() const
{
    return QLatin1String("decorate_tools");
}

QString TextureToolPlugin::handbookReference() const
{
    return QLatin1String("decorate-texture");
}

QList<DPluginAuthor> TextureToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2010"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void TextureToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Apply Texture..."));
    ac->setObjectName(QLatin1String("editorwindow_decorate_texture"));
    ac->setActionCategory(DPluginAction::EditorDecorate);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotTexture()));

    addAction(ac);
}

void TextureToolPlugin::slotTexture()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        TextureTool* const tool = new TextureTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorTextureToolPlugin

#include "moc_texturetoolplugin.cpp"
