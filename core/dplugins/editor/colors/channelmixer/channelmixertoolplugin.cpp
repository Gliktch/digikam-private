/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to mix color channels
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "channelmixertoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "channelmixertool.h"

namespace DigikamEditorChannelMixerToolPlugin
{

ChannelMixerToolPlugin::ChannelMixerToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString ChannelMixerToolPlugin::name() const
{
    return i18nc("@title", "Channel Mixer");
}

QString ChannelMixerToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ChannelMixerToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("channelmixer"));
}

QString ChannelMixerToolPlugin::description() const
{
    return i18nc("@info", "A tool to mix color channel");
}

QString ChannelMixerToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can mix color channels from image.");
}

QString ChannelMixerToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString ChannelMixerToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString ChannelMixerToolPlugin::handbookReference() const
{
    return QLatin1String("color-mixer");
}

QList<DPluginAuthor> ChannelMixerToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void ChannelMixerToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Channel Mixer..."));
    ac->setObjectName(QLatin1String("editorwindow_color_channelmixer"));
    ac->setShortcut(Qt::CTRL | Qt::Key_H);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelMixer()));

    addAction(ac);
}

void ChannelMixerToolPlugin::slotChannelMixer()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ChannelMixerTool* const tool = new ChannelMixerTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorChannelMixerToolPlugin

#include "moc_channelmixertoolplugin.cpp"
