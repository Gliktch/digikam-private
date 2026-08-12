/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to MediaWiki web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawikiplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mediawikiwindow.h"

namespace DigikamGenericMediaWikiPlugin
{

MediaWikiPlugin::MediaWikiPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void MediaWikiPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString MediaWikiPlugin::name() const
{
    return i18nc("@title", "MediaWiki");
}

QString MediaWikiPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon MediaWikiPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-mediawiki"));
}

QString MediaWikiPlugin::description() const
{
    return i18nc("@info", "A tool to export to MediaWiki web-service");
}

QString MediaWikiPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to MediaWiki web-service.<br><br>"
                 "See MediaWiki web site for details: %1",
                 QLatin1String("<a href='https://www.mediawiki.org/'>https://www.mediawiki.org/</a>"));
}

QString MediaWikiPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString MediaWikiPlugin::handbookChapter() const
{
    return QLatin1String("media_wiki");
}

QList<DPluginAuthor> MediaWikiPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Alexandre Mendes"),
                             QString::fromUtf8("alex dot mendes1988 at gmail dot com"),
                             QString::fromUtf8("(C) 2011"))
            << DPluginAuthor(QString::fromUtf8("Guillaume Hormiere"),
                             QString::fromUtf8("hormiere dot guillaume at gmail dot com"),
                             QString::fromUtf8("(C) 2011-2012"))
            << DPluginAuthor(QString::fromUtf8("Peter Potrowl"),
                             QString::fromUtf8("peter dot potrowl at gmail dot com"),
                             QString::fromUtf8("(C) 2013"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2011-2025"))
            ;
}

void MediaWikiPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to MediaWiki..."));
    ac->setObjectName(QLatin1String("export_MediaWiki"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotMediaWiki()));

    addAction(ac);
}

void MediaWikiPlugin::slotMediaWiki()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new MediaWikiWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericMediaWikiPlugin

#include "moc_mediawikiplugin.cpp"
