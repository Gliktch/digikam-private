/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export and import items with SmugMug web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "smugplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "smugwindow.h"

namespace DigikamGenericSmugPlugin
{

SmugPlugin::SmugPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void SmugPlugin::cleanUp()
{
    delete m_toolDlgExport;
    delete m_toolDlgImport;
}

QString SmugPlugin::name() const
{
    return i18nc("@title", "SmugMug");
}

QString SmugPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SmugPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-smugmug"));
}

QString SmugPlugin::description() const
{
    return i18nc("@info", "A tool to export and import items with SmugMug web-service");
}

QString SmugPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export and import items with SmugMug web-service.<br><br>"
                 "See SmugMug web site for details: %1",
                 QLatin1String("<a href='https://www.smugmug.com'>https://www.smugmug.com</a></p>"));
}

QString SmugPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString SmugPlugin::handbookChapter() const
{
    return QLatin1String("smugmug_export");
}

QList<DPluginAuthor> SmugPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Luka Renko"),
                             QString::fromUtf8("lure at kubuntu dot org"),
                             QString::fromUtf8("(C) 2008-2009"))
            << DPluginAuthor(QString::fromUtf8("Vardhman Jain"),
                             QString::fromUtf8("vardhman at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2008"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2021"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2025"))
            ;
}

void SmugPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &SmugMug..."));
    ac->setObjectName(QLatin1String("export_smugmug"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_S);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotSmugMugExport()));

    addAction(ac);

    DPluginAction* const ac2 = new DPluginAction(parent);
    ac2->setIcon(icon());
    ac2->setText(i18nc("@action", "Import from &SmugMug..."));
    ac2->setObjectName(QLatin1String("import_smugmug"));
    ac2->setActionCategory(DPluginAction::GenericImport);
    ac2->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_S);

    connect(ac2, SIGNAL(triggered(bool)),
            this, SLOT(slotSmugMugImport()));

    addAction(ac2);

}

void SmugPlugin::slotSmugMugExport()
{
    if (!reactivateToolDialog(m_toolDlgExport))
    {
        delete m_toolDlgExport;
        m_toolDlgExport = new SmugWindow(infoIface(sender()), nullptr);
        m_toolDlgExport->setPlugin(this);
        m_toolDlgExport->show();
    }
}

void SmugPlugin::slotSmugMugImport()
{
    if (!reactivateToolDialog(m_toolDlgImport))
    {
        DInfoInterface* const iface = infoIface(sender());

        delete m_toolDlgImport;
        m_toolDlgImport = new SmugWindow(iface, nullptr, true);
        m_toolDlgImport->setPlugin(this);

        connect(m_toolDlgImport, SIGNAL(updateHostApp(QUrl)),
                iface, SLOT(slotMetadataChangedForUrl(QUrl)));

        m_toolDlgImport->show();
    }
}

} // namespace DigikamGenericSmugPlugin

#include "moc_smugplugin.cpp"
