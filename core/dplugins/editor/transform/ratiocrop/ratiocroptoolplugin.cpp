/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to crop an image with ratio.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ratiocroptoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "ratiocroptool.h"

namespace DigikamEditorRatioCropToolPlugin
{

RatioCropToolPlugin::RatioCropToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString RatioCropToolPlugin::name() const
{
    return i18nc("@title", "Aspect Ratio Crop");
}

QString RatioCropToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RatioCropToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-crop"));
}

QString RatioCropToolPlugin::description() const
{
    return i18nc("@info", "A tool to crop an image with ratio");
}

QString RatioCropToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can crop an image with ratio.");
}

QString RatioCropToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString RatioCropToolPlugin::handbookChapter() const
{
    return QLatin1String("transform_tools");
}

QString RatioCropToolPlugin::handbookReference() const
{
    return QLatin1String("transform-proportionalcrop");
}

QList<DPluginAuthor> RatioCropToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Jaromir Malenko"),
                             QString::fromUtf8("malenko at email dot cz"),
                             QString::fromUtf8("(C) 2007"))
            << DPluginAuthor(QString::fromUtf8("Roberto Castagnola"),
                             QString::fromUtf8("roberto dot castagnola at gmail dot com"),
                             QString::fromUtf8("(C) 2008"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void RatioCropToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Aspect Ratio Crop..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_ratiocrop"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRatioCrop()));

    addAction(ac);
}

void RatioCropToolPlugin::slotRatioCrop()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RatioCropTool* const tool = new RatioCropTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorRatioCropToolPlugin

#include "moc_ratiocroptoolplugin.cpp"
