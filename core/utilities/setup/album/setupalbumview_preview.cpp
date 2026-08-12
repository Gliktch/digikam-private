/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupalbumview_p.h"

namespace Digikam
{

void SetupAlbumView::setupPreView()
{
    QWidget* const pwpanel        = new QWidget(d->tab);
    QGridLayout* const grid3      = new QGridLayout(pwpanel);

    d->previewFastPreview         = new QRadioButton(i18nc("@option:radio",
                                                           "Preview shows embedded view if available (faster)"));
    d->previewFullView            = new QRadioButton(i18nc("@option:radio",
                                                           "Preview shows the full image"));
    QLabel* const rawPreviewLabel = new QLabel(i18nc("@label:listbox Mode of RAW preview decoding",
                                                     "Raw images:"));
    d->previewRawMode             = new QComboBox;
    d->previewRawMode->addItem(i18nc("@item Automatic choice of RAW image preview source",
                                     "Automatic"), PreviewSettings::RawPreviewAutomatic);
    d->previewRawMode->addItem(i18nc("@item Embedded preview as RAW image preview source",
                                     "Embedded preview"), PreviewSettings::RawPreviewFromEmbeddedPreview);
    d->previewRawMode->addItem(i18nc("@item Original, half-size data as RAW image preview source",
                                     "Raw data in half size"), PreviewSettings::RawPreviewFromRawHalfSize);
    d->previewRawMode->addItem(i18nc("@item Original, full-size data as RAW image preview source",
                                     "Raw data in full size (high quality)"), PreviewSettings::RawPreviewFromRawFullSize);

    d->previewConvertToEightBit   = new QCheckBox(i18n("Preview image is converted to 8 bits for a faster viewing"), pwpanel);
    d->previewConvertToEightBit->setWhatsThis(i18n("Check this if you want to convert a 16 bits preview image to 8 bits."));

    d->previewScaleFitToWindow    = new QCheckBox(i18n("Preview image is always scaled to fit to window"), pwpanel);
    d->previewScaleFitToWindow->setWhatsThis(i18n("Check this if you want to scale small images to fit to window."));

    d->previewSmoothScaled        = new QCheckBox(i18n("Preview image use Anti-aliasing to scale up"), pwpanel);
    d->previewSmoothScaled->setWhatsThis(i18n("Check this if you want to use Anti-aliasing to scale up."));

    // ---

    QGroupBox* const magnifierBox = new QGroupBox(i18n("Magnifier"), pwpanel);
    QGridLayout* const mgrid      = new QGridLayout(magnifierBox);

    QLabel* const magnifierLabel  = new QLabel(i18nc("@label:listbox Preview magnifier tool zoom factor",
                                                     "Zoom Factor:"), magnifierBox);
    d->magnifierZoomFactor        = new QComboBox(magnifierBox);

    const QList<qreal> zooms      = MagnifierItem::zoomFactors();

    for (qreal z : zooms)
    {
        d->magnifierZoomFactor->addItem(QString::fromLatin1("x%1").arg(z), z);
    }

    QLabel* const magSizeLabel    = new QLabel(i18nc("@label:listbox Preview magnifier tool size",
                                                     "Tool Size:"), magnifierBox);
    d->magnifierSize              = new QComboBox(magnifierBox);
    d->magnifierSize->addItem(i18nc("@item Magnifier tool size", "Small"),      150);
    d->magnifierSize->addItem(i18nc("@item Magnifier tool size", "Medium"),     300);
    d->magnifierSize->addItem(i18nc("@item Magnifier tool size", "Large"),      450);
    d->magnifierSize->addItem(i18nc("@item Magnifier tool size", "Very Large"), 600);

    mgrid->addWidget(magnifierLabel,         0, 0, 1, 1);
    mgrid->addWidget(d->magnifierZoomFactor, 0, 2, 1, 1);
    mgrid->addWidget(magSizeLabel,           1, 0, 1, 1);
    mgrid->addWidget(d->magnifierSize,       1, 2, 1, 1);
    mgrid->setColumnStretch(1, 10);
    mgrid->setSpacing(d->spacing);

    // ---

    d->previewShowIcons           = new QCheckBox(i18n("Show icons and text over preview"), pwpanel);
    d->previewShowIcons->setWhatsThis(i18n("Check this if you want to see icons and text in the image preview."));

    d->previewAutoPlay            = new QCheckBox(i18n("Auto play of videos or animations"), pwpanel);
    d->previewAutoPlay->setWhatsThis(i18n("Check this if you want to auto play the videos or animations."));

    d->previewOverlay             = new QCheckBox(i18n("Show a text overlay over preview."), pwpanel);
    d->previewOverlay->setWhatsThis(i18n("Check this if you want to show a canvas overlay with technical shot settings and properties."));

    d->osdWidget                  = new PreviewOsdWidget(&d->osdSettings, pwpanel);

    grid3->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid3->setSpacing(d->spacing);
    grid3->addWidget(d->previewFastPreview,       0, 0, 1, 2);
    grid3->addWidget(d->previewFullView,          1, 0, 1, 2);
    grid3->addWidget(rawPreviewLabel,             2, 0, 1, 1);
    grid3->addWidget(d->previewRawMode,           2, 1, 1, 1);
    grid3->addWidget(d->previewConvertToEightBit, 3, 0, 1, 2);
    grid3->addWidget(d->previewScaleFitToWindow,  4, 0, 1, 2);
    grid3->addWidget(d->previewSmoothScaled,      5, 0, 1, 2);
    grid3->addWidget(magnifierBox,                6, 0, 1, 2);
    grid3->addWidget(d->previewShowIcons,         7, 0, 1, 2);
    grid3->addWidget(d->previewAutoPlay,          8, 0, 1, 2);
    grid3->addWidget(d->previewOverlay,           9, 0, 1, 2);
    grid3->addWidget(d->osdWidget,               10, 0, 1, 2);
    grid3->setRowStretch(11, 10);

    d->previewFastPreview->setChecked(true);
    d->previewRawMode->setCurrentIndex(0);
    d->previewRawMode->setEnabled(false);

    d->tab->insertTab(Preview, pwpanel, i18nc("@title:tab", "Preview"));

    // --------------------------------------------------------

    connect(d->previewFullView, SIGNAL(toggled(bool)),
            d->previewRawMode, SLOT(setEnabled(bool)));

    connect(d->previewOverlay, SIGNAL(toggled(bool)),
            d->osdWidget, SLOT(setEnabled(bool)));
}

} // namespace Digikam
