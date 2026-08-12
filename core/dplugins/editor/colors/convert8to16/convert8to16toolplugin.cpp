/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert 8 bits color depth to 16
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "convert8to16toolplugin.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"

namespace DigikamEditorConvert8To16ToolPlugin
{

Convert8To16ToolPlugin::Convert8To16ToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString Convert8To16ToolPlugin::name() const
{
    return i18nc("@title", "Convert to 16 bits");
}

QString Convert8To16ToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon Convert8To16ToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("depth8to16"));
}

QString Convert8To16ToolPlugin::description() const
{
    return i18nc("@info", "A tool to convert color depth to 16 bits");
}

QString Convert8To16ToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can convert image color depth to 16 bits.");
}

QString Convert8To16ToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString Convert8To16ToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString Convert8To16ToolPlugin::handbookReference() const
{
    return QLatin1String("color-depth");
}

QList<DPluginAuthor> Convert8To16ToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void Convert8To16ToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Convert to 16 bits"));
    ac->setObjectName(QLatin1String("editorwindow_convertto16bits"));
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotConvert8To16()));

    addAction(ac);
}

void Convert8To16ToolPlugin::slotConvert8To16()
{
    ImageIface iface;

    if (iface.originalSixteenBit())
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18nc("@info", "This image is already using a depth of 16 bits / color / pixel."));
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(64);
    qApp->restoreOverrideCursor();
}

} // namespace DigikamEditorConvert8To16ToolPlugin

#include "moc_convert8to16toolplugin.cpp"
