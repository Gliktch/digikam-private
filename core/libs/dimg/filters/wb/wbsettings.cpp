/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-26
 * Description : White Balance settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wbsettings.h"

// Qt includes

#include <QString>
#include <QButtonGroup>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "dcombobox.h"

namespace Digikam
{

class Q_DECL_HIDDEN WBSettings::Private
{

public:

    enum TemperaturePreset
    {
        None        = -1,
        Candle      = 1850,
        Lamp40W     = 2680,
        Lamp100W    = 2800,
        Lamp200W    = 3000,
        Sunrise     = 3200,
        StudioLamp  = 3400,
        MoonLight   = 4100,
        Neutral     = 4750,
        DaylightD50 = 5000,
        Flash       = 5500,
        Sun         = 5770,
        XenonLamp   = 6420,
        DaylightD65 = 6500
    };

public:

    Private() = default;

    QString addTemperatureDescription(const QString& desc, TemperaturePreset preset) const
    {
        int index        = temperaturePresetCB->combo()->findData((int)preset);
        QString itemText = temperaturePresetCB->combo()->itemText(index);
        QString tempDesc = QString::fromLatin1("<p><b>%1</b>: %2 (%3K).</p>")
                           .arg(itemText)
                           .arg(desc)
                           .arg((int)preset);

        if (preset == None)
        {
            tempDesc.remove(QRegularExpression(QLatin1String("\\(.*\\)")));
        }

        return tempDesc;
    }

public:


    const QString configDarkInputEntry              = QLatin1String("Dark");
    const QString configBlackInputEntry             = QLatin1String("Black");
    const QString configMainExposureEntry           = QLatin1String("MainExposure");
    const QString configFineExposureEntry           = QLatin1String("FineExposure");
    const QString configGammaInputEntry             = QLatin1String("Gamma");
    const QString configSaturationInputEntry        = QLatin1String("Saturation");
    const QString configGreenInputEntry             = QLatin1String("Green");
    const QString configTemperatureInputEntry       = QLatin1String("Temperature");

    const int               DefaultTemperature      = DaylightD65;

    QToolButton*            pickTemperature         = nullptr;
    QToolButton*            autoAdjustExposure      = nullptr;

    QLabel*                 adjTemperatureLabel     = nullptr;
    QLabel*                 temperaturePresetLabel  = nullptr;
    QLabel*                 darkLabel               = nullptr;
    QLabel*                 blackLabel              = nullptr;
    QLabel*                 mainExposureLabel       = nullptr;
    QLabel*                 fineExposureLabel       = nullptr;
    QLabel*                 gammaLabel              = nullptr;
    QLabel*                 saturationLabel         = nullptr;
    QLabel*                 greenLabel              = nullptr;
    QLabel*                 exposureLabel           = nullptr;
    QLabel*                 temperatureLabel        = nullptr;

    DComboBox*              temperaturePresetCB     = nullptr;

    DDoubleNumInput*        temperatureInput        = nullptr;
    DDoubleNumInput*        darkInput               = nullptr;
    DDoubleNumInput*        blackInput              = nullptr;
    DDoubleNumInput*        mainExposureInput       = nullptr;
    DDoubleNumInput*        fineExposureInput       = nullptr;
    DDoubleNumInput*        gammaInput              = nullptr;
    DDoubleNumInput*        saturationInput         = nullptr;
    DDoubleNumInput*        greenInput              = nullptr;
};

// --------------------------------------------------------

WBSettings::WBSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing       = layoutSpacing();
    QGridLayout* const grid = new QGridLayout(parent);
    d->temperatureLabel     = new QLabel(QString::fromUtf8("<a href='https://en.wikipedia.org/wiki/Color_temperature'>%1</a> (K): ")
                                         .arg(i18nc("@title: WB", "Color Temperature")));
    d->temperatureLabel->setOpenExternalLinks(true);

    d->adjTemperatureLabel  = new QLabel(i18nc("@label: WB settings", "Adjustment:"));
    d->temperatureInput     = new DDoubleNumInput;
    d->temperatureInput->setDecimals(1);
    d->temperatureInput->setRange(1750.0, 12000.0, 10.0);
    d->temperatureInput->setDefaultValue((double)d->DefaultTemperature);
    d->temperatureInput->setWhatsThis(i18nc("@info: WB settings", "Set here the white balance color temperature in Kelvin."));

