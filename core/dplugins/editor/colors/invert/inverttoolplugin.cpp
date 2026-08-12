/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to invert colors.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inverttoolplugin.h"

// Qt includes

#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"
#include "invertfilter.h"

namespace DigikamEditorInvertToolPlugin
{

InvertToolPlugin::InvertToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString InvertToolPlugin::name() const
{
    return i18nc("@title", "Invert Colors");
}

QString InvertToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon InvertToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("edit-select-invert"));
}

QString InvertToolPlugin::description() const
{
    return i18nc("@info", "A tool to invert image colors");
}

QString InvertToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can invert colors from image.");
}

QString InvertToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString InvertToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString InvertToolPlugin::handbookReference() const
{
    return QLatin1String("color-invert");
}

QList<DPluginAuthor> InvertToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void InvertToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Invert"));
    ac->setObjectName(QLatin1String("editorwindow_color_invert"));

    // NOTE: Photoshop 7 use CTRL+I.

    ac->setShortcut(Qt::CTRL | Qt::Key_I);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotInvert()));

    addAction(ac);
}

void InvertToolPlugin::slotInvert()
{
    qApp->setOverrideCursor(Qt::WaitCursor);

    ImageIface iface;
    InvertFilter invert(iface.original(), nullptr);
    invert.startFilterDirectly();
    iface.setOriginal(i18nc("@title", "Invert"), invert.filterAction(), invert.getTargetImage());

    qApp->restoreOverrideCursor();
}

} // namespace DigikamEditorInvertToolPlugin

#include "moc_inverttoolplugin.cpp"
