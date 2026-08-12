/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : a stack of widgets to set image file save
 *               options into image editor.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filesaveoptionsbox.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImageReader>
#include <QLabel>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "dpluginloader.h"

namespace Digikam
{

class Q_DECL_HIDDEN FileSaveOptionsBox::Private
{
public:

    Private() = default;

public:

    QWidget*            noneOptions     = nullptr;

    QGridLayout*        noneGrid        = nullptr;

    QLabel*             labelNone       = nullptr;

    DImgLoaderSettings* JPEGOptions     = nullptr;
    DImgLoaderSettings* PNGOptions      = nullptr;
    DImgLoaderSettings* TIFFOptions     = nullptr;

#ifdef HAVE_JASPER

    DImgLoaderSettings* JPEG2000Options = nullptr;

#endif // HAVE_JASPER

#ifdef HAVE_X265

    DImgLoaderSettings* HEIFOptions     = nullptr;

#endif // HAVE_X265

    DImgLoaderSettings* PGFOptions      = nullptr;
    DImgLoaderSettings* JXLOptions      = nullptr;
    DImgLoaderSettings* WEBPOptions     = nullptr;
    DImgLoaderSettings* AVIFOptions     = nullptr;
};

FileSaveOptionsBox::FileSaveOptionsBox(QWidget* const parent)
    : QStackedWidget(parent),
      d             (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    //-- NONE Settings ------------------------------------------------------

    d->noneOptions               = new QWidget(this);
    d->noneGrid                  = new QGridLayout(d->noneOptions);
    d->noneGrid->setSpacing(layoutSpacing());

    d->noneOptions->setLayout(d->noneGrid);
    d->labelNone                 = new QLabel(i18n("No options available"), d->noneOptions);
    d->noneGrid->addWidget(d->labelNone, 0, 0, 0, 1);
    DPluginLoader* const ploader = DPluginLoader::instance();

    //-- JPEG Settings ------------------------------------------------------

    d->JPEGOptions     = ploader->exportWidget(QLatin1String("JPEG"));
    d->JPEGOptions->setParent(this);

    //-- PNG Settings -------------------------------------------------------

    d->PNGOptions      = ploader->exportWidget(QLatin1String("PNG"));
    d->PNGOptions->setParent(this);

    //-- TIFF Settings ------------------------------------------------------

    d->TIFFOptions     = ploader->exportWidget(QLatin1String("TIFF"));
    d->TIFFOptions->setParent(this);

    //-- JPEG 2000 Settings -------------------------------------------------

#ifdef HAVE_JASPER

    d->JPEG2000Options = ploader->exportWidget(QLatin1String("JP2"));

    if (d->JPEG2000Options)
    {
        d->JPEG2000Options->setParent(this);
    }

#endif // HAVE_JASPER

    //-- PGF Settings -------------------------------------------------

    d->PGFOptions      = ploader->exportWidget(QLatin1String("PGF"));
    d->PGFOptions->setParent(this);

    //-- JXL Settings -------------------------------------------------

    // NOTE: JXL support depend of JXL QImage loader plugin availability.

    d->JXLOptions      = ploader->exportWidget(QLatin1String("JXL"));

    if (d->JXLOptions)
    {
        d->JXLOptions->setParent(this);
    }

    //-- WEBP Settings -------------------------------------------------

    // NOTE: WEBP support depend of WEBP QImage loader plugin availability.

    d->WEBPOptions     = ploader->exportWidget(QLatin1String("WEBP"));

    if (d->WEBPOptions)
    {
        d->WEBPOptions->setParent(this);
    }

    //-- AVIF Settings -------------------------------------------------

    // NOTE: AVIF support depend of JXL QImage loader plugin availability.

    d->AVIFOptions     = ploader->exportWidget(QLatin1String("AVIF"));

    if (d->AVIFOptions)
    {
        d->AVIFOptions->setParent(this);
    }

    //-- HEIF Settings -------------------------------------------------

#ifdef HAVE_X265

    d->HEIFOptions     = ploader->exportWidget(QLatin1String("HEIF"));

    if (d->HEIFOptions)
    {
        d->HEIFOptions->setParent(this);
    }

#endif // HAVE_X265

    //-----------------------------------------------------------------------

    insertWidget(NONE,        d->noneOptions);
    insertWidget(JPEG,        d->JPEGOptions);
    insertWidget(PNG,         d->PNGOptions);
    insertWidget(TIFF,        d->TIFFOptions);

#ifdef HAVE_JASPER

    if (d->JPEG2000Options)
    {
        insertWidget(JP2K,    d->JPEG2000Options);
    }

#endif // HAVE_JASPER

    insertWidget(PGF,         d->PGFOptions);

#ifdef HAVE_X265

    if (d->HEIFOptions)
    {
        insertWidget(HEIF,    d->HEIFOptions);
    }

#endif // HAVE_X265

    if (d->JXLOptions)
    {
        insertWidget(JXL,     d->JXLOptions);
    }

    if (d->WEBPOptions)
    {
        insertWidget(WEBP,    d->WEBPOptions);
    }

    if (d->AVIFOptions)
    {
        insertWidget(AVIF,    d->AVIFOptions);
    }

    //-----------------------------------------------------------------------

    readSettings();
}

FileSaveOptionsBox::~FileSaveOptionsBox()
{
    delete d;
}

void FileSaveOptionsBox::setImageFileFormat(const QString& ext)
{
    qCDebug(DIGIKAM_WIDGETS_LOG) << "Format selected: " << ext;
    setCurrentIndex(discoverFormat(ext, NONE));
}

FileSaveOptionsBox::FORMAT FileSaveOptionsBox::discoverFormat(const QString& filename,
                                                              FileSaveOptionsBox::FORMAT fallback)
{
    qCDebug(DIGIKAM_WIDGETS_LOG) << "Trying to discover format based on filename '" << filename
                                 << "', fallback = " << fallback;

    QStringList splitParts = filename.split(QLatin1Char('.'));
    QString ext;

    if (splitParts.size() < 2)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "filename '" << filename
                                     << "' does not contain an extension separated by a point.";
        ext = filename;
    }
    else
    {
        ext = splitParts.at(splitParts.size() - 1);
    }

