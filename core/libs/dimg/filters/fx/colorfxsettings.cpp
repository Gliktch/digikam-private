/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-11-08
 * Description : Color effects settings view.
 *
 * SPDX-FileCopyrightText: 2012 by Alexander Dymo <adymo at kdevelop dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorfxsettings.h"

// Qt includes

#include <QStandardPaths>
#include <QStackedWidget>
#include <QApplication>
#include <QDirIterator>
#include <QGridLayout>
#include <QStringList>
#include <QLabel>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "previewlist.h"
#include "imageiface.h"
#include "dcombobox.h"
#include "dnuminput.h"

namespace Digikam
{

class Q_DECL_HIDDEN ColorFXSettings::Private
{
public:

    Private() = default;

    const QString configEffectTypeEntry             = QLatin1String("EffectType");
    const QString configLevelAdjustmentEntry        = QLatin1String("LevelAdjustment");
    const QString configIterationAdjustmentEntry    = QLatin1String("IterationAdjustment");
    const QString configLut3DFilterEntry            = QLatin1String("Lut3D Color Correction Filter");
    const QString configLut3DIntensityEntry         = QLatin1String("Lut3D Color Correction Intensity");

    QStackedWidget* stack                           = nullptr;

    DComboBox*      effectType                      = nullptr;

    DIntNumInput*   levelInput                      = nullptr;
    DIntNumInput*   iterationInput                  = nullptr;
    DIntNumInput*   intensityInput                  = nullptr;

    QLabel*         iterationLabel                  = nullptr;

    PreviewList*    correctionTools                 = nullptr;

    QStringList     luts;
};

// --------------------------------------------------------

ColorFXSettings::ColorFXSettings(QWidget* const parent, bool useGenericImg)
    : QWidget(parent),
      d      (new Private)
{
    DImg thumbImage;

    findLuts();

    if (useGenericImg)
    {
        QString backGround = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                    QLatin1String("digikam/about/images/body-background.jpg"));

        thumbImage         = DImg(backGround).smoothScale(128, 128, Qt::KeepAspectRatio);
    }
    else
    {
        ImageIface iface;
        thumbImage = iface.original()->smoothScale(128, 128, Qt::KeepAspectRatio);
    }

    // -------------------------------------------------------------

    const int spacing             = layoutSpacing();
    QGridLayout* const grid       = new QGridLayout(parent);

    QLabel* const effectTypeLabel = new QLabel(i18nc("@label", "Type:"), parent);
    d->effectType                 = new DComboBox(parent);
    d->effectType->addItem(i18nc("@item: color effect", "Solarize"));
    d->effectType->addItem(i18nc("@item: color effect", "Vivid"));
    d->effectType->addItem(i18nc("@item: color effect", "Neon"));
    d->effectType->addItem(i18nc("@item: color effect", "Find Edges"));
    d->effectType->addItem(i18nc("@item: color effect", "Lut3D"));
    d->effectType->setDefaultIndex(ColorFXFilter::Solarize);
    d->effectType->setWhatsThis(i18nc("@info",
                                      "Select the effect type to apply to the image here.\n"
                                      "\"Solarize\": simulates solarization of photograph.\n"
                                      "\"Vivid\": simulates the Velvia(tm) slide film colors.\n"
                                      "\"Neon\": coloring the edges in a photograph to\n"
                                      "reproduce a fluorescent light effect.\n"
                                      "\"Find Edges\": detects the edges in a photograph\n"
                                      "and their strength.\n"
                                      "\"Lut3D\": coloring images with Lut3D filters"));

    d->stack                      = new QStackedWidget(parent);

