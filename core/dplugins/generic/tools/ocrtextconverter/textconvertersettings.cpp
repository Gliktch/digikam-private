/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : OCR settings widgets
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "textconvertersettings.h"

// Qt includes

#include <QGridLayout>
#include <QCheckBox>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"
#include "dcombobox.h"
#include "dmetadata.h"
#include "dprogresswdg.h"
#include "dexpanderbox.h"
#include "dnuminput.h"
#include "textconverterlist.h"
#include "localizeselector.h"
#include "localizesettings.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class Q_DECL_HIDDEN TextConverterSettings::Private
{
public:

    Private() = default;

public:

    // Tesseract options

    DComboBox*            ocrTesseractLanguageMode  = nullptr;

    DComboBox*            ocrTesseractPSMMode       = nullptr;

    DComboBox*            ocrTesseractOEMMode       = nullptr;

    DIntNumInput*         ocrTesseractDpi           = nullptr;

    QCheckBox*            saveTextFile              = nullptr;

    QCheckBox*            saveXMP                   = nullptr;

    LocalizeSelectorList* localizeList              = nullptr;

    QCheckBox*            multicores                = nullptr;
};

TextConverterSettings::TextConverterSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QLabel* const ocrTesseractLanguageLabel   = new QLabel(i18nc("@label", "Languages:"));
    d->ocrTesseractLanguageMode               = new DComboBox(this);

    d->ocrTesseractLanguageMode->setDefaultIndex(int(OcrOptions::LanguageModes::DEFAULT));
    d->ocrTesseractLanguageMode->setToolTip(i18nc("@info", "Specify the language used for OCR. "
                                                           "In the Default mode of Language settings for digital text with multiple languages, \n"
                                                           "Tesseract can automatically recognize languages using Latin alphabets such as English or French, \n"
                                                           "but is not compatible with languages using hieroglyphs such as Chinese, Japanese.\n"
                                                           "You can use the Orientation and Script Detection mode instead or a specific language "
                                                           "module if available."));

    // ------------

    QLabel* const ocrTesseractPSMLabel  = new QLabel(i18nc("@label", "Segmentation mode:"));
    d->ocrTesseractPSMMode              = new DComboBox(this);

    QMap<OcrOptions::PageSegmentationModes, QPair<QString, QString> >                psmMap = OcrOptions::psmNames();
    QMap<OcrOptions::PageSegmentationModes, QPair<QString, QString> >::const_iterator it1   = psmMap.constBegin();

    while (it1 != psmMap.constEnd())
    {
        d->ocrTesseractPSMMode->addItem(it1.value().first, (int)it1.key());
        d->ocrTesseractPSMMode->combo()->setItemData((int)it1.key(), it1.value().second, Qt::ToolTipRole);
        ++it1;
    }

    d->ocrTesseractPSMMode->setDefaultIndex(int(OcrOptions::PageSegmentationModes::DEFAULT));
    d->ocrTesseractPSMMode->setToolTip(i18nc("@info", "Specify page segmentation mode."));

    // ------------

    QLabel* const ocrTesseractOEMLabel  = new QLabel(i18nc("@label", "Engine mode:"));
    d->ocrTesseractOEMMode              = new DComboBox(this);

    QMap<OcrOptions::EngineModes, QPair<QString, QString> >                oemMap  = OcrOptions::oemNames();
    QMap<OcrOptions::EngineModes, QPair<QString, QString> >::const_iterator it2    = oemMap.constBegin();

    while (it2 !=  oemMap.constEnd())
    {
        d->ocrTesseractOEMMode->addItem(it2.value().first, (int)it2.key());
        d->ocrTesseractOEMMode->combo()->setItemData((int)it2.key(), it2.value().second, Qt::ToolTipRole);
        ++it2;
    }

    d->ocrTesseractOEMMode->setDefaultIndex(int(OcrOptions::EngineModes::DEFAULT));
    d->ocrTesseractOEMMode->setToolTip(i18nc("@info", "Specify OCR engine mode."));

    // ------------

    QLabel* const ocrTesseractDpiLabel  = new QLabel(i18nc("@label", "Resolution Dpi:"));
    d->ocrTesseractDpi                  = new DIntNumInput(this);
    d->ocrTesseractDpi->setRange(70, 2400, 1);
    d->ocrTesseractDpi->setToolTip(i18nc("@info", "Specify DPI for input image."));
    d->ocrTesseractDpi->setDefaultValue(300);
    ocrTesseractDpiLabel->setBuddy(d->ocrTesseractDpi);

    // ------------

    QLabel* const saveOcrResultLabel = new QLabel(i18nc("@label", "Store result in : "));
    d->saveTextFile                  = new QCheckBox(i18nc("@option:check", "Text file"), this);
    d->saveTextFile->setToolTip(i18nc("@info", "Store OCR result in separated text file"));
    d->saveTextFile->setChecked(true);

    d->saveXMP                       = new QCheckBox(i18nc("@option:check", "Metadata"), this);
    d->saveXMP->setToolTip(i18nc("@info", "Store OCR result in XMP metadata"));
    d->saveXMP->setChecked(true);

    d->localizeList                  = new LocalizeSelectorList(this);
    slotLocalizeChanged();

    d->multicores                    = new QCheckBox(i18nc("@option:check", "Use Multi-cores"), this);
    d->multicores->setToolTip(i18nc("@info", "If this option is enabled, files will be processed in parallel"));
    d->multicores->setChecked(true);

    // ------------

    QGridLayout* const settingsBoxLayout = new QGridLayout(this);
    settingsBoxLayout->addWidget(ocrTesseractLanguageLabel,        0, 0, 1, 1);
    settingsBoxLayout->addWidget(d->ocrTesseractLanguageMode,      0, 1, 1, 1);
    settingsBoxLayout->addWidget(ocrTesseractPSMLabel,             1, 0, 1, 1);
    settingsBoxLayout->addWidget(d->ocrTesseractPSMMode,           1, 1, 1, 1);
    settingsBoxLayout->addWidget(ocrTesseractOEMLabel,             2, 0, 1, 1);
    settingsBoxLayout->addWidget(d->ocrTesseractOEMMode,           2, 1, 1, 1);
    settingsBoxLayout->addWidget(ocrTesseractDpiLabel,             3, 0, 1, 1);
    settingsBoxLayout->addWidget(d->ocrTesseractDpi,               3, 1, 1, 1);
    settingsBoxLayout->addWidget(d->multicores,                    4, 0, 1, 2);
    settingsBoxLayout->addWidget(saveOcrResultLabel,               5, 0, 1, 1);
    settingsBoxLayout->addWidget(d->saveTextFile,                  6, 0, 1, 1);
    settingsBoxLayout->addWidget(d->saveXMP,                       6, 1, 1, 1);
    settingsBoxLayout->addWidget(d->localizeList,                  7, 0, 1, 2);
    settingsBoxLayout->setRowStretch(7, 10);
    settingsBoxLayout->setContentsMargins(QMargins());

    // ------------------------------------------------------------------------

    connect(d->ocrTesseractLanguageMode, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->ocrTesseractPSMMode, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->ocrTesseractOEMMode, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(LocalizeSettings::instance(), &LocalizeSettings::signalSettingsChanged,
            this, &TextConverterSettings::slotLocalizeChanged);
}

