/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool error view.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolloadingview.h"

// Qt includes

#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class Q_DECL_HIDDEN ExifToolLoadingView::Private
{

public:

    Private() = default;

    QLabel*         msgLbl          = nullptr;

    bool            busy            = false;
    int             progressCount   = 0;
    DWorkingPixmap* progressPix     = nullptr;
    QTimer*         progressTimer   = nullptr;
    QLabel*         progressLabel   = nullptr;
};

ExifToolLoadingView::ExifToolLoadingView(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const int spacing        = layoutSpacing();


    QGridLayout* const grid  = new QGridLayout(this);

    d->progressPix           = new DWorkingPixmap(this);
    d->progressLabel         = new QLabel(this);
    d->progressLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    d->msgLbl                = new QLabel(i18nc("info", "Loading in progress"), this);
    d->msgLbl->setAlignment(Qt::AlignCenter);
    d->msgLbl->setWordWrap(true);

    grid->addWidget(d->progressLabel, 1, 1, 1, 1);
    grid->addWidget(d->msgLbl,        2, 1, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setRowStretch(0, 10);
    grid->setRowStretch(3, 10);

    d->progressTimer         = new QTimer(this);

    connect(d->progressTimer, &QTimer::timeout,
            this, &ExifToolLoadingView::slotProgressTimerDone);
}

ExifToolLoadingView::~ExifToolLoadingView()
{
    delete d;
}

void ExifToolLoadingView::setBusy(bool b)
{
    d->busy = b;

    if (d->busy)
    {
        setCursor(Qt::WaitCursor);
        d->progressTimer->start(300);
    }
    else
    {
        unsetCursor();
        d->progressTimer->stop();
        d->progressLabel->setPixmap(QPixmap());
    }
}

void ExifToolLoadingView::slotProgressTimerDone()
{
    d->progressLabel->setPixmap(d->progressPix->frameAt(d->progressCount));
    d->progressCount++;

    if (d->progressCount == 8)
    {
        d->progressCount = 0;
    }

    d->progressTimer->start(300);
}

} // namespace Digikam

#include "moc_exiftoolloadingview.cpp"