    grid->addWidget(effectTypeLabel,                         0, 0, 1, 1);
    grid->addWidget(d->effectType,                           1, 0, 1, 1);
    grid->addWidget(new DLineWidget(Qt::Horizontal, parent), 2, 0, 1, 1);
    grid->addWidget(d->stack,                                3, 0, 1, 1);
    grid->setRowStretch(3, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    QWidget* const solarizeSettings = new QWidget(d->stack);
    QGridLayout* const grid1        = new QGridLayout(solarizeSettings);

    QLabel* const levelLabel        = new QLabel(i18nc("@label: level of the effect", "Level:"), solarizeSettings);
    d->levelInput                   = new DIntNumInput(solarizeSettings);
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setDefaultValue(3);
    d->levelInput->setWhatsThis(i18nc("@info", "Set here the level of the effect."));

    d->iterationLabel               = new QLabel(i18nc("@label", "Iteration:"), solarizeSettings);
    d->iterationInput               = new DIntNumInput(solarizeSettings);
    d->iterationInput->setRange(0, 100, 1);
    d->iterationInput->setDefaultValue(2);
    d->iterationInput->setWhatsThis(i18nc("@info", "This value controls the number of iterations "
                                          "to use with the Neon and Find Edges effects."));

    grid1->addWidget(levelLabel,        0, 0, 1, 1);
    grid1->addWidget(d->levelInput,     1, 0, 1, 1);
    grid1->addWidget(d->iterationLabel, 2, 0, 1, 1);
    grid1->addWidget(d->iterationInput, 3, 0, 1, 1);
    grid1->setRowStretch(4, 10);
    grid1->setContentsMargins(QMargins());
    grid1->setSpacing(0);

    d->stack->insertWidget(0, solarizeSettings);

    // -------------------------------------------------------------

    QWidget* const lut3DSettings = new QWidget(d->stack);
    QGridLayout* const grid2     = new QGridLayout(lut3DSettings);

    d->correctionTools           = new PreviewList(lut3DSettings);

    for (int idx = 0 ; idx < d->luts.count() ; ++idx)
    {
        ColorFXContainer prm;
        prm.colorFXType = ColorFXFilter::Lut3D;
        prm.path        = d->luts[idx];

        QFileInfo fi(prm.path);

        d->correctionTools->addItem(new ColorFXFilter(&thumbImage, lut3DSettings, prm),
                                                      translateLuts(fi.baseName()), idx);
    }

    QLabel* const intensityLabel = new QLabel(i18nc("@label", "Intensity:"), lut3DSettings);
    d->intensityInput            = new DIntNumInput(lut3DSettings);
    d->intensityInput->setRange(1, 100, 1);
    d->intensityInput->setDefaultValue(100);
    d->intensityInput->setWhatsThis(i18nc("@info", "Set here the intensity of the filter."));

    grid2->addWidget(d->correctionTools, 0, 0, 1, 1);
    grid2->addWidget(intensityLabel,     1, 0, 1, 1);
    grid2->addWidget(d->intensityInput,  2, 0, 1, 1);
    grid2->setRowStretch(0, 10);
    grid2->setContentsMargins(QMargins());
    grid2->setSpacing(0);

    d->stack->insertWidget(1, lut3DSettings);

    // -------------------------------------------------------------

    connect(d->effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));

