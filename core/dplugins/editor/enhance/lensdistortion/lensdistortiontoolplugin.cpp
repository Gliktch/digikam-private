/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce lens distortions
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensdistortiontoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "lensdistortiontool.h"

namespace DigikamEditorLensDistortionToolPlugin
{

LensDistortionToolPlugin::LensDistortionToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString LensDistortionToolPlugin::name() const
{
    return i18nc("@title", "Lens Distortion");
}

QString LensDistortionToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon LensDistortionToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("lensdistortion"));
}

QString LensDistortionToolPlugin::description() const
{
    return i18nc("@info", "A tool to adjust lens distortions");
}

QString LensDistortionToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust lens distortions from an image.");
}

QString LensDistortionToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString LensDistortionToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString LensDistortionToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-lensdistortion");
}

QList<DPluginAuthor> LensDistortionToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2010"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void LensDistortionToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Distortion..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_lensdistortion"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotLensDistortion()));

    addAction(ac);
}

void LensDistortionToolPlugin::slotLensDistortion()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        LensDistortionTool* const tool = new LensDistortionTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorLensDistortionToolPlugin

#include "moc_lensdistortiontoolplugin.cpp"
