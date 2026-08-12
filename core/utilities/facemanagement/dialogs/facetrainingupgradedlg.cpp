/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-08
 * Description : confirmation of changing the face recognition model
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facetrainingupgradedlg.h"

// Qt includes

#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"
#include "facescansettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN FaceTrainingUpgradeDlg::Private
{
public:

    Private() = default;

public:

    QWidget*                page                = nullptr;
    QVBoxLayout*            verticalLayout      = nullptr;
    QGridLayout*            layoutGrid          = nullptr;
    QLabel*                 message             = nullptr;
    QLabel*                 icon                = nullptr;

    QPushButton*            startButton         = nullptr;
    QPushButton*            cancelButton        = nullptr;
};

FaceTrainingUpgradeDlg::FaceTrainingUpgradeDlg(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "Facial Recognition Database"));

    QString messageString = i18nc("@info",
                                  "Your face recognition database is outdated and requires retraining. "
                                  "This process might take anywhere from a few seconds to several minutes. "
                                  "Facial recognition will not be reliable until retraining is finished. "
                                  "Retraining will run in the background and you can continue to use digiKam. "
                                  "\n\nDo you want to retrain your database now?");

    d->page             = new QWidget(this);
    d->verticalLayout   = new QVBoxLayout(d->page);
    d->layoutGrid       = new QGridLayout(this);

    d->icon             = new QLabel(this);
    d->icon->setPixmap(QIcon::fromTheme(QLatin1String("edit-image-face-show")).pixmap(QSize(64, 64)));

    d->message          = new QLabel(messageString, this);
    d->message->setWordWrap(true);

    d->startButton      = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-ok-apply")), i18n("OK"), this);
    d->startButton->setToolTip(i18nc("@action:button", "Begin"));
    d->startButton->setDefault(true);

    d->cancelButton     = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("Cancel"), this);
    d->cancelButton->setToolTip(i18nc("@action:button", "Cancel"));

    d->layoutGrid->addWidget(d->icon,          1, 0, 2, 2);
    d->layoutGrid->addWidget(d->message,       0, 2, 10, 10);
    d->layoutGrid->addWidget(d->startButton,   11, 2, 1, 3);
    d->layoutGrid->addWidget(d->cancelButton,  11, 8, 1, 3);
    d->verticalLayout->addStretch();

    connect(d->startButton, SIGNAL(clicked()),
            this, SLOT(slotStart()));

    connect(d->cancelButton, SIGNAL(clicked()),
            this, SLOT(slotCancel()));
}

FaceTrainingUpgradeDlg::~FaceTrainingUpgradeDlg()
{
    delete d;
}

void FaceTrainingUpgradeDlg::slotStart()
{
    accept();
}

void FaceTrainingUpgradeDlg::slotCancel()
{
    reject();
}

} // namespace Digikam

#include "moc_facetrainingupgradedlg.cpp"
