/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : EXIF adjustments settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exifadjust.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QDoubleSpinBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "metadatacheckbox.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN EXIFAdjust::Private
{
public:

    Private() = default;

    QCheckBox*        brightnessCheck       = nullptr;

    QComboBox*        gainControlCB         = nullptr;
    QComboBox*        contrastCB            = nullptr;
    QComboBox*        saturationCB          = nullptr;
    QComboBox*        sharpnessCB           = nullptr;
    QComboBox*        customRenderedCB      = nullptr;

    QDoubleSpinBox*   brightnessEdit        = nullptr;

    MetadataCheckBox* gainControlCheck      = nullptr;
    MetadataCheckBox* contrastCheck         = nullptr;
    MetadataCheckBox* saturationCheck       = nullptr;
    MetadataCheckBox* sharpnessCheck        = nullptr;
    MetadataCheckBox* customRenderedCheck   = nullptr;
};

EXIFAdjust::EXIFAdjust(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->brightnessCheck = new QCheckBox(i18n("Brightness (APEX):"), this);
    d->brightnessEdit  = new QDoubleSpinBox(this);
    d->brightnessEdit->setRange(-99.99, 99.99);
    d->brightnessEdit->setSingleStep(0.1);
    d->brightnessEdit->setValue(0.0);
    d->brightnessEdit->setWhatsThis(i18n("Set here the brightness adjustment value in APEX unit "
                                         "used by camera to take the picture."));

    // --------------------------------------------------------

    d->gainControlCheck = new MetadataCheckBox(i18n("Gain Control:"), this);
    d->gainControlCB    = new QComboBox(this);
    d->gainControlCB->insertItem(0, i18nc("gain control", "None"));
    d->gainControlCB->insertItem(1, i18nc("gain control", "Low gain up"));
    d->gainControlCB->insertItem(2, i18nc("gain control", "High gain up"));
    d->gainControlCB->insertItem(3, i18nc("gain control", "Low gain down"));
    d->gainControlCB->insertItem(4, i18nc("gain control", "High gain down"));
    d->gainControlCB->setWhatsThis(i18n("Set here the degree of overall image gain adjustment "
                                        "used by camera to take the picture."));

    // --------------------------------------------------------

    d->contrastCheck = new MetadataCheckBox(i18n("Contrast:"), this);
    d->contrastCB    = new QComboBox(this);
    d->contrastCB->insertItem(0, i18nc("contrast mode", "Normal"));
    d->contrastCB->insertItem(1, i18nc("contrast mode", "Soft"));
    d->contrastCB->insertItem(2, i18nc("contrast mode", "Hard"));
    d->contrastCB->setWhatsThis(i18n("Set here the direction of contrast processing "
                                     "applied by the camera to take the picture."));

    // --------------------------------------------------------

    d->saturationCheck = new MetadataCheckBox(i18n("Saturation:"), this);
    d->saturationCB    = new QComboBox(this);
    d->saturationCB->insertItem(0, i18nc("saturation mode", "Normal"));
    d->saturationCB->insertItem(1, i18nc("saturation mode", "Low"));
    d->saturationCB->insertItem(2, i18nc("saturation mode", "High"));
    d->saturationCB->setWhatsThis(i18n("Set here the direction of saturation processing "
                                       "applied by the camera to take the picture."));

    // --------------------------------------------------------

    d->sharpnessCheck = new MetadataCheckBox(i18n("Sharpness:"), this);
    d->sharpnessCB    = new QComboBox(this);
    d->sharpnessCB->insertItem(0, i18nc("sharpness mode", "Normal"));
    d->sharpnessCB->insertItem(1, i18nc("sharpness mode", "Soft"));
    d->sharpnessCB->insertItem(2, i18nc("sharpness mode", "Hard"));
    d->sharpnessCB->setWhatsThis(i18n("Set here the direction of sharpness processing "
                                      "applied by the camera to take the picture."));

    // --------------------------------------------------------

    d->customRenderedCheck = new MetadataCheckBox(i18n("Custom rendered:"), this);
    d->customRenderedCB    = new QComboBox(this);
    d->customRenderedCB->insertItem(0, i18n("Normal process"));
    d->customRenderedCB->insertItem(1, i18n("Custom process"));
    d->customRenderedCB->setWhatsThis(i18n("Set here the use of special processing on "
                                           "image data, such as rendering geared to output."));

    grid->addWidget(d->brightnessCheck,     0, 0, 1, 1);
    grid->addWidget(d->brightnessEdit,      0, 2, 1, 1);
    grid->addWidget(d->gainControlCheck,    1, 0, 1, 1);
    grid->addWidget(d->gainControlCB,       1, 2, 1, 1);
    grid->addWidget(d->contrastCheck,       2, 0, 1, 1);
    grid->addWidget(d->contrastCB,          2, 2, 1, 1);
    grid->addWidget(d->saturationCheck,     3, 0, 1, 1);
    grid->addWidget(d->saturationCB,        3, 2, 1, 1);
    grid->addWidget(d->sharpnessCheck,      4, 0, 1, 1);
    grid->addWidget(d->sharpnessCB,         4, 2, 1, 1);
    grid->addWidget(d->customRenderedCheck, 5, 0, 1, 1);
    grid->addWidget(d->customRenderedCB,    5, 2, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(6, 10);

    int spacing = layoutSpacing();

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->brightnessCheck, SIGNAL(toggled(bool)),
            d->brightnessEdit, SLOT(setEnabled(bool)));

    connect(d->gainControlCheck, SIGNAL(toggled(bool)),
            d->gainControlCB, SLOT(setEnabled(bool)));

    connect(d->contrastCheck, SIGNAL(toggled(bool)),
            d->contrastCB, SLOT(setEnabled(bool)));

    connect(d->saturationCheck, SIGNAL(toggled(bool)),
            d->saturationCB, SLOT(setEnabled(bool)));

    connect(d->sharpnessCheck, SIGNAL(toggled(bool)),
            d->sharpnessCB, SLOT(setEnabled(bool)));

    connect(d->customRenderedCheck, SIGNAL(toggled(bool)),
            d->customRenderedCB, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->brightnessCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->gainControlCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->contrastCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->saturationCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sharpnessCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->customRenderedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->gainControlCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->contrastCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->saturationCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->sharpnessCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->customRenderedCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->brightnessEdit, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalModified()));
}

