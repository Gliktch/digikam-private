/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix hot pixels
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hotpixelstoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "hotpixelstool.h"

namespace DigikamEditorHotPixelsToolPlugin
{

HotPixelsToolPlugin::HotPixelsToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString HotPixelsToolPlugin::name() const
{
    return i18nc("@title", "Hot Pixels");
}

QString HotPixelsToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HotPixelsToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("hotpixels"));
}

QString HotPixelsToolPlugin::description() const
{
    return i18nc("@info", "A tool to fix hot pixels");
}

QString HotPixelsToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can fix hot pixels from an image.");
}

QString HotPixelsToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString HotPixelsToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString HotPixelsToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-hotpixels");
}

QList<DPluginAuthor> HotPixelsToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Unai Garro"),
                             QString::fromUtf8("ugarro at users dot sourceforge dot net"),
                             QString::fromUtf8("(C) 2005-2006"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void HotPixelsToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Hot Pixels..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_hotpixels"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotHotPixels()));

    addAction(ac);

    HotPixelsTool::registerFilter();
}

void HotPixelsToolPlugin::slotHotPixels()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        HotPixelsTool* const tool = new HotPixelsTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorHotPixelsToolPlugin

#include "moc_hotpixelstoolplugin.cpp"
