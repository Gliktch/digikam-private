/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to emulate film color
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filmtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "filmtool.h"

namespace DigikamEditorFilmToolPlugin
{

FilmToolPlugin::FilmToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString FilmToolPlugin::name() const
{
    return i18nc("@title", "Color Negative Film");
}

QString FilmToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FilmToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorneg"));
}

QString FilmToolPlugin::description() const
{
    return i18nc("@info", "A tool to emulate color negative film");
}

QString FilmToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can emulate color negative film from image.");
}

QString FilmToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString FilmToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString FilmToolPlugin::handbookReference() const
{
    return QLatin1String("color-negative");
}

QList<DPluginAuthor> FilmToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Matthias Welwarsky"),
                             QString::fromUtf8("matthias at welwarsky dot de"),
                             QString::fromUtf8("(C) 2012"))
            ;
}

void FilmToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Color Negative..."));
    ac->setObjectName(QLatin1String("editorwindow_color_film"));
    ac->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_I);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFilmTool()));

    addAction(ac);
}

void FilmToolPlugin::slotFilmTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        FilmTool* const tool = new FilmTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorFilmToolPlugin

#include "moc_filmtoolplugin.cpp"
