/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-06
 * Description : PGF image export settings widget.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpgfexportsettings.h"

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

class Q_DECL_HIDDEN DImgPGFExportSettings::Private
{

public:

    Private() = default;

public:

    QGridLayout*  PGFGrid               = nullptr;

    QLabel*       labelPGFcompression   = nullptr;

    QCheckBox*    PGFLossLess           = nullptr;

    DIntNumInput* PGFcompression        = nullptr;
};

DImgPGFExportSettings::DImgPGFExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->PGFGrid        = new QGridLayout(this);
    d->PGFLossLess    = new QCheckBox(i18n("Lossless PGF files"), this);

    d->PGFLossLess->setWhatsThis(i18n("<p>Toggle lossless compression for PGF images.</p>"
                                      "<p>If this option is enabled, a lossless method will be used "
                                      "to compress PGF pictures.</p>"));

    d->PGFcompression = new DIntNumInput(this);
    d->PGFcompression->setDefaultValue(3);
    d->PGFcompression->setRange(1, 9, 1);
    d->labelPGFcompression = new QLabel(i18n("PGF quality:"), this);

    d->PGFcompression->setWhatsThis(i18n("<p>The quality value for PGF images:</p>"
                                         "<p><b>1</b>: high quality (no compression and "
                                         "large file size)<br>"
                                         "<b>3</b>: good quality (default)<br>"
                                         "<b>6</b>: medium quality<br>"
                                         "<b>9</b>: low quality (high compression and small "
                                         "file size)</p>"
                                         "<p><b>Note: PGF is not a lossless image "
                                         "compression format when you use this setting.</b></p>"));

    d->PGFGrid->addWidget(d->PGFLossLess,         0, 0, 1, 2);
    d->PGFGrid->addWidget(d->labelPGFcompression, 1, 0, 1, 2);
    d->PGFGrid->addWidget(d->PGFcompression,      2, 0, 1, 2);
    d->PGFGrid->setColumnStretch(1, 10);
    d->PGFGrid->setRowStretch(3, 10);
    d->PGFGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->PGFGrid->setSpacing(spacing);

    connect(d->PGFLossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotTogglePGFLossLess(bool)));

    connect(d->PGFLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->PGFcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgPGFExportSettings::~DImgPGFExportSettings()
{
    delete d;
}

void DImgPGFExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if      (it.key() == QLatin1String("quality"))
        {
            d->PGFcompression->setValue(it.value().toInt());
        }
        else if (it.key() == QLatin1String("lossless"))
        {
            d->PGFLossLess->setChecked(it.value().toBool());
        }
    }

    slotTogglePGFLossLess(d->PGFLossLess->isChecked());
}

DImgLoaderPrms DImgPGFExportSettings::settings() const
{
    DImgLoaderPrms set;
    set.insert(QLatin1String("quality"),  d->PGFcompression->value());
    set.insert(QLatin1String("lossless"), d->PGFLossLess->isChecked());

    return set;
}

void DImgPGFExportSettings::slotTogglePGFLossLess(bool b)
{
    d->PGFcompression->setEnabled(!b);
    d->labelPGFcompression->setEnabled(!b);
}

} // namespace Digikam

#include "moc_dimgpgfexportsettings.cpp"
