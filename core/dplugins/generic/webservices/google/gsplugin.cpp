/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export and import items with Google web-service.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gsplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "gswindow.h"

namespace DigikamGenericGoogleServicesPlugin
{

GSPlugin::GSPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void GSPlugin::cleanUp()
{
    delete m_toolDlgExportGphoto;
    delete m_toolDlgImportGphoto;
    delete m_toolDlgExportGdrive;
}

QString GSPlugin::name() const
{
    return i18nc("@title", "Google");
}

QString GSPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon GSPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-googlephoto"));
}

QString GSPlugin::description() const
{
    return i18nc("@info", "A tool to export and import items with Google web-service");
}

QString GSPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export and import items with Google web-services.<br><br>"
                 "Google Photos and Google Drive web services are supported.<br><br>"
                 "See Google web sites for details: %1",
                 QLatin1String("</ br><a href='https://photos.google.com'>https://photos.google.com</a></ br><a href='https://www.google.com/drive/'>https://www.google.com/drive/</a>"));
}

QString GSPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString GSPlugin::handbookChapter() const
{
    return QLatin1String("google_export");
}

QList<DPluginAuthor> GSPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Saurabh Patel"),
                             QString::fromUtf8("saurabhpatel7717 at gmail dot co"),
                             QString::fromUtf8("(C) 2013"))
            << DPluginAuthor(QString::fromUtf8("Shourya Singh Gupta"),
                             QString::fromUtf8("shouryasgupta at gmail dot com"),
                             QString::fromUtf8("(C) 2015"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2025"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2013-2025"))
            ;
}

void GSPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac1 = new DPluginAction(parent);
    ac1->setIcon(icon());
    ac1->setText(i18nc("@action", "Export to &Google Photos..."));
    ac1->setObjectName(QLatin1String("export_googlephoto"));
    ac1->setActionCategory(DPluginAction::GenericExport);
    ac1->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_P);

    connect(ac1, SIGNAL(triggered(bool)),
            this, SLOT(slotExportGphoto()));

    addAction(ac1);

    DPluginAction* const ac2 = new DPluginAction(parent);
    ac2->setIcon(icon());
    ac2->setText(i18nc("@action", "Import from &Google Photos..."));
    ac2->setObjectName(QLatin1String("import_googlephoto"));
    ac2->setActionCategory(DPluginAction::GenericImport);
    ac2->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_P);

    connect(ac2, SIGNAL(triggered(bool)),
            this, SLOT(slotImportGphoto()));

    addAction(ac2);

    DPluginAction* const ac3 = new DPluginAction(parent);
    ac3->setIcon(QIcon::fromTheme(QLatin1String("dk-googledrive")));
    ac3->setText(i18nc("@action", "Export to &Google Drive..."));
    ac3->setObjectName(QLatin1String("export_googledrive"));
    ac3->setActionCategory(DPluginAction::GenericExport);
    ac3->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_G);

    connect(ac3, SIGNAL(triggered(bool)),
            this, SLOT(slotExportGdrive()));

    addAction(ac3);
}

void GSPlugin::slotImportGphoto()
{
    if (!reactivateToolDialog(m_toolDlgImportGphoto))
    {
        DInfoInterface* const iface = infoIface(sender());

        delete m_toolDlgImportGphoto;
        m_toolDlgImportGphoto = new GSWindow(iface, nullptr, QLatin1String("googlephotoimport"));
        m_toolDlgImportGphoto->setPlugin(this);

        connect(m_toolDlgImportGphoto, SIGNAL(updateHostApp(QUrl)),
                iface, SLOT(slotMetadataChangedForUrl(QUrl)));

        m_toolDlgImportGphoto->show();
    }
}

void GSPlugin::slotExportGphoto()
{
    if (!reactivateToolDialog(m_toolDlgExportGphoto))
    {
        delete m_toolDlgExportGphoto;
        m_toolDlgExportGphoto = new GSWindow(infoIface(sender()), nullptr, QLatin1String("googlephotoexport"));
        m_toolDlgExportGphoto->setPlugin(this);
        m_toolDlgExportGphoto->show();
    }
}

void GSPlugin::slotExportGdrive()
{
    if (!reactivateToolDialog(m_toolDlgExportGdrive))
    {
        delete m_toolDlgExportGdrive;
        m_toolDlgExportGdrive = new GSWindow(infoIface(sender()), nullptr, QLatin1String("googledriveexport"));
        m_toolDlgExportGdrive->setPlugin(this);
        m_toolDlgExportGdrive->show();
    }
}

} // namespace DigikamGenericGoogleServicesPlugin

#include "moc_gsplugin.cpp"