    d->temperaturePresetLabel = new QLabel(i18nc("@label: WB settings", "Preset:"));
    d->temperaturePresetCB    = new DComboBox;
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Candle"),       QVariant(d->Candle));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "40W Lamp"),     QVariant(d->Lamp40W));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "100W Lamp"),    QVariant(d->Lamp100W));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "200W Lamp"),    QVariant(d->Lamp200W));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Sunrise"),      QVariant(d->Sunrise));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Studio Lamp"),  QVariant(d->StudioLamp));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Moonlight"),    QVariant(d->MoonLight));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Neutral"),      QVariant(d->Neutral));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Daylight D50"), QVariant(d->DaylightD50));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Photo Flash"),  QVariant(d->Flash));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Sun"),          QVariant(d->Sun));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Xenon Lamp"),   QVariant(d->XenonLamp));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "Daylight D65"), QVariant(d->DaylightD65));
    d->temperaturePresetCB->combo()->addItem(i18nc("@item: WB temperature preset", "None"),         QVariant(d->None));
    d->temperaturePresetCB->setDefaultIndex(d->temperaturePresetCB->combo()->findData(QVariant(d->DefaultTemperature)));

    QString toolTip = QString::fromLatin1("<p>%1</p>").arg(i18nc("@info: WB settings", "Select the white balance color temperature preset to use."));
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "candle light"),                          d->Candle);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "40 Watt incandescent lamp"),             d->Lamp40W);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "100 Watt incandescent lamp"),            d->Lamp100W);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "200 Watt incandescent lamp"),            d->Lamp200W);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "sunrise or sunset light"),               d->Sunrise);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "tungsten lamp used in photo studio or "
                                                  "light at 1 hour from dusk/dawn"),        d->StudioLamp);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "moon light"),                            d->MoonLight);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "neutral color temperature"),             d->Neutral);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "sunny daylight around noon"),            d->DaylightD50);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "electronic photo flash"),                d->Flash);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "effective sun temperature"),             d->Sun);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "xenon lamp or light arc"),               d->XenonLamp);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "overcast sky light"),                    d->DaylightD65);
    toolTip += d->addTemperatureDescription(i18nc("@item: WB settings", "no preset value"),                       d->None);
    d->temperaturePresetCB->setToolTip(toolTip);

    d->pickTemperature = new QToolButton;
    d->pickTemperature->setIcon(QIcon::fromTheme(QLatin1String("color-picker-grey")));
    d->pickTemperature->setCheckable(true);
    d->pickTemperature->setToolTip(i18nc("@info: WB settings", "Temperature tone color picker."));
    d->pickTemperature->setWhatsThis(i18nc("@info: WB settings", "With this button, you can pick the color from the original "
                                           "image used to set the white color balance temperature and "
                                           "green component."));

    DLineWidget* const line = new DLineWidget(Qt::Horizontal);

    // -------------------------------------------------------------

    d->blackLabel = new QLabel(i18nc("@label: WB settings", "Black point:"));
    d->blackInput = new DDoubleNumInput;
    d->blackInput->setDecimals(2);
    d->blackInput->setRange(0.0, 0.05, 0.01);
    d->blackInput->setWhatsThis(i18nc("@info: WB settings", "Set here the black level value."));
    d->blackInput->setDefaultValue(0.0);

    d->darkLabel = new QLabel(i18nc("@label: WB settings", "Shadows:"));
    d->darkInput = new DDoubleNumInput;
    d->darkInput->setDecimals(2);
    d->darkInput->setRange(0.0, 1.0, 0.01);
    d->darkInput->setDefaultValue(0.0);
    d->darkInput->setWhatsThis(i18nc("@info: WB settings", "Set here the shadow noise suppression level."));

    d->saturationLabel = new QLabel(i18nc("@label: WB settings", "Saturation:"));
    d->saturationInput = new DDoubleNumInput;
    d->saturationInput->setDecimals(2);
    d->saturationInput->setRange(0.0, 2.0, 0.01);
    d->saturationInput->setDefaultValue(1.0);
    d->saturationInput->setWhatsThis(i18nc("@info: WB settings", "Set here the saturation value."));

    d->gammaLabel = new QLabel(i18nc("@label: WB settings", "Gamma:"));
    d->gammaInput = new DDoubleNumInput;
    d->gammaInput->setDecimals(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setWhatsThis(i18nc("@info: WB settings", "Set here the gamma correction value."));

    d->greenLabel = new QLabel(i18nc("@label: WB settings", "Green:"));
    d->greenInput = new DDoubleNumInput;
    d->greenInput->setDecimals(2);
    d->greenInput->setRange(0.2, 2.5, 0.01);
    d->greenInput->setDefaultValue(1.0);
    d->greenInput->setWhatsThis(i18nc("@info: WB settings", "Set here the green component to control the magenta color "
                                      "cast removal level."));

    DLineWidget* const line2 = new DLineWidget(Qt::Horizontal);

    // -------------------------------------------------------------

    d->exposureLabel = new QLabel(QString::fromUtf8("<a href='https://en.wikipedia.org/wiki/Exposure_value'>%1</a> (E.V): ")
                                  .arg(i18nc("@label: WB settings", "Exposure Compensation")));
    d->exposureLabel->setOpenExternalLinks(true);

    d->mainExposureLabel  = new QLabel(i18nc("@label: WB settings", "Main:"));
    d->autoAdjustExposure = new QToolButton;
    d->autoAdjustExposure->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    d->autoAdjustExposure->setToolTip(i18nc("@info: WB settings", "Auto exposure adjustments"));
    d->autoAdjustExposure->setWhatsThis(i18nc("@info: WB settings", "With this button, you can automatically adjust Exposure "
                                              "and Black Point values."));
    d->mainExposureInput = new DDoubleNumInput;
    d->mainExposureInput->setDecimals(2);
    d->mainExposureInput->setRange(-6.0, 8.0, 0.1);
    d->mainExposureInput->setDefaultValue(0.0);
    d->mainExposureInput->setWhatsThis(i18nc("@info: WB settings", "Set here the main exposure compensation value in E.V."));

    d->fineExposureLabel = new QLabel(i18nc("@label: WB settings", "Fine:"));
    d->fineExposureInput = new DDoubleNumInput;
    d->fineExposureInput->setDecimals(2);
    d->fineExposureInput->setRange(-0.5, 0.5, 0.01);
    d->fineExposureInput->setDefaultValue(0.0);
    d->fineExposureInput->setWhatsThis(i18nc("@info: WB settings", "This value in E.V will be added to main exposure "
                                            "compensation value to set fine exposure adjustment."));

    // -------------------------------------------------------------

    grid->addWidget(d->temperatureLabel,        0, 0, 1, 6);
    grid->addWidget(d->adjTemperatureLabel,     1, 0, 1, 1);
    grid->addWidget(d->pickTemperature,         1, 1, 1, 1);
    grid->addWidget(d->temperatureInput,        1, 2, 1, 4);
    grid->addWidget(d->temperaturePresetLabel,  2, 0, 1, 1);
    grid->addWidget(d->temperaturePresetCB,     2, 2, 1, 4);
    grid->addWidget(line,                       3, 0, 1, 6);
    grid->addWidget(d->blackLabel,              4, 0, 1, 1);
    grid->addWidget(d->blackInput,              4, 1, 1, 5);
    grid->addWidget(d->darkLabel,               5, 0, 1, 1);
    grid->addWidget(d->darkInput,               5, 1, 1, 5);
    grid->addWidget(d->saturationLabel,         6, 0, 1, 1);
    grid->addWidget(d->saturationInput,         6, 1, 1, 5);
    grid->addWidget(d->gammaLabel,              7, 0, 1, 1);
    grid->addWidget(d->gammaInput,              7, 1, 1, 5);
    grid->addWidget(d->greenLabel,              8, 0, 1, 1);
    grid->addWidget(d->greenInput,              8, 1, 1, 5);
    grid->addWidget(line2,                      9, 0, 1, 6);
    grid->addWidget(d->exposureLabel,          10, 0, 1, 6);
    grid->addWidget(d->mainExposureLabel,      11, 0, 1, 1);
    grid->addWidget(d->autoAdjustExposure,     11, 1, 1, 1);
    grid->addWidget(d->mainExposureInput,      11, 2, 1, 4);
    grid->addWidget(d->fineExposureLabel,      12, 0, 1, 2);
    grid->addWidget(d->fineExposureInput,      12, 2, 1, 4);
    grid->setRowStretch(13, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->temperaturePresetCB, SIGNAL(activated(int)),
            this, SLOT(slotTemperaturePresetChanged(int)));

    connect(d->temperatureInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTemperatureChanged(double)));

    connect(d->darkInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->blackInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->mainExposureInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->fineExposureInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->gammaInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->saturationInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->greenInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->autoAdjustExposure, SIGNAL(clicked()),
            this, SIGNAL(signalAutoAdjustExposure()));

    connect(d->pickTemperature, SIGNAL(released()),
            this, SIGNAL(signalPickerColorButtonActived()));
}

