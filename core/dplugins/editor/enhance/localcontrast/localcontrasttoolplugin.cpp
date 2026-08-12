/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to restore an image
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localcontrasttoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "localcontrasttool.h"

namespace DigikamEditorLocalContrastToolPlugin
{

LocalContrastToolPlugin::LocalContrastToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString LocalContrastToolPlugin::name() const
{
    return i18nc("@title", "Local Contrast");
}

QString LocalContrastToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon LocalContrastToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("contrast"));
}

QString LocalContrastToolPlugin::description() const
{
    return i18nc("@info", "A tool to emulate tone mapping");
}

QString LocalContrastToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can emulate tone mapping over an image.");
}

QString LocalContrastToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString LocalContrastToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString LocalContrastToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-localcontrast");
}

QList<DPluginAuthor> LocalContrastToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Julien Pontabry"),
                             QString::fromUtf8("julien dot pontabry at gmail dot com"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2024"))
            ;
}

void LocalContrastToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Local Contrast..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_localcontrast"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotLocalContrast()));

    addAction(ac);
}

void LocalContrastToolPlugin::slotLocalContrast()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        LocalContrastTool* const tool = new LocalContrastTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorLocalContrastToolPlugin

#include "moc_localcontrasttoolplugin.cpp"
