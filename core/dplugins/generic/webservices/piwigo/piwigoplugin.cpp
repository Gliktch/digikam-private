/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Piwigo web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "piwigoplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "piwigowindow.h"

namespace DigikamGenericPiwigoPlugin
{

PiwigoPlugin::PiwigoPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void PiwigoPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString PiwigoPlugin::name() const
{
    return i18nc("@title", "Piwigo");
}

QString PiwigoPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PiwigoPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-piwigo"));
}

QString PiwigoPlugin::description() const
{
    return i18nc("@info", "A tool to export to Piwigo web-service");
}

QString PiwigoPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export items to Piwigo web-service."
                 "See Piwigo web site for details: %1",
                 QLatin1String("<a href='https://piwigo.org/'>https://piwigo.org/</a></p>"));
}

QString PiwigoPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString PiwigoPlugin::handbookChapter() const
{
    return QLatin1String("piwigo_export");
}

QList<DPluginAuthor> PiwigoPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2003-2005"))
            << DPluginAuthor(QString::fromUtf8("Colin Guthrie"),
                             QString::fromUtf8("kde at colin dot guthr dot ie"),
                             QString::fromUtf8("(C)  2006-2007"))
            << DPluginAuthor(QString::fromUtf8("Andrea Diamantini"),
                             QString::fromUtf8("adjam7 at gmail dot com"),
                             QString::fromUtf8("(C) 2008"))
            << DPluginAuthor(QString::fromUtf8("Frédéric Coiffier"),
                             QString::fromUtf8("frederic dot coiffier at free dot com"),
                             QString::fromUtf8("(C) 2010-2021"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2006-2025"))
            ;
}

void PiwigoPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Piwigo..."));
    ac->setObjectName(QLatin1String("export_piwigo"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPiwigo()));

    addAction(ac);
}

void PiwigoPlugin::slotPiwigo()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new PiwigoWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericPiwigoPlugin

#include "moc_piwigoplugin.cpp"