WBSettings::~WBSettings()
{
    delete d;
}

bool WBSettings::pickTemperatureIsChecked()
{
    return d->pickTemperature->isChecked();
}

void WBSettings::setCheckedPickTemperature(bool b)
{
    d->pickTemperature->setChecked(b);
}

void WBSettings::showAdvancedButtons(bool b)
{
    d->pickTemperature->setVisible(b);
    d->autoAdjustExposure->setVisible(b);
}

void WBSettings::slotTemperatureChanged(double temperature)
{
    int index = d->temperaturePresetCB->combo()->findData(QVariant((int)temperature));

    if (index == -1)
    {
        index = d->temperaturePresetCB->combo()->findData(QVariant((int)d->None));
    }

    d->temperaturePresetCB->setCurrentIndex(index);

    Q_EMIT signalSettingsChanged();
}

void WBSettings::slotTemperaturePresetChanged(int tempPreset)
{
    bool ok         = true;
    int temperature = d->temperaturePresetCB->combo()->itemData(tempPreset).toInt(&ok);

    if (!ok)
    {
        temperature = d->DefaultTemperature;
    }

    if (temperature != -1)
    {
        d->temperatureInput->setValue((double)temperature);
    }

    Q_EMIT signalSettingsChanged();
}

WBContainer WBSettings::settings() const
{
    WBContainer prm;

    prm.black          = d->blackInput->value();
    prm.expositionMain = d->mainExposureInput->value();
    prm.expositionFine = d->fineExposureInput->value();
    prm.temperature    = d->temperatureInput->value();
    prm.green          = d->greenInput->value();
    prm.dark           = d->darkInput->value();
    prm.gamma          = d->gammaInput->value();
    prm.saturation     = d->saturationInput->value();

    return prm;
}

