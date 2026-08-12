/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to add rain drops over an image.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "raindroptoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "raindroptool.h"

namespace DigikamEditorRainDropToolPlugin
{

RainDropToolPlugin::RainDropToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString RainDropToolPlugin::name() const
{
    return i18nc("@title", "Raindrops");
}

QString RainDropToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RainDropToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("raindrop"));
}

QString RainDropToolPlugin::description() const
{
    return i18nc("@info", "A tool to add rain drops over an image");
}

QString RainDropToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can add rain drops over an image.");
}

QString RainDropToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString RainDropToolPlugin::handbookChapter() const
{
    return QLatin1String("effects_tools");
}

QString RainDropToolPlugin::handbookReference() const
{
    return QLatin1String("effects-raindrop");
}

QList<DPluginAuthor> RainDropToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2024"))
            ;
}

void RainDropToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Raindrops..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_raindrop"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRainDrop()));

    addAction(ac);
}

void RainDropToolPlugin::slotRainDrop()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RainDropTool* const tool = new RainDropTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorRainDropToolPlugin

#include "moc_raindroptoolplugin.cpp"
