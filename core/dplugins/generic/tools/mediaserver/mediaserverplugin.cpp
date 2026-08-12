/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to share items with DLNA server.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediaserverplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmediaserverdlg.h"
#include "dmediaservermngr.h"

namespace DigikamGenericMediaServerPlugin
{

MediaServerPlugin::MediaServerPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
    // Start the Media Server if necessary

    DMediaServerMngr::instance()->loadAtStartup();
}

void MediaServerPlugin::cleanUp()
{
    // Stop the Media Server if necessary

    DMediaServerMngr::instance()->saveAtShutdown();

    delete m_toolDlg;
}

QString MediaServerPlugin::name() const
{
    return i18n("DLNA Export");
}

QString MediaServerPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon MediaServerPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("go-next-skip"));
}

QString MediaServerPlugin::description() const
{
    return i18n("A tool to export items to a DLNA compatible device");
}

QString MediaServerPlugin::details() const
{
    return i18n("<p>This tool allows users to share items on the local network through a DLNA server.</p>"
                "<p>Items to share can be selected one by one or by group through a selection of albums.</p>"
                "<p>Many kind of electronic devices can support DLNA, as tablets, cellulars, TV, etc.</p>");
}

QString MediaServerPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString MediaServerPlugin::handbookChapter() const
{
    return QLatin1String("media_server");
}

QList<DPluginAuthor> MediaServerPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Ahmed Fathi"),
                             QString::fromUtf8("ahmed dot fathi dot abdelmageed at gmail dot com"),
                             QString::fromUtf8("(C) 2015"))
            << DPluginAuthor(QString::fromUtf8("Smit Mehta"),
                             QString::fromUtf8("smit dot meh at gmail dot com"),
                             QString::fromUtf8("(C) 2012-2013"))
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2012-2013"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2012-2025"),
                             i18n("Developer and Maintainer"))
            ;
}

void MediaServerPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Share with DLNA..."));
    ac->setObjectName(QLatin1String("mediaserver"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotMediaServer()));

    addAction(ac);
}

void MediaServerPlugin::slotMediaServer()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new DMediaServerDlg(nullptr, infoIface(sender()));
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericMediaServerPlugin

#include "moc_mediaserverplugin.cpp"
