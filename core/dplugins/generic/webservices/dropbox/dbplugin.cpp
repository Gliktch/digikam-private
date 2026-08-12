/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Dropbox web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dbwindow.h"

namespace DigikamGenericDropBoxPlugin
{

DBPlugin::DBPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void DBPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString DBPlugin::name() const
{
    return i18nc("@title", "Dropbox");
}

QString DBPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DBPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-dropbox"));
}

QString DBPlugin::description() const
{
    return i18nc("@info", "A tool to export to Dropbox web-service");
}

QString DBPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to Dropbox web-service.<br><br>"
                 "See Dropbox web site for details: %1",
                 QLatin1String("<a href='https://www.dropbox.com/'>https://www.dropbox.com/</a>"));
}

QString DBPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString DBPlugin::handbookChapter() const
{
    return QLatin1String("dropbox_export");
}

QList<DPluginAuthor> DBPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Pankaj Kumar"),
                             QString::fromUtf8("me at panks dot me"),
                             QString::fromUtf8("(C) 2013"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2018-2021"))
            ;
}

void DBPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Dropbox..."));
    ac->setObjectName(QLatin1String("export_dropbox"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_D);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotDropBox()));

    addAction(ac);
}

void DBPlugin::slotDropBox()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new DBWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericDropBoxPlugin

#include "moc_dbplugin.cpp"
