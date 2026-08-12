/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : TIFF image export settings widget.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgtiffexportsettings.h"

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

namespace Digikam
{

class Q_DECL_HIDDEN DImgTIFFExportSettings::Private
{

public:

    Private() = default;

    QGridLayout* TIFFGrid           = nullptr;

    QCheckBox*   TIFFcompression    = nullptr;
};

DImgTIFFExportSettings::DImgTIFFExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->TIFFGrid        = new QGridLayout(this);
    d->TIFFcompression = new QCheckBox(i18n("Compress TIFF files"), this);

    d->TIFFcompression->setWhatsThis(i18n("<p>Toggle compression for TIFF images.</p>"
                                          "<p>If this option is enabled, the final size "
                                          "of the TIFF image is reduced.</p>"
                                          "<p>A lossless compression format (Deflate) "
                                          "is used to save the file.</p>"));

    d->TIFFGrid->addWidget(d->TIFFcompression, 0, 0, 1, 2);
    d->TIFFGrid->setColumnStretch(1, 10);
    d->TIFFGrid->setRowStretch(1, 10);
    d->TIFFGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->TIFFGrid->setSpacing(spacing);

    connect(d->TIFFcompression, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgTIFFExportSettings::~DImgTIFFExportSettings()
{
    delete d;
}

void DImgTIFFExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if (it.key() == QLatin1String("compress"))
        {
            d->TIFFcompression->setChecked(it.value().toBool());
        }
    }
}

DImgLoaderPrms DImgTIFFExportSettings::settings() const
{
    DImgLoaderPrms set;
    set.insert(QLatin1String("compress"),  d->TIFFcompression->isChecked());

    return set;
}

} // namespace Digikam

#include "moc_dimgtiffexportsettings.cpp"
