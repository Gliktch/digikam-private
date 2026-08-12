/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : EXIF light settings page.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiflight.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QMap>
#include <QGridLayout>
#include <QComboBox>
#include <QApplication>
#include <QStyle>
#include <QDoubleSpinBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "metadatacheckbox.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN FlashMode
{
public:

    FlashMode() = default;

    FlashMode(int id, const QString& desc)
      : m_id  (id),
        m_desc(desc)
    {
    }

    int id()       const
    {
        return m_id;
    }

    QString desc() const
    {
        return m_desc;
    }

private:

    int     m_id    = 0;
    QString m_desc;
};

// --------------------------------------------------------------------------

class Q_DECL_HIDDEN EXIFLight::Private
{
public:

    Private()
    {
        flashModeMap.insert(0,  FlashMode( 0x00, i18n("No flash") ));
        flashModeMap.insert(1,  FlashMode( 0x01, i18n("Fired") ));
        flashModeMap.insert(2,  FlashMode( 0x05, i18n("Fired, no strobe return light") ));
        flashModeMap.insert(3,  FlashMode( 0x07, i18n("Fired, strobe return light") ));
        flashModeMap.insert(4,  FlashMode( 0x09, i18n("Yes, compulsory") ));
        flashModeMap.insert(5,  FlashMode( 0x0d, i18n("Yes, compulsory, no return light") ));
        flashModeMap.insert(6,  FlashMode( 0x0f, i18n("Yes, compulsory, return light") ));
        flashModeMap.insert(7,  FlashMode( 0x10, i18n("No, compulsory") ));
        flashModeMap.insert(8,  FlashMode( 0x18, i18n("No, auto") ));
        flashModeMap.insert(9,  FlashMode( 0x19, i18n("Yes, auto") ));
        flashModeMap.insert(10, FlashMode( 0x1d, i18n("Yes, auto, no return light") ));
        flashModeMap.insert(11, FlashMode( 0x1f, i18n("Yes, auto, return light") ));
        flashModeMap.insert(12, FlashMode( 0x20, i18n("No flash function") ));
        flashModeMap.insert(13, FlashMode( 0x41, i18n("Yes, red-eye") ));
        flashModeMap.insert(14, FlashMode( 0x45, i18n("Yes, red-eye, no return light") ));
        flashModeMap.insert(15, FlashMode( 0x47, i18n("Yes, red-eye, return light") ));
        flashModeMap.insert(16, FlashMode( 0x49, i18n("Yes, compulsory, red-eye") ));
        flashModeMap.insert(17, FlashMode( 0x4d, i18n("Yes, compulsory, red-eye, no return light") ));
        flashModeMap.insert(18, FlashMode( 0x4f, i18n("Yes, compulsory, red-eye, return light") ));
        flashModeMap.insert(19, FlashMode( 0x59, i18n("Yes, auto, red-eye") ));
        flashModeMap.insert(20, FlashMode( 0x5d, i18n("Yes, auto, red-eye, no return light") ));
        flashModeMap.insert(21, FlashMode( 0x5f, i18n("Yes, auto, red-eye, return light") ));
    }

    typedef QMap<int, FlashMode> FlashModeMap;

    FlashModeMap      flashModeMap;

    QCheckBox*        flashEnergyCheck  = nullptr;

    QComboBox*        lightSourceCB     = nullptr;
    QComboBox*        flashModeCB       = nullptr;
    QComboBox*        whiteBalanceCB    = nullptr;

    QDoubleSpinBox*   flashEnergyEdit   = nullptr;

    MetadataCheckBox* lightSourceCheck  = nullptr;
    MetadataCheckBox* flashModeCheck    = nullptr;
    MetadataCheckBox* whiteBalanceCheck = nullptr;
};

// --------------------------------------------------------------------------