EXIFAdjust::~EXIFAdjust()
{
    delete d;
}

void EXIFAdjust::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    long int num = 1, den = 1;
    long     val = 0;

    d->brightnessEdit->setValue(0.0);
    d->brightnessCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.BrightnessValue", num, den))
    {
        d->brightnessEdit->setValue((double)(num) / (double)(den));
        d->brightnessCheck->setChecked(true);
    }

    d->brightnessEdit->setEnabled(d->brightnessCheck->isChecked());

    d->gainControlCB->setCurrentIndex(0);
    d->gainControlCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.GainControl", val))
    {
        if ((val >= 0) && (val <= 4))
        {
            d->gainControlCB->setCurrentIndex(val);
            d->gainControlCheck->setChecked(true);
        }
        else
        {
            d->gainControlCheck->setValid(false);
        }
    }

    d->gainControlCB->setEnabled(d->gainControlCheck->isChecked());

    d->contrastCB->setCurrentIndex(0);
    d->contrastCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.Contrast", val))
    {
        if ((val >= 0) && (val <= 2))
        {
            d->contrastCB->setCurrentIndex(val);
            d->contrastCheck->setChecked(true);
        }
        else
        {
            d->contrastCheck->setValid(false);
        }
    }

    d->contrastCB->setEnabled(d->contrastCheck->isChecked());

    d->saturationCB->setCurrentIndex(0);
    d->saturationCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.Saturation", val))
    {
        if ((val >= 0) && (val <= 2))
        {
            d->saturationCB->setCurrentIndex(val);
            d->saturationCheck->setChecked(true);
        }
        else
        {
            d->saturationCheck->setValid(false);
        }
    }

    d->saturationCB->setEnabled(d->saturationCheck->isChecked());

    d->sharpnessCB->setCurrentIndex(0);
    d->sharpnessCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.Sharpness", val))
    {
        if ((val >= 0) && (val <= 2))
        {
            d->sharpnessCB->setCurrentIndex(val);
            d->sharpnessCheck->setChecked(true);
        }
        else
        {
            d->sharpnessCheck->setValid(false);
        }
    }

    d->sharpnessCB->setEnabled(d->sharpnessCheck->isChecked());

    d->customRenderedCB->setCurrentIndex(0);
    d->customRenderedCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.CustomRendered", val))
    {
        if ((val >= 0) && (val <= 1))
        {
            d->customRenderedCB->setCurrentIndex(val);
            d->customRenderedCheck->setChecked(true);
        }
        else
        {
            d->customRenderedCheck->setValid(false);
        }
    }

    d->customRenderedCB->setEnabled(d->customRenderedCheck->isChecked());

    blockSignals(false);
}

void EXIFAdjust::applyMetadata(const DMetadata& meta)
{
    long int num = 1, den = 1;

    if (d->brightnessCheck->isChecked())
    {
        meta.convertToRational(d->brightnessEdit->value(), &num, &den, 1);
        meta.setExifTagRational("Exif.Photo.BrightnessValue", num, den);
    }
    else
    {
        meta.removeExifTag("Exif.Photo.BrightnessValue");
    }

    if      (d->gainControlCheck->isChecked())
    {
        meta.setExifTagUShort("Exif.Photo.GainControl", d->gainControlCB->currentIndex());
    }
    else if (d->gainControlCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.GainControl");
    }

    if      (d->contrastCheck->isChecked())
    {
        meta.setExifTagUShort("Exif.Photo.Contrast", d->contrastCB->currentIndex());
    }
    else if (d->contrastCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.Contrast");
    }

    if      (d->saturationCheck->isChecked())
    {
        meta.setExifTagUShort("Exif.Photo.Saturation", d->saturationCB->currentIndex());
    }
    else if (d->saturationCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.Saturation");
    }

    if      (d->sharpnessCheck->isChecked())
    {
        meta.setExifTagUShort("Exif.Photo.Sharpness", d->sharpnessCB->currentIndex());
    }
    else if (d->sharpnessCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.Sharpness");
    }

    if      (d->customRenderedCheck->isChecked())
    {
        meta.setExifTagUShort("Exif.Photo.CustomRendered", d->customRenderedCB->currentIndex());
    }
    else if (d->customRenderedCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.CustomRendered");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_exifadjust.cpp"
