/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to emboss an image.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "embosstoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "embosstool.h"

namespace DigikamEditorEmbossToolPlugin
{

EmbossToolPlugin::EmbossToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString EmbossToolPlugin::name() const
{
    return i18nc("@title", "Emboss");
}

QString EmbossToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon EmbossToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("embosstool"));
}

QString EmbossToolPlugin::description() const
{
    return i18nc("@info", "A tool to emboss an image");
}

QString EmbossToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can emboss an image.");
}

QString EmbossToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString EmbossToolPlugin::handbookChapter() const
{
    return QLatin1String("effects_tools");
}

QString EmbossToolPlugin::handbookReference() const
{
    return QLatin1String("effects-emboss");
}

QList<DPluginAuthor> EmbossToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void EmbossToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Emboss..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_emboss"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotEmboss()));

    addAction(ac);
}

void EmbossToolPlugin::slotEmboss()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        EmbossTool* const tool = new EmbossTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorEmbossToolPlugin

#include "moc_embosstoolplugin.cpp"
