/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-08-12
 * Description : advanced settings for camera interface.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advancedsettings.h"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QComboBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dpluginloader.h"
#include "templateselector.h"
#include "ddatetimeedit.h"
#include "setupversioning.h"
#include "template.h"

namespace Digikam
{

class Q_DECL_HIDDEN AdvancedSettings::Private
{
public:

    Private() = default;

    QLabel*           formatLabel       = nullptr;

    QCheckBox*        autoRotateCheck   = nullptr;
    QCheckBox*        convertJpegCheck  = nullptr;
    QCheckBox*        fixDateTimeCheck  = nullptr;
    QCheckBox*        documentNameCheck = nullptr;

    QComboBox*        losslessFormat    = nullptr;

    DDateTimeEdit*    dateTimeEdit      = nullptr;

    TemplateSelector* templateSelector  = nullptr;
};

AdvancedSettings::AdvancedSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing = layoutSpacing();


    QVBoxLayout* const onFlyVlay = new QVBoxLayout(this);
    d->templateSelector          = new TemplateSelector(this);
    d->documentNameCheck         = new QCheckBox(i18nc("@option:check", "Write the document name to EXIF"), this);
    d->fixDateTimeCheck          = new QCheckBox(i18nc("@option:check", "Fix internal date && time"), this);
    d->dateTimeEdit              = new DDateTimeEdit(this, QLatin1String("datepicker"));
    d->autoRotateCheck           = new QCheckBox(i18nc("@option:check", "Auto-rotate/flip image"), this);
    d->convertJpegCheck          = new QCheckBox(i18nc("@option:check", "Convert to lossless file format"), this);
    DHBox* const hbox2           = new DHBox(this);
    d->formatLabel               = new QLabel(i18nc("@label", "New image format:"), hbox2);
    d->losslessFormat            = new QComboBox(hbox2);
    d->losslessFormat->addItem(i18nc("@label:listbox", "PNG"),         QLatin1String("PNG"));
    d->losslessFormat->addItem(i18nc("@label:listbox", "TIFF"),        QLatin1String("TIF"));
    d->losslessFormat->addItem(i18nc("@label:listbox", "PGF"),         QLatin1String("PGF"));

#ifdef HAVE_JASPER

    d->losslessFormat->addItem(i18nc("@label:listbox", "JPEG-2000"),   QLatin1String("JP2"));

#endif // HAVE_JASPER

#ifdef HAVE_X265

    d->losslessFormat->addItem(i18nc("@label:listbox", "HEIF"),        QLatin1String("HEIF"));

#endif // HAVE_X265

    bool hasJXLSupport = DPluginLoader::instance()->canExport(QLatin1String("JXL"));

    if (hasJXLSupport)
    {
        d->losslessFormat->addItem(i18nc("@label:listbox", "JPEG-XL"), QLatin1String("JXL"));
    }

    bool hasWEBPSupport = DPluginLoader::instance()->canExport(QLatin1String("WEBP"));

    if (hasWEBPSupport)
    {
        d->losslessFormat->addItem(i18nc("@label:listbox", "WEBP"),    QLatin1String("WEBP"));
    }

    bool hasAVIFSupport = DPluginLoader::instance()->canExport(QLatin1String("AVIF"));

    if (hasAVIFSupport)
    {
        d->losslessFormat->addItem(i18nc("@label:listbox", "AVIF"),    QLatin1String("AVIF"));
    }

    onFlyVlay->addWidget(d->templateSelector);
    onFlyVlay->addWidget(d->documentNameCheck);
    onFlyVlay->addWidget(d->fixDateTimeCheck);
    onFlyVlay->addWidget(d->dateTimeEdit);
    onFlyVlay->addWidget(d->autoRotateCheck);
    onFlyVlay->addWidget(d->convertJpegCheck);
    onFlyVlay->addWidget(hbox2);
    onFlyVlay->addStretch();
    onFlyVlay->setContentsMargins(spacing, spacing, spacing, spacing);
    onFlyVlay->setSpacing(spacing);

