/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Color Balance settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cbsettings.h"

// Qt includes

#include <QGridLayout>
#include <QSlider>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dnuminput.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN CBSettings::Private
{
public:

    Private() = default;

    const QString configRedAdjustmentEntry      = QLatin1String("RedAdjustment");
    const QString configGreenAdjustmentEntry    = QLatin1String("GreenAdjustment");
    const QString configBlueAdjustmentEntry     = QLatin1String("BlueAdjustment");

    DIntNumInput* rInput                        = nullptr;
    DIntNumInput* gInput                        = nullptr;
    DIntNumInput* bInput                        = nullptr;
};

// --------------------------------------------------------

CBSettings::CBSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing          = layoutSpacing();
    QGridLayout* const grid    = new QGridLayout(parent);

    QLabel* const labelCyan    = new QLabel(i18nc("@label: color", "Cyan"));
    labelCyan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->rInput                  = new DIntNumInput();
    d->rInput->setRange(-100, 100, 1);
    d->rInput->setDefaultValue(0);
    d->rInput->setWhatsThis(i18nc("@info", "Set here the cyan/red color adjustment of the image."));

    QLabel* const labelRed     = new QLabel(i18nc("@label: color", "Red"));
    labelRed->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // -------------------------------------------------------------

    QLabel* const labelMagenta = new QLabel(i18nc("@label: color", "Magenta"));
    labelMagenta->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->gInput = new DIntNumInput();
    d->gInput->setRange(-100, 100, 1);
    d->gInput->setDefaultValue(0);
    d->gInput->setWhatsThis(i18nc("@info", "Set here the magenta/green color adjustment of the image."));

    QLabel* const labelGreen  = new QLabel(i18nc("@label: color", "Green"));
    labelGreen->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // -------------------------------------------------------------

    QLabel* const labelYellow = new QLabel(i18nc("@label: color", "Yellow"));
    labelYellow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel* const labelBlue   = new QLabel(i18nc("@label: color", "Blue"));
    labelBlue->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->bInput                 = new DIntNumInput();
    d->bInput->setRange(-100, 100, 1);
    d->bInput->setDefaultValue(0);
    d->bInput->setWhatsThis(i18nc("@info", "Set here the yellow/blue color adjustment of the image."));

    // -------------------------------------------------------------

    grid->addWidget(labelCyan,    0, 0, 1, 1);
    grid->addWidget(d->rInput,    0, 1, 1, 1);
    grid->addWidget(labelRed,     0, 2, 1, 1);
    grid->addWidget(labelMagenta, 1, 0, 1, 1);
    grid->addWidget(d->gInput,    1, 1, 1, 1);
    grid->addWidget(labelGreen,   1, 2, 1, 1);
    grid->addWidget(labelYellow,  2, 0, 1, 1);
    grid->addWidget(d->bInput,    2, 1, 1, 1);
    grid->addWidget(labelBlue,    2, 2, 1, 1);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    grid->setRowStretch(3, 10);

    // -------------------------------------------------------------

    connect(d->rInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->gInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->bInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

CBSettings::~CBSettings()
{
    delete d;
}

CBContainer CBSettings::settings() const
{
    CBContainer prm;

    prm.red   = ((double)d->rInput->value() + 100.0) / 100.0;
    prm.green = ((double)d->gInput->value() + 100.0) / 100.0;
    prm.blue  = ((double)d->bInput->value() + 100.0) / 100.0;

    return prm;
}

void CBSettings::setSettings(const CBContainer& settings)
{
    blockSignals(true);
    d->rInput->setValue((int)((settings.red   * 100.0) - 100.0));
    d->gInput->setValue((int)((settings.green * 100.0) - 100.0));
    d->bInput->setValue((int)((settings.blue  * 100.0) - 100.0));
    blockSignals(false);
}

void CBSettings::resetToDefault()
{
    blockSignals(true);

    d->rInput->slotReset();
    d->gInput->slotReset();
    d->bInput->slotReset();

    blockSignals(false);
}

CBContainer CBSettings::defaultSettings() const
{
    CBContainer prm;

    prm.red   = ((double)d->rInput->defaultValue() + 100.0) / 100.0;
    prm.green = ((double)d->gInput->defaultValue() + 100.0) / 100.0;
    prm.blue  = ((double)d->bInput->defaultValue() + 100.0) / 100.0;

    return prm;
}

void CBSettings::readSettings(const KConfigGroup& group)
{
    CBContainer prm;
    CBContainer defaultPrm = defaultSettings();

    prm.red   = group.readEntry(d->configRedAdjustmentEntry,   defaultPrm.red);
    prm.green = group.readEntry(d->configGreenAdjustmentEntry, defaultPrm.green);
    prm.blue  = group.readEntry(d->configBlueAdjustmentEntry,  defaultPrm.blue);

    setSettings(prm);
}

void CBSettings::writeSettings(KConfigGroup& group)
{
    CBContainer prm = settings();

    group.writeEntry(d->configRedAdjustmentEntry,   prm.red);
    group.writeEntry(d->configGreenAdjustmentEntry, prm.green);
    group.writeEntry(d->configBlueAdjustmentEntry,  prm.blue);
}

} // namespace Digikam

#include "moc_cbsettings.cpp"