    ext = ext.toUpper();

    FORMAT format = fallback;

    if      (ext.contains(QLatin1String("JPEG")) || ext.contains(QLatin1String("JPG")) || ext.contains(QLatin1String("JPE")))
    {
        format = JPEG;
    }
    else if (ext.contains(QLatin1String("PNG")))
    {
        format = PNG;
    }
    else if (ext.contains(QLatin1String("TIFF")) || ext.contains(QLatin1String("TIF")))
    {
        format = TIFF;
    }

#ifdef HAVE_JASPER

    else if (ext.contains(QLatin1String("JP2")) || ext.contains(QLatin1String("JPX")) || ext.contains(QLatin1String("JPC")) ||
             ext.contains(QLatin1String("PGX")) || ext.contains(QLatin1String("J2K")))
    {
        format = JP2K;
    }

#endif // HAVE_JASPER

#ifdef HAVE_X265

    else if (ext.contains(QLatin1String("HEIC")) || ext.contains(QLatin1String("HEIF")) || ext.contains(QLatin1String("HIF")))
    {
        format = HEIF;
    }

#endif // HAVE_X265

    else if (ext.contains(QLatin1String("PGF")))
    {
        format = PGF;
    }
    else if (ext.contains(QLatin1String("JXL")))
    {
        format = JXL;
    }
    else if (ext.contains(QLatin1String("WEBP")))
    {
        format = WEBP;
    }
    else if (ext.contains(QLatin1String("AVIF")))
    {
        format = AVIF;
    }
    else
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Using fallback format " << fallback;
    }

    qCDebug(DIGIKAM_WIDGETS_LOG) << "Discovered format: " << format;

    return format;
}

void FileSaveOptionsBox::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    group.writeEntry(QLatin1String("JPEGCompression"),     d->JPEGOptions->settings().value(QLatin1String("quality")).toInt());
    group.writeEntry(QLatin1String("JPEGSubSampling"),     d->JPEGOptions->settings().value(QLatin1String("subsampling")).toInt());

    // ---

    group.writeEntry(QLatin1String("PNGCompression"),      d->PNGOptions->settings().value(QLatin1String("quality")).toInt());

    // ---

    group.writeEntry(QLatin1String("TIFFCompression"),     d->TIFFOptions->settings().value(QLatin1String("compress")).toBool());

