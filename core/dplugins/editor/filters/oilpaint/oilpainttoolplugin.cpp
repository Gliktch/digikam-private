/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to emulate oil painting.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "oilpainttoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "oilpainttool.h"

namespace DigikamEditorOilPaintToolPlugin
{

OilPaintToolPlugin::OilPaintToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString OilPaintToolPlugin::name() const
{
    return i18nc("@title", "Oil Paint");
}

QString OilPaintToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon OilPaintToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("oilpaint"));
}

QString OilPaintToolPlugin::description() const
{
    return i18nc("@info", "A tool to emulate oil painting");
}

QString OilPaintToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can emulate oil painting over an image.");
}

QString OilPaintToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString OilPaintToolPlugin::handbookChapter() const
{
    return QLatin1String("effects_tools");
}

QString OilPaintToolPlugin::handbookReference() const
{
    return QLatin1String("effects-oilpaint");
}

QList<DPluginAuthor> OilPaintToolPlugin::authors() const
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

void OilPaintToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Oil Paint..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_oilpaint"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotOilPaint()));

    addAction(ac);
}

void OilPaintToolPlugin::slotOilPaint()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        OilPaintTool* const tool = new OilPaintTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorOilPaintToolPlugin

#include "moc_oilpainttoolplugin.cpp"