    connect(d->levelInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->iterationInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->correctionTools, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->intensityInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

ColorFXSettings::~ColorFXSettings()
{
    delete d;
}

void ColorFXSettings::startPreviewFilters()
{
    d->correctionTools->startFilters();
}

void ColorFXSettings::slotEffectTypeChanged(int type)
{
    d->iterationInput->blockSignals(true);
    d->levelInput->blockSignals(true);

    int w = (type == ColorFXFilter::Lut3D ? 1 : 0);
    d->stack->setCurrentWidget(d->stack->widget(w));

    switch (type)
    {
        case ColorFXFilter::Solarize:
        {
            d->levelInput->setRange(0, 100, 1);
            d->levelInput->setValue(20);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;
        }

        case ColorFXFilter::Vivid:
        {
            d->levelInput->setRange(0, 50, 1);
            d->levelInput->setValue(5);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;
        }

        case ColorFXFilter::Neon:
        case ColorFXFilter::FindEdges:
        {
            d->levelInput->setRange(0, 5, 1);
            d->levelInput->setValue(3);
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            d->iterationInput->setRange(0, 5, 1);
            d->iterationInput->setValue(2);
            break;
        }
    }

    d->iterationInput->blockSignals(false);
    d->levelInput->blockSignals(false);

    Q_EMIT signalSettingsChanged();
}

ColorFXContainer ColorFXSettings::settings() const
{
    ColorFXContainer prm;

    prm.colorFXType = d->effectType->currentIndex();
    prm.level       = d->levelInput->value();
    prm.iterations  = d->iterationInput->value();
    prm.intensity   = d->intensityInput->value();
    prm.path        = d->luts.value(d->correctionTools->currentId());

    return prm;
}

void ColorFXSettings::setSettings(const ColorFXContainer& settings)
{
    blockSignals(true);

    d->effectType->setCurrentIndex(settings.colorFXType);
    slotEffectTypeChanged(settings.colorFXType);

    d->levelInput->setValue(settings.level);
    d->iterationInput->setValue(settings.iterations);

    int filterId = d->luts.indexOf(settings.path);

    if (filterId == -1)
    {
        filterId = 0;
    }

    d->intensityInput->setValue(settings.intensity);
    d->correctionTools->setCurrentId(filterId);

    blockSignals(false);
}

void ColorFXSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

ColorFXContainer ColorFXSettings::defaultSettings() const
{
    return ColorFXContainer();
}

void ColorFXSettings::readSettings(const KConfigGroup& group)
{
    ColorFXContainer prm;
    ColorFXContainer defaultPrm = defaultSettings();

    prm.colorFXType = group.readEntry(d->configEffectTypeEntry,          defaultPrm.colorFXType);
    prm.level       = group.readEntry(d->configLevelAdjustmentEntry,     defaultPrm.level);
    prm.iterations  = group.readEntry(d->configIterationAdjustmentEntry, defaultPrm.iterations);
    prm.intensity   = group.readEntry(d->configLut3DIntensityEntry,      defaultPrm.intensity);
    prm.path        = group.readEntry(d->configLut3DFilterEntry,         defaultPrm.path);

    setSettings(prm);
}

void ColorFXSettings::writeSettings(KConfigGroup& group)
{
    ColorFXContainer prm = settings();

    group.writeEntry(d->configEffectTypeEntry,          prm.colorFXType);
    group.writeEntry(d->configLevelAdjustmentEntry,     prm.level);
    group.writeEntry(d->configIterationAdjustmentEntry, prm.iterations);
    group.writeEntry(d->configLut3DIntensityEntry,      prm.intensity);
    group.writeEntry(d->configLut3DFilterEntry,         prm.path);
}

void ColorFXSettings::findLuts()
{
    QStringList dirpaths;
    dirpaths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                          QLatin1String("digikam/data/lut3d"),
                                          QStandardPaths::LocateDirectory);

    for (const QString& dirpath : std::as_const(dirpaths))
    {
        QDirIterator dirIt(dirpath, QDirIterator::Subdirectories);

        while (dirIt.hasNext())
        {
            dirIt.next();

            if (QFileInfo(dirIt.filePath()).isFile())
            {
                d->luts << dirIt.filePath();
            }
        }
    }

    d->luts.sort();
}

QString ColorFXSettings::translateLuts(const QString& name) const
{
    if      (name.toLower() == QLatin1String("bleach"))
    {
        return i18nc("@info: effect type", "Bleach");
    }
    else if (name.toLower() == QLatin1String("blue_crush"))
    {
        return i18nc("@info: effect type", "Blue Crush");
    }
    else if (name.toLower() == QLatin1String("bw_contrast"))
    {
        return i18nc("@info: effect type", "BW Contrast");
    }
    else if (name.toLower() == QLatin1String("instant"))
    {
        return i18nc("@info: effect type", "Instant");
    }
    else if (name.toLower() == QLatin1String("original"))
    {
        return i18nc("@info: effect type", "Original");
    }
    else if (name.toLower() == QLatin1String("punch"))
    {
        return i18nc("@info: effect type", "Punch");
    }
    else if (name.toLower() == QLatin1String("summer"))
    {
        return i18nc("@info: effect type", "Summer");
    }
    else if (name.toLower() == QLatin1String("tokyo"))
    {
        return i18nc("@info: effect type", "Tokyo");
    }
    else if (name.toLower() == QLatin1String("vintage"))
    {
        return i18nc("@info: effect type", "Vintage");
    }
    else if (name.toLower() == QLatin1String("washout"))
    {
        return i18nc("@info: effect type", "Washout");
    }
    else if (name.toLower() == QLatin1String("washout_color"))
    {
        return i18nc("@info: effect type", "Washout Color");
    }
    else if (name.toLower() == QLatin1String("x_process"))
    {
        return i18nc("@info: effect type", "X Process");
    }

    return name;
}

} // namespace Digikam

#include "moc_colorfxsettings.cpp"
