/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : PNG image export settings widget.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgpngexportsettings.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dnuminput.h"

namespace Digikam
{

class Q_DECL_HIDDEN DImgPNGExportSettings::Private
{

public:

    Private() = default;

    QGridLayout*  PNGGrid               = nullptr;

    QLabel*       labelPNGcompression   = nullptr;

    DIntNumInput* PNGcompression        = nullptr;
};

DImgPNGExportSettings::DImgPNGExportSettings(QWidget* parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->PNGGrid             = new QGridLayout(this);
    d->PNGcompression      = new DIntNumInput(this);
    d->PNGcompression->setDefaultValue(6);
    d->PNGcompression->setRange(1, 9, 1);
    d->labelPNGcompression = new QLabel(i18n("PNG compression:"), this);

    d->PNGcompression->setWhatsThis(i18n("<p>The compression value for PNG images:</p>"
                                         "<p><b>1</b>: low compression (large file size but "
                                         "short compression duration - default)<br>"
                                         "<b>5</b>: medium compression<br>"
                                         "<b>9</b>: high compression (small file size but "
                                         "long compression duration)</p>"
                                         "<p><b>Note: PNG is always a lossless image "
                                         "compression format.</b></p>"));

    d->PNGGrid->addWidget(d->labelPNGcompression, 0, 0, 1, 2);
    d->PNGGrid->addWidget(d->PNGcompression,      1, 1, 1, 2);
    d->PNGGrid->setColumnStretch(1, 10);
    d->PNGGrid->setRowStretch(2, 10);
    d->PNGGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->PNGGrid->setSpacing(spacing);

    connect(d->PNGcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgPNGExportSettings::~DImgPNGExportSettings()
{
    delete d;
}

void DImgPNGExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if (it.key() == QLatin1String("quality"))
        {
            d->PNGcompression->setValue(it.value().toInt());
        }
    }
}

DImgLoaderPrms DImgPNGExportSettings::settings() const
{
    DImgLoaderPrms set;
    set.insert(QLatin1String("quality"),  d->PNGcompression->value());

    return set;
}

} // namespace Digikam

#include "moc_dimgpngexportsettings.cpp"
