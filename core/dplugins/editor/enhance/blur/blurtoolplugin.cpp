/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to blur an image
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blurtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "blurtool.h"

namespace DigikamEditorBlurToolPlugin
{

BlurToolPlugin::BlurToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString BlurToolPlugin::name() const
{
    return i18nc("@title", "Blur");
}

QString BlurToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BlurToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("blurimage"));
}

QString BlurToolPlugin::description() const
{
    return i18nc("@info", "A tool to blur an image");
}

QString BlurToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can blur an image.");
}

QString BlurToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString BlurToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString BlurToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-blur");
}

QList<DPluginAuthor> BlurToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Andi Clemens"),
                             QString::fromUtf8("andi dot clemens at gmail dot com"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void BlurToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Blur..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_blur"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBlur()));

    addAction(ac);
}

void BlurToolPlugin::slotBlur()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BlurTool* const tool = new BlurTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorBlurToolPlugin

#include "moc_blurtoolplugin.cpp"
