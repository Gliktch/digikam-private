/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to sharp image
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sharpentoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "sharpentool.h"

namespace DigikamEditorSharpenToolPlugin
{

SharpenToolPlugin::SharpenToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString SharpenToolPlugin::name() const
{
    return i18nc("@title", "Sharpen");
}

QString SharpenToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SharpenToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("sharpenimage"));
}

QString SharpenToolPlugin::description() const
{
    return i18nc("@info", "A tool to sharpen an image");
}

QString SharpenToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can sharpen an image.");
}

QString SharpenToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString SharpenToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString SharpenToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-sharpen");
}

QList<DPluginAuthor> SharpenToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2025"))
            ;
}

void SharpenToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Sharpen..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_sharpen"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotSharpen()));

    addAction(ac);
}

void SharpenToolPlugin::slotSharpen()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        SharpenTool* const tool = new SharpenTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorSharpenToolPlugin

#include "moc_sharpentoolplugin.cpp"
