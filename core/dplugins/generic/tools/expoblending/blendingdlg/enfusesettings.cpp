/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "enfusesettings.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QApplication>
#include <QSpinBox>
#include <QDoubleSpinBox>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"

namespace DigikamGenericExpoBlendingPlugin
{

QString EnfuseSettings::asCommentString() const
{
    QString ret;

    ret.append(hardMask ? i18nc("@info", "Hardmask: enabled") : i18nc("@info", "Hardmask: disabled"));
    ret.append(QLatin1Char('\n'));
    ret.append(ciecam02 ? i18nc("@info", "CIECAM02: enabled") : i18nc("@info", "CIECAM02: disabled"));
    ret.append(QLatin1Char('\n'));
    ret.append(autoLevels ? i18nc("@info", "Levels: auto")    : i18nc("@info", "Levels: <numid>%1</numid>", levels));
    ret.append(QLatin1Char('\n'));
    ret.append(i18nc("@info", "Exposure: <numid>%1</numid>",   exposure));
    ret.append(QLatin1Char('\n'));
    ret.append(i18nc("@info", "Saturation: <numid>%1</numid>", saturation));
    ret.append(QLatin1Char('\n'));
    ret.append(i18nc("@info", "Contrast: <numid>%1</numid>",   contrast));

    return ret;
}

QString EnfuseSettings::inputImagesList() const
{
    QString ret;

    for (const QUrl& url : std::as_const(inputUrls))
    {
        ret.append(url.fileName() + QLatin1String(" ; "));
    }

    ret.truncate(ret.length() - 3);

    return ret;
}

class Q_DECL_HIDDEN EnfuseSettingsWidget::Private
{
public:

    Private() = default;

public:

    QCheckBox*       autoLevelsCB       = nullptr;
    QCheckBox*       hardMaskCB         = nullptr;
    QCheckBox*       ciecam02CB         = nullptr;

    QLabel*          levelsLabel        = nullptr;
    QLabel*          exposureLabel      = nullptr;
    QLabel*          saturationLabel    = nullptr;
    QLabel*          contrastLabel      = nullptr;

    QSpinBox*        levelsInput        = nullptr;

    QDoubleSpinBox*  exposureInput      = nullptr;
    QDoubleSpinBox*  saturationInput    = nullptr;
    QDoubleSpinBox*  contrastInput      = nullptr;
};

EnfuseSettingsWidget::EnfuseSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing       = layoutSpacing();

    QGridLayout* const grid = new QGridLayout(this);

    // ------------------------------------------------------------------------

    d->autoLevelsCB = new QCheckBox(i18nc("@option:check Enfuse setting", "Automatic Local/Global Image Features Balance (Levels)"), this);
    d->autoLevelsCB->setToolTip(i18nc("@info:tooltip",
                                      "Optimize image features (contrast, saturation, . . .) to be as global as possible."));
    d->autoLevelsCB->setWhatsThis(i18nc("@info:whatsthis",
                                        "Set automatic level selection (maximized) for pyramid blending, "
                                        "i.e. optimize image features (contrast, saturation, . . .) to be as global as possible."));

    d->levelsLabel  = new QLabel(i18nc("@label:slider Enfuse settings", "Image Features Balance:"));
    d->levelsInput  = new QSpinBox(this);
    d->levelsInput->setRange(1, 29);
    d->levelsInput->setSingleStep(1);
    d->levelsInput->setToolTip(i18nc("@info:tooltip",
                                     "Balances between local features (small number) or global features (high number)."));
    d->levelsInput->setWhatsThis(i18nc("@info:whatsthis",
                                       "Set the number of levels for pyramid blending. "
                                       "Balances towards local features (small number) or global features (high number). "
                                       "Additionally, a low number trades off quality of results for faster "
                                       "execution time and lower memory usage."));

    d->hardMaskCB = new QCheckBox(i18nc("@option:check", "Hard Mask"), this);
    d->hardMaskCB->setToolTip(i18nc("@info:tooltip",
                                    "Useful only for focus stack to improve sharpness."));
    d->hardMaskCB->setWhatsThis(i18nc("@info:whatsthis",
                                      "Force hard blend masks without averaging on finest "
                                      "scale. This is only useful for focus "
                                      "stacks with thin and high contrast features. "
                                      "It improves sharpness at the expense of increased noise."));

    d->exposureLabel = new QLabel(i18nc("@label:slider Enfuse settings", "Well-Exposedness Contribution:"));
    d->exposureInput = new QDoubleSpinBox(this);
    d->exposureInput->setRange(0.0, 1.0);
    d->exposureInput->setSingleStep(0.01);
    d->exposureInput->setToolTip(i18nc("@info:tooltip",
                                       "Contribution of well exposed pixels to the blending process."));
    d->exposureInput->setWhatsThis(i18nc("@info:whatsthis",
                                         "Set the well-exposedness criterion contribution for the blending process. "
                                         "Higher values will favor well-exposed pixels."));

    d->saturationLabel = new QLabel(i18nc("@label:slider enfuse settings", "High-Saturation Contribution:"));
    d->saturationInput = new QDoubleSpinBox(this);
    d->saturationInput->setDecimals(2);
    d->saturationInput->setRange(0.0, 1.0);
    d->saturationInput->setSingleStep(0.01);
    d->saturationInput->setToolTip(i18nc("@info:tooltip",
                                         "Contribution of highly saturated pixels to the blending process."));
    d->saturationInput->setWhatsThis(i18nc("@info:whatsthis",
                                           "Increasing this value makes pixels with high "
                                           "saturation contribute more to the final output."));

