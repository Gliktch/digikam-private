/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Twitter (x.com) web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "twitterplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "twitterwindow.h"

namespace DigikamGenericTwitterPlugin
{

TwitterPlugin::TwitterPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void TwitterPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString TwitterPlugin::name() const
{
    return i18nc("@title", "xdotcom");
}

QString TwitterPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TwitterPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("internet-web-browser"));
}

QString TwitterPlugin::description() const
{
    return i18nc("@info", "A tool to export to x.com (aka Twitter) web-service");
}

QString TwitterPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to x.com (aka Twitter) web-service.<br><br>"
                 "See Twitter web site for details: %1",
                 QLatin1String("<a href='https://x.com/'>https://x.com/</a>"));
}

QString TwitterPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString TwitterPlugin::handbookChapter() const
{
    return QLatin1String("twitter_export");
}

QList<DPluginAuthor> TwitterPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Thanh Trung Dinh"),
                             QString::fromUtf8("dinhthanhtrung1996 at gmail dot com"),
                             QString::fromUtf8("(C) 2019"))
            << DPluginAuthor(QString::fromUtf8("Tarek Talaat"),
                             QString::fromUtf8("tarektalaat93 at gmail dot com"),
                             QString::fromUtf8("(C) 2018"))
            ;
}

void TwitterPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to x.com (aka &Twitter)..."));
    ac->setObjectName(QLatin1String("export_twitter"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotTwitter()));

    addAction(ac);
}

void TwitterPlugin::slotTwitter()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new TwWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericTwitterPlugin

#include "moc_twitterplugin.cpp"
