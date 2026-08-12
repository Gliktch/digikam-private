/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Flickr web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "flickrplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "flickrwindow.h"

namespace DigikamGenericFlickrPlugin
{

FlickrPlugin::FlickrPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void FlickrPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString FlickrPlugin::name() const
{
    return i18nc("@title", "Flickr");
}

QString FlickrPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FlickrPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-flickr"));
}

QString FlickrPlugin::description() const
{
    return i18nc("@info", "A tool to export to Flickr web-service");
}

QString FlickrPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to Flickr web-service.<br><br>"
                 "See Flickr web site for details: %1",
                 QLatin1String("<a href='https://www.flickr.com/'>https://www.flickr.com/</a>"));
}

QString FlickrPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString FlickrPlugin::handbookChapter() const
{
    return QLatin1String("flickr_export");
}

QList<DPluginAuthor> FlickrPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Vardhman Jain"),
                             QString::fromUtf8("vardhman at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2008"))
            << DPluginAuthor(QString::fromUtf8("Luka Renko"),
                             QString::fromUtf8("lure at kubuntu dot org"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Shourya Singh Gupta"),
                             QString::fromUtf8("shouryasgupta at gmail dot com"),
                             QString::fromUtf8("(C) 2015"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2021"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2024"))
            ;
}

void FlickrPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Flickr..."));
    ac->setObjectName(QLatin1String("export_flickr"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_R);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFlickr()));

    addAction(ac);
}

void FlickrPlugin::slotFlickr()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new FlickrWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericFlickrPlugin

#include "moc_flickrplugin.cpp"
