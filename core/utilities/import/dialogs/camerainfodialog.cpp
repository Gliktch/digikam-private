/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-28
 * Description : a dialog to display camera information.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "camerainfodialog.h"

// Qt includes

#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextEdit>
#include <QDialogButtonBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

CameraInfoDialog::CameraInfoDialog(QWidget* const parent,
                                   const QString& summary,
                                   const QString& manual,
                                   const QString& about)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "Device Information"));
    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    resize(500, 400);

    QTabWidget* const tab           = new QTabWidget(this);

    // ----------------------------------------------------------

    QTextEdit* const summaryView    = new QTextEdit(this);
    summaryView->setWordWrapMode(QTextOption::WordWrap);
    summaryView->setText(summary);
    summaryView->setReadOnly(true);
    tab->insertTab(0, summaryView, QIcon::fromTheme(QLatin1String("dialog-information")), i18nc("@title", "Device Summary"));

    // ----------------------------------------------------------

    QTextEdit* const manualView     = new QTextEdit(this);
    manualView->setWordWrapMode(QTextOption::WordWrap);
    manualView->setText(manual);
    manualView->setReadOnly(true);
    tab->insertTab(1, manualView, QIcon::fromTheme(QLatin1String("help-contents")), i18nc("@title", "Device Manual"));

    // ----------------------------------------------------------

    QTextEdit* const aboutView      = new QTextEdit(this);
    aboutView->setWordWrapMode(QTextOption::WordWrap);
    aboutView->setText(about);
    aboutView->setReadOnly(true);
    tab->insertTab(2, aboutView, QIcon::fromTheme(QLatin1String("camera-photo")), i18nc("@title", "About Driver"));

    // ----------------------------------------------------------

    QVBoxLayout* const vbx          = new QVBoxLayout(this);
    vbx->addWidget(tab);
    vbx->addWidget(buttons);
    setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

void CameraInfoDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("import_tools"),
                            QLatin1String("camera_import"));
}

} // namespace Digikam

#include "moc_camerainfodialog.cpp"
