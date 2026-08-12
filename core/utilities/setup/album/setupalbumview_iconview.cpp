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

void SetupAlbumView::setupIconView()
{
    QWidget* const iwpanel     = new QWidget(d->tab);

    QGridLayout* const grid    = new QGridLayout(iwpanel);

    d->iconShowNameBox         = new QCheckBox(i18n("Show file&name"), iwpanel);
    d->iconShowNameBox->setWhatsThis(i18n("Set this option to show the filename below the thumbnail."));

    d->iconShowSizeBox         = new QCheckBox(i18n("Show file si&ze"), iwpanel);
    d->iconShowSizeBox->setWhatsThis(i18n("Set this option to show the file size below the thumbnail."));

    d->iconShowDateBox         = new QCheckBox(i18n("Show camera creation &date"), iwpanel);
    d->iconShowDateBox->setWhatsThis(i18n("Set this option to show the camera creation date "
                                          "below the thumbnail."));

    d->iconShowModDateBox      = new QCheckBox(i18n("Show file &modification date"), iwpanel);
    d->iconShowModDateBox->setWhatsThis(i18n("Set this option to show the file modification date "
                                             "below the thumbnail if it is different than camera creation date. "
                                             "This option is useful to identify quickly which items have been modified."));

    d->iconShowResolutionBox   = new QCheckBox(i18n("Show item dimensions"), iwpanel);
    d->iconShowResolutionBox->setWhatsThis(i18n("Set this option to show the item size in pixels "
                                                "below the thumbnail."));

    d->iconShowAspectRatioBox  = new QCheckBox(i18n("Show item aspect ratio"), iwpanel);
    d->iconShowAspectRatioBox->setWhatsThis(i18n("Set this option to show the item aspect ratio "
                                                "below the thumbnail."));

    d->iconShowFormatBox       = new QCheckBox(i18n("Show item format"), iwpanel);
    d->iconShowFormatBox->setWhatsThis(i18n("Set this option to show item mime type over thumbnail."));

    d->iconShowTitleBox        = new QCheckBox(i18n("Show digiKam tit&le"), iwpanel);
    d->iconShowTitleBox->setWhatsThis(i18n("Set this option to show the digiKam title "
                                           "below the thumbnail."));

    d->iconShowCommentsBox     = new QCheckBox(i18n("Show digiKam &captions"), iwpanel);
    d->iconShowCommentsBox->setWhatsThis(i18n("Set this option to show the digiKam captions "
                                              "below the thumbnail."));

    d->iconShowTagsBox         = new QCheckBox(i18n("Show digiKam &tags"), iwpanel);
    d->iconShowTagsBox->setWhatsThis(i18n("Set this option to show the digiKam tags list "
                                          "below the thumbnail."));

    d->iconShowRatingBox       = new QCheckBox(i18n("Show digiKam &rating"), iwpanel);
    d->iconShowRatingBox->setWhatsThis(i18n("Set this option to show the digiKam rating "
                                            "below the thumbnail."));

    d->iconShowPickLabelBox    = new QCheckBox(i18n("Show digiKam &pick label"), iwpanel);
    d->iconShowPickLabelBox->setWhatsThis(i18n("Set this option to show the digiKam pick label "
                                            "below the thumbnail."));

    d->iconShowColorLabelBox   = new QCheckBox(i18n("Show digiKam color &label"), iwpanel);
    d->iconShowColorLabelBox->setWhatsThis(i18n("Set this option to show the digiKam color label "
                                                "as a line below the thumbnail."));

    DLineWidget* const line    = new DLineWidget(Qt::Horizontal, iwpanel);

    d->iconShowOverlaysBox     = new QCheckBox(i18n("Show rotation overlay buttons"), iwpanel);
    d->iconShowOverlaysBox->setWhatsThis(i18n("Set this option to show the transformation overlay buttons over "
                                              "the thumbnail to process left or right rotation."));

    d->iconShowFullscreenBox   = new QCheckBox(i18n("Show fullscreen overlay button"), iwpanel);
    d->iconShowFullscreenBox->setWhatsThis(i18n("Set this option to show an overlay button over the "
                                                "thumbnail to open it in fullscreen mode."));

    d->iconShowCoordinatesBox  = new QCheckBox(i18n("Show geolocation overlay button"), iwpanel);
    d->iconShowCoordinatesBox->setWhatsThis(i18n("Set this option to show an overlay button over the "
                                                 "thumbnail to identify geolocated item and open the map tab."));

    DLineWidget* const line2   = new DLineWidget(Qt::Horizontal, iwpanel);

    QLabel* leftClickLabel     = new QLabel(i18n("Thumbnail click action:"), iwpanel);
    d->leftClickActionComboBox = new QComboBox(iwpanel);
    d->leftClickActionComboBox->addItem(i18n("Show Preview"),                  ApplicationSettings::ShowPreview);
    d->leftClickActionComboBox->addItem(i18n("Start Image Editor"),            ApplicationSettings::StartEditor);
    d->leftClickActionComboBox->addItem(i18n("Show on Light Table"),           ApplicationSettings::ShowOnTable);
    d->leftClickActionComboBox->addItem(i18n("Open with Default Application"), ApplicationSettings::OpenDefault);
    d->leftClickActionComboBox->setToolTip(i18n("Choose what should happen when you click on a thumbnail."));

    d->iconViewFontSelect      = new DFontSelect(i18n("Icon View font:"), iwpanel);
    d->iconViewFontSelect->setToolTip(i18n("Select here the font used to display text in icon views."));

    d->largeThumbsBox          = new QCheckBox(i18n("Use large thumbnail size for high screen resolution"), iwpanel);
    d->largeThumbsBox->setWhatsThis(i18n("Set this option to render icon-view with large thumbnail size, for example in case of 4K monitor is used.\n"
                                         "By default this option is turned off and the maximum thumbnail size is limited to 256x256 pixels. "
                                         "When this option is enabled, thumbnail size can be extended to 512x512 pixels.\n"
                                         "This option will store more data in thumbnail database and will use more system memory. "
                                         "digiKam needs to be restarted to take effect, and Rebuild Thumbnails option from Maintenance tool "
                                         "needs to be processed over whole collections."));

    grid->addWidget(d->iconShowNameBox,          0,  0, 1, 1);
    grid->addWidget(d->iconShowSizeBox,          1,  0, 1, 1);
    grid->addWidget(d->iconShowDateBox,          2,  0, 1, 1);
    grid->addWidget(d->iconShowModDateBox,       3,  0, 1, 1);
    grid->addWidget(d->iconShowResolutionBox,    4,  0, 1, 1);
    grid->addWidget(d->iconShowAspectRatioBox,   5,  0, 1, 1);
    grid->addWidget(d->iconShowFormatBox,        6,  0, 1, 1);

    grid->addWidget(d->iconShowTitleBox,         0,  1, 1, 1);
    grid->addWidget(d->iconShowCommentsBox,      1,  1, 1, 1);
    grid->addWidget(d->iconShowTagsBox,          2,  1, 1, 1);
    grid->addWidget(d->iconShowRatingBox,        3,  1, 1, 1);
    grid->addWidget(d->iconShowPickLabelBox,     4,  1, 1, 1);
    grid->addWidget(d->iconShowColorLabelBox,    5,  1, 1, 1);

    grid->addWidget(line,                        7,  0, 1, 2);
    grid->addWidget(d->iconShowCoordinatesBox,   8,  0, 1, 1);
    grid->addWidget(d->iconShowFullscreenBox,    8,  1, 1, 1);
    grid->addWidget(d->iconShowOverlaysBox,      9,  0, 1, 1);
    grid->addWidget(line2,                       10, 0, 1, 2);

    grid->addWidget(leftClickLabel,              11, 0, 1, 1);
    grid->addWidget(d->leftClickActionComboBox,  11, 1, 1, 1);
    grid->addWidget(d->iconViewFontSelect,       12, 0, 1, 2);
    grid->addWidget(d->largeThumbsBox,           13, 0, 1, 2);

    grid->setRowMinimumHeight(8, 10); // vertical space
    grid->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid->setSpacing(d->spacing);
    grid->setRowStretch(14, 10);

    d->tab->insertTab(IconView, iwpanel, i18nc("@title:tab", "Icons"));

    // ---

    connect(d->largeThumbsBox, SIGNAL(toggled(bool)),
            this, SLOT(slotUseLargeThumbsToggled(bool)));
}

bool SetupAlbumView::useLargeThumbsHasChanged() const
{
    return (d->largeThumbsBox->isChecked() != d->useLargeThumbsOriginal);
}

void SetupAlbumView::slotUseLargeThumbsToggled(bool b)
{
    // Show info if large thumbs were enabled, and only once.

    if (b && d->useLargeThumbsShowedInfo && useLargeThumbsHasChanged())
    {
        d->useLargeThumbsShowedInfo = true;
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info",
                                       "This option changes the size in which thumbnails are generated. "
                                       "You need to restart digiKam for this option to take effect. "
                                       "Furthermore, you need to regenerate all already stored thumbnails via "
                                       "the <interface>Tools / Maintenance</interface> menu."));
    }
}

} // namespace Digikam
