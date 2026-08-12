/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow end view
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideend.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPalette>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

SlideEnd::SlideEnd(QWidget* const parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAutoFillBackground(true);

    const int spacing = layoutSpacing();

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    palette.setColor(foregroundRole(), Qt::white);
    setPalette(palette);

    QFont fn(font());
    fn.setPointSize(fn.pointSize() + 10);
    fn.setBold(true);
    setFont(fn);

    QLabel* const logoLabel = new QLabel(this);
    logoLabel->setAlignment(Qt::AlignTop);

    QPixmap logo;

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        logo = QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48));
    }
    else
    {
        logo = QIcon::fromTheme(QLatin1String("showfoto")).pixmap(QSize(48,48));
    }

    logoLabel->setPixmap(logo);

    QLabel* const textLabel = new QLabel(i18n("Slideshow Completed.\nClick To Exit\nor press ESC..."), this);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(logoLabel, 1, 1, 1, 1);
    grid->addWidget(textLabel, 1, 2, 1, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
}

} // namespace DigikamGenericSlideShowPlugin

#include "moc_slideend.cpp"
