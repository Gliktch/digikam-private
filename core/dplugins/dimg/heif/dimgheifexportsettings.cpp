/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-10-02
 * Description : HEIF image export settings widget.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgheifexportsettings.h"

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

class Q_DECL_HIDDEN DImgHEIFExportSettings::Private
{

public:

    Private() = default;

    QGridLayout*  HEIFGrid              = nullptr;

    QLabel*       labelHEIFcompression  = nullptr;

    QCheckBox*    HEIFLossLess          = nullptr;

    DIntNumInput* HEIFcompression       = nullptr;
};

DImgHEIFExportSettings::DImgHEIFExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->HEIFGrid     = new QGridLayout(this);
    d->HEIFLossLess = new QCheckBox(i18n("Lossless HEIF files"), this);

    d->HEIFLossLess->setWhatsThis(i18n("<p>Toggle lossless compression for HEIF images.</p>"
                                       "<p>If this option is enabled, a lossless method will be used "
                                       "to compress HEIF pictures.</p>"));

    d->HEIFcompression = new DIntNumInput(this);
    d->HEIFcompression->setDefaultValue(75);
    d->HEIFcompression->setRange(1, 100, 1);
    d->labelHEIFcompression = new QLabel(i18n("HEIF quality:"), this);

    d->HEIFcompression->setWhatsThis(i18n("<p>The quality value for HEIF images:</p>"
                                          "<p><b>1</b>: low quality (high compression and small "
                                          "file size)<br>"
                                          "<b>50</b>: medium quality<br>"
                                          "<b>75</b>: good quality (default)<br>"
                                          "<b>100</b>: high quality (no compression and "
                                          "large file size)</p>"
                                          "<p><b>Note: HEIF is not a lossless image "
                                          "compression format when you use this setting.</b></p>"));

    d->HEIFGrid->addWidget(d->HEIFLossLess,         0, 0, 1, 2);
    d->HEIFGrid->addWidget(d->labelHEIFcompression, 1, 0, 1, 2);
    d->HEIFGrid->addWidget(d->HEIFcompression,      2, 0, 1, 2);
    d->HEIFGrid->setColumnStretch(1, 10);
    d->HEIFGrid->setRowStretch(3, 10);
    d->HEIFGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->HEIFGrid->setSpacing(spacing);

    connect(d->HEIFLossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleHEIFLossLess(bool)));

    connect(d->HEIFLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->HEIFcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgHEIFExportSettings::~DImgHEIFExportSettings()
{
    delete d;
}

void DImgHEIFExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if      (it.key() == QLatin1String("quality"))
        {
            d->HEIFcompression->setValue(it.value().toInt());
        }
        else if (it.key() == QLatin1String("lossless"))
        {
            d->HEIFLossLess->setChecked(it.value().toBool());
        }
    }

    slotToggleHEIFLossLess(d->HEIFLossLess->isChecked());
}

DImgLoaderPrms DImgHEIFExportSettings::settings() const
{
    DImgLoaderPrms set;
    set.insert(QLatin1String("quality"),  d->HEIFcompression->value());
    set.insert(QLatin1String("lossless"), d->HEIFLossLess->isChecked());

    return set;
}

void DImgHEIFExportSettings::slotToggleHEIFLossLess(bool b)
{
    d->HEIFcompression->setEnabled(!b);
    d->labelHEIFcompression->setEnabled(!b);
}

} // namespace Digikam

#include "moc_dimgheifexportsettings.cpp"
