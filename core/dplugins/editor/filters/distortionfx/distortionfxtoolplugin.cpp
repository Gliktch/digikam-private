/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to apply distortion effects.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "distortionfxtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "distortionfxtool.h"

namespace DigikamEditorDistortionFxToolPlugin
{

DistortionFXToolPlugin::DistortionFXToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString DistortionFXToolPlugin::name() const
{
    return i18nc("@title", "Distortion Effects");
}

QString DistortionFXToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DistortionFXToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("draw-spiral"));
}

QString DistortionFXToolPlugin::description() const
{
    return i18nc("@info", "A tool to apply distortion effects to an image");
}

QString DistortionFXToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can apply distortion effects to an image.");
}

QString DistortionFXToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString DistortionFXToolPlugin::handbookChapter() const
{
    return QLatin1String("effects_tools");
}

QString DistortionFXToolPlugin::handbookReference() const
{
    return QLatin1String("effects-distrotionfx");
}

QList<DPluginAuthor> DistortionFXToolPlugin::authors() const
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

void DistortionFXToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Distortion Effects..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_distortionfx"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotDistortionFX()));

    addAction(ac);
}

void DistortionFXToolPlugin::slotDistortionFX()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        DistortionFXTool* const tool = new DistortionFXTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorDistortionFxToolPlugin

#include "moc_distortionfxtoolplugin.cpp"
