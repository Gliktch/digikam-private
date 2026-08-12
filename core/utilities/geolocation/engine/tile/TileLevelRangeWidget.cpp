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

#include "TileLevelRangeWidget.h"

// Local includes

#include "ui_TileLevelRangeWidget.h"

namespace Marble
{

class Q_DECL_HIDDEN TileLevelRangeWidget::Private
{
public:

    explicit Private(QWidget* const parent);

    Ui::TileLevelRangeWidget m_ui;
};

TileLevelRangeWidget::Private::Private(QWidget* const parent)
{
    m_ui.setupUi(parent);
}

TileLevelRangeWidget::TileLevelRangeWidget(QWidget* const parent, Qt::WindowFlags const f)
    : QWidget(parent, f),
      d(new Private(this))
{
    connect(d->m_ui.topSpinBox, SIGNAL(valueChanged(int)), SIGNAL(topLevelChanged(int)));
    connect(d->m_ui.bottomSpinBox, SIGNAL(valueChanged(int)),
            SIGNAL(bottomLevelChanged(int)));

    connect(d->m_ui.topSpinBox, SIGNAL(valueChanged(int)), SLOT(setMinimumBottomLevel(int)));
    connect(d->m_ui.bottomSpinBox, SIGNAL(valueChanged(int)), SLOT(setMaximumTopLevel(int)));
}

TileLevelRangeWidget::~TileLevelRangeWidget()
{
    delete d;
}

QSize TileLevelRangeWidget::sizeHint() const
{
    return size();
}

void TileLevelRangeWidget::setAllowedLevelRange(int const minimumLevel, int const maximumLevel)
{
    d->m_ui.topSpinBox->setRange(minimumLevel, qMin(d->m_ui.bottomSpinBox->value(),
                                                    maximumLevel));
    d->m_ui.bottomSpinBox->setRange(qMax(d->m_ui.topSpinBox->value(), minimumLevel),
                                    maximumLevel);
}

void TileLevelRangeWidget::setDefaultLevel(int const level)
{
    d->m_ui.topSpinBox->setValue(level);
    d->m_ui.bottomSpinBox->setValue(level);
}

int TileLevelRangeWidget::bottomLevel() const
{
    return d->m_ui.bottomSpinBox->value();
}

int TileLevelRangeWidget::topLevel() const
{
    return d->m_ui.topSpinBox->value();
}

void TileLevelRangeWidget::setMaximumTopLevel(int const level)
{
    d->m_ui.topSpinBox->setMaximum(level);
}

void TileLevelRangeWidget::setMinimumBottomLevel(int const level)
{
    d->m_ui.bottomSpinBox->setMinimum(level);
}

} // namespace Marble

#include "moc_TileLevelRangeWidget.cpp"
