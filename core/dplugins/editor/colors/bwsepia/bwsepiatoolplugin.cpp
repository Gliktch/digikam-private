/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert to Black and White
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bwsepiatoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "bwsepiatool.h"

namespace DigikamEditorBWSepiaToolPlugin
{

BWSepiaToolPlugin::BWSepiaToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString BWSepiaToolPlugin::name() const
{
    return i18nc("@title", "Black and White");
}

QString BWSepiaToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BWSepiaToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bwtonal"));
}

QString BWSepiaToolPlugin::description() const
{
    return i18nc("@info", "A tool to convert to black and white");
}

QString BWSepiaToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can convert image to black and white.");
}

QString BWSepiaToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString BWSepiaToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString BWSepiaToolPlugin::handbookReference() const
{
    return QLatin1String("color-bw");
}

QList<DPluginAuthor> BWSepiaToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2005"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2024"))
            ;
}

void BWSepiaToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Black && White..."));
    ac->setObjectName(QLatin1String("editorwindow_color_blackwhite"));
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBWSepia()));

    addAction(ac);
}

void BWSepiaToolPlugin::slotBWSepia()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BWSepiaTool* const tool = new BWSepiaTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorBWSepiaToolPlugin

#include "moc_bwsepiatoolplugin.cpp"
