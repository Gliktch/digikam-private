/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix white balance
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "whitebalancetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "whitebalancetool.h"

namespace DigikamEditorWhiteBalanceToolPlugin
{

WhiteBalanceToolPlugin::WhiteBalanceToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString WhiteBalanceToolPlugin::name() const
{
    return i18nc("@title", "White Balance");
}

QString WhiteBalanceToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WhiteBalanceToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("whitebalance"));
}

QString WhiteBalanceToolPlugin::description() const
{
    return i18nc("@info", "A tool to adjust white balance");
}

QString WhiteBalanceToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can adjust the white balance from image.");
}

QString WhiteBalanceToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString WhiteBalanceToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString WhiteBalanceToolPlugin::handbookReference() const
{
    return QLatin1String("color-wb");
}

QList<DPluginAuthor> WhiteBalanceToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Guillaume Castagnino"),
                             QString::fromUtf8("casta at xwing dot info"),
                             QString::fromUtf8("(C) 2008-2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"))
            ;
}

void WhiteBalanceToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "White Balance..."));
    ac->setObjectName(QLatin1String("editorwindow_color_whitebalance"));
    ac->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_W);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotWhiteBalance()));

    addAction(ac);
}

void WhiteBalanceToolPlugin::slotWhiteBalance()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        WhiteBalanceTool* const tool = new WhiteBalanceTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorWhiteBalanceToolPlugin

#include "moc_whitebalancetoolplugin.cpp"
