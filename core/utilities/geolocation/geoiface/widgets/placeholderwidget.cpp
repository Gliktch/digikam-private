/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-05
 * Description : Placeholder widget for when backends are activated
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "placeholderwidget.h"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN PlaceholderWidget::Private
{
public:

    Private() = default;

    QLabel* messageLabel = nullptr;
};

PlaceholderWidget::PlaceholderWidget(QWidget* const parent)
    : QFrame(parent),
      d     (new Private)
{
    QVBoxLayout* const vboxlayout = new QVBoxLayout();
    setLayout(vboxlayout);

    d->messageLabel               = new QLabel(i18n("Geolocation Interface"), this);
}

PlaceholderWidget::~PlaceholderWidget()
{
}

void PlaceholderWidget::setMessage(const QString& message)
{
    d->messageLabel->setText(message);
}

} // namespace Digikam

#include "moc_placeholderwidget.cpp"
