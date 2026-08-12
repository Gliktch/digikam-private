/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to ImgUr web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imgurplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imgurwindow.h"

namespace DigikamGenericImgUrPlugin
{

ImgUrPlugin::ImgUrPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void ImgUrPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString ImgUrPlugin::name() const
{
    return i18nc("@title", "ImgUr");
}

QString ImgUrPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ImgUrPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-imgur"));
}

QString ImgUrPlugin::description() const
{
    return i18nc("@info", "A tool to export to ImgUr web-service");
}

QString ImgUrPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to ImgUr web-service.<br><br>"
                 "You can export items as anonymous or with an user account.<br><br>"
                 "See ImgUr web site for details: %1",
                 QLatin1String("<a href='https://imgur.com/'>https://imgur.com/</a>"));
}

QString ImgUrPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString ImgUrPlugin::handbookChapter() const
{
    return QLatin1String("imgur_export");
}

QList<DPluginAuthor> ImgUrPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marius Orcsik"),
                             QString::fromUtf8("marius at habarnam dot ro"),
                             QString::fromUtf8("(C) 2012-2013"))
            << DPluginAuthor(QString::fromUtf8("Fabian Vogt"),
                             QString::fromUtf8("fabian at ritter dash vogt dot de"),
                             QString::fromUtf8("(C) 2014"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2021"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2012-2025"))
            ;
}

void ImgUrPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Imgur..."));
    ac->setObjectName(QLatin1String("export_imgur"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotImgUr()));

    addAction(ac);
}

void ImgUrPlugin::slotImgUr()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new ImgurWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericImgUrPlugin

#include "moc_imgurplugin.cpp"
