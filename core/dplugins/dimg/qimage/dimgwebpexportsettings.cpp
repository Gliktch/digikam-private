/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-04
 * Description : WEBP image export settings widget.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgwebpexportsettings.h"

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

class Q_DECL_HIDDEN DImgWEBPExportSettings::Private
{
public:

    Private() = default;

public:

    QGridLayout*  WEBPGrid              = nullptr;

    QLabel*       labelWEBPcompression  = nullptr;

    QCheckBox*    WEBPLossLess          = nullptr;

    DIntNumInput* WEBPcompression       = nullptr;
};

DImgWEBPExportSettings::DImgWEBPExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->WEBPGrid     = new QGridLayout(this);
    d->WEBPLossLess = new QCheckBox(i18n("Lossless WEBP files"), this);

    d->WEBPLossLess->setWhatsThis(i18n("<p>Toggle lossless compression for WEBP images.</p>"
                                       "<p>If this option is enabled, a lossless method will be used "
                                       "to compress WEBP pictures.</p>"));

    d->WEBPcompression      = new DIntNumInput(this);
    d->WEBPcompression->setDefaultValue(75);
    d->WEBPcompression->setRange(1, 99, 1);
    d->labelWEBPcompression = new QLabel(i18n("WEBP quality:"), this);

    d->WEBPcompression->setWhatsThis(i18n("<p>The quality value for WEBP images:</p>"
                                          "<p><b>1</b>: low quality (high compression and small "
                                          "file size)<br>"
                                          "<b>50</b>: medium quality<br>"
                                          "<b>75</b>: good quality (default)<br>"
                                          "<b>99</b>: high quality (no compression and "
                                          "large file size)</p>"
                                          "<p><b>Note: WEBP is not a lossless image "
                                          "compression format when you use this setting.</b></p>"));

    d->WEBPGrid->addWidget(d->WEBPLossLess,         0, 0, 1, 2);
    d->WEBPGrid->addWidget(d->labelWEBPcompression, 1, 0, 1, 2);
    d->WEBPGrid->addWidget(d->WEBPcompression,      2, 0, 1, 2);
    d->WEBPGrid->setColumnStretch(1, 10);
    d->WEBPGrid->setRowStretch(3, 10);
    d->WEBPGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->WEBPGrid->setSpacing(spacing);

    connect(d->WEBPLossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleWEBPLossLess(bool)));

    connect(d->WEBPLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->WEBPcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgWEBPExportSettings::~DImgWEBPExportSettings()
{
    delete d;
}

void DImgWEBPExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if      (it.key() == QLatin1String("quality"))
        {
            d->WEBPcompression->setValue(it.value().toInt());
        }
        else if (it.key() == QLatin1String("lossless"))
        {
            d->WEBPLossLess->setChecked(it.value().toBool());
        }
    }

    slotToggleWEBPLossLess(d->WEBPLossLess->isChecked());
}

DImgLoaderPrms DImgWEBPExportSettings::settings() const
{
    DImgLoaderPrms set;

    // NOTE: if quality = 100 --> lossless compression.

    set.insert(QLatin1String("quality"),  d->WEBPLossLess->isChecked() ? 100 : d->WEBPcompression->value());
    set.insert(QLatin1String("lossless"), d->WEBPLossLess->isChecked());

    return set;
}

void DImgWEBPExportSettings::slotToggleWEBPLossLess(bool b)
{
    d->WEBPcompression->setEnabled(!b);
    d->labelWEBPcompression->setEnabled(!b);
}

} // namespace Digikam

#include "moc_dimgwebpexportsettings.cpp"
