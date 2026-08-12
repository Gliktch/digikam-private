/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to acquire images with a digital scanner.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dscannerplugin.h"

// Qt includes

#include <QPointer>
#include <QMessageBox>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "scandialog.h"

namespace DigikamGenericDScannerPlugin
{

DigitalScannerPlugin::DigitalScannerPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

void DigitalScannerPlugin::cleanUp()
{
    if (m_saneWidget)
    {
        delete m_saneWidget;
    }
}

QString DigitalScannerPlugin::name() const
{
    return i18n("Digital Scanner");
}

QString DigitalScannerPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DigitalScannerPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("scanner"));
}

QString DigitalScannerPlugin::description() const
{
    return i18n("A tool to acquire images with a digital scanner");
}

QString DigitalScannerPlugin::details() const
{
    return i18n("<p>This tool allows users to acquire new images from a digital scanner.</p>"
                "<p>Plenty of scanner devices are supported through the Sane library.</p>"
                "<p>Target image can be post processed as crop and rotate.</p>");
}

QString DigitalScannerPlugin::handbookSection() const
{
    return QLatin1String("import_tools");
}

QString DigitalScannerPlugin::handbookChapter() const
{
    return QLatin1String("scanner_import");
}

QList<DPluginAuthor> DigitalScannerPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Kare Sars"),
                             QString::fromUtf8("kare dot sars at kolumbus dot fi"),
                             QString::fromUtf8("(C) 2003-2005"))
            << DPluginAuthor(QString::fromUtf8("Angelo Naselli"),
                             QString::fromUtf8("anaselli at linux dot it"),
                             QString::fromUtf8("(C) 2006-2007"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2003-2025"))
            ;
}

void DigitalScannerPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Import from Scanner..."));
    ac->setObjectName(QLatin1String("import_scan"));
    ac->setActionCategory(DPluginAction::GenericImport);

    connect(ac, &DPluginAction::triggered,
            this, &DigitalScannerPlugin::slotDigitalScanner);

    addAction(ac);
}

void DigitalScannerPlugin::slotDigitalScanner()
{
    if (!m_saneWidget)
    {
        m_saneWidget = new KSaneIface::KSaneWidget(nullptr);
    }

    if (m_saneWidget)
    {
        QString dev = m_saneWidget->selectDevice(nullptr);

        if (dev.isEmpty())
        {
            QMessageBox::warning(nullptr, qApp->applicationName(), i18n("There is no scanner device available.\n"
                                                                        "Please check your configuration"));
            return;
        }

        if (!m_saneWidget->openDevice(dev))
        {
            // could not open a scanner

            QMessageBox::warning(nullptr, qApp->applicationName(), i18n("Cannot open scanner device \"%1\".\n"
                                                                        "Please check your configuration", dev));
            return;
        }

        DInfoInterface* const iface = infoIface(sender());
        QPointer<ScanDialog> dlg    = new ScanDialog(m_saneWidget);

        connect(dlg, &ScanDialog::signalImportedImage,
                iface, &DInfoInterface::signalImportedImage);

        dlg->setTargetDir(iface->defaultUploadUrl().toLocalFile());
        dlg->setPlugin(this);
        dlg->show();
    }
}

} // namespace DigikamGenericDScannerPlugin

#include "moc_dscannerplugin.cpp"