EXIFLight::EXIFLight(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    // --------------------------------------------------------

    d->lightSourceCheck = new MetadataCheckBox(i18n("Light source:"), this);
    d->lightSourceCB    = new QComboBox(this);
    d->lightSourceCB->insertItem(0,  i18nc("light source", "Unknown"));
    d->lightSourceCB->insertItem(1,  i18nc("light source", "Daylight"));
    d->lightSourceCB->insertItem(2,  i18nc("light source", "Fluorescent"));
    d->lightSourceCB->insertItem(3,  i18nc("light source", "Tungsten (incandescent light)"));
    d->lightSourceCB->insertItem(4,  i18nc("light source", "Flash"));
    d->lightSourceCB->insertItem(5,  i18nc("light source", "Fine weather"));
    d->lightSourceCB->insertItem(6,  i18nc("light source", "Cloudy weather"));
    d->lightSourceCB->insertItem(7,  i18nc("light source", "Shade"));
    d->lightSourceCB->insertItem(8,  i18nc("light source", "Daylight fluorescent (D 5700 - 7100K)"));
    d->lightSourceCB->insertItem(9,  i18nc("light source", "Day white fluorescent (N 4600 - 5400K)"));
    d->lightSourceCB->insertItem(10, i18nc("light source", "Cool white fluorescent (W 3900 - 4500K)"));
    d->lightSourceCB->insertItem(11, i18nc("light source", "White fluorescent (WW 3200 - 3700K)"));
    d->lightSourceCB->insertItem(12, i18nc("light source", "Standard light A"));
    d->lightSourceCB->insertItem(13, i18nc("light source", "Standard light B"));
    d->lightSourceCB->insertItem(14, i18nc("light source", "Standard light C"));
    d->lightSourceCB->insertItem(15, i18nc("light source", "D55"));
    d->lightSourceCB->insertItem(16, i18nc("light source", "D65"));
    d->lightSourceCB->insertItem(17, i18nc("light source", "D75"));
    d->lightSourceCB->insertItem(18, i18nc("light source", "D50"));
    d->lightSourceCB->insertItem(19, i18nc("light source", "ISO studio tungsten"));
    d->lightSourceCB->insertItem(20, i18nc("light source", "Other light source"));
    d->lightSourceCB->setWhatsThis(i18n("Select here the kind of light source used "
                                        "to take the picture."));

    // --------------------------------------------------------

    d->flashModeCheck = new MetadataCheckBox(i18n("Flash mode:"), this);
    d->flashModeCB    = new QComboBox(this);

    for (Private::FlashModeMap::Iterator it = d->flashModeMap.begin(); it != d->flashModeMap.end(); ++it)
    {
       d->flashModeCB->addItem(it.value().desc());
    }

    d->flashModeCB->setWhatsThis(i18n("Select here the flash program mode used by the camera "
                                      "to take the picture."));

    // --------------------------------------------------------

    d->flashEnergyCheck = new QCheckBox(i18n("Flash energy (BCPS):"), this);
    d->flashEnergyEdit  = new QDoubleSpinBox(this);
    d->flashEnergyEdit->setRange(1.0, 10000.0);
    d->flashEnergyEdit->setSingleStep(1.0);
    d->flashEnergyEdit->setValue(1.0);
    d->flashEnergyEdit->setDecimals(1);
    d->flashEnergyEdit->setWhatsThis(i18n("Set here the flash energy used to take the picture "
                                          "in BCPS units. Beam Candle Power Seconds is the measure "
                                          "of effective intensity of a light source when it is "
                                          "focused into a beam by a reflector or lens. This value "
                                          "is the effective intensity for a period of one second."));

    // --------------------------------------------------------

    d->whiteBalanceCheck = new MetadataCheckBox(i18n("White balance:"), this);
    d->whiteBalanceCB    = new QComboBox(this);
    d->whiteBalanceCB->insertItem(0, i18nc("@item: white balance", "Auto"));
    d->whiteBalanceCB->insertItem(1, i18nc("@item: white balance", "Manual"));
    d->whiteBalanceCB->setWhatsThis(i18nc("@info", "Select here the white balance mode set by the camera when "
                                         "the picture was taken."));

    // --------------------------------------------------------

    grid->addWidget(d->lightSourceCheck,    0, 0, 1, 1);
    grid->addWidget(d->lightSourceCB,       0, 2, 1, 2);
    grid->addWidget(d->flashModeCheck,      1, 0, 1, 1);
    grid->addWidget(d->flashModeCB,         1, 2, 1, 2);
    grid->addWidget(d->flashEnergyCheck,    2, 0, 1, 1);
    grid->addWidget(d->flashEnergyEdit,     2, 2, 1, 1);
    grid->addWidget(d->whiteBalanceCheck,   3, 0, 1, 1);
    grid->addWidget(d->whiteBalanceCB,      3, 2, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(4, 10);

    int spacing = layoutSpacing();


    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->lightSourceCheck, SIGNAL(toggled(bool)),
            d->lightSourceCB, SLOT(setEnabled(bool)));

    connect(d->flashModeCheck, SIGNAL(toggled(bool)),
            d->flashModeCB, SLOT(setEnabled(bool)));

    connect(d->flashEnergyCheck, SIGNAL(toggled(bool)),
            d->flashEnergyEdit, SLOT(setEnabled(bool)));

    connect(d->whiteBalanceCheck, SIGNAL(toggled(bool)),
            d->whiteBalanceCB, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->flashEnergyCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->lightSourceCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->flashModeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->whiteBalanceCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->lightSourceCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->flashModeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->whiteBalanceCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->flashEnergyEdit, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalModified()));
}

