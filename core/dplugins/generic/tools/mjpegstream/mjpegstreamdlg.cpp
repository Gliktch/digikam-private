/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : MJPEG Stream configuration dialog
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mjpegstreamdlg_p.h"

namespace DigikamGenericMjpegStreamPlugin
{

MjpegStreamDlg::MjpegStreamDlg(QObject* const /*parent*/,
                               DInfoInterface* const iface)
    : DPluginDialog(nullptr, MjpegServerMngr::instance()->configGroupName()),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "Share Files With MJPEG Stream Server"));
    setModal(false);
    d->spacing               = layoutSpacing();

    d->tabView               = new QTabWidget(this);

    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.

    d->settings.iface        = iface;
    d->settings.iface->setObjectName(QLatin1String("SetupMjpegStreamIface"));

    // ---

    QWidget* const itemsSel  = setupItemsView();
    setupServerView();
    setupStreamView();
    setupTransitionView();
    setupEffectView();
    setupOSDView();

    // ---

    m_buttons->addButton(QDialogButtonBox::Cancel);
    m_buttons->addButton(QDialogButtonBox::Ok);
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    vlay->addWidget(itemsSel);
    vlay->addWidget(d->tabView);
    vlay->addWidget(m_buttons);
    vlay->setStretchFactor(itemsSel,   10);
    vlay->setStretchFactor(d->tabView, 1);
    vlay->setSpacing(d->spacing);
    setLayout(vlay);

    // ---

    connect(m_buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &MjpegStreamDlg::reject);

    connect(m_buttons->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &MjpegStreamDlg::accept);

    // ---

    readSettings();
}

MjpegStreamDlg::~MjpegStreamDlg()
{
    delete d;
}

void MjpegStreamDlg::accept()
{
    if (d->dirty)
    {
        bool empty = false;

        if (d->albumSupport)
        {
            empty = d->settings.iface->albumChooserItems().isEmpty();
        }
        else
        {
            empty = d->listView->imageUrls().isEmpty();
        }

        if (!empty)
        {
            int rc = QMessageBox::question(this, i18nc("@title:window", "MJPEG Server Contents"),
                                           i18nc("@info", "The items list to share has changed. "
                                                 "Do you want to start now the MJPEG server with this contents?"));
            if (rc == QMessageBox::Yes)
            {
                startMjpegServer();
            }
        }
    }

    saveSettings();
    QDialog::accept();
}

void MjpegStreamDlg::slotOpenPreview()
{
    QDesktopServices::openUrl(QUrl(QString::fromLatin1("http://localhost:%1")       // krazy:exclude=insecurenet
                                   .arg(d->settings.port)));
}

} // namespace DigikamGenericMjpegStreamPlugin

#include "moc_mjpegstreamdlg.cpp"
