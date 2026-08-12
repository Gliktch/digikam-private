/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "TileCreatorDialog.h"
#include "ui_TileCreatorDialog.h"

// Qt includes

#include <QPushButton>
#include <QTimer>

// Local includes

#include "TileCreator.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN TileCreatorDialogPrivate
{
public:

    Ui::TileCreatorDialog  uiWidget;

    TileCreator*           m_creator = nullptr;
};

TileCreatorDialog::TileCreatorDialog(TileCreator* creator, QWidget* parent)
    : QDialog(parent),
      d(new TileCreatorDialogPrivate)
{
    d->m_creator = creator;

    d->uiWidget.setupUi(this);

    connect(d->m_creator, SIGNAL(progress(int)),
            this, SLOT(setProgress(int)), Qt::QueuedConnection);
    connect(d->uiWidget.buttonBox, SIGNAL(rejected()),
            this, SLOT(cancelTileCreation()));

    // Start the creation process
    d->m_creator->start();
}

void TileCreatorDialog::cancelTileCreation()
{
    d->uiWidget.buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);

    /** @todo: Cancelling mostly crashes Marble. Fix that and uncomment below */
    // d->m_creator->cancelTileCreation();
}

TileCreatorDialog::~TileCreatorDialog()
{
    disconnect(d->m_creator, SIGNAL(progress(int)),
               this, SLOT(setProgress(int)));

    if (d->m_creator->isRunning())
    {
        d->m_creator->cancelTileCreation();
    }

    d->m_creator->wait();
    d->m_creator->deleteLater();
    delete d;
}

void TileCreatorDialog::setProgress(int progress)
{
    d->uiWidget.progressBar->setValue(progress);

    if (progress == 100)
    {
        QTimer::singleShot(0, this, SLOT(accept()));
    }
}

void TileCreatorDialog::setSummary(const QString& name,
                                   const QString& description)
{
    const QString summary = QLatin1String("<b>") + name + QLatin1String("</b><br>") + description;
    d->uiWidget.descriptionLabel->setText(summary);
}

} // namespace Marble

#include "moc_TileCreatorDialog.cpp"
