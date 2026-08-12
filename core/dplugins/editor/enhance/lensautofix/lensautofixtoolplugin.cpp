/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce lens artifacts
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensautofixtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "lensautofixtool.h"

namespace DigikamEditorLensAutoFixToolPlugin
{

LensAutoFixToolPlugin::LensAutoFixToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString LensAutoFixToolPlugin::name() const
{
    return i18nc("@title", "Lens Auto-Correction");
}

QString LensAutoFixToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon LensAutoFixToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("lensautofix"));
}

QString LensAutoFixToolPlugin::description() const
{
    return i18nc("@info", "A tool to fix automatically lens artifacts");
}

QString LensAutoFixToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can fix automatically lens artifacts over an image.");
}

QString LensAutoFixToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString LensAutoFixToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString LensAutoFixToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-lensauto");
}

QList<DPluginAuthor> LensAutoFixToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Adrian Schroeter"),
                             QString::fromUtf8("adrian at suse dot de"),
                             QString::fromUtf8("(C) 2008"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            ;
}

void LensAutoFixToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Lens Auto-Correction..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_lensautofix"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotLensAutoFix()));

    addAction(ac);
}

void LensAutoFixToolPlugin::slotLensAutoFix()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        LensAutoFixTool* const tool = new LensAutoFixTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorLensAutoFixToolPlugin

#include "moc_lensautofixtoolplugin.cpp"
