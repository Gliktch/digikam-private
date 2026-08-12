/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to print an image
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "printplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"
#include "editorwindow.h"
#include "printhelper.h"

namespace DigikamEditorPrintToolPlugin
{

PrintToolPlugin::PrintToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString PrintToolPlugin::name() const
{
    return i18nc("@title", "Print Image");
}

QString PrintToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PrintToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("document-print-frame"));
}

QString PrintToolPlugin::description() const
{
    return i18nc("@info", "A tool to print an image");
}

QString PrintToolPlugin::details() const
{
    return i18nc("@info", "This Image Editor tool can print an image.");
}

QString PrintToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString PrintToolPlugin::handbookChapter() const
{
    return QLatin1String("basic_operations");
}

QString PrintToolPlugin::handbookReference() const
{
    return QLatin1String("printing-images");
}

QList<DPluginAuthor> PrintToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Angelo Naselli"),
                             QString::fromUtf8("anaselli at linux dot it"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2026"))
            ;
}

void PrintToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Print Image..."));
    ac->setObjectName(QLatin1String("editorwindow_print"));
    ac->setShortcut(Qt::CTRL | Qt::Key_P);
    ac->setActionCategory(DPluginAction::EditorFile);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPrint()));

    addAction(ac);
}

void PrintToolPlugin::slotPrint()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ImageIface iface;
        DImg* const image = iface.original();

        if (!image || image->isNull())
        {
            return;
        }

        PrintHelper printHelp(editor);
        printHelp.print(*image);
    }
}

} // namespace DigikamEditorPrintToolPlugin

#include "moc_printplugin.cpp"