EXIFLight::~EXIFLight()
{
    delete d;
}

void EXIFLight::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    long int num = 1, den = 1;
    long     val = 0;

    d->lightSourceCB->setCurrentIndex(0);
    d->lightSourceCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.LightSource", val))
    {
        if (((val >= 0) && (val <= 4)) || ((val > 8) && (val < 16)) || ((val > 16) && (val < 25)) || (val == 255))
        {
            if      ((val > 8) && (val < 16))
            {
                val = val - 4;
            }
            else if ((val > 16) && (val < 25))
            {
                val = val - 5;
            }
            else if (val == 255)
            {
                val = 20;
            }

            d->lightSourceCB->setCurrentIndex(val);
            d->lightSourceCheck->setChecked(true);
        }
        else
        {
            d->lightSourceCheck->setValid(false);
        }
    }

    d->lightSourceCB->setEnabled(d->lightSourceCheck->isChecked());

    d->flashModeCB->setCurrentIndex(0);
    d->flashModeCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.Flash", val))
    {
        int item = -1;

        for (Private::FlashModeMap::Iterator it = d->flashModeMap.begin();
            it != d->flashModeMap.end(); ++it )
        {
            if (it.value().id() == val)
            {
                item = it.key();
            }
        }

        if (item != -1)
        {
            d->flashModeCB->setCurrentIndex(item);
            d->flashModeCheck->setChecked(true);
        }
        else
        {
            d->flashModeCheck->setValid(false);
        }
    }

    d->flashModeCB->setEnabled(d->flashModeCheck->isChecked());

    d->flashEnergyEdit->setValue(1.0);
    d->flashEnergyCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.FlashEnergy", num, den))
    {
        d->flashEnergyEdit->setValue((double)(num) / (double)(den));
        d->flashEnergyCheck->setChecked(true);
    }

    d->flashEnergyEdit->setEnabled(d->flashEnergyCheck->isChecked());

    d->whiteBalanceCB->setCurrentIndex(0);
    d->whiteBalanceCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.WhiteBalance", val))
    {
        if ((val >= 0) && (val <= 1))
        {
            d->whiteBalanceCB->setCurrentIndex(val);
            d->whiteBalanceCheck->setChecked(true);
        }
        else
        {
            d->whiteBalanceCheck->setValid(false);
        }
    }

    d->whiteBalanceCB->setEnabled(d->whiteBalanceCheck->isChecked());

    blockSignals(false);
}

void EXIFLight::applyMetadata(const DMetadata& meta)
{
    long int num = 1, den = 1;

    if      (d->lightSourceCheck->isChecked())
    {
        long val = d->lightSourceCB->currentIndex();

        if      ((val > 4) && (val < 12))
        {
            val = val + 4;
        }
        else if ((val > 11) && (val < 20))
        {
            val = val + 5;
        }
        else if (val == 20)
        {
            val = 255;
        }

        meta.setExifTagUShort("Exif.Photo.LightSource", val);
    }
    else if (d->lightSourceCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.LightSource");
    }

    if      (d->flashModeCheck->isChecked())
    {
        long val = d->flashModeCB->currentIndex();
        meta.setExifTagUShort("Exif.Photo.Flash", d->flashModeMap[val].id());
    }
    else if (d->flashModeCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.Flash");
    }

    if (d->flashEnergyCheck->isChecked())
    {
        meta.convertToRational(d->flashEnergyEdit->value(), &num, &den, 1);
        meta.setExifTagURational("Exif.Photo.FlashEnergy", num, den);
    }
    else
    {
        meta.removeExifTag("Exif.Photo.FlashEnergy");
    }

    if      (d->whiteBalanceCheck->isChecked())
    {
        meta.setExifTagUShort("Exif.Photo.WhiteBalance", d->whiteBalanceCB->currentIndex());
    }
    else if (d->whiteBalanceCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.WhiteBalance");
    }
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_exiflight.cpp"
