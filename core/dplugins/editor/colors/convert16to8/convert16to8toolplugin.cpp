/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert 16 bits color depth to 8.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "convert16to8toolplugin.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"
#include "dmessagebox.h"

namespace DigikamEditorConvert16To8ToolPlugin
{

Convert16To8ToolPlugin::Convert16To8ToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString Convert16To8ToolPlugin::name() const
{
    return i18nc("@title", "Convert to 8 bits");
}

QString Convert16To8ToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon Convert16To8ToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("depth16to8"));
}

QString Convert16To8ToolPlugin::description() const
{
    return i18nc("@info", "A tool to convert color depth to 8 bits");
}

QString Convert16To8ToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can convert image color depth to 8 bits.");
}

QString Convert16To8ToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString Convert16To8ToolPlugin::handbookChapter() const
{
    return QLatin1String("colors_tools");
}

QString Convert16To8ToolPlugin::handbookReference() const
{
    return QLatin1String("color-depth");
}

QList<DPluginAuthor> Convert16To8ToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2024"))
            ;
}

void Convert16To8ToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Convert to 8 bits"));
    ac->setObjectName(QLatin1String("editorwindow_convertto8bits"));
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotConvert16To8()));

    addAction(ac);
}

void Convert16To8ToolPlugin::slotConvert16To8()
{
    ImageIface iface;

    if (!iface.originalSixteenBit())
    {
        QMessageBox::critical(qApp->activeWindow(),
                              qApp->applicationName(),
                              i18nc("@info", "This image is already using a depth of 8 bits / color / pixel."));
        return;
    }
    else
    {
        if (DMessageBox::showContinueCancel(QMessageBox::Warning,
                                            qApp->activeWindow(),
                                            qApp->applicationName(),
                                            i18nc("@info", "Performing this operation will reduce image color quality. "
                                            "Do you want to continue?"),
                                            QLatin1String("ToolColor16To8Bits"))
            == QMessageBox::Cancel)
        {
            return;
        }
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    iface.convertOriginalColorDepth(32);
    qApp->restoreOverrideCursor();
}

} // namespace DigikamEditorConvert16To8ToolPlugin

#include "moc_convert16to8toolplugin.cpp"