void WBSettings::setSettings(const WBContainer& settings)
{
    blockSignals(true);

    d->blackInput->setValue(settings.black);
    d->mainExposureInput->setValue(settings.expositionMain);
    d->fineExposureInput->setValue(settings.expositionFine);
    d->temperatureInput->setValue(settings.temperature);
    d->greenInput->setValue(settings.green);
    d->darkInput->setValue(settings.dark);
    d->gammaInput->setValue(settings.gamma);
    d->saturationInput->setValue(settings.saturation);
    slotTemperatureChanged(d->temperatureInput->value());

    blockSignals(false);
}

void WBSettings::resetToDefault()
{
    blockSignals(true);

    // Neutral color temperature settings is D65.

    d->blackInput->slotReset();
    d->darkInput->slotReset();
    d->gammaInput->slotReset();
    d->greenInput->slotReset();
    d->mainExposureInput->slotReset();
    d->fineExposureInput->slotReset();
    d->saturationInput->slotReset();
    d->temperatureInput->slotReset();
    d->temperaturePresetCB->slotReset();
    slotTemperaturePresetChanged(d->temperaturePresetCB->defaultIndex());

    blockSignals(false);
}

WBContainer WBSettings::defaultSettings() const
{
    WBContainer prm;

    prm.black          = d->blackInput->defaultValue();
    prm.expositionMain = d->mainExposureInput->defaultValue();
    prm.expositionFine = d->fineExposureInput->defaultValue();
    prm.temperature    = d->temperatureInput->defaultValue();
    prm.green          = d->greenInput->defaultValue();
    prm.dark           = d->darkInput->defaultValue();
    prm.gamma          = d->gammaInput->defaultValue();
    prm.saturation     = d->saturationInput->defaultValue();

    return prm;
}

