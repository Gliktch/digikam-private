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

void SetupToolTip::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setToolTipsFont(d->fontSelect->font());

    settings->setShowToolTips(d->showToolTipsBox->isChecked());
    settings->setToolTipsShowFileName(d->showFileNameBox->isChecked());
    settings->setToolTipsShowFileDate(d->showFileDateBox->isChecked());
    settings->setToolTipsShowFileSize(d->showFileSizeBox->isChecked());
    settings->setToolTipsShowImageAR(d->showImageARBox->isChecked());
    settings->setToolTipsShowImageType(d->showImageTypeBox->isChecked());
    settings->setToolTipsShowImageDim(d->showImageDimBox->isChecked());

    settings->setToolTipsShowPhotoMake(d->showPhotoMakeBox->isChecked());
    settings->setToolTipsShowPhotoLens(d->showPhotoLensBox->isChecked());
    settings->setToolTipsShowPhotoDate(d->showPhotoDateBox->isChecked());
    settings->setToolTipsShowPhotoFocal(d->showPhotoFocalBox->isChecked());
    settings->setToolTipsShowPhotoExpo(d->showPhotoExpoBox->isChecked());
    settings->setToolTipsShowPhotoMode(d->showPhotoModeBox->isChecked());
    settings->setToolTipsShowPhotoFlash(d->showPhotoFlashBox->isChecked());
    settings->setToolTipsShowPhotoWB(d->showPhotoWbBox->isChecked());

    settings->setToolTipsShowVideoAspectRatio(d->showVideoAspectRatio->isChecked());
    settings->setToolTipsShowVideoAudioBitRate(d->showVideoAudioBitRate->isChecked());
    settings->setToolTipsShowVideoAudioChannelType(d->showVideoAudioChannelType->isChecked());
    settings->setToolTipsShowVideoAudioCodec(d->showVideoAudioCodec->isChecked());
    settings->setToolTipsShowVideoDuration(d->showVideoDuration->isChecked());
    settings->setToolTipsShowVideoFrameRate(d->showVideoFrameRate->isChecked());
    settings->setToolTipsShowVideoVideoCodec(d->showVideoVideoCodec->isChecked());

    settings->setToolTipsShowAlbumName(d->showAlbumNameBox->isChecked());
    settings->setToolTipsShowTitles(d->showTitlesBox->isChecked());
    settings->setToolTipsShowComments(d->showCommentsBox->isChecked());
    settings->setToolTipsShowTags(d->showTagsBox->isChecked());
    settings->setToolTipsShowLabelRating(d->showLabelsBox->isChecked());

    settings->setShowAlbumToolTips(d->showAlbumToolTipsBox->isChecked());
    settings->setToolTipsShowAlbumTitle(d->showAlbumTitleBox->isChecked());
    settings->setToolTipsShowAlbumDate(d->showAlbumDateBox->isChecked());
    settings->setToolTipsShowAlbumCollection(d->showAlbumCollectionBox->isChecked());
    settings->setToolTipsShowAlbumCategory(d->showAlbumCategoryBox->isChecked());
    settings->setToolTipsShowAlbumCaption(d->showAlbumCaptionBox->isChecked());
    settings->setToolTipsShowAlbumPreview(d->showAlbumPreviewBox->isChecked());

    settings->saveSettings();

    // -- Import Settings ------------------------------------------------------------------------

    ImportSettings* const importSettings = ImportSettings::instance();

    if (!importSettings)
    {
        return;
    }

    importSettings->setShowToolTips(d->showImportToolTipsBox->isChecked());
    importSettings->setToolTipsShowFileName(d->showItemTitleBox->isChecked());
    importSettings->setToolTipsShowFileDate(d->showItemDateBox->isChecked());
    importSettings->setToolTipsShowFileSize(d->showItemSizeBox->isChecked());
    importSettings->setToolTipsShowImageType(d->showItemTypeBox->isChecked());
    importSettings->setToolTipsShowImageDim(d->showItemDimensionsBox->isChecked());
    importSettings->setToolTipsShowPhotoMake(d->showItemPhotoMakeBox->isChecked());
    importSettings->setToolTipsShowPhotoLens(d->showItemPhotoLensBox->isChecked());
    importSettings->setToolTipsShowPhotoFocal(d->showItemPhotoFocalBox->isChecked());
    importSettings->setToolTipsShowPhotoExpo(d->showItemPhotoExpoBox->isChecked());
    importSettings->setToolTipsShowPhotoFlash(d->showItemPhotoFlashBox->isChecked());
    importSettings->setToolTipsShowPhotoWB(d->showItemPhotoWBBox->isChecked());

    importSettings->saveSettings();
}

