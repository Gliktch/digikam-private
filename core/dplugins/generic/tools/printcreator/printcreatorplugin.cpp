/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to create print compositions.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "printcreatorplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintwizard.h"

namespace DigikamGenericPrintCreatorPlugin
{

PrintCreatorPlugin::PrintCreatorPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void PrintCreatorPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString PrintCreatorPlugin::name() const
{
    return i18n("Print Creator");
}

QString PrintCreatorPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PrintCreatorPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("document-print"));
}

QString PrintCreatorPlugin::description() const
{
    return i18n("A tool to create print composition from images");
}

QString PrintCreatorPlugin::details() const
{
    return i18n("<p>This tool allows users to back-process items (e.g., assemble them) before to print.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Different pre-defined paper sizes and layouts can be used to process files.</p>");
}

QString PrintCreatorPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString PrintCreatorPlugin::handbookChapter() const
{
    return QLatin1String("print_creator");
}

QList<DPluginAuthor> PrintCreatorPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Todd Shoemaker"),
                             QString::fromUtf8("todd at theshoemakers dot net"),
                             QString::fromUtf8("(C) 2003-2004"),
                             i18n("Author"))
            << DPluginAuthor(QString::fromUtf8("Angelo Naselli"),
                             QString::fromUtf8("anaselli at linux dot it"),
                             QString::fromUtf8("(C) 2007-2013"))
            << DPluginAuthor(QString::fromUtf8("Andreas Trink"),
                             QString::fromUtf8("atrink at nociaro dot org"),
                             QString::fromUtf8("(C) 2010"),
                             i18n("Contributor"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2025"),
                             i18n("Developer and Maintainer"))
            ;
}

void PrintCreatorPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Print Creator..."));
    ac->setObjectName(QLatin1String("printcreator"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPrintCreator()));

    addAction(ac);
}

void PrintCreatorPlugin::slotPrintCreator()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new AdvPrintWizard(nullptr, infoIface(sender()));
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_printcreatorplugin.cpp"