void WBSettings::readSettings(const KConfigGroup& group)
{
    WBContainer prm;
    prm.black          = group.readEntry(d->configBlackInputEntry,       d->blackInput->defaultValue());
    prm.temperature    = group.readEntry(d->configTemperatureInputEntry, d->temperatureInput->defaultValue());
    prm.green          = group.readEntry(d->configGreenInputEntry,       d->greenInput->defaultValue());
    prm.dark           = group.readEntry(d->configDarkInputEntry,        d->darkInput->defaultValue());
    prm.gamma          = group.readEntry(d->configGammaInputEntry,       d->gammaInput->defaultValue());
    prm.saturation     = group.readEntry(d->configSaturationInputEntry,  d->saturationInput->defaultValue());
    prm.expositionMain = group.readEntry(d->configMainExposureEntry,     d->mainExposureInput->defaultValue());
    prm.expositionFine = group.readEntry(d->configFineExposureEntry,     d->fineExposureInput->defaultValue());

    setSettings(prm);
}

void WBSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configDarkInputEntry,        d->darkInput->value());
    group.writeEntry(d->configBlackInputEntry,       d->blackInput->value());
    group.writeEntry(d->configMainExposureEntry,     d->mainExposureInput->value());
    group.writeEntry(d->configFineExposureEntry,     d->fineExposureInput->value());
    group.writeEntry(d->configGammaInputEntry,       d->gammaInput->value());
    group.writeEntry(d->configSaturationInputEntry,  d->saturationInput->value());
    group.writeEntry(d->configGreenInputEntry,       d->greenInput->value());
    group.writeEntry(d->configTemperatureInputEntry, d->temperatureInput->value());
}

void WBSettings::loadSettings()
{
    QUrl loadWhiteBalanceFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18nc("@title:window", "White Color Balance Settings File to Load"),
                                                            QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                            QLatin1String("*"));

    if (loadWhiteBalanceFile.isEmpty())
    {
        return;
    }

    QFile file(loadWhiteBalanceFile.toLocalFile());

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        if (stream.readLine() != QLatin1String("# White Color Balance Configuration File V2"))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                  i18nc("@info", "\"%1\" is not a White Color Balance settings text file.",
                                       loadWhiteBalanceFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
        d->temperatureInput->setValue(stream.readLine().toDouble());
        d->darkInput->setValue(stream.readLine().toDouble());
        d->blackInput->setValue(stream.readLine().toDouble());
        d->mainExposureInput->setValue(stream.readLine().toDouble());
        d->fineExposureInput->setValue(stream.readLine().toDouble());
        d->gammaInput->setValue(stream.readLine().toDouble());
        d->saturationInput->setValue(stream.readLine().toDouble());
        d->greenInput->setValue(stream.readLine().toDouble());
        slotTemperatureChanged(d->temperatureInput->value());
        blockSignals(false);
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18nc("@info", "Cannot load settings from the White Color Balance text file."));
    }

    file.close();
}

void WBSettings::saveAsSettings()
{
    QUrl saveWhiteBalanceFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18nc("@title:window", "White Color Balance Settings File to Save"),
                                                            QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                            QLatin1String("*"));

    if (saveWhiteBalanceFile.isEmpty())
    {
        return;
    }

    QFile file(saveWhiteBalanceFile.toLocalFile());

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << QLatin1String("# White Color Balance Configuration File V2\n");
        stream << d->temperatureInput->value()  << QLatin1Char('\n');
        stream << d->darkInput->value()         << QLatin1Char('\n');
        stream << d->blackInput->value()        << QLatin1Char('\n');
        stream << d->mainExposureInput->value() << QLatin1Char('\n');
        stream << d->fineExposureInput->value() << QLatin1Char('\n');
        stream << d->gammaInput->value()        << QLatin1Char('\n');
        stream << d->saturationInput->value()   << QLatin1Char('\n');
        stream << d->greenInput->value()        << QLatin1Char('\n');
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18nc("@info", "Cannot save settings to the White Color Balance text file."));
    }

    file.close();
}

} // namespace Digikam

#include "moc_wbsettings.cpp"
