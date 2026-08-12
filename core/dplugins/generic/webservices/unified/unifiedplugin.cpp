/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export items to web-services (unified version).
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "unifiedplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericUnifiedPlugin
{

UnifiedPlugin::UnifiedPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void UnifiedPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString UnifiedPlugin::name() const
{
    return i18nc("@title", "Unified");
}

QString UnifiedPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon UnifiedPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("internet-web-browser"));
}

QString UnifiedPlugin::description() const
{
    return i18nc("@info", "A tool to export items to web-services");
}

QString UnifiedPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to many web-services.<br><br>"
                 "This is a unified tool, grouping many tools in one.");
}

QString UnifiedPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString UnifiedPlugin::handbookChapter() const
{
    return QLatin1String("unified_tool");
}

QList<DPluginAuthor> UnifiedPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Thanh Trung Dinh"),
                             QString::fromUtf8("dinhthanhtrung1996 at gmail dot com"),
                             QString::fromUtf8("(C) 2018"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2018-2021"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2018-2024"))
            ;
}

void UnifiedPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to Webservices..."));
    ac->setObjectName(QLatin1String("export_unified"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotUnified()));

    addAction(ac);
}

void UnifiedPlugin::slotUnified()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new WSWizard(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericUnifiedPlugin

#include "moc_unifiedplugin.cpp"
