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

#include "exiftoolerrorview.h"

// Qt includes

#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QGridLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN ExifToolErrorView::Private
{

public:

    Private() = default;

    QLabel*           errorLbl = nullptr;

    QPushButton*      btn      = nullptr;
};

ExifToolErrorView::ExifToolErrorView(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const int spacing        = layoutSpacing();


    QGridLayout* const grid  = new QGridLayout(this);

    d->errorLbl              = new QLabel(this);
    d->errorLbl->setAlignment(Qt::AlignCenter);
    d->errorLbl->setWordWrap(true);

    d->btn                   = new QPushButton(this);
    d->btn->setText(i18nc("@action: button", "Open Setup Dialog..."));

    connect(d->btn, SIGNAL(clicked()),
            this, SIGNAL(signalSetupExifTool()));

    grid->addWidget(d->errorLbl, 1, 1, 1, 1);
    grid->addWidget(d->btn,      2, 1, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setRowStretch(0, 10);
    grid->setRowStretch(3, 10);
}

ExifToolErrorView::~ExifToolErrorView()
{
    delete d;
}

void ExifToolErrorView::setErrorText(const QString& err)
{
    d->errorLbl->setText(err);
}

} // namespace Digikam

#include "moc_exiftoolerrorview.cpp"