    setWhatsThis(i18nc("@info", "Set here all options to fix/transform JPEG files automatically "
                       "as they are downloaded."));
    d->autoRotateCheck->setWhatsThis(i18nc("@info", "Enable this option if you want images automatically "
                                           "rotated or flipped using Exif information provided by the camera."));
    d->templateSelector->setWhatsThis(i18nc("@info", "Select here which metadata template you want to apply "
                                            "to images."));
    d->documentNameCheck->setWhatsThis(i18nc("@info", "Enable this option to write the document name to the Exif metadata. "
                                             "The document name is the original file name of the imported file."));
    d->fixDateTimeCheck->setWhatsThis(i18nc("@info", "Enable this option to set date and time metadata "
                                            "tags to the right values if your camera does not set "
                                            "these tags correctly when pictures are taken. The values will "
                                            "be saved in the DateTimeDigitized and DateTimeCreated EXIF, XMP, and IPTC tags."));
    d->convertJpegCheck->setWhatsThis(i18nc("@info", "Enable this option to automatically convert "
                                            "all JPEG files to a lossless image format. Note: Image conversion can take a "
                                            "while on a slow computer."));

    QString formatHelp = xi18nc("@info:whatsthis",
                                "<title>Preferred Lossless Image File Format</title>"
                                "<para>Select the file format in which JPEG image will be converted to a "
                                "lossless container with metadata support.</para>");

    SetupVersioning::losslessFormatToolTip(formatHelp, hasJXLSupport, hasWEBPSupport, hasAVIFSupport);
    d->losslessFormat->setWhatsThis(formatHelp);

    // ---------------------------------------------------------------------------------------

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->losslessFormat, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->formatLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalDownloadNameChanged()));

    connect(d->losslessFormat, SIGNAL(activated(int)),
            this, SIGNAL(signalDownloadNameChanged()));

    connect(d->fixDateTimeCheck, SIGNAL(toggled(bool)),
            d->dateTimeEdit, SLOT(setEnabled(bool)));
}

AdvancedSettings::~AdvancedSettings()
{
    delete d;
}

void AdvancedSettings::readSettings(const KConfigGroup& group)
{
    d->autoRotateCheck->setChecked(group.readEntry(QLatin1String("AutoRotate"),         true));
    d->fixDateTimeCheck->setChecked(group.readEntry(QLatin1String("FixDateTime"),       false));
    d->documentNameCheck->setChecked(group.readEntry(QLatin1String("DocumentName"),     false));
    d->templateSelector->setTemplateIndex(group.readEntry(QLatin1String("Template"),    1));      // do not change
    d->convertJpegCheck->setChecked(group.readEntry(QLatin1String("ConvertJpeg"),       false));
    d->losslessFormat->setCurrentIndex(group.readEntry(QLatin1String("LossLessFormat"), 0));      // PNG by default

    d->dateTimeEdit->setEnabled(d->fixDateTimeCheck->isChecked());
    d->losslessFormat->setEnabled(d->convertJpegCheck->isChecked());
    d->formatLabel->setEnabled(d->convertJpegCheck->isChecked());
}

void AdvancedSettings::saveSettings(KConfigGroup& group)
{
    group.writeEntry(QLatin1String("AutoRotate"),     d->autoRotateCheck->isChecked());
    group.writeEntry(QLatin1String("FixDateTime"),    d->fixDateTimeCheck->isChecked());
    group.writeEntry(QLatin1String("DocumentName"),   d->documentNameCheck->isChecked());
    group.writeEntry(QLatin1String("Template"),       d->templateSelector->getTemplateIndex());
    group.writeEntry(QLatin1String("ConvertJpeg"),    d->convertJpegCheck->isChecked());
    group.writeEntry(QLatin1String("LossLessFormat"), d->losslessFormat->currentIndex());
}

DownloadSettings AdvancedSettings::settings() const
{
    DownloadSettings settings;

    settings.autoRotate     = d->autoRotateCheck->isChecked();
    settings.fixDateTime    = d->fixDateTimeCheck->isChecked();
    settings.convertJpeg    = d->convertJpegCheck->isChecked();
    settings.newDateTime    = d->dateTimeEdit->dateTime();
    settings.documentName   = d->documentNameCheck->isChecked();
    settings.losslessFormat = d->losslessFormat->itemData(d->losslessFormat->currentIndex()).toString();
    settings.templateTitle  = d->templateSelector->getTemplate().templateTitle();

    return settings;
}

} // namespace Digikam

#include "moc_advancedsettings.cpp"
