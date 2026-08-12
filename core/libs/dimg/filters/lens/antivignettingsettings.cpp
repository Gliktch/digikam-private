/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-15
 * Description : Anti vignetting settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "antivignettingsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QPainter>
#include <QPixmap>
#include <QUrl>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dnuminput.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN AntiVignettingSettings::Private
{
public:

    Private() = default;

    const QString configAddVignettingAdjustmentEntry    = QLatin1String("AddVignettingAdjustment");
    const QString configDensityAdjustmentEntry          = QLatin1String("DensityAdjustment");
    const QString configPowerAdjustmentEntry            = QLatin1String("PowerAdjustment");
    const QString configInnerRadiusAdjustmentEntry      = QLatin1String("InnerRadiusAdjustment");
    const QString configOuterRadiusAdjustmentEntry      = QLatin1String("OuterRadiusAdjustment");
    const QString configAddVignettingEntry              = QLatin1String("AddVignetting");
    const QString configXOffsetEntry                    = QLatin1String("XOffset");
    const QString configYOffsetEntry                    = QLatin1String("YOffset");

    QCheckBox*            addVignettingCheck            = nullptr;

    QLabel*               maskPreviewLabel              = nullptr;

    DDoubleNumInput*      densityInput                  = nullptr;
    DDoubleNumInput*      powerInput                    = nullptr;
    DDoubleNumInput*      innerRadiusInput              = nullptr;
    DDoubleNumInput*      outerRadiusInput              = nullptr;
    DDoubleNumInput*      xOffsetInput                  = nullptr;
    DDoubleNumInput*      yOffsetInput                  = nullptr;
};

// --------------------------------------------------------

