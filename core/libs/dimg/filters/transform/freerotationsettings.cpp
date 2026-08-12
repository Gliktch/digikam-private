/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : Free rotation settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "freerotationsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QUrl>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dcolorselector.h"
#include "dexpanderbox.h"
#include "dnuminput.h"
#include "dcombobox.h"

namespace Digikam
{

class Q_DECL_HIDDEN FreeRotationSettings::Private
{
public:

    Private() = default;

public:

    const QString configAutoCropTypeEntry       = QLatin1String("Auto Crop Type");
    const QString configAntiAliasingEntry       = QLatin1String("Anti Aliasing");
    const QString configBackgroundColorEntry    = QLatin1String("Background Color");

    QCheckBox*       antialiasInput             = nullptr;

    DIntNumInput*    angleInput                 = nullptr;
    DDoubleNumInput* fineAngleInput             = nullptr;
    DComboBox*       autoCropCB                 = nullptr;
    DColorSelector*  backgroundColor            = nullptr;
};

// --------------------------------------------------------

FreeRotationSettings::FreeRotationSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing       = layoutSpacing();


    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    QLabel* const label3 = new QLabel(i18n("Main angle:"));
    d->angleInput        = new DIntNumInput;
    d->angleInput->setRange(-180, 180, 1);
    d->angleInput->setDefaultValue(0);
    d->angleInput->setWhatsThis(i18n("An angle in degrees by which to rotate the image. "
                                     "A positive angle rotates the image clockwise; "
                                     "a negative angle rotates it counter-clockwise."));

    QLabel* const label4 = new QLabel(i18n("Fine angle:"));
    d->fineAngleInput    = new DDoubleNumInput;
    d->fineAngleInput->setRange(-1.0, 1.0, 0.01);
    d->fineAngleInput->setDefaultValue(0);
    d->fineAngleInput->setWhatsThis(i18n("This value in degrees will be added to main angle value "
                                         "to set fine target angle."));

    d->antialiasInput = new QCheckBox(i18n("Anti-Aliasing"));
    d->antialiasInput->setWhatsThis(i18n("Enable this option to apply the anti-aliasing filter "
                                         "to the rotated image. "
                                         "In order to smooth the target image, it will be blurred a little."));

    QLabel* const label5 = new QLabel(i18n("Auto-crop:"));
    d->autoCropCB        = new DComboBox;
    d->autoCropCB->addItem(i18nc("no autocrop", "None"));
    d->autoCropCB->addItem(i18n("Widest Area"));
    d->autoCropCB->addItem(i18n("Largest Area"));
    d->autoCropCB->setDefaultIndex(FreeRotationContainer::NoAutoCrop);
    d->autoCropCB->setWhatsThis(i18n("Select the method to process image auto-cropping "
                                     "to remove black frames around a rotated image here."));

    QLabel* const label6 = new QLabel(i18n("Background color:"));
    d->backgroundColor   = new DColorSelector();
    d->backgroundColor->setColor(Qt::black);

    // -------------------------------------------------------------

    grid->addWidget(label3,             0, 0, 1, 1);
    grid->addWidget(d->angleInput,      1, 0, 1, 2);
    grid->addWidget(label4,             2, 0, 1, 1);
    grid->addWidget(d->fineAngleInput,  3, 0, 1, 2);
    grid->addWidget(d->antialiasInput,  4, 0, 1, -1);
    grid->addWidget(label5,             5, 0, 1, 1);
    grid->addWidget(d->autoCropCB,      5, 1, 1, 1);
    grid->addWidget(label6,             6, 0, 1, 1);
    grid->addWidget(d->backgroundColor, 6, 1, 1, 1);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->antialiasInput, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->autoCropCB, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->angleInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->fineAngleInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->backgroundColor, SIGNAL(signalColorSelected(QColor)),
            this, SIGNAL(signalSettingsChanged()));
}

FreeRotationSettings::~FreeRotationSettings()
{
    delete d;
}

FreeRotationContainer FreeRotationSettings::settings() const
{
    FreeRotationContainer prm;
    prm.angle           = (double)d->angleInput->value() + d->fineAngleInput->value();
    prm.antiAlias       = d->antialiasInput->isChecked();
    prm.autoCrop        = d->autoCropCB->currentIndex();
    prm.backgroundColor = d->backgroundColor->color();

    return prm;
}

void FreeRotationSettings::setSettings(const FreeRotationContainer& settings)
{
    blockSignals(true);

    d->angleInput->setValue((int)(settings.angle));
    d->fineAngleInput->setValue(settings.angle - (double)d->angleInput->value());
    d->antialiasInput->setChecked(settings.antiAlias);
    d->autoCropCB->setCurrentIndex(settings.autoCrop);
    d->backgroundColor->setColor(settings.backgroundColor);

    blockSignals(false);
}

void FreeRotationSettings::resetToDefault()
{
    blockSignals(true);

    d->angleInput->slotReset();
    d->fineAngleInput->slotReset();
    d->antialiasInput->setChecked(true);
    d->autoCropCB->slotReset();
    d->backgroundColor->setColor(Qt::black);

    blockSignals(false);
}

FreeRotationContainer FreeRotationSettings::defaultSettings() const
{
    FreeRotationContainer prm;
    prm.angle           = d->angleInput->defaultValue();
    prm.antiAlias       = true;
    prm.autoCrop        = d->autoCropCB->defaultIndex();
    prm.backgroundColor = Qt::black;

    return prm;
}

void FreeRotationSettings::readSettings(const KConfigGroup& group)
{
    d->autoCropCB->setCurrentIndex(group.readEntry(d->configAutoCropTypeEntry,  d->autoCropCB->defaultIndex()));
    d->backgroundColor->setColor(group.readEntry(d->configBackgroundColorEntry, QColor(Qt::black)));
    d->antialiasInput->setChecked(group.readEntry(d->configAntiAliasingEntry,   true));
    d->angleInput->slotReset();
    d->fineAngleInput->slotReset();
}

void FreeRotationSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configAutoCropTypeEntry,    d->autoCropCB->currentIndex());
    group.writeEntry(d->configAntiAliasingEntry,    d->antialiasInput->isChecked());
    group.writeEntry(d->configBackgroundColorEntry, d->backgroundColor->color());
}

} // namespace Digikam

#include "moc_freerotationsettings.cpp"