#ifdef HAVE_JASPER

    if (d->JPEG2000Options)
    {
        group.writeEntry(QLatin1String("JPEG2000Compression"), d->JPEG2000Options->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(QLatin1String("JPEG2000LossLess"),    d->JPEG2000Options->settings().value(QLatin1String("lossless")).toBool());
    }

#endif // HAVE_JASPER

    group.writeEntry(QLatin1String("PGFCompression"),      d->PGFOptions->settings().value(QLatin1String("quality")).toInt());
    group.writeEntry(QLatin1String("PGFLossLess"),         d->PGFOptions->settings().value(QLatin1String("lossless")).toBool());

#ifdef HAVE_X265

    if (d->HEIFOptions)
    {
        group.writeEntry(QLatin1String("HEIFCompression"),     d->HEIFOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(QLatin1String("HEIFLossLess"),        d->HEIFOptions->settings().value(QLatin1String("lossless")).toBool());
    }

#endif // HAVE_X265

    if (d->JXLOptions)
    {
        group.writeEntry(QLatin1String("JXLCompression"),  d->JXLOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(QLatin1String("JXLLossLess"),     d->JXLOptions->settings().value(QLatin1String("lossless")).toBool());
    }

    if (d->WEBPOptions)
    {
        group.writeEntry(QLatin1String("WEBPCompression"), d->WEBPOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(QLatin1String("WEBPLossLess"),    d->WEBPOptions->settings().value(QLatin1String("lossless")).toBool());
    }

    if (d->AVIFOptions)
    {
        group.writeEntry(QLatin1String("AVIFCompression"),  d->AVIFOptions->settings().value(QLatin1String("quality")).toInt());
        group.writeEntry(QLatin1String("AVIFLossLess"),     d->AVIFOptions->settings().value(QLatin1String("lossless")).toBool());
    }

    config->sync();
}

void FileSaveOptionsBox::readSettings()
{
    DImgLoaderPrms set;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));

    set.clear();
    set.insert(QLatin1String("quality"),     group.readEntry(QLatin1String("JPEGCompression"),     75));
    set.insert(QLatin1String("subsampling"), group.readEntry(QLatin1String("JPEGSubSampling"),     1));  ///< Medium subsampling
    d->JPEGOptions->setSettings(set);

    // ---

    set.clear();
    set.insert(QLatin1String("quality"),  group.readEntry(QLatin1String("PNGCompression"),         9));
    d->PNGOptions->setSettings(set);

    // ---

    set.clear();
    set.insert(QLatin1String("compress"), group.readEntry(QLatin1String("TIFFCompression"),        false));
    d->TIFFOptions->setSettings(set);

#ifdef HAVE_JASPER

    if (d->JPEG2000Options)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(QLatin1String("JPEG2000Compression"),    75));
        set.insert(QLatin1String("lossless"), group.readEntry(QLatin1String("JPEG2000LossLess"),       true));
        d->JPEG2000Options->setSettings(set);
    }

#endif // HAVE_JASPER

    set.clear();
    set.insert(QLatin1String("quality"),  group.readEntry(QLatin1String("PGFCompression"),         3));
    set.insert(QLatin1String("lossless"), group.readEntry(QLatin1String("PGFLossLess"),            true));
    d->PGFOptions->setSettings(set);

#ifdef HAVE_X265

    if (d->HEIFOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(QLatin1String("HEIFCompression"),        75));
        set.insert(QLatin1String("lossless"), group.readEntry(QLatin1String("HEIFLossLess"),           true));
        d->HEIFOptions->setSettings(set);
    }

#endif // HAVE_X265

    if (d->JXLOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(QLatin1String("JXLCompression"),     75));
        set.insert(QLatin1String("lossless"), group.readEntry(QLatin1String("JXLLossLess"),        true));
        d->JXLOptions->setSettings(set);
    }

    if (d->WEBPOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(QLatin1String("WEBPCompression"),     75));
        set.insert(QLatin1String("lossless"), group.readEntry(QLatin1String("WEBPLossLess"),        true));
        d->WEBPOptions->setSettings(set);
    }

    if (d->AVIFOptions)
    {
        set.clear();
        set.insert(QLatin1String("quality"),  group.readEntry(QLatin1String("AVIFCompression"),     75));
        set.insert(QLatin1String("lossless"), group.readEntry(QLatin1String("AVIFLossLess"),        true));
        d->AVIFOptions->setSettings(set);
    }
}

} // namespace Digikam

#include "moc_filesaveoptionsbox.cpp"
