/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-02
 * Description : AVIF image export settings widget.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgavifexportsettings.h"

// Qt includes

#include <QString>
#include <QLabel>
#include <QCheckBox>
#include <QLayout>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dnuminput.h"

namespace Digikam
{

class Q_DECL_HIDDEN DImgAVIFExportSettings::Private
{

public:

    Private() = default;

    QGridLayout*  AVIFGrid              = nullptr;

    QLabel*       labelAVIFcompression  = nullptr;

    QCheckBox*    AVIFLossLess          = nullptr;

    DIntNumInput* AVIFcompression       = nullptr;
};

DImgAVIFExportSettings::DImgAVIFExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->AVIFGrid     = new QGridLayout(this);
    d->AVIFLossLess = new QCheckBox(i18n("Lossless AVIF files"), this);

    d->AVIFLossLess->setWhatsThis(i18n("<p>Toggle lossless compression for AVIF images.</p>"
                                       "<p>If this option is enabled, a lossless method will be used "
                                       "to compress JPEG XL pictures.</p>"));

    d->AVIFcompression = new DIntNumInput(this);
    d->AVIFcompression->setDefaultValue(75);
    d->AVIFcompression->setRange(1, 99, 1);
    d->labelAVIFcompression = new QLabel(i18n("AVIF quality:"), this);

    d->AVIFcompression->setWhatsThis(i18n("<p>The quality value for AVIF images:</p>"
                                          "<p><b>1</b>: low quality (high compression and small "
                                          "file size)<br>"
                                          "<b>50</b>: medium quality<br>"
                                          "<b>75</b>: good quality (default)<br>"
                                          "<b>99</b>: high quality (no compression and "
                                          "large file size)</p>"
                                          "<p><b>Note: AVIF is not a lossless image "
                                          "compression format when you use this setting.</b></p>"));

    d->AVIFGrid->addWidget(d->AVIFLossLess,         0, 0, 1, 2);
    d->AVIFGrid->addWidget(d->labelAVIFcompression, 1, 0, 1, 2);
    d->AVIFGrid->addWidget(d->AVIFcompression,      2, 0, 1, 2);
    d->AVIFGrid->setColumnStretch(1, 10);
    d->AVIFGrid->setRowStretch(3, 10);
    d->AVIFGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->AVIFGrid->setSpacing(spacing);

    connect(d->AVIFLossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleAVIFLossLess(bool)));

    connect(d->AVIFLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->AVIFcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgAVIFExportSettings::~DImgAVIFExportSettings()
{
    delete d;
}

void DImgAVIFExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if      (it.key() == QLatin1String("quality"))
        {
            d->AVIFcompression->setValue(it.value().toInt());
        }
        else if (it.key() == QLatin1String("lossless"))
        {
            d->AVIFLossLess->setChecked(it.value().toBool());
        }
    }

    slotToggleAVIFLossLess(d->AVIFLossLess->isChecked());
}

DImgLoaderPrms DImgAVIFExportSettings::settings() const
{
    DImgLoaderPrms set;

    // NOTE: if quality = 100 --> lossless compression.

    set.insert(QLatin1String("quality"),  d->AVIFLossLess->isChecked() ? 100 : d->AVIFcompression->value());
    set.insert(QLatin1String("lossless"), d->AVIFLossLess->isChecked());

    return set;
}

void DImgAVIFExportSettings::slotToggleAVIFLossLess(bool b)
{
    d->AVIFcompression->setEnabled(!b);
    d->labelAVIFcompression->setEnabled(!b);
}

} // namespace Digikam

#include "moc_dimgavifexportsettings.cpp"
