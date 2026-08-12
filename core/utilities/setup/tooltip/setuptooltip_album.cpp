/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setuptooltip_p.h"

namespace Digikam
{

void SetupToolTip::setupAlbum()
{
    DVBox* const vbox2        = new DVBox(d->panel);
    d->showAlbumToolTipsBox   = new QCheckBox(i18n("Show album items' tool-tips"), vbox2);
    d->albumSettingBox        = new QGroupBox(i18n("Album Information"), vbox2);

    d->showAlbumToolTipsBox->setWhatsThis(i18n("Set this option to display album information when "
                                               "the mouse hovers over a folder-view item."));

    d->showAlbumTitleBox      = new QCheckBox(i18n("Album name"));
    d->showAlbumTitleBox->setWhatsThis(i18n("Set this option to display the album name."));

    d->showAlbumDateBox       = new QCheckBox(i18n("Album date"));
    d->showAlbumDateBox->setWhatsThis(i18n("Set this option to display the album date."));

    d->showAlbumCollectionBox = new QCheckBox(i18n("Album collection"));
    d->showAlbumCollectionBox->setWhatsThis(i18n("Set this option to display the album collection."));

    d->showAlbumCategoryBox   = new QCheckBox(i18n("Album category"));
    d->showAlbumCategoryBox->setWhatsThis(i18n("Set this option to display the album category."));

    d->showAlbumCaptionBox    = new QCheckBox(i18n("Album caption"));
    d->showAlbumCaptionBox->setWhatsThis(i18n("Set this option to display the album caption."));

    d->showAlbumPreviewBox    = new QCheckBox(i18n("Album preview"));
    d->showAlbumPreviewBox->setWhatsThis(i18n("Set this option to display the album preview."));

    QGridLayout* const albumSettingBoxLayout = new QGridLayout;
    albumSettingBoxLayout->addWidget(d->showAlbumTitleBox,      0, 0, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumDateBox,       1, 0, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumCollectionBox, 2, 0, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumCategoryBox,   0, 1, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumCaptionBox,    1, 1, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumPreviewBox,    2, 1, 1, 1);
    d->albumSettingBox->setLayout(albumSettingBoxLayout);

    QWidget* const space2 = new QWidget(vbox2);
    vbox2->setStretchFactor(space2, 10);
    vbox2->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    vbox2->setSpacing(d->spacing);

    d->tab->insertTab(AlbumItems,  vbox2, i18n("Album Items"));

    connect(d->showAlbumToolTipsBox, SIGNAL(toggled(bool)),
            d->albumSettingBox, SLOT(setEnabled(bool)));

}

} // namespace Digikam