TextConverterSettings::~TextConverterSettings()
{
    delete d;
}

void TextConverterSettings::setDefaultSettings()
{
    d->ocrTesseractLanguageMode->slotReset();
    d->ocrTesseractOEMMode->slotReset();
    d->ocrTesseractPSMMode->slotReset();
    d->ocrTesseractDpi->slotReset();
    d->saveTextFile->setChecked(true);
    d->localizeList->clearLanguages();
    d->multicores->setChecked(false);
}

void TextConverterSettings::setOcrOptions(const OcrOptions& opt)
{
    int id = d->ocrTesseractLanguageMode->combo()->findData(opt.language);

    d->ocrTesseractLanguageMode->setCurrentIndex((id == -1) ? int(OcrOptions::LanguageModes::DEFAULT) : id);
    d->ocrTesseractPSMMode->setCurrentIndex(opt.psm);
    d->ocrTesseractOEMMode->setCurrentIndex(opt.oem);
    d->ocrTesseractDpi->setValue(opt.dpi);
    d->saveTextFile->setChecked(opt.isSaveTextFile);
    d->saveXMP->setChecked(opt.isSaveXMP);

    for (const QString& lg : std::as_const(opt.translations))
    {
        d->localizeList->addLanguage(lg);
    }

    d->multicores->setChecked(opt.multicores);
}

