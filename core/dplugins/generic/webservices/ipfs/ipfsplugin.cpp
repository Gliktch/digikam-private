/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to IPFS web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ipfsplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ipfswindow.h"

namespace DigikamGenericIpfsPlugin
{

IpfsPlugin::IpfsPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

IpfsPlugin::~IpfsPlugin()
{
}

void IpfsPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString IpfsPlugin::name() const
{
    return i18nc("@title", "IPFS");
}

QString IpfsPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon IpfsPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-ipfs"));
}

QString IpfsPlugin::description() const
{
    return i18nc("@info", "A tool to export to IPFS web-service");
}

QString IpfsPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to IPFS web-service.<br><br>"
                 "See IPFS web site for details: %1",
                 QLatin1String("<a href='https://ipfs.io/'>https://ipfs.io/</a>"));
}

QString IpfsPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString IpfsPlugin::handbookChapter() const
{
    return QLatin1String("ipfs_export");
}

QList<DPluginAuthor> IpfsPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Amar Lakshya"),
                             QString::fromUtf8("amar dot lakshya at xaviers dot edu dot in"),
                             QString::fromUtf8("(C) 2018"))
            ;
}

void IpfsPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Ipfs..."));
    ac->setObjectName(QLatin1String("export_ipfs"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotIpfs()));

    addAction(ac);
}

void IpfsPlugin::slotIpfs()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new IpfsWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericIpfsPlugin

#include "moc_ipfsplugin.cpp"
