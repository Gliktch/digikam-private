/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-23
 * Description : setup image editor output files settings.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupiofiles.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "dpluginloader.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupIOFiles::Private
{
public:

    Private() = default;

    // cppcheck-suppress constParameterPointer
    QWidget* createGroupBox(QWidget* const w) const
    {
        QGroupBox* const  box     = new QGroupBox;
        QVBoxLayout* const layout = new QVBoxLayout;
        layout->addWidget(w);
        box->setLayout(layout);

        return box;
    }

public:

    const QString configGroupName                   = QLatin1String("ImageViewer Settings");
    const QString configJPEGCompressionEntry        = QLatin1String("JPEGCompression");
    const QString configJPEGSubSamplingEntry        = QLatin1String("JPEGSubSampling");
    const QString configPNGCompressionEntry         = QLatin1String("PNGCompression");
    const QString configTIFFCompressionEntry        = QLatin1String("TIFFCompression");
    const QString configJPEG2000CompressionEntry    = QLatin1String("JPEG2000Compression");
    const QString configJPEG2000LossLessEntry       = QLatin1String("JPEG2000LossLess");
    const QString configPGFCompressionEntry         = QLatin1String("PGFCompression");
    const QString configPGFLossLessEntry            = QLatin1String("PGFLossLess");
    const QString configHEIFCompressionEntry        = QLatin1String("HEIFCompression");
    const QString configHEIFLossLessEntry           = QLatin1String("HEIFLossLess");
    const QString configShowImageSettingsDialog     = QLatin1String("ShowImageSettingsDialog");
    const QString configJXLCompressionEntry         = QLatin1String("JXLCompression");
    const QString configJXLLossLessEntry            = QLatin1String("JXLLossLess");
    const QString configWEBPCompressionEntry        = QLatin1String("WEBPCompression");
    const QString configWEBPLossLessEntry           = QLatin1String("WEBPLossLess");
    const QString configAVIFCompressionEntry        = QLatin1String("AVIFCompression");
    const QString configAVIFLossLessEntry           = QLatin1String("AVIFLossLess");

    DImgLoaderSettings*  JPEGOptions                = nullptr;
    DImgLoaderSettings*  PNGOptions                 = nullptr;
    DImgLoaderSettings*  TIFFOptions                = nullptr;

#ifdef HAVE_JASPER

    DImgLoaderSettings*  JPEG2000Options            = nullptr;

#endif // HAVE_JASPER

    DImgLoaderSettings*  PGFOptions                 = nullptr;

#ifdef HAVE_X265

    DImgLoaderSettings*  HEIFOptions                = nullptr;

#endif // HAVE_X265

    DImgLoaderSettings*  JXLOptions                 = nullptr;
    DImgLoaderSettings*  WEBPOptions                = nullptr;
    DImgLoaderSettings*  AVIFOptions                = nullptr;
    QCheckBox*           showImageSettingsDialog    = nullptr;
};

// --------------------------------------------------------

