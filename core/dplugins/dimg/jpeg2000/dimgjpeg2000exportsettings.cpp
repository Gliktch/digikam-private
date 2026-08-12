/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : JPEG-2000 image export settings widget.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgjpeg2000exportsettings.h"

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

class Q_DECL_HIDDEN DImgJPEG2000ExportSettings::Private
{

public:

    Private() = default;

    QGridLayout*  JPEG2000Grid              = nullptr;

    QLabel*       labelJPEG2000compression  = nullptr;

    QCheckBox*    JPEG2000LossLess          = nullptr;

    DIntNumInput* JPEG2000compression       = nullptr;
};

DImgJPEG2000ExportSettings::DImgJPEG2000ExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->JPEG2000Grid     = new QGridLayout(this);
    d->JPEG2000LossLess = new QCheckBox(i18n("Lossless JPEG 2000 files"), this);

    d->JPEG2000LossLess->setWhatsThis(i18n("<p>Toggle lossless compression for JPEG 2000 images.</p>"
                                           "<p>If this option is enabled, a lossless method will be used "
                                           "to compress JPEG 2000 pictures.</p>"));

    d->JPEG2000compression      = new DIntNumInput(this);
    d->JPEG2000compression->setDefaultValue(75);
    d->JPEG2000compression->setRange(1, 100, 1);
    d->labelJPEG2000compression = new QLabel(i18n("JPEG 2000 quality:"), this);

    d->JPEG2000compression->setWhatsThis(i18n("<p>The quality value for JPEG 2000 images:</p>"
                                              "<p><b>1</b>: low quality (high compression and small "
                                              "file size)<br>"
                                              "<b>50</b>: medium quality<br>"
                                              "<b>75</b>: good quality (default)<br>"
                                              "<b>100</b>: high quality (no compression and "
                                              "large file size)</p>"
                                              "<p><b>Note: JPEG 2000 is not a lossless image "
                                              "compression format when you use this setting.</b></p>"));

    d->JPEG2000Grid->addWidget(d->JPEG2000LossLess,         0, 0, 1, 2);
    d->JPEG2000Grid->addWidget(d->labelJPEG2000compression, 1, 0, 1, 2);
    d->JPEG2000Grid->addWidget(d->JPEG2000compression,      2, 0, 1, 2);
    d->JPEG2000Grid->setColumnStretch(1, 10);
    d->JPEG2000Grid->setRowStretch(3, 10);
    d->JPEG2000Grid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->JPEG2000Grid->setSpacing(spacing);

    connect(d->JPEG2000LossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleJPEG2000LossLess(bool)));

    connect(d->JPEG2000LossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->JPEG2000compression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgJPEG2000ExportSettings::~DImgJPEG2000ExportSettings()
{
    delete d;
}

void DImgJPEG2000ExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if      (it.key() == QLatin1String("quality"))
        {
            d->JPEG2000compression->setValue(it.value().toInt());
        }
        else if (it.key() == QLatin1String("lossless"))
        {
            d->JPEG2000LossLess->setChecked(it.value().toBool());
        }
    }

    slotToggleJPEG2000LossLess(d->JPEG2000LossLess->isChecked());
}

DImgLoaderPrms DImgJPEG2000ExportSettings::settings() const
{
    DImgLoaderPrms set;
    set.insert(QLatin1String("quality"),  d->JPEG2000compression->value());
    set.insert(QLatin1String("lossless"), d->JPEG2000LossLess->isChecked());

    return set;
}

void DImgJPEG2000ExportSettings::slotToggleJPEG2000LossLess(bool b)
{
    d->JPEG2000compression->setEnabled(!b);
    d->labelJPEG2000compression->setEnabled(!b);
}

} // namespace Digikam

#include "moc_dimgjpeg2000exportsettings.cpp"
