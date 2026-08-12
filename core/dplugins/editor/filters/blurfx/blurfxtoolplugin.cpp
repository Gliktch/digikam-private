/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to apply blur effects.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blurfxtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "blurfxtool.h"

namespace DigikamEditorBlurFxToolPlugin
{

BlurFXToolPlugin::BlurFXToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString BlurFXToolPlugin::name() const
{
    return i18nc("@title", "Blur Effects");
}

QString BlurFXToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BlurFXToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("blurfx"));
}

QString BlurFXToolPlugin::description() const
{
    return i18nc("@info", "A tool to apply blur effects to an image");
}

QString BlurFXToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can apply blur effects to an image.");
}

QString BlurFXToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString BlurFXToolPlugin::handbookChapter() const
{
    return QLatin1String("effects_tools");
}

QString BlurFXToolPlugin::handbookReference() const
{
    return QLatin1String("effects-blurfx");
}

QList<DPluginAuthor> BlurFXToolPlugin::authors() const
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

void BlurFXToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Blur Effects..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_blurfx"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBlurFX()));

    addAction(ac);
}

void BlurFXToolPlugin::slotBlurFX()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BlurFXTool* const tool = new BlurFXTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorBlurFxToolPlugin

#include "moc_blurfxtoolplugin.cpp"
