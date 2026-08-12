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

void SetupToolTip::setupImport()
{
    DVBox* const vbox3       = new DVBox(d->panel);
    d->showImportToolTipsBox = new QCheckBox(i18n("Show import items' tool-tips"), vbox3);
    d->importSettingBox      = new QGroupBox(i18n("Import Information"), vbox3);

    d->showAlbumToolTipsBox->setWhatsThis(i18n("Set this option to display album information when "
                                               "the mouse hovers over a folder-view item."));

    d->showItemTitleBox      = new QCheckBox(i18n("Item name"));
    d->showItemTitleBox->setWhatsThis(i18n("Set this option to display the item name."));

    d->showItemDateBox       = new QCheckBox(i18n("Item date"));
    d->showItemDateBox->setWhatsThis(i18n("Set this option to display the item date."));

    d->showItemSizeBox       = new QCheckBox(i18n("Item size"));
    d->showItemSizeBox->setWhatsThis(i18n("Set this option to display the item size."));

    d->showItemTypeBox       = new QCheckBox(i18n("Item type"));
    d->showItemTypeBox->setWhatsThis(i18n("Set this option to display the item type."));

    d->showItemDimensionsBox = new QCheckBox(i18n("Item dimensions"));
    d->showItemDimensionsBox->setWhatsThis(i18n("Set this option to display the item dimensions."));

    DLineWidget* const line  = new DLineWidget(Qt::Horizontal, d->photoSettingBox);
    QLabel* const label      = new QLabel(i18n("Note: these settings require \"Use File Metadata\" option "
                                               "from Camera Setup Behavior page."), d->photoSettingBox);

    d->showItemPhotoMakeBox  = new QCheckBox(i18n("Camera make and model"), d->photoSettingBox);
    d->showItemPhotoMakeBox->setWhatsThis(i18n("Set this option to display the make and model of the "
                                               "camera with which the image has been taken."));

    d->showItemPhotoLensBox  = new QCheckBox(i18n("Camera lens model"), d->photoSettingBox);
    d->showItemPhotoLensBox->setWhatsThis(i18n("Set this option to display the lens model with "
                                               "which the image was taken."));

    d->showItemPhotoFocalBox = new QCheckBox(i18n("Camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis(i18n("Set this option to display the camera aperture and focal settings "
                                            "used to take the image."));

    d->showItemPhotoExpoBox  = new QCheckBox(i18n("Camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis(i18n("Set this option to display the camera exposure and sensitivity "
                                           "used to take the image."));

    d->showItemPhotoFlashBox = new QCheckBox(i18n("Camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis(i18n("Set this option to display the camera flash settings "
                                            "used to take the image."));

    d->showItemPhotoWBBox    = new QCheckBox(i18n("Camera white balance settings"), d->photoSettingBox);
    d->showItemPhotoWBBox->setWhatsThis(i18n("Set this option to display the camera white balance settings "
                                             "used to take the image."));

    QGridLayout* const importSettingBoxLayout = new QGridLayout;
    importSettingBoxLayout->addWidget(d->showItemTitleBox,      0, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemDateBox,       1, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemSizeBox,       2, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemTypeBox,       0, 1, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemDimensionsBox, 1, 1, 1, 1);
    importSettingBoxLayout->addWidget(line,                     3, 0, 1, 2);
    importSettingBoxLayout->addWidget(label,                    4, 0, 1, 2);
    importSettingBoxLayout->addWidget(d->showItemPhotoMakeBox,  5, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoLensBox,  6, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoFocalBox, 7, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoExpoBox,  5, 1, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoFlashBox, 6, 1, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoWBBox,    7, 1, 1, 1);
    d->importSettingBox->setLayout(importSettingBoxLayout);

    QWidget* const space3 = new QWidget(vbox3);
    vbox3->setStretchFactor(space3, 10);
    vbox3->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    vbox3->setSpacing(d->spacing);

    d->tab->insertTab(ImportItems, vbox3, i18n("Import Items"));

    connect(d->showImportToolTipsBox, SIGNAL(toggled(bool)),
            this, SLOT(slotImportToolTipsChanged()));
}

} // namespace Digikam
