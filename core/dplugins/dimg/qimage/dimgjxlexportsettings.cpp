/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-01
 * Description : JPEG-XL image export settings widget.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgjxlexportsettings.h"

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

class Q_DECL_HIDDEN DImgJXLExportSettings::Private
{

public:

    Private() = default;

    QGridLayout*  JXLGrid               = nullptr;

    QLabel*       labelJXLcompression   = nullptr;

    QCheckBox*    JXLLossLess           = nullptr;

    DIntNumInput* JXLcompression        = nullptr;
};

DImgJXLExportSettings::DImgJXLExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->JXLGrid     = new QGridLayout(this);
    d->JXLLossLess = new QCheckBox(i18n("Lossless JPEG XL files"), this);

    d->JXLLossLess->setWhatsThis(i18n("<p>Toggle lossless compression for JPEG XL images.</p>"
                                      "<p>If this option is enabled, a lossless method will be used "
                                      "to compress JPEG XL pictures.</p>"));

    d->JXLcompression = new DIntNumInput(this);
    d->JXLcompression->setDefaultValue(75);
    d->JXLcompression->setRange(1, 99, 1);
    d->labelJXLcompression = new QLabel(i18n("JPEG XL quality:"), this);

    d->JXLcompression->setWhatsThis(i18n("<p>The quality value for JPEG XL images:</p>"
                                         "<p><b>1</b>: low quality (high compression and small "
                                         "file size)<br>"
                                         "<b>50</b>: medium quality<br>"
                                         "<b>75</b>: good quality (default)<br>"
                                         "<b>99</b>: high quality (no compression and "
                                         "large file size)</p>"
                                         "<p><b>Note: JPEG XL is not a lossless image "
                                         "compression format when you use this setting.</b></p>"));

    d->JXLGrid->addWidget(d->JXLLossLess,         0, 0, 1, 2);
    d->JXLGrid->addWidget(d->labelJXLcompression, 1, 0, 1, 2);
    d->JXLGrid->addWidget(d->JXLcompression,      2, 0, 1, 2);
    d->JXLGrid->setColumnStretch(1, 10);
    d->JXLGrid->setRowStretch(3, 10);
    d->JXLGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->JXLGrid->setSpacing(spacing);

    connect(d->JXLLossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleJXLLossLess(bool)));

    connect(d->JXLLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->JXLcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgJXLExportSettings::~DImgJXLExportSettings()
{
    delete d;
}

void DImgJXLExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if      (it.key() == QLatin1String("quality"))
        {
            d->JXLcompression->setValue(it.value().toInt());
        }
        else if (it.key() == QLatin1String("lossless"))
        {
            d->JXLLossLess->setChecked(it.value().toBool());
        }
    }

    slotToggleJXLLossLess(d->JXLLossLess->isChecked());
}

DImgLoaderPrms DImgJXLExportSettings::settings() const
{
    DImgLoaderPrms set;

    // NOTE: if quality = 100 --> lossless compression.

    set.insert(QLatin1String("quality"),  d->JXLLossLess->isChecked() ? 100 : d->JXLcompression->value());
    set.insert(QLatin1String("lossless"), d->JXLLossLess->isChecked());

    return set;
}

void DImgJXLExportSettings::slotToggleJXLLossLess(bool b)
{
    d->JXLcompression->setEnabled(!b);
    d->labelJXLcompression->setEnabled(!b);
}

} // namespace Digikam

#include "moc_dimgjxlexportsettings.cpp"
