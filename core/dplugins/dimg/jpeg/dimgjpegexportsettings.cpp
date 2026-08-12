/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : JPEG image export settings widget.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgjpegexportsettings.h"

// Qt includes

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QString>
#include <QComboBox>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dnuminput.h"

namespace Digikam
{

class Q_DECL_HIDDEN DImgJPEGExportSettings::Private
{

public:

    Private() = default;

    QGridLayout*  JPEGGrid              = nullptr;

    QLabel*       labelJPEGcompression  = nullptr;
    QLabel*       labelWarning          = nullptr;
    QLabel*       labelSubSampling      = nullptr;

    QComboBox*    subSamplingCB         = nullptr;

    DIntNumInput* JPEGcompression       = nullptr;
};

DImgJPEGExportSettings::DImgJPEGExportSettings(QWidget* const parent)
    : DImgLoaderSettings(parent),
      d                 (new Private)
{
    const int spacing = layoutSpacing();

    d->JPEGGrid             = new QGridLayout(this);
    d->JPEGcompression      = new DIntNumInput(this);
    d->JPEGcompression->setDefaultValue(75);
    d->JPEGcompression->setRange(1, 100, 1);
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), this);

    d->JPEGcompression->setWhatsThis(i18n("<p>The JPEG quality:</p>"
                                          "<p><b>1</b>: low quality (high compression and small "
                                          "file size)<br>"
                                          "<b>50</b>: medium quality<br>"
                                          "<b>75</b>: good quality (default)<br>"
                                          "<b>100</b>: high quality (no compression and "
                                          "large file size)</p>"
                                          "<p><b>Note: JPEG always uses lossy compression.</b></p>"));

    d->labelWarning = new QLabel(i18n("<font color='red'><i>"
                                      "Warning: <a href='https://en.wikipedia.org/wiki/JPEG'>JPEG</a> is a "
                                      "lossy image compression format."
                                      "</i></font>"), this);

    d->labelWarning->setOpenExternalLinks(true);
    d->labelWarning->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->labelWarning->setLineWidth(1);
    d->labelWarning->setFrameShape(QFrame::Box);

    d->labelSubSampling = new QLabel(i18n("Chroma subsampling:"), this);

    d->subSamplingCB = new QComboBox(this);
    d->subSamplingCB->insertItem(0, i18n("4:4:4 (best quality)")); // 1x1, 1x1, 1x1 (4:4:4)
    d->subSamplingCB->insertItem(1, i18n("4:2:2 (good quality)")); // 2x1, 1x1, 1x1 (4:2:2)
    d->subSamplingCB->insertItem(2, i18n("4:2:0 (low quality)"));  // 2x2, 1x1, 1x1 (4:2:0)
    d->subSamplingCB->insertItem(3, i18n("4:1:1 (low quality)"));  // 4x1, 1x1, 1x1 (4:1:1)
    d->subSamplingCB->setWhatsThis(i18n("<p>Chroma subsampling reduces file size by taking advantage of the "
                                        "eye's lesser sensitivity to color resolution. How perceptible the "
                                        "difference is depends on the image - large photos will generally "
                                        "show no difference, while sharp, down-scaled pixel graphics may "
                                        "lose fine color detail.</p>"
                                        "<p><b>4:4:4</b> - No chroma subsampling, highest "
                                        "quality but lowest compression.</p>"
                                        "<p><b>4:2:2</b> - Chroma halved horizontally, average "
                                        "compression, average quality.</p>"
                                        "<p><b>4:2:0</b> - Chroma quartered in 2x2 blocks, "
                                        "high compression but low quality.</p>"
                                        "<p><b>4:1:1</b> - Chroma quartered in 4x1 blocks, "
                                        "high compression but low quality.</p>"
                                        "<p><b>Note: JPEG always uses lossy compression.</b></p>"));

    d->JPEGGrid->addWidget(d->labelJPEGcompression, 0, 0, 1, 2);
    d->JPEGGrid->addWidget(d->JPEGcompression,      1, 0, 1, 2);
    d->JPEGGrid->addWidget(d->labelSubSampling,     2, 0, 1, 2);
    d->JPEGGrid->addWidget(d->subSamplingCB,        3, 0, 1, 2);
    d->JPEGGrid->addWidget(d->labelWarning,         4, 0, 1, 1);
    d->JPEGGrid->setColumnStretch(1, 10);
    d->JPEGGrid->setRowStretch(5, 10);
    d->JPEGGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->JPEGGrid->setSpacing(spacing);

    connect(d->JPEGcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->subSamplingCB, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DImgJPEGExportSettings::~DImgJPEGExportSettings()
{
    delete d;
}

void DImgJPEGExportSettings::setSettings(const DImgLoaderPrms& set)
{
    for (DImgLoaderPrms::const_iterator it = set.constBegin() ; it != set.constEnd() ; ++it)
    {
        if      (it.key() == QLatin1String("quality"))
        {
            d->JPEGcompression->setValue(it.value().toInt());
        }
        else if (it.key() == QLatin1String("subsampling"))
        {
            d->subSamplingCB->setCurrentIndex(it.value().toInt());
        }
    }
}

DImgLoaderPrms DImgJPEGExportSettings::settings() const
{
    DImgLoaderPrms set;
    set.insert(QLatin1String("quality"),     d->JPEGcompression->value());
    set.insert(QLatin1String("subsampling"), d->subSamplingCB->currentIndex());

    return set;
}

} // namespace Digikam

#include "moc_dimgjpegexportsettings.cpp"