OcrOptions TextConverterSettings::ocrOptions() const
{
    OcrOptions opt;

    opt.language       = d->ocrTesseractLanguageMode->combo()->currentData().toString();
    opt.psm            = d->ocrTesseractPSMMode->currentIndex();
    opt.oem            = d->ocrTesseractOEMMode->currentIndex();
    opt.dpi            = d->ocrTesseractDpi->value();
    opt.isSaveTextFile = d->saveTextFile->isChecked();
    opt.isSaveXMP      = d->saveXMP->isChecked();
    opt.translations   = d->localizeList->languagesList();
    opt.multicores     = d->multicores->isChecked();

    return opt;
}

void TextConverterSettings::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Text Converter Settings"));
    OcrOptions opt;
    opt.language       = group.readEntry("OcrLanguages",          QString());
    opt.psm            = group.readEntry("PageSegmentationModes", int(OcrOptions::PageSegmentationModes::DEFAULT));
    opt.oem            = group.readEntry("EngineModes",           int(OcrOptions::EngineModes::DEFAULT));
    opt.dpi            = group.readEntry("Dpi",                   300);
    opt.isSaveTextFile = group.readEntry("Check Save Test File",  true);
    opt.isSaveXMP      = group.readEntry("Check Save in XMP",     true);
    opt.translations   = group.readEntry("Translation Codes",     QStringList());
    opt.multicores     = group.readEntry("Multicores",            false);

    setOcrOptions(opt);
}

void TextConverterSettings::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Text Converter Settings"));
    OcrOptions opt            = ocrOptions();

    group.writeEntry("OcrLanguages",              opt.language);
    group.writeEntry("PageSegmentationModes",     (int)opt.psm);
    group.writeEntry("EngineModes",               (int)opt.oem);
    group.writeEntry("Dpi",                       (int)opt.dpi);
    group.writeEntry("Check Save Test File",      (bool)opt.isSaveTextFile);
    group.writeEntry("Check Save in XMP",         (bool)opt.isSaveXMP);
    group.writeEntry("Translation Codes",         opt.translations);
    group.writeEntry("Multicores",                (bool)opt.multicores);

    config->sync();
}

void TextConverterSettings::populateLanguagesMode(const QStringList& langs)
{
    if (langs.isEmpty())
    {
        return;
    }

    QStringList tlanguages = langs;

    d->ocrTesseractLanguageMode->insertItem(int(OcrOptions::LanguageModes::DEFAULT),
                                            i18nc("@option: default Tesseract mode", "Default"),
                                            QString());

    if (tlanguages.contains(QLatin1String("osd")))
    {
        d->ocrTesseractLanguageMode->insertItem(int(OcrOptions::LanguageModes::OSD),
                                                i18nc("@option: osd Tesseract mode", "Orientation and Script Detection"),
                                                QLatin1String("osd"));
        tlanguages.removeAll(QLatin1String("osd"));
    }

    d->ocrTesseractLanguageMode->combo()->insertSeparator(d->ocrTesseractLanguageMode->combo()->count() + 1);

    // All others languages are based on 3 letters ISO 639-2

    DMetadata::CountryCodeMap codes = DMetadata::countryCodeMap2();

    for (const QString& lg : std::as_const(tlanguages))
    {
         d->ocrTesseractLanguageMode->addItem(codes.value(lg, lg), lg);
    }
}

void TextConverterSettings::slotLocalizeChanged()
{
    d->localizeList->setTitle(i18nc("@label", "Translate with %1:",
                              DOnlineTranslator::engineName(LocalizeSettings::instance()->settings().translatorEngine)));
}

} // namespace DigikamGenericTextConverterPlugin

#include "moc_textconvertersettings.cpp"
