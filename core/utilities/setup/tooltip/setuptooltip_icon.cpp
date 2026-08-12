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

void SetupToolTip::setupIcon()
{
    DVBox* const vbox        = new DVBox(d->panel);

    d->showToolTipsBox       = new QCheckBox(i18n("Show icon-view and thumb-bar items' tool-tips"), vbox);
    d->showToolTipsBox->setWhatsThis(i18n("Set this option to display image information when "
                                          "the mouse hovers over an icon-view or thumb-bar item."));

    d->fileSettingBox        = new QGroupBox(i18n("File/Item Information"), vbox);
    QGridLayout* const grid2 = new QGridLayout(d->fileSettingBox);

    d->showFileNameBox       = new QCheckBox(i18n("File name"), d->fileSettingBox);
    d->showFileNameBox->setWhatsThis(i18n("Set this option to display the image file name."));

    d->showFileDateBox       = new QCheckBox(i18n("File date"), d->fileSettingBox);
    d->showFileDateBox->setWhatsThis(i18n("Set this option to display the image file date."));

    d->showFileSizeBox       = new QCheckBox(i18n("File size"), d->fileSettingBox);
    d->showFileSizeBox->setWhatsThis(i18n("Set this option to display the image file size."));

    d->showImageTypeBox      = new QCheckBox(i18n("Image type"), d->fileSettingBox);
    d->showImageTypeBox->setWhatsThis(i18n("Set this option to display the image type."));

    d->showImageDimBox       = new QCheckBox(i18n("Image dimensions"), d->fileSettingBox);
    d->showImageDimBox->setWhatsThis(i18n("Set this option to display the image dimensions in pixels."));

    d->showImageARBox        = new QCheckBox(i18n("Image aspect ratio"), d->fileSettingBox);
    d->showImageARBox->setWhatsThis(i18n("Set this option to display the image aspect ratio."));

    grid2->addWidget(d->showFileNameBox,  0, 0, 1, 1);
    grid2->addWidget(d->showFileDateBox,  1, 0, 1, 1);
    grid2->addWidget(d->showFileSizeBox,  2, 0, 1, 1);
    grid2->addWidget(d->showImageTypeBox, 0, 1, 1, 1);
    grid2->addWidget(d->showImageDimBox,  1, 1, 1, 1);
    grid2->addWidget(d->showImageARBox,   2, 1, 1, 1);
    grid2->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid2->setSpacing(0);

    // --------------------------------------------------------

    d->photoSettingBox       = new QGroupBox(i18n("Photograph Information"), vbox);
    QGridLayout* const grid3 = new QGridLayout(d->photoSettingBox);

    d->showPhotoMakeBox      = new QCheckBox(i18n("Camera make and model"), d->photoSettingBox);
    d->showPhotoMakeBox->setWhatsThis(i18n("Set this option to display the make and model of the "
                                           "camera with which the image has been taken."));

    d->showPhotoLensBox      = new QCheckBox(i18n("Camera lens model"), d->photoSettingBox);
    d->showPhotoLensBox->setWhatsThis(i18n("Set this option to display the lens model with "
                                           "which the image was taken."));

    d->showPhotoDateBox      = new QCheckBox(i18n("Camera date"), d->photoSettingBox);
    d->showPhotoDateBox->setWhatsThis(i18n("Set this option to display the date when the image was taken."));

    d->showPhotoFocalBox     = new QCheckBox(i18n("Camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis(i18n("Set this option to display the camera aperture and focal settings "
                                            "used to take the image."));

    d->showPhotoExpoBox      = new QCheckBox(i18n("Camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis(i18n("Set this option to display the camera exposure and sensitivity "
                                           "used to take the image."));

    d->showPhotoModeBox      = new QCheckBox(i18n("Camera mode and program"), d->photoSettingBox);
    d->showPhotoModeBox->setWhatsThis(i18n("Set this option to display the camera mode and program "
                                           "used to take the image."));

    d->showPhotoFlashBox     = new QCheckBox(i18n("Camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis(i18n("Set this option to display the camera flash settings "
                                            "used to take the image."));

    d->showPhotoWbBox        = new QCheckBox(i18n("Camera white balance settings"), d->photoSettingBox);
    d->showPhotoWbBox->setWhatsThis(i18n("Set this option to display the camera white balance settings "
                                         "used to take the image."));

    grid3->addWidget(d->showPhotoMakeBox,  0, 0, 1, 1);
    grid3->addWidget(d->showPhotoLensBox,  1, 0, 1, 1);
    grid3->addWidget(d->showPhotoDateBox,  2, 0, 1, 1);
    grid3->addWidget(d->showPhotoFocalBox, 3, 0, 1, 1);
    grid3->addWidget(d->showPhotoExpoBox,  0, 1, 1, 1);
    grid3->addWidget(d->showPhotoModeBox,  1, 1, 1, 1);
    grid3->addWidget(d->showPhotoFlashBox, 2, 1, 1, 1);
    grid3->addWidget(d->showPhotoWbBox,    3, 1, 1, 1);
    grid3->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid3->setSpacing(0);

    // --------------------------------------------------------

    d->digikamSettingBox     = new QGroupBox(i18n("digiKam Information"), vbox);
    QGridLayout* const grid4 = new QGridLayout(d->digikamSettingBox);

    d->showAlbumNameBox      = new QCheckBox(i18n("Album name"), d->digikamSettingBox);
    d->showAlbumNameBox->setWhatsThis(i18n("Set this option to display the album name."));

    d->showTitlesBox         = new QCheckBox(i18n("Image title"), d->digikamSettingBox);
    d->showTitlesBox->setWhatsThis(i18n("Set this option to display the image title."));

    d->showCommentsBox       = new QCheckBox(i18n("Image caption"), d->digikamSettingBox);
    d->showCommentsBox->setWhatsThis(i18n("Set this option to display the image captions."));

    d->showTagsBox           = new QCheckBox(i18n("Image tags"), d->digikamSettingBox);
    d->showTagsBox->setWhatsThis(i18n("Set this option to display the image tags."));

    d->showLabelsBox         = new QCheckBox(i18n("Image labels"), d->digikamSettingBox);
    d->showLabelsBox->setWhatsThis(i18n("Set this option to display the image pick, color, rating labels."));

    grid4->addWidget(d->showAlbumNameBox, 0, 0, 1, 1);
    grid4->addWidget(d->showTitlesBox,    1, 0, 1, 1);
    grid4->addWidget(d->showCommentsBox,  2, 0, 1, 1);
    grid4->addWidget(d->showTagsBox,      0, 1, 1, 1);
    grid4->addWidget(d->showLabelsBox,    1, 1, 1, 1);
    grid4->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid4->setSpacing(0);

    // --------------------------------------------------------

    d->videoSettingBox           = new QGroupBox(i18n("Audio/Video Information"), vbox);
    QGridLayout* const grid5     = new QGridLayout(d->videoSettingBox);

    d->showVideoAspectRatio      = new QCheckBox(i18n("Video Aspect Ratio"), d->videoSettingBox);
    d->showVideoAspectRatio->setWhatsThis(i18n("Set this option to display the Aspect Ratio of the Video."));

    d->showVideoAudioBitRate     = new QCheckBox(i18n("Audio Bit Rate"), d->videoSettingBox);
    d->showVideoAudioBitRate->setWhatsThis(i18n("Set this option to display the Audio Bit Rate of the Video."));

    d->showVideoAudioChannelType = new QCheckBox(i18n("Audio Channel Type"), d->videoSettingBox);
    d->showVideoAudioChannelType->setWhatsThis(i18n("Set this option to display the Audio Channel Type of the Video."));

    d->showVideoAudioCodec  = new QCheckBox(i18n("Audio Codec"), d->videoSettingBox);
    d->showVideoAudioCodec->setWhatsThis(i18n("Set this option to display the Audio Codec of the Video."));

    d->showVideoDuration         = new QCheckBox(i18n("Video Duration"), d->videoSettingBox);
    d->showVideoDuration->setWhatsThis(i18n("Set this option to display the Duration of the Video."));

    d->showVideoFrameRate        = new QCheckBox(i18n("Video Frame Rate"), d->videoSettingBox);
    d->showVideoFrameRate->setWhatsThis(i18n("Set this option to display the Aspect Ratio of the Video."));

    d->showVideoVideoCodec       = new QCheckBox(i18n("Video Codec"), d->videoSettingBox);
    d->showVideoVideoCodec->setWhatsThis(i18n("Set this option to display the Codec of the Video."));

    grid5->addWidget(d->showVideoAspectRatio,      0, 0, 1, 1);
    grid5->addWidget(d->showVideoAudioBitRate,     1, 0, 1, 1);
    grid5->addWidget(d->showVideoAudioChannelType, 2, 0, 1, 1);
    grid5->addWidget(d->showVideoAudioCodec,       3, 0, 1, 1);
    grid5->addWidget(d->showVideoDuration,         0, 1, 1, 1);
    grid5->addWidget(d->showVideoFrameRate,        1, 1, 1, 1);
    grid5->addWidget(d->showVideoVideoCodec,       2, 1, 1, 1);
    grid5->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    grid5->setSpacing(0);

    QWidget* const space = new QWidget(vbox);
    vbox->setStretchFactor(space, 10);
    vbox->setContentsMargins(d->spacing, d->spacing, d->spacing, d->spacing);
    vbox->setSpacing(d->spacing);

    d->tab->insertTab(IconItems,   vbox,  i18n("Icon Items"));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fileSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->photoSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->videoSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->digikamSettingBox, SLOT(setEnabled(bool)));
}

} // namespace Digikam