void SetupToolTip::readSettings()
{
    const ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    d->fontSelect->setFont(settings->getToolTipsFont());

    d->showToolTipsBox->setChecked(settings->getShowToolTips());
    d->showFileNameBox->setChecked(settings->getToolTipsShowFileName());
    d->showFileDateBox->setChecked(settings->getToolTipsShowFileDate());
    d->showFileSizeBox->setChecked(settings->getToolTipsShowFileSize());
    d->showImageARBox->setChecked(settings->getToolTipsShowImageAR());
    d->showImageTypeBox->setChecked(settings->getToolTipsShowImageType());
    d->showImageDimBox->setChecked(settings->getToolTipsShowImageDim());

    d->showPhotoMakeBox->setChecked(settings->getToolTipsShowPhotoMake());
    d->showPhotoLensBox->setChecked(settings->getToolTipsShowPhotoLens());
    d->showPhotoDateBox->setChecked(settings->getToolTipsShowPhotoDate());
    d->showPhotoFocalBox->setChecked(settings->getToolTipsShowPhotoFocal());
    d->showPhotoExpoBox->setChecked(settings->getToolTipsShowPhotoExpo());
    d->showPhotoModeBox->setChecked(settings->getToolTipsShowPhotoMode());
    d->showPhotoFlashBox->setChecked(settings->getToolTipsShowPhotoFlash());
    d->showPhotoWbBox->setChecked(settings->getToolTipsShowPhotoWB());

    d->showVideoAspectRatio->setChecked(settings->getToolTipsShowVideoAspectRatio());
    d->showVideoAudioBitRate->setChecked(settings->getToolTipsShowVideoAudioBitRate());
    d->showVideoAudioChannelType->setChecked(settings->getToolTipsShowVideoAudioChannelType());
    d->showVideoAudioCodec->setChecked(settings->getToolTipsShowVideoAudioCodec());
    d->showVideoDuration->setChecked(settings->getToolTipsShowVideoDuration());
    d->showVideoFrameRate->setChecked(settings->getToolTipsShowVideoFrameRate());
    d->showVideoVideoCodec->setChecked(settings->getToolTipsShowVideoVideoCodec());

    d->showAlbumNameBox->setChecked(settings->getToolTipsShowAlbumName());
    d->showTitlesBox->setChecked(settings->getToolTipsShowTitles());
    d->showCommentsBox->setChecked(settings->getToolTipsShowComments());
    d->showTagsBox->setChecked(settings->getToolTipsShowTags());
    d->showLabelsBox->setChecked(settings->getToolTipsShowLabelRating());

    d->fileSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->photoSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->digikamSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->videoSettingBox->setEnabled(d->showToolTipsBox->isChecked());

    d->albumSettingBox->setEnabled(d->showAlbumToolTipsBox->isChecked());

    d->showAlbumToolTipsBox->setChecked(settings->getShowAlbumToolTips());
    d->showAlbumTitleBox->setChecked(settings->getToolTipsShowAlbumTitle());
    d->showAlbumDateBox->setChecked(settings->getToolTipsShowAlbumDate());
    d->showAlbumCollectionBox->setChecked(settings->getToolTipsShowAlbumCollection());
    d->showAlbumCategoryBox->setChecked(settings->getToolTipsShowAlbumCategory());
    d->showAlbumCaptionBox->setChecked(settings->getToolTipsShowAlbumCaption());
    d->showAlbumPreviewBox->setChecked(settings->getToolTipsShowAlbumPreview());

    // -- Import Settings ------------------------------------------------------------------------

    const ImportSettings* const importSettings = ImportSettings::instance();

    if (!importSettings)
    {
        return;
    }

    d->showImportToolTipsBox->setChecked(importSettings->getShowToolTips());
    d->showItemTitleBox->setChecked(importSettings->getToolTipsShowFileName());
    d->showItemDateBox->setChecked(importSettings->getToolTipsShowFileDate());
    d->showItemSizeBox->setChecked(importSettings->getToolTipsShowFileSize());
    d->showItemTypeBox->setChecked(importSettings->getToolTipsShowImageType());
    d->showItemDimensionsBox->setChecked(importSettings->getToolTipsShowImageDim());
    d->showItemPhotoMakeBox->setChecked(importSettings->getToolTipsShowPhotoMake());
    d->showItemPhotoLensBox->setChecked(importSettings->getToolTipsShowPhotoLens());
    d->showItemPhotoFocalBox->setChecked(importSettings->getToolTipsShowPhotoFocal());
    d->showItemPhotoExpoBox->setChecked(importSettings->getToolTipsShowPhotoExpo());
    d->showItemPhotoFlashBox->setChecked(importSettings->getToolTipsShowPhotoFlash());
    d->showItemPhotoWBBox->setChecked(importSettings->getToolTipsShowPhotoWB());

    refreshCameraOptions();
}

} // namespace Digikam