SetupIOFiles::SetupIOFiles(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    QWidget* const panel         = new QWidget;
    QVBoxLayout* const vbox      = new QVBoxLayout;
    DPluginLoader* const ploader = DPluginLoader::instance();

    d->JPEGOptions               = ploader->exportWidget(QLatin1String("JPEG"));

    if (d->JPEGOptions)
    {
        d->JPEGOptions->setParent(this);
    }

    d->PNGOptions                = ploader->exportWidget(QLatin1String("PNG"));

    if (d->PNGOptions)
    {
        d->PNGOptions->setParent(this);
    }

    d->TIFFOptions               = ploader->exportWidget(QLatin1String("TIFF"));

    if (d->TIFFOptions)
    {
        d->TIFFOptions->setParent(this);
    }

#ifdef HAVE_JASPER

    d->JPEG2000Options           = ploader->exportWidget(QLatin1String("JP2"));

    if (d->JPEG2000Options)
    {
        d->JPEG2000Options->setParent(this);
    }

#endif // HAVE_JASPER

    d->PGFOptions                = ploader->exportWidget(QLatin1String("PGF"));

    if (d->PGFOptions)
    {
        d->PGFOptions->setParent(this);
    }

#ifdef HAVE_X265

    d->HEIFOptions               = ploader->exportWidget(QLatin1String("HEIF"));

    if (d->HEIFOptions)
    {
        d->HEIFOptions->setParent(this);
    }

#endif // HAVE_X265

    d->JXLOptions                = ploader->exportWidget(QLatin1String("JXL"));

    if (d->JXLOptions)
    {
        d->JXLOptions->setParent(this);
    }

    d->WEBPOptions               = ploader->exportWidget(QLatin1String("WEBP"));

    if (d->WEBPOptions)
    {
        d->WEBPOptions->setParent(this);
    }

    d->AVIFOptions               = ploader->exportWidget(QLatin1String("AVIF"));

    if (d->AVIFOptions)
    {
        d->AVIFOptions->setParent(this);
    }

    // Show Settings Dialog Option

    d->showImageSettingsDialog   = new QCheckBox(panel);
    d->showImageSettingsDialog->setText(i18n("Show Settings Dialog when Saving Image Files"));
    d->showImageSettingsDialog->setWhatsThis(i18n("<ul><li>Checked: A dialog where settings can be changed when saving image files</li>"
                                                  "<li>Unchecked: Default settings are used when saving image files</li></ul>"));

    if (d->JPEGOptions)
    {
        vbox->addWidget(d->createGroupBox(d->JPEGOptions));
    }

    if (d->PNGOptions)
    {
        vbox->addWidget(d->createGroupBox(d->PNGOptions));
    }

    if (d->TIFFOptions)
    {
        vbox->addWidget(d->createGroupBox(d->TIFFOptions));
    }

#ifdef HAVE_JASPER

    if (d->JPEG2000Options)
    {
        vbox->addWidget(d->createGroupBox(d->JPEG2000Options));
    }

#endif // HAVE_JASPER

    if (d->PGFOptions)
    {
        vbox->addWidget(d->createGroupBox(d->PGFOptions));
    }

#ifdef HAVE_X265

    if (d->HEIFOptions)
    {
        vbox->addWidget(d->createGroupBox(d->HEIFOptions));
    }

#endif // HAVE_265

    if (d->JXLOptions)
    {
        vbox->addWidget(d->createGroupBox(d->JXLOptions));
    }

    if (d->WEBPOptions)
    {
        vbox->addWidget(d->createGroupBox(d->WEBPOptions));
    }

    if (d->AVIFOptions)
    {
        vbox->addWidget(d->createGroupBox(d->AVIFOptions));
    }

    vbox->addWidget(d->createGroupBox(d->showImageSettingsDialog));
    vbox->addStretch();

    panel->setLayout(vbox);
    setWidget(panel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    readSettings();
}

SetupIOFiles::~SetupIOFiles()
{
    delete d;
}

void SetupIOFiles::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    if (d->JPEGOptions)
    {
        group.writeEntry(d->configJPEGCompressionEntry,     d->JPEGOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(d->configJPEGSubSamplingEntry,     d->JPEGOptions->settings().value(QLatin1String("subsampling")).toInt());
    }

    // ---

    if (d->PNGOptions)
    {
        group.writeEntry(d->configPNGCompressionEntry,      d->PNGOptions->settings().value(QLatin1String("quality")).toInt());
    }

    // ---

    if (d->TIFFOptions)
    {
        group.writeEntry(d->configTIFFCompressionEntry,     d->TIFFOptions->settings().value(QLatin1String("compress")).toBool());
    }

#ifdef HAVE_JASPER

    if (d->JPEG2000Options)
    {
        group.writeEntry(d->configJPEG2000CompressionEntry, d->JPEG2000Options->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(d->configJPEG2000LossLessEntry,    d->JPEG2000Options->settings().value(QLatin1String("lossless")).toBool());
    }

#endif // HAVE_JASPER

    if (d->PGFOptions)
    {
        group.writeEntry(d->configPGFCompressionEntry,      d->PGFOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(d->configPGFLossLessEntry,         d->PGFOptions->settings().value(QLatin1String("lossless")).toBool());
    }

#ifdef HAVE_X265

    if (d->HEIFOptions)
    {
        group.writeEntry(d->configHEIFCompressionEntry,     d->HEIFOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(d->configHEIFLossLessEntry,        d->HEIFOptions->settings().value(QLatin1String("lossless")).toBool());
    }

#endif // HAVE_X265

    if (d->JXLOptions)
    {
        group.writeEntry(d->configJXLCompressionEntry,      d->JXLOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(d->configJXLLossLessEntry,         d->JXLOptions->settings().value(QLatin1String("lossless")).toBool());
    }

    if (d->WEBPOptions)
    {
        group.writeEntry(d->configWEBPCompressionEntry,     d->WEBPOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(d->configWEBPLossLessEntry,        d->WEBPOptions->settings().value(QLatin1String("lossless")).toBool());
    }

    if (d->AVIFOptions)
    {
        group.writeEntry(d->configAVIFCompressionEntry,     d->AVIFOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(d->configAVIFLossLessEntry,        d->AVIFOptions->settings().value(QLatin1String("lossless")).toBool());
    }

    group.writeEntry(d->configShowImageSettingsDialog,      d->showImageSettingsDialog->isChecked());
    config->sync();
}

void SetupIOFiles::readSettings()
{
    DImgLoaderPrms set;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    if (d->JPEGOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),     group.readEntry(d->configJPEGCompressionEntry,  75));
        set.insert(QLatin1String("subsampling"), group.readEntry(d->configJPEGSubSamplingEntry,  1));  // Medium sub-sampling
        d->JPEGOptions->setSettings(set);
    }

    // ---

    if (d->PNGOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"), group.readEntry(d->configPNGCompressionEntry,       9));
        d->PNGOptions->setSettings(set);
    }

    // ---

    if (d->TIFFOptions)
    {
        set.clear();
        set.insert(QLatin1String("compress"), group.readEntry(d->configTIFFCompressionEntry,     false));
        d->TIFFOptions->setSettings(set);
    }

#ifdef HAVE_JASPER

    if (d->JPEG2000Options)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(d->configJPEG2000CompressionEntry, 75));
        set.insert(QLatin1String("lossless"), group.readEntry(d->configJPEG2000LossLessEntry,    true));
        d->JPEG2000Options->setSettings(set);
    }

#endif // HAVE_JASPER

    if (d->PGFOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(d->configPGFCompressionEntry,      3));
        set.insert(QLatin1String("lossless"), group.readEntry(d->configPGFLossLessEntry,         true));
        d->PGFOptions->setSettings(set);
    }

#ifdef HAVE_X265

    if (d->HEIFOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(d->configHEIFCompressionEntry,     75));
        set.insert(QLatin1String("lossless"), group.readEntry(d->configHEIFLossLessEntry,        true));
        d->HEIFOptions->setSettings(set);
    }

#endif // HAVE_X265

    if (d->JXLOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(d->configJXLCompressionEntry,      75));
        set.insert(QLatin1String("lossless"), group.readEntry(d->configJXLLossLessEntry,         true));
        d->JXLOptions->setSettings(set);
    }

    if (d->WEBPOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(d->configWEBPCompressionEntry,     75));
        set.insert(QLatin1String("lossless"), group.readEntry(d->configWEBPLossLessEntry,        true));
        d->WEBPOptions->setSettings(set);
    }

    if (d->AVIFOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(d->configAVIFCompressionEntry,     75));
        set.insert(QLatin1String("lossless"), group.readEntry(d->configAVIFLossLessEntry,        true));
        d->AVIFOptions->setSettings(set);
    }

    d->showImageSettingsDialog->setChecked(group.readEntry(d->configShowImageSettingsDialog,     true));
}

} // namespace Digikam

#include "moc_setupiofiles.cpp"
