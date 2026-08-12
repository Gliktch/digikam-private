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

void SetupAlbumView::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setTreeViewIconSize(d->iconTreeThumbSize->currentData().toInt());
    settings->setTreeViewFaceSize(d->iconTreeFaceSize->currentData().toInt());
    settings->setTreeViewFont(d->treeViewFontSelect->font());
    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowAspectRatio(d->iconShowAspectRatioBox->isChecked());
    settings->setIconShowTitle(d->iconShowTitleBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowOverlays(d->iconShowOverlaysBox->isChecked());
    settings->setIconShowFullscreen(d->iconShowFullscreenBox->isChecked());
    settings->setIconShowCoordinates(d->iconShowCoordinatesBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());
    settings->setIconShowPickLabel(d->iconShowPickLabelBox->isChecked());
    settings->setIconShowColorLabel(d->iconShowColorLabelBox->isChecked());
    settings->setIconShowImageFormat(d->iconShowFormatBox->isChecked());
    settings->setIconViewFont(d->iconViewFontSelect->font());
    settings->setItemLeftClickAction(d->leftClickActionComboBox->currentIndex());

    PreviewSettings previewSettings;
    previewSettings.quality           = d->previewFastPreview->isChecked() ? PreviewSettings::FastPreview : PreviewSettings::HighQualityPreview;
    previewSettings.rawLoading        = (PreviewSettings::RawLoading)d->previewRawMode->itemData(d->previewRawMode->currentIndex()).toInt();
    previewSettings.convertToEightBit = d->previewConvertToEightBit->isChecked();
    settings->setPreviewSettings(previewSettings);

    settings->setPreviewAutoPlay(d->previewAutoPlay->isChecked());
    settings->setPreviewOverlay(d->previewOverlay->isChecked());
    settings->setPreviewShowIcons(d->previewShowIcons->isChecked());
    settings->setPreviewSmoothScaled(d->previewSmoothScaled->isChecked());
    settings->setScaleFitToWindow(d->previewScaleFitToWindow->isChecked());
    settings->setMagnifierZoomFactor(d->magnifierZoomFactor->currentData().toReal());
    settings->setMagnifierSize(d->magnifierSize->currentData().toInt());
    settings->setShowFolderTreeViewItemsCount(d->showFolderTreeViewItemsCount->isChecked());
    settings->saveSettings();

    d->osdWidget->writeSettings();
    d->osdSettings.writeToConfig(QLatin1String("Preview OSD Settings"));

    KConfigGroup group = KSharedConfig::openConfig()->group(settings->generalConfigGroupName());
    d->fullScreenSettings->saveSettings(group);

    d->category->applySettings();
    d->mimetype->applySettings();

    // Method ThumbnailSize::setUseLargeThumbs() is not called here to prevent
    // dysfunction between Thumbs DB and icon if
    // thumb size is over 256 and when large thumbs size support is disabled.
    // digiKam need to be restarted to take effect.

    ThumbnailSize::saveSettings(group, d->largeThumbsBox->isChecked());
}

void SetupAlbumView::readSettings()
{
    const ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    d->iconTreeThumbSize->setCurrentIndex(d->iconTreeThumbSize->findData(settings->getTreeViewIconSize()));
    d->iconTreeFaceSize->setCurrentIndex(d->iconTreeFaceSize->findData(settings->getTreeViewFaceSize()));

    d->treeViewFontSelect->setFont(settings->getTreeViewFont());
    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowAspectRatioBox->setChecked(settings->getIconShowAspectRatio());
    d->iconShowTitleBox->setChecked(settings->getIconShowTitle());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowOverlaysBox->setChecked(settings->getIconShowOverlays());
    d->iconShowFullscreenBox->setChecked(settings->getIconShowFullscreen());
    d->iconShowCoordinatesBox->setChecked(settings->getIconShowCoordinates());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());
    d->iconShowPickLabelBox->setChecked(settings->getIconShowPickLabel());
    d->iconShowColorLabelBox->setChecked(settings->getIconShowColorLabel());
    d->iconShowFormatBox->setChecked(settings->getIconShowImageFormat());
    d->iconViewFontSelect->setFont(settings->getIconViewFont());

    d->leftClickActionComboBox->setCurrentIndex(settings->getItemLeftClickAction());

    PreviewSettings previewSettings = settings->getPreviewSettings();

    if (previewSettings.quality == PreviewSettings::FastPreview)
    {
        d->previewFastPreview->setChecked(true);
    }
    else
    {
        d->previewFullView->setChecked(true);
        d->previewRawMode->setEnabled(true);
    }

    d->previewRawMode->setCurrentIndex(d->previewRawMode->findData(previewSettings.rawLoading));

    d->previewAutoPlay->setChecked(settings->getPreviewAutoPlay());
    d->previewOverlay->setChecked(settings->getPreviewOverlay());
    d->previewShowIcons->setChecked(settings->getPreviewShowIcons());
    d->previewSmoothScaled->setChecked(settings->getPreviewSmoothScaled());
    d->previewScaleFitToWindow->setChecked(settings->getScaleFitToWindow());
    d->magnifierZoomFactor->setCurrentIndex(d->magnifierZoomFactor->findData(settings->getMagnifierZoomFactor()));
    d->magnifierSize->setCurrentIndex(d->magnifierSize->findData(settings->getMagnifierSize()));
    d->previewConvertToEightBit->setChecked(previewSettings.convertToEightBit);
    d->showFolderTreeViewItemsCount->setChecked(settings->getShowFolderTreeViewItemsCount());

    d->osdSettings.readFromConfig(QLatin1String("Preview OSD Settings"));
    d->osdWidget->readSettings();

    KConfigGroup group = KSharedConfig::openConfig()->group(settings->generalConfigGroupName());
    d->fullScreenSettings->readSettings(group);

    ThumbnailSize::readSettings(group);
    d->useLargeThumbsOriginal = ThumbnailSize::getUseLargeThumbs();
    d->largeThumbsBox->setChecked(d->useLargeThumbsOriginal);

    d->category->readSettings();
    d->mimetype->readSettings();
}

} // namespace Digikam
