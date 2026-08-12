/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : plugin to email items.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sendbymailplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mailwizard.h"

namespace DigikamGenericSendByMailPlugin
{

SendByMailPlugin::SendByMailPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void SendByMailPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString SendByMailPlugin::name() const
{
    return i18n("Send by Email");
}

QString SendByMailPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SendByMailPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("mail-send"));
}

QString SendByMailPlugin::description() const
{
    return i18n("A tool to send images by E-mail");
}

QString SendByMailPlugin::details() const
{
    return i18n("<p>This tool allows users to back-process items (as resize) before to send by e-mail.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Different mail client application can be used to process files on the network.</p>");
}

QString SendByMailPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString SendByMailPlugin::handbookChapter() const
{
    return QLatin1String("send_images");
}

QList<DPluginAuthor> SendByMailPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Michael Hoechstetter"),
                             QString::fromUtf8("michael dot hoechstetter at gmx dot de"),
                             QString::fromUtf8("(C) 2006"))
            << DPluginAuthor(QString::fromUtf8("Tom Albers"),
                             QString::fromUtf8("tomalbers at kde dot nl"),
                             QString::fromUtf8("(C) 2007"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2024"),
                             i18n("Author and Maintainer"))
            ;
}

void SendByMailPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Send by Mail..."));
    ac->setObjectName(QLatin1String("sendbymail"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotSendByMail()));

    addAction(ac);
}

void SendByMailPlugin::slotSendByMail()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new MailWizard(nullptr, infoIface(sender()));
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericSendByMailPlugin

#include "moc_sendbymailplugin.cpp"
