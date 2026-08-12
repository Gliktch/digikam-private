/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-14
 * Description : a dialog to display image file save options.
 *
 * SPDX-FileCopyrightText: 2009      by David Eriksson <meldavid at acc umu se>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filesaveoptionsdlg.h"

// Qt includes

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

FileSaveOptionsDlg::FileSaveOptionsDlg(QWidget* const parent, FileSaveOptionsBox* const options)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Settings for Saving Image File"));

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QVBoxLayout* const vbx          = new QVBoxLayout(this);
    vbx->addWidget(options);
    vbx->addWidget(buttons);
    setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

} // namespace Digikam

#include "moc_filesaveoptionsdlg.cpp"
