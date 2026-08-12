/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-16
 * Description : a dialog to display ICC profile information.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccprofileinfodlg.h"

// Qt includes

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "iccprofilewidget.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

ICCProfileInfoDlg::ICCProfileInfoDlg(QWidget* const parent, const QString& profilePath, const IccProfile& profile)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "Color Profile Info - %1", profilePath));

    QDialogButtonBox* const buttons       = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    ICCProfileWidget* const profileWidget = new ICCProfileWidget(this, 340, 256);

    if (profile.isNull())
    {
        profileWidget->loadFromURL(QUrl::fromLocalFile(profilePath));
    }
    else
    {
        profileWidget->loadProfile(profilePath, profile);
    }

    QVBoxLayout* const vbx                = new QVBoxLayout(this);
    vbx->addWidget(profileWidget);
    vbx->addWidget(buttons);
    setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

void ICCProfileInfoDlg::slotHelp()
{
    openOnlineDocumentation(QLatin1String("color_management"));
}

} // namespace Digikam

#include "moc_iccprofileinfodlg.cpp"
