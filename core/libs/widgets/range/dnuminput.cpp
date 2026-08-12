/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-16
 * Description : Integer and double num input widget
 *               re-implemented with a reset button to switch to
 *               a default value
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnuminput.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dsliderspinbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN DIntNumInput::Private
{

public:

    Private() = default;

    int             defaultValue    = 0;

    QToolButton*    resetButton     = nullptr;

    DSliderSpinBox* input           = nullptr;
};

DIntNumInput::DIntNumInput(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QHBoxLayout* const hlay  = new QHBoxLayout(this);
    d->input                 = new DSliderSpinBox(this);
    d->resetButton           = new QToolButton(this);
    d->resetButton->setAutoRaise(true);
    d->resetButton->setFocusPolicy(Qt::NoFocus);
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18nc("@info:tooltip", "Reset to default value"));

    hlay->addWidget(d->input);
    hlay->addWidget(d->resetButton);
    hlay->setContentsMargins(QMargins());
    hlay->setStretchFactor(d->input, 10);
    hlay->setSpacing(layoutSpacing());


    // -------------------------------------------------------------

    connect(d->resetButton, &QToolButton::clicked,
            this, &DIntNumInput::slotReset);

    connect(d->input, &DSliderSpinBox::valueChanged,
            this, &DIntNumInput::slotValueChanged);
}

DIntNumInput::~DIntNumInput()
{
    delete d;
}

void DIntNumInput::setRange(int min, int max, int step)
{
    d->input->setRange(min, max);
    d->input->setSingleStep(step);
}

int DIntNumInput::value() const
{
    return d->input->value();
}

void DIntNumInput::setValue(int v)
{
    d->input->setValue(v);
}

int DIntNumInput::defaultValue() const
{
    return d->defaultValue;
}

void DIntNumInput::setDefaultValue(int v)
{
    d->defaultValue = v;
    d->input->setValue(d->defaultValue);
    slotValueChanged(v);
}

void DIntNumInput::setSuffix(const QString& suffix)
{
    d->input->setSuffix(suffix);
}

void DIntNumInput::slotReset()
{
    d->input->setValue(d->defaultValue);
    d->resetButton->setEnabled(false);

    Q_EMIT reset();
}

void DIntNumInput::slotValueChanged(int v)
{
    d->resetButton->setEnabled(v != d->defaultValue);

    Q_EMIT valueChanged(v);
}

// ----------------------------------------------------

class Q_DECL_HIDDEN DDoubleNumInput::Private
{

public:

    Private() = default;

    double                defaultValue  = 0.0;

    QToolButton*          resetButton   = nullptr;

    DDoubleSliderSpinBox* input         = nullptr;
};

DDoubleNumInput::DDoubleNumInput(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QHBoxLayout* const hlay  = new QHBoxLayout(this);
    d->input                 = new DDoubleSliderSpinBox(this);
    d->resetButton           = new QToolButton(this);
    d->resetButton->setAutoRaise(true);
    d->resetButton->setFocusPolicy(Qt::NoFocus);
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18nc("@info:tooltip", "Reset to default value"));

    hlay->addWidget(d->input);
    hlay->addWidget(d->resetButton);
    hlay->setContentsMargins(QMargins());
    hlay->setStretchFactor(d->input, 10);
    hlay->setSpacing(layoutSpacing());


    // -------------------------------------------------------------

    connect(d->resetButton, &QToolButton::clicked,
            this, &DDoubleNumInput::slotReset);

    connect(d->input, &DDoubleSliderSpinBox::valueChanged,
            this, &DDoubleNumInput::slotValueChanged);
}

DDoubleNumInput::~DDoubleNumInput()
{
    delete d;
}

void DDoubleNumInput::setDecimals(int p)
{
    d->input->setRange(d->input->minimum(), d->input->maximum(), p);
}

void DDoubleNumInput::setRange(double min, double max, double step)
{
    d->input->setRange(min, max, (int)qAbs(floor(log10(step))));
    d->input->setFastSliderStep(5 * step);
    d->input->setSingleStep(step);
}

double DDoubleNumInput::value() const
{
    return d->input->value();
}

void DDoubleNumInput::setValue(double v)
{
    d->input->setValue(v);
}

double DDoubleNumInput::defaultValue() const
{
    return d->defaultValue;
}

void DDoubleNumInput::setDefaultValue(double v)
{
    d->defaultValue = v;
    d->input->setValue(d->defaultValue);
    slotValueChanged(v);
}

void DDoubleNumInput::setSuffix(const QString& suffix)
{
    d->input->setSuffix(suffix);
}

void DDoubleNumInput::slotReset()
{
    d->input->setValue(d->defaultValue);
    d->resetButton->setEnabled(false);

    Q_EMIT reset();
}

void DDoubleNumInput::slotValueChanged(double v)
{
    d->resetButton->setEnabled(v != d->defaultValue);

    Q_EMIT valueChanged(v);
}

} // namespace Digikam

#include "moc_dnuminput.cpp"