    d->contrastLabel = new QLabel(i18nc("@label:slider enfuse settings", "High-Contrast Contribution:"));
    d->contrastInput = new QDoubleSpinBox(this);
    d->contrastInput->setDecimals(2);
    d->contrastInput->setRange(0.0, 1.0);
    d->contrastInput->setSingleStep(0.01);
    d->contrastInput->setToolTip(i18nc("@info:tooltip",
                                       "Contribution of highly contrasted pixels to the blending process."));
    d->contrastInput->setWhatsThis(i18nc("@info:whatsthis",
                                         "Sets the relative weight of high-contrast pixels. "
                                         "Increasing this weight makes pixels with neighboring differently colored "
                                         "pixels contribute more to the final output. Particularly useful for focus stacks."));

    d->ciecam02CB = new QCheckBox(i18nc("@option:check", "Use Color Appearance Model (CIECAM02)"), this);
    d->ciecam02CB->setToolTip(i18nc("@info:tooltip",
                                    "Convert to CIECAM02 color appearance model during the blending process instead of RGB."));
    d->ciecam02CB->setWhatsThis(i18nc("@info:whatsthis",
                                      "Use Color Appearance Modelling (CIECAM02) to render detailed colors. "
                                      "Your input files should have embedded ICC profiles. If no ICC profile is present, "
                                      "sRGB color space will be assumed. The difference between using this option "
                                      "and default color blending algorithm is very slight, and will be most noticeable "
                                      "when you need to blend areas of different primary colors together."));

    // ------------------------------------------------------------------------

    grid->addWidget(d->autoLevelsCB,    0, 0, 1, 2);
    grid->addWidget(d->levelsLabel,     1, 0, 1, 1);
    grid->addWidget(d->levelsInput,     1, 1, 1, 1);
    grid->addWidget(d->hardMaskCB,      2, 0, 1, 2);
    grid->addWidget(d->exposureLabel,   3, 0, 1, 1);
    grid->addWidget(d->exposureInput,   3, 1, 1, 1);
    grid->addWidget(d->saturationLabel, 4, 0, 1, 1);
    grid->addWidget(d->saturationInput, 4, 1, 1, 1);
    grid->addWidget(d->contrastLabel,   5, 0, 1, 1);
    grid->addWidget(d->contrastInput,   5, 1, 1, 1);
    grid->addWidget(d->ciecam02CB,      6, 0, 1, 2);
    grid->setRowStretch(7, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // ------------------------------------------------------------------------

    connect(d->autoLevelsCB, SIGNAL(toggled(bool)),
            d->levelsLabel, SLOT(setDisabled(bool)));

    connect(d->autoLevelsCB, SIGNAL(toggled(bool)),
            d->levelsInput, SLOT(setDisabled(bool)));
}

EnfuseSettingsWidget::~EnfuseSettingsWidget()
{
    delete d;
}

void EnfuseSettingsWidget::resetToDefault()
{
    d->autoLevelsCB->setChecked(true);
    d->levelsInput->setValue(20);
    d->hardMaskCB->setChecked(false);
    d->exposureInput->setValue(1.0);
    d->saturationInput->setValue(0.2);
    d->contrastInput->setValue(0.0);
    d->ciecam02CB->setChecked(false);
}

void EnfuseSettingsWidget::setSettings(const EnfuseSettings& settings)
{
    d->autoLevelsCB->setChecked(settings.autoLevels);
    d->levelsInput->setValue(settings.levels);
    d->hardMaskCB->setChecked(settings.hardMask);
    d->exposureInput->setValue(settings.exposure);
    d->saturationInput->setValue(settings.saturation);
    d->contrastInput->setValue(settings.contrast);
    d->ciecam02CB->setChecked(settings.ciecam02);
}

EnfuseSettings EnfuseSettingsWidget::settings() const
{
    EnfuseSettings settings;
    settings.autoLevels = d->autoLevelsCB->isChecked();
    settings.levels     = d->levelsInput->value();
    settings.hardMask   = d->hardMaskCB->isChecked();
    settings.exposure   = d->exposureInput->value();
    settings.saturation = d->saturationInput->value();
    settings.contrast   = d->contrastInput->value();
    settings.ciecam02   = d->ciecam02CB->isChecked();

    return settings;
}

void EnfuseSettingsWidget::readSettings(const KConfigGroup& group)
{
    d->autoLevelsCB->setChecked(group.readEntry("Auto Levels",       true));
    d->levelsInput->setValue(group.readEntry("Levels Value",         20));
    d->hardMaskCB->setChecked(group.readEntry("Hard Mask",           false));
    d->exposureInput->setValue(group.readEntry("Exposure Value",     1.0));
    d->saturationInput->setValue(group.readEntry("Saturation Value", 0.2));
    d->contrastInput->setValue(group.readEntry("Contrast Value",     0.0));
    d->ciecam02CB->setChecked(group.readEntry("CIECAM02",            false));
}

void EnfuseSettingsWidget::writeSettings(KConfigGroup& group)
{
    group.writeEntry("Auto Levels",      d->autoLevelsCB->isChecked());
    group.writeEntry("Levels Value",     d->levelsInput->value());
    group.writeEntry("Hard Mask",        d->hardMaskCB->isChecked());
    group.writeEntry("Exposure Value",   d->exposureInput->value());
    group.writeEntry("Saturation Value", d->saturationInput->value());
    group.writeEntry("Contrast Value",   d->contrastInput->value());
    group.writeEntry("CIECAM02",         d->ciecam02CB->isChecked());
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_enfusesettings.cpp"
