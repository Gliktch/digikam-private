/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-08-24
 * Description : image editor plugin to blur the background of an image
 *
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "backgroundblurtoolplugin.h"

// Qt includes

#include <QPointer>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "imageiface.h"
#include "backgroundblurtool.h"

namespace DigikamEditorBackgroundBlurToolPlugin
{

BackgroundBlurToolPlugin::BackgroundBlurToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString BackgroundBlurToolPlugin::name() const
{
    return i18nc("@title", "Background Blur");
}

QString BackgroundBlurToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BackgroundBlurToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("blur"));
}

QString BackgroundBlurToolPlugin::description() const
{
    return i18nc("@info", "A tool to blur the background of an image");
}

QString BackgroundBlurToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can blur the background of the subject "
                          "from an image to reproduce the depth-of-field effect from a lens.");
}

QString BackgroundBlurToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString BackgroundBlurToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString BackgroundBlurToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-backgroundblur");
}

QList<DPluginAuthor> BackgroundBlurToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2025-2026"))
            ;
}

void BackgroundBlurToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Background Blur..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_backgroundblur"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBackgroundBlur()));

    addAction(ac);
}

void BackgroundBlurToolPlugin::slotBackgroundBlur()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ImageIface iface;

        if (iface.selectionRect().isEmpty())
        {
            QMessageBox::warning(qApp->activeWindow(), i18n("Background Blur Tool"),
                                 i18n("You must select before a rectangular area including the subject.<br>"
                                      "Note: you can readjust the zone later in the tool."));

            return;
        }

        BackgroundBlurTool* const tool = new BackgroundBlurTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorBackgroundBlurToolPlugin

#include "moc_backgroundblurtoolplugin.cpp"
