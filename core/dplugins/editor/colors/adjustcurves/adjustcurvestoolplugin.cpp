/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to adjust color curves.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "adjustcurvestoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "adjustcurvestool.h"

namespace DigikamEditorAdjustCurvesToolPlugin
{

AdjustCurvesToolPlugin::AdjustCurvesToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString AdjustCurvesToolPlugin::name() const
{
    return i18nc("@title", "Adjust Curves");
}

QString AdjustCurvesToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AdjustCurvesToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjustcurves"));
}

QString AdjustCurvesToolPlugin::description() const
{
    return i18nc("@info", "A tool to adjust color curves");
}

QString AdjustCurvesToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust the color curves from image.");
}

QString AdjustCurvesToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString AdjustCurvesToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString AdjustCurvesToolPlugin::handbookReference() const
{
    return QLatin1String("color-curves");
}

QList<DPluginAuthor> AdjustCurvesToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2025"))
            ;
}

void AdjustCurvesToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Curves Adjust..."));
    ac->setObjectName(QLatin1String("editorwindow_color_adjustcurves"));
    // NOTE: Photoshop 7 use CTRL+M (but it's used in KDE to toggle menu bar).
    ac->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_C);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAdjustCurvesTool()));

    addAction(ac);
}

void AdjustCurvesToolPlugin::slotAdjustCurvesTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        AdjustCurvesTool* const tool = new AdjustCurvesTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorAdjustCurvesToolPlugin

#include "moc_adjustcurvestoolplugin.cpp"