AntiVignettingSettings::AntiVignettingSettings(QWidget* parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing       = layoutSpacing();
    QGridLayout* const grid = new QGridLayout(parent);

    d->addVignettingCheck   = new QCheckBox(i18n("Add vignetting"));
    d->addVignettingCheck->setWhatsThis(i18n("This option adds vignetting to the image instead of removing it. "
                                             "Use it for creative effects."));
    d->addVignettingCheck->setChecked(false);

    // -------------------------------------------------------------

    d->maskPreviewLabel     = new QLabel();
    d->maskPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->maskPreviewLabel->setPixmap(QPixmap(120, 120));
    d->maskPreviewLabel->setWhatsThis(i18n("You can see here a thumbnail preview of the anti-vignetting "
                                           "mask applied to the image."));

    // -------------------------------------------------------------

    QLabel* const label1 = new QLabel(i18n("Amount:"));
    d->densityInput      = new DDoubleNumInput();
    d->densityInput->setDecimals(1);
    d->densityInput->setRange(0.05, 5.0, 0.05);
    d->densityInput->setDefaultValue(1.0);
    d->densityInput->setWhatsThis(i18n("This value controls the degree of intensity "
                                       "of the filter at its point of maximum density."));

    // -------------------------------------------------------------

    QLabel* const label2 = new QLabel(i18n("Feather:"));
    d->powerInput        = new DDoubleNumInput();
    d->powerInput->setDecimals(1);
    d->powerInput->setRange(1.0, 4.0, 0.05);
    d->powerInput->setDefaultValue(1.5);
    d->powerInput->setWhatsThis(i18n("This value is used as the exponent controlling the "
                                     "fall-off in density from the inner circle of the filter "
                                     "to the outer circle."));

    // -------------------------------------------------------------

    QLabel* const label3 = new QLabel(i18n("Inner Radius:"));
    d->innerRadiusInput  = new DDoubleNumInput();
    d->innerRadiusInput->setDecimals(1);
    d->innerRadiusInput->setRange(0.0, 0.9, 0.05);
    d->innerRadiusInput->setDefaultValue(0.5);
    d->innerRadiusInput->setWhatsThis(i18n("This value is the radius of the inner circle of the filter. "
                                           "In the inner circle the image is preserved. "
                                           "It is expressed as a multiple of the outer radius of the filter."));

    // -------------------------------------------------------------

    QLabel* const label4 = new QLabel(i18n("Outer Radius:"));
    d->outerRadiusInput  = new DDoubleNumInput();
    d->outerRadiusInput->setDecimals(1);
    d->outerRadiusInput->setRange(0.1, 2.0, 0.05);
    d->outerRadiusInput->setDefaultValue(1.0);
    d->outerRadiusInput->setWhatsThis(i18n("This value is the radius of the outer circle of the filter. "
                                           "Outside the outer circle the effect of the filter is at its maximum. "
                                           "It is expressed as a multiple of the diagonal of the image."));

    // -------------------------------------------------------------

    QLabel* const label5 = new QLabel(i18n("X offset:"));
    d->xOffsetInput      = new DDoubleNumInput();
    d->xOffsetInput->setDecimals(0);
    d->xOffsetInput->setRange(-100, 100, 1);
    d->xOffsetInput->setDefaultValue(0);
    d->xOffsetInput->setWhatsThis(i18n("X offset "));

    // -------------------------------------------------------------

    QLabel* const label6 = new QLabel(i18n("Y offset:"));
    d->yOffsetInput      = new DDoubleNumInput();
    d->yOffsetInput->setDecimals(0);
    d->yOffsetInput->setRange(-100, 100, 1);
    d->yOffsetInput->setDefaultValue(0);
    d->yOffsetInput->setWhatsThis(i18n("Y offset "));

    DLineWidget* const line = new DLineWidget(Qt::Horizontal);

    // -------------------------------------------------------------

    grid->addWidget(d->maskPreviewLabel,    0, 0, 1, 3);
    grid->addWidget(label1,                 1, 0, 1, 3);
    grid->addWidget(d->densityInput,        2, 0, 1, 3);
    grid->addWidget(label2,                 3, 0, 1, 3);
    grid->addWidget(d->powerInput,          4, 0, 1, 3);
    grid->addWidget(label3,                 5, 0, 1, 3);
    grid->addWidget(d->innerRadiusInput,    6, 0, 1, 3);
    grid->addWidget(label4,                 7, 0, 1, 3);
    grid->addWidget(d->outerRadiusInput,    8, 0, 1, 3);
    grid->addWidget(label5,                 9, 0, 1, 3);
    grid->addWidget(d->xOffsetInput,       10, 0, 1, 3);
    grid->addWidget(label6,                11, 0, 1, 3);
    grid->addWidget(d->yOffsetInput,       12, 0, 1, 3);
    grid->addWidget(line,                  13, 0, 1, 3);
    grid->addWidget(d->addVignettingCheck, 14, 0, 1, 3);
    grid->setRowStretch(15, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->densityInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(d->powerInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(d->innerRadiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(d->outerRadiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(d->xOffsetInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(d->yOffsetInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(d->addVignettingCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    // -------------------------------------------------------------

    slotSettingsChanged();
}

AntiVignettingSettings::~AntiVignettingSettings()
{
    delete d;
}

AntiVignettingContainer AntiVignettingSettings::settings() const
{
    AntiVignettingContainer prm;

    prm.addvignetting = d->addVignettingCheck->isChecked();
    prm.density       = d->densityInput->value();
    prm.power         = d->powerInput->value();
    prm.innerradius   = d->innerRadiusInput->value();
    prm.outerradius   = d->outerRadiusInput->value();
    prm.xshift        = d->xOffsetInput->value();
    prm.yshift        = d->yOffsetInput->value();

    return prm;
}

void AntiVignettingSettings::setSettings(const AntiVignettingContainer& settings)
{
    blockSignals(true);

    d->addVignettingCheck->setChecked(settings.addvignetting);
    d->densityInput->setValue(settings.density);
    d->powerInput->setValue(settings.power);
    d->innerRadiusInput->setValue(settings.innerradius);
    d->outerRadiusInput->setValue(settings.outerradius);
    d->xOffsetInput->setValue(settings.xshift);
    d->yOffsetInput->setValue(settings.yshift);

    blockSignals(false);
}

void AntiVignettingSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

AntiVignettingContainer AntiVignettingSettings::defaultSettings() const
{
    AntiVignettingContainer prm;

    return prm;
}

void AntiVignettingSettings::readSettings(const KConfigGroup& group)
{
    AntiVignettingContainer prm;
    AntiVignettingContainer defaultPrm = defaultSettings();

    prm.addvignetting = group.readEntry(d->configAddVignettingEntry,         defaultPrm.addvignetting);
    prm.density       = group.readEntry(d->configDensityAdjustmentEntry,     defaultPrm.density);
    prm.power         = group.readEntry(d->configPowerAdjustmentEntry,       defaultPrm.power);
    prm.innerradius   = group.readEntry(d->configInnerRadiusAdjustmentEntry, defaultPrm.innerradius);
    prm.outerradius   = group.readEntry(d->configOuterRadiusAdjustmentEntry, defaultPrm.outerradius);
    prm.xshift        = group.readEntry(d->configXOffsetEntry,               defaultPrm.xshift);
    prm.yshift        = group.readEntry(d->configYOffsetEntry,               defaultPrm.yshift);

    setSettings(prm);
}

void AntiVignettingSettings::writeSettings(KConfigGroup& group)
{
    AntiVignettingContainer prm = settings();

    group.writeEntry(d->configAddVignettingEntry,         prm.addvignetting);
    group.writeEntry(d->configDensityAdjustmentEntry,     prm.density);
    group.writeEntry(d->configPowerAdjustmentEntry,       prm.power);
    group.writeEntry(d->configInnerRadiusAdjustmentEntry, prm.innerradius);
    group.writeEntry(d->configOuterRadiusAdjustmentEntry, prm.outerradius);
    group.writeEntry(d->configXOffsetEntry,               prm.xshift);
    group.writeEntry(d->configYOffsetEntry,               prm.yshift);
}

void AntiVignettingSettings::slotSettingsChanged()
{
    // Compute preview mask.

    DImg preview(120, 120, false);
    memset(preview.bits(), 255, preview.numBytes());
    AntiVignettingFilter maskPreview(&preview, nullptr, settings());
    maskPreview.startFilterDirectly();
    QPixmap pix = maskPreview.getTargetImage().convertToPixmap();
    QPainter pt(&pix);
    pt.setPen(QPen(Qt::black, 1));
    pt.drawRect(0, 0, pix.width(), pix.height());
    pt.end();
    d->maskPreviewLabel->setPixmap(pix);

    Q_EMIT signalSettingsChanged();
}

} // namespace Digikam

#include "moc_antivignettingsettings.cpp"
