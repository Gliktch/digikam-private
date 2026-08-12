/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to shear an image.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sheartoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "sheartool.h"

namespace DigikamEditorShearToolPlugin
{

ShearToolPlugin::ShearToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString ShearToolPlugin::name() const
{
    return i18nc("@title", "Shear Image");
}

QString ShearToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ShearToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-shear-left"));
}

QString ShearToolPlugin::description() const
{
    return i18nc("@info", "A tool to shear an image");
}

QString ShearToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can shear an image.");
}

QString ShearToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString ShearToolPlugin::handbookChapter() const
{
    return QLatin1String("transform_tools");
}

QString ShearToolPlugin::handbookReference() const
{
    return QLatin1String("transform-shear");
}

QList<DPluginAuthor> ShearToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void ShearToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "&Shear..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_sheartool"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotShear()));

    addAction(ac);
}

void ShearToolPlugin::slotShear()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ShearTool* const tool = new ShearTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorShearToolPlugin

#include "moc_sheartoolplugin.cpp"
