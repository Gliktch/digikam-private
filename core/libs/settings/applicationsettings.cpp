/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applicationsettings_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ApplicationSettingsCreator
{
public:

    ApplicationSettings object;
};

Q_GLOBAL_STATIC(ApplicationSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ApplicationSettings::ApplicationSettings()
    : QObject(),
      d      (new Private(this))
{
    d->config = KSharedConfig::openConfig();
    d->init();
    readSettings();

    // Init Max Thumbnail Size at startup.

    ThumbnailSize::readSettings(generalConfigGroup());
}

ApplicationSettings::~ApplicationSettings()
{
    delete d;
}

ApplicationSettings* ApplicationSettings::instance()
{
    return &creator->object;
}

QString ApplicationSettings::generalConfigGroupName() const
{
    return d->configGroupGeneral;
}

KConfigGroup ApplicationSettings::generalConfigGroup() const
{
    return d->config->group(generalConfigGroupName());
}

void ApplicationSettings::emitSetupChanged()
{
    Q_EMIT setupChanged();
}

void ApplicationSettings::applyBalooSettings()
{

#ifdef HAVE_KFILEMETADATA

    BalooWrap::instance()->setSyncToBaloo(d->syncToBaloo);
    BalooWrap::instance()->setSyncToDigikam(d->syncToDigikam);

#endif

}

void ApplicationSettings::readSettings()
{
    KSharedConfigPtr config    = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group         = config->group(d->configGroupDefault);
    QStringList collectionList = group.readEntry(d->configAlbumCollectionsEntry, QStringList());

    if (!collectionList.isEmpty())
    {
        collectionList.sort();
        d->albumCategoryNames = collectionList;
    }

    d->albumSortRole                     = (ApplicationSettings::AlbumSortRole)
                                               (group.readEntry(d->configAlbumSortRoleEntry,           (int)ApplicationSettings::ByFolder));

    d->imageSortOrder                    = group.readEntry(d->configImageSortOrderEntry,               (int)ItemSortSettings::SortByFileName);
    d->imageSorting                      = group.readEntry(d->configImageSortingEntry,                 (int)ItemSortSettings::AscendingOrder);
    d->imageSeparationMode               = group.readEntry(d->configImageSeparationModeEntry,          (int)ItemSortSettings::CategoryByAlbum);
    d->imageSeparationSortOrder          = group.readEntry(d->configImageSeparationSortOrderEntry,     (int)ItemSortSettings::AscendingOrder);

    d->itemLeftClickAction               = (ApplicationSettings::ItemLeftClickAction)
                                               (group.readEntry(d->configItemLeftClickActionEntry,     (int)ApplicationSettings::ShowPreview));

    d->thumbnailSize                     = group.readEntry(d->configDefaultIconSizeEntry,              (int)ThumbnailSize::Medium);
    d->treeThumbnailSize                 = group.readEntry(d->configDefaultTreeIconSizeEntry,          22);
    d->treeThumbFaceSize                 = group.readEntry(d->configDefaultTreeFaceSizeEntry,          48);
    d->treeviewFont                      = group.readEntry(d->configTreeViewFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    d->currentTheme                      = group.readEntry(d->configThemeEntry,                        ThemeManager::instance()->defaultThemeName());

    d->updateType                        = group.readEntry(d->configUpdateType,                        0);
    d->sidebarTitleStyle                 = (DMultiTabBar::TextStyle)
                                               group.readEntry(d->configSidebarTitleStyleEntry,        (int)DMultiTabBar::AllIconsText);

    d->ratingFilterCond                  = group.readEntry(d->configRatingFilterConditionEntry,        (int)ItemFilterSettings::GreaterEqualCondition);

    d->albumMonitoring                   = group.readEntry(d->configAlbumMonitoringEntry,              false);
    d->recursiveAlbums                   = group.readEntry(d->configRecursiveAlbumsEntry,              false);
    d->recursiveTags                     = group.readEntry(d->configRecursiveTagsEntry,                true);
    d->allGroupsOpen                     = group.readEntry(d->configAllGroupsOpenEntry,                false);

    d->iconShowName                      = group.readEntry(d->configIconShowNameEntry,                 true);
    d->iconShowSize                      = group.readEntry(d->configIconShowSizeEntry,                 false);
    d->iconShowDate                      = group.readEntry(d->configIconShowDateEntry,                 true);
    d->iconShowModDate                   = group.readEntry(d->configIconShowModificationDateEntry,     false);
    d->iconShowTitle                     = group.readEntry(d->configIconShowTitleEntry,                true);
    d->iconShowComments                  = group.readEntry(d->configIconShowCommentsEntry,             true);
    d->iconShowResolution                = group.readEntry(d->configIconShowResolutionEntry,           false);
    d->iconShowAspectRatio               = group.readEntry(d->configIconShowAspectRatioEntry,          false);
    d->iconShowTags                      = group.readEntry(d->configIconShowTagsEntry,                 true);
    d->iconShowOverlays                  = group.readEntry(d->configIconShowOverlaysEntry,             true);
    d->iconShowFullscreen                = group.readEntry(d->configIconShowFullscreenEntry,           true);
    d->iconShowRating                    = group.readEntry(d->configIconShowRatingEntry,               true);
    d->iconShowPickLabel                 = group.readEntry(d->configIconShowPickLabelEntry,            true);
    d->iconShowColorLabel                = group.readEntry(d->configIconShowColorLabelEntry,           true);
    d->iconShowImageFormat               = group.readEntry(d->configIconShowImageFormatEntry,          true);
    d->iconShowCoordinates               = group.readEntry(d->configIconShowCoordinatesEntry,          true);
    d->iconviewFont                      = group.readEntry(d->configIconViewFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    d->toolTipsFont                      = group.readEntry(d->configToolTipsFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    d->showToolTips                      = group.readEntry(d->configShowToolTipsEntry,                 false);
    d->tooltipShowFileName               = group.readEntry(d->configToolTipsShowFileNameEntry,         true);
    d->tooltipShowFileDate               = group.readEntry(d->configToolTipsShowFileDateEntry,         false);
    d->tooltipShowFileSize               = group.readEntry(d->configToolTipsShowFileSizeEntry,         false);
    d->tooltipShowImageType              = group.readEntry(d->configToolTipsShowImageTypeEntry,        false);
    d->tooltipShowImageDim               = group.readEntry(d->configToolTipsShowImageDimEntry,         true);
    d->tooltipShowImageAR                = group.readEntry(d->configToolTipsShowImageAREntry,          true);
    d->tooltipShowPhotoMake              = group.readEntry(d->configToolTipsShowPhotoMakeEntry,        true);
    d->tooltipShowPhotoLens              = group.readEntry(d->configToolTipsShowPhotoLensEntry,        true);
    d->tooltipShowPhotoDate              = group.readEntry(d->configToolTipsShowPhotoDateEntry,        true);
    d->tooltipShowPhotoFocal             = group.readEntry(d->configToolTipsShowPhotoFocalEntry,       true);
    d->tooltipShowPhotoExpo              = group.readEntry(d->configToolTipsShowPhotoExpoEntry,        true);
    d->tooltipShowPhotoMode              = group.readEntry(d->configToolTipsShowPhotoModeEntry,        true);
    d->tooltipShowPhotoFlash             = group.readEntry(d->configToolTipsShowPhotoFlashEntry,       false);
    d->tooltipShowPhotoWb                = group.readEntry(d->configToolTipsShowPhotoWBEntry,          false);
    d->tooltipShowAlbumName              = group.readEntry(d->configToolTipsShowAlbumNameEntry,        false);
    d->tooltipShowTitles                 = group.readEntry(d->configToolTipsShowTitlesEntry,           true);
    d->tooltipShowComments               = group.readEntry(d->configToolTipsShowCommentsEntry,         true);
    d->tooltipShowTags                   = group.readEntry(d->configToolTipsShowTagsEntry,             true);
    d->tooltipShowLabelRating            = group.readEntry(d->configToolTipsShowLabelRatingEntry,      true);

    d->tooltipShowVideoAspectRatio       = group.readEntry(d->configToolTipsShowVideoAspectRatioEntry,      true);
    d->tooltipShowVideoAudioBitRate      = group.readEntry(d->configToolTipsShowVideoAudioBitRateEntry,     true);
    d->tooltipShowVideoAudioChannelType  = group.readEntry(d->configToolTipsShowVideoAudioChannelTypeEntry, true);
    d->tooltipShowVideoAudioCodec        = group.readEntry(d->configToolTipsShowVideoAudioCodecEntry,       true);
    d->tooltipShowVideoDuration          = group.readEntry(d->configToolTipsShowVideoDurationEntry,         true);
    d->tooltipShowVideoFrameRate         = group.readEntry(d->configToolTipsShowVideoFrameRateEntry,        true);
    d->tooltipShowVideoVideoCodec        = group.readEntry(d->configToolTipsShowVideoVideoCodecEntry,       true);

    d->showAlbumToolTips                 = group.readEntry(d->configShowAlbumToolTipsEntry,            false);
    d->tooltipShowAlbumTitle             = group.readEntry(d->configToolTipsShowAlbumTitleEntry,       true);
    d->tooltipShowAlbumDate              = group.readEntry(d->configToolTipsShowAlbumDateEntry,        true);
    d->tooltipShowAlbumCollection        = group.readEntry(d->configToolTipsShowAlbumCollectionEntry,  true);
    d->tooltipShowAlbumCategory          = group.readEntry(d->configToolTipsShowAlbumCategoryEntry,    true);
    d->tooltipShowAlbumCaption           = group.readEntry(d->configToolTipsShowAlbumCaptionEntry,     true);
    d->tooltipShowAlbumPreview           = group.readEntry(d->configToolTipsShowAlbumPreviewEntry,     false);

    if (group.readEntry(d->configPreviewLoadFullItemSizeEntry, true))
    {
        d->previewSettings.quality       = PreviewSettings::HighQualityPreview;
        d->previewSettings.rawLoading    = (PreviewSettings::RawLoading)
                                               group.readEntry(d->configPreviewRawUseLoadingDataEntry, (int)PreviewSettings::RawPreviewAutomatic);
    }
    else
    {
        d->previewSettings.quality       = PreviewSettings::FastPreview;
    }

    d->previewSettings.convertToEightBit = group.readEntry(d->configPreviewConvertToEightBitEntry,     false);
    d->imageSmoothScaled                 = group.readEntry(d->configPreviewImageSmoothScaledEntry,     true);

    setPreviewSmoothScaled(d->imageSmoothScaled);

    d->scaleFitToWindow                  = group.readEntry(d->configPreviewScaleFitToWindowEntry,      false);
    d->magnifierZoomFactor               = group.readEntry(d->configPreviewMagnifierZoomFactorEntry,   1.5);
    d->magnifierSize                     = group.readEntry(d->configPreviewMagnifierSizeEntry,         150);
    d->previewShowIcons                  = group.readEntry(d->configPreviewShowIconsEntry,             true);
    d->previewAutoPlay                   = group.readEntry(d->configPreviewAutoPlayEntry,              true);
    d->previewOverlay                    = group.readEntry(d->configPreviewOverlayEntry,               false);
    d->showThumbbar                      = group.readEntry(d->configShowThumbbarEntry,                 true);

    d->showFolderTreeViewItemsCount      = group.readEntry(d->configShowFolderTreeViewItemsCountEntry, false);

    // ---------------------------------------------------------------------

    group                                = generalConfigGroup();

    d->showSplash                        = group.readEntry(d->configShowSplashEntry,                                  true);
    d->useTrash                          = group.readEntry(d->configUseTrashEntry,                                    true);
    d->showTrashDeleteDialog             = group.readEntry(d->configShowTrashDeleteDialogEntry,                       true);
    d->showPermanentDeleteDialog         = group.readEntry(d->configShowPermanentDeleteDialogEntry,                   true);
    d->sidebarApplyDirectly              = group.readEntry(d->configApplySidebarChangesDirectlyEntry,                 false);

#ifdef Q_OS_MACOS

    d->useNativeFileDialog               = group.readEntry(d->configUseNativeFileDialogEntry,                         true);

#else

    d->useNativeFileDialog               = group.readEntry(d->configUseNativeFileDialogEntry,                         false);

#endif

    d->drawFramesToGrouped               = group.readEntry(d->configDrawFramesToGroupedEntry,                         true);
    d->selectFirstAlbumItem              = group.readEntry(d->configSelectFirstAlbumItemEntry,                        true);
    d->expandNewCurrentItem              = group.readEntry(d->configExpandNewCurrentItemEntry,                        true);
    d->scrollItemToCenter                = group.readEntry(d->configScrollItemToCenterEntry,                          false);
    d->showOnlyPersonTagsInPeopleSidebar = group.readEntry(d->configShowOnlyPersonTagsInPeopleSidebarEntry,           true);
    d->detectFacesInNewImages            = group.readEntry(d->configDetectFacesInNewImagesEntry,                      false);
    d->faceRecognitionBackgroundScan     = group.readEntry(d->configFaceRecognitionBackgroundScanEntry,               false);
    d->stringComparisonType              = (StringComparisonType)
                                               group.readEntry(d->configStringComparisonTypeEntry,                    (int) Natural);

#if defined Q_OS_WIN || defined Q_OS_MACOS

    QString defaultStyle                 = QLatin1String("Breeze");

#else

    QString defaultStyle                 = qApp->style()->objectName();

#endif

#ifdef HAVE_APPSTYLE_SUPPORT

    QString style = group.readEntry(d->configApplicationStyleEntry, defaultStyle);

    if (style == QLatin1String("Macintosh"))        // See bug @475572
    {
        style = QLatin1String("Breeze");
    }

    setApplicationStyle(style);

#else

    setApplicationStyle(QLatin1String("Breeze"));

#endif

    d->applicationIcon                   = group.readEntry(d->configIconThemeEntry,                                   QString());

    setDateTimeFormat(group.readEntry(d->configDateTimeFormatEntry,
                                      QLocale().dateTimeFormat(QLocale::ShortFormat)));

    setApplicationFont(group.readEntry(d->configApplicationFontEntry,
                                       QFontDatabase::systemFont(QFontDatabase::GeneralFont)));

    d->scanAtStart                       = group.readEntry(d->configScanAtStartEntry,                                 true);
    d->cleanAtStart                      = group.readEntry(d->configCleanAtStartEntry,                                false);

    // ---------------------------------------------------------------------

    d->databaseParams.readFromConfig();

#ifdef HAVE_KFILEMETADATA

    group                                = config->group(d->configGroupBaloo);

    d->syncToDigikam                     = group.readEntry(d->configSyncBalootoDigikamEntry, false);
    d->syncToBaloo                       = group.readEntry(d->configSyncDigikamtoBalooEntry, false);

    Q_EMIT balooSettingsChanged();

#endif // HAVE_KFILEMETADATA

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.readFromConfig(group);

    // ---------------------------------------------------------------------

    group                                                = config->group(d->configGroupDuplicatesSearch);

    d->minimumSimilarityBound                            = group.readEntry(d->configMinimumSimilarityBound,               40);
    d->duplicatesSearchLastMinSimilarity                 = group.readEntry(d->configDuplicatesSearchLastMinSimilarity,    90);
    d->duplicatesSearchLastMaxSimilarity                 = group.readEntry(d->configDuplicatesSearchLastMaxSimilarity,    100);
    d->duplicatesSearchLastAlbumTagRelation              = group.readEntry(d->configDuplicatesSearchLastAlbumTagRelation, 0);
    d->duplicatesSearchLastRestrictions                  = group.readEntry(d->configDuplicatesSearchLastRestrictions,     0);
    int method                                           = group.readEntry(d->configDuplicatesSearchReferenceSelectionMethod,
                                                                           (int)HaarIface::RefImageSelMethod::OlderOrLarger);
    d->duplicatesSearchLastReferenceImageSelectionMethod = (HaarIface::RefImageSelMethod)method;

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupGrouping);

    for (ApplicationSettings::OperationModes::key_iterator it = d->groupingOperateOnAll.keyBegin() ;
         it != d->groupingOperateOnAll.keyEnd() ; ++it)
    {
        d->groupingOperateOnAll.insert(*it, (ApplicationSettings::ApplyToEntireGroup)
                                                group.readEntry(d->configGroupingOperateOnAll.value(*it),
                                                                (int)ApplicationSettings::Ask));
    }

    Q_EMIT setupChanged();
    Q_EMIT recurseSettingsChanged();
    Q_EMIT balooSettingsChanged();
}

void ApplicationSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group = config->group(d->configGroupDefault);

    group.writeEntry(d->configAlbumCollectionsEntry,                    d->albumCategoryNames);
    group.writeEntry(d->configAlbumSortRoleEntry,                       (int)d->albumSortRole);
    group.writeEntry(d->configImageSortOrderEntry,                      (int)d->imageSortOrder);
    group.writeEntry(d->configImageSortingEntry,                        (int)d->imageSorting);
    group.writeEntry(d->configImageSeparationModeEntry,                 (int)d->imageSeparationMode);
    group.writeEntry(d->configImageSeparationSortOrderEntry,            (int)d->imageSeparationSortOrder);

    group.writeEntry(d->configItemLeftClickActionEntry,                 (int)d->itemLeftClickAction);
    group.writeEntry(d->configDefaultIconSizeEntry,                     (int)d->thumbnailSize);
    group.writeEntry(d->configDefaultTreeIconSizeEntry,                 (int)d->treeThumbnailSize);
    group.writeEntry(d->configDefaultTreeFaceSizeEntry,                 (int)d->treeThumbFaceSize);
    group.writeEntry(d->configTreeViewFontEntry,                        d->treeviewFont);
    group.writeEntry(d->configRatingFilterConditionEntry,               d->ratingFilterCond);
    group.writeEntry(d->configAlbumMonitoringEntry,                     d->albumMonitoring);
    group.writeEntry(d->configRecursiveAlbumsEntry,                     d->recursiveAlbums);
    group.writeEntry(d->configRecursiveTagsEntry,                       d->recursiveTags);
    group.writeEntry(d->configAllGroupsOpenEntry,                       d->allGroupsOpen);
    group.writeEntry(d->configThemeEntry,                               d->currentTheme);
    group.writeEntry(d->configUpdateType,                               d->updateType);
    group.writeEntry(d->configSidebarTitleStyleEntry,                   (int)d->sidebarTitleStyle);

    group.writeEntry(d->configIconShowNameEntry,                        d->iconShowName);
    group.writeEntry(d->configIconShowSizeEntry,                        d->iconShowSize);
    group.writeEntry(d->configIconShowDateEntry,                        d->iconShowDate);
    group.writeEntry(d->configIconShowModificationDateEntry,            d->iconShowModDate);
    group.writeEntry(d->configIconShowTitleEntry,                       d->iconShowTitle);
    group.writeEntry(d->configIconShowCommentsEntry,                    d->iconShowComments);
    group.writeEntry(d->configIconShowResolutionEntry,                  d->iconShowResolution);
    group.writeEntry(d->configIconShowAspectRatioEntry,                 d->iconShowAspectRatio);
    group.writeEntry(d->configIconShowTagsEntry,                        d->iconShowTags);
    group.writeEntry(d->configIconShowOverlaysEntry,                    d->iconShowOverlays);
    group.writeEntry(d->configIconShowFullscreenEntry,                  d->iconShowFullscreen);
    group.writeEntry(d->configIconShowRatingEntry,                      d->iconShowRating);
    group.writeEntry(d->configIconShowPickLabelEntry,                   d->iconShowPickLabel);
    group.writeEntry(d->configIconShowColorLabelEntry,                  d->iconShowColorLabel);
    group.writeEntry(d->configIconShowImageFormatEntry,                 d->iconShowImageFormat);
    group.writeEntry(d->configIconShowCoordinatesEntry,                 d->iconShowCoordinates);
    group.writeEntry(d->configIconViewFontEntry,                        d->iconviewFont);

    group.writeEntry(d->configToolTipsFontEntry,                        d->toolTipsFont);
    group.writeEntry(d->configShowToolTipsEntry,                        d->showToolTips);
    group.writeEntry(d->configToolTipsShowFileNameEntry,                d->tooltipShowFileName);
    group.writeEntry(d->configToolTipsShowFileDateEntry,                d->tooltipShowFileDate);
    group.writeEntry(d->configToolTipsShowFileSizeEntry,                d->tooltipShowFileSize);
    group.writeEntry(d->configToolTipsShowImageTypeEntry,               d->tooltipShowImageType);
    group.writeEntry(d->configToolTipsShowImageDimEntry,                d->tooltipShowImageDim);
    group.writeEntry(d->configToolTipsShowImageAREntry,                 d->tooltipShowImageAR);
    group.writeEntry(d->configToolTipsShowPhotoMakeEntry,               d->tooltipShowPhotoMake);
    group.writeEntry(d->configToolTipsShowPhotoLensEntry,               d->tooltipShowPhotoLens);
    group.writeEntry(d->configToolTipsShowPhotoDateEntry,               d->tooltipShowPhotoDate);
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry,              d->tooltipShowPhotoFocal);
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,               d->tooltipShowPhotoExpo);
    group.writeEntry(d->configToolTipsShowPhotoModeEntry,               d->tooltipShowPhotoMode);
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry,              d->tooltipShowPhotoFlash);
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,                 d->tooltipShowPhotoWb);
    group.writeEntry(d->configToolTipsShowAlbumNameEntry,               d->tooltipShowAlbumName);
    group.writeEntry(d->configToolTipsShowTitlesEntry,                  d->tooltipShowTitles);
    group.writeEntry(d->configToolTipsShowCommentsEntry,                d->tooltipShowComments);
    group.writeEntry(d->configToolTipsShowTagsEntry,                    d->tooltipShowTags);
    group.writeEntry(d->configToolTipsShowLabelRatingEntry,             d->tooltipShowLabelRating);

    group.writeEntry(d->configToolTipsShowVideoAspectRatioEntry,        d->tooltipShowVideoAspectRatio);
    group.writeEntry(d->configToolTipsShowVideoAudioBitRateEntry,       d->tooltipShowVideoAudioBitRate);
    group.writeEntry(d->configToolTipsShowVideoAudioChannelTypeEntry,   d->tooltipShowVideoAudioChannelType);
    group.writeEntry(d->configToolTipsShowVideoAudioCodecEntry,         d->tooltipShowVideoAudioCodec);
    group.writeEntry(d->configToolTipsShowVideoDurationEntry,           d->tooltipShowVideoDuration);
    group.writeEntry(d->configToolTipsShowVideoFrameRateEntry,          d->tooltipShowVideoFrameRate);
    group.writeEntry(d->configToolTipsShowVideoVideoCodecEntry,         d->tooltipShowVideoVideoCodec);

    group.writeEntry(d->configShowAlbumToolTipsEntry,                   d->showAlbumToolTips);
    group.writeEntry(d->configToolTipsShowAlbumTitleEntry,              d->tooltipShowAlbumTitle);
    group.writeEntry(d->configToolTipsShowAlbumDateEntry,               d->tooltipShowAlbumDate);
    group.writeEntry(d->configToolTipsShowAlbumCollectionEntry,         d->tooltipShowAlbumCollection);
    group.writeEntry(d->configToolTipsShowAlbumCategoryEntry,           d->tooltipShowAlbumCategory);
    group.writeEntry(d->configToolTipsShowAlbumCaptionEntry,            d->tooltipShowAlbumCaption);
    group.writeEntry(d->configToolTipsShowAlbumPreviewEntry,            d->tooltipShowAlbumPreview);

    if (d->previewSettings.quality == PreviewSettings::HighQualityPreview)
    {
        group.writeEntry(d->configPreviewLoadFullItemSizeEntry,         true);
        group.writeEntry(d->configPreviewRawUseLoadingDataEntry,        (int)d->previewSettings.rawLoading);
    }
    else
    {
        group.writeEntry(d->configPreviewLoadFullItemSizeEntry,         false);
    }

    group.writeEntry(d->configPreviewConvertToEightBitEntry,            d->previewSettings.convertToEightBit);
    group.writeEntry(d->configPreviewImageSmoothScaledEntry,            d->imageSmoothScaled);
    group.writeEntry(d->configPreviewMagnifierZoomFactorEntry,          d->magnifierZoomFactor);
    group.writeEntry(d->configPreviewMagnifierSizeEntry,                d->magnifierSize);
    group.writeEntry(d->configPreviewScaleFitToWindowEntry,             d->scaleFitToWindow);
    group.writeEntry(d->configPreviewShowIconsEntry,                    d->previewShowIcons);
    group.writeEntry(d->configPreviewAutoPlayEntry,                     d->previewAutoPlay);
    group.writeEntry(d->configPreviewOverlayEntry,                      d->previewOverlay);
    group.writeEntry(d->configShowThumbbarEntry,                        d->showThumbbar);
    group.writeEntry(d->configShowFolderTreeViewItemsCountEntry,        d->showFolderTreeViewItemsCount);

    // ---------------------------------------------------------------------

    group = generalConfigGroup();

    group.writeEntry(d->configShowSplashEntry,                          d->showSplash);
    group.writeEntry(d->configUseTrashEntry,                            d->useTrash);
    group.writeEntry(d->configShowTrashDeleteDialogEntry,               d->showTrashDeleteDialog);
    group.writeEntry(d->configShowPermanentDeleteDialogEntry,           d->showPermanentDeleteDialog);
    group.writeEntry(d->configApplySidebarChangesDirectlyEntry,         d->sidebarApplyDirectly);
    group.writeEntry(d->configUseNativeFileDialogEntry,                 d->useNativeFileDialog);
    group.writeEntry(d->configDrawFramesToGroupedEntry,                 d->drawFramesToGrouped);
    group.writeEntry(d->configSelectFirstAlbumItemEntry,                d->selectFirstAlbumItem);
    group.writeEntry(d->configExpandNewCurrentItemEntry,                d->expandNewCurrentItem);
    group.writeEntry(d->configScrollItemToCenterEntry,                  d->scrollItemToCenter);
    group.writeEntry(d->configShowOnlyPersonTagsInPeopleSidebarEntry,   d->showOnlyPersonTagsInPeopleSidebar);
    group.writeEntry(d->configDetectFacesInNewImagesEntry,              d->detectFacesInNewImages);
    group.writeEntry(d->configFaceRecognitionBackgroundScanEntry,       d->faceRecognitionBackgroundScan);
    group.writeEntry(d->configStringComparisonTypeEntry,                (int) d->stringComparisonType);

#ifdef HAVE_APPSTYLE_SUPPORT

    group.writeEntry(d->configApplicationStyleEntry,                    d->applicationStyle);

#endif

    group.writeEntry(d->configIconThemeEntry,                           d->applicationIcon);
    group.writeEntry(d->configDateTimeFormatEntry,                      DATETIMEFORMATSTRING);
    group.writeEntry(d->configApplicationFontEntry,                     d->applicationFont);

    group.writeEntry(d->configScanAtStartEntry,                         d->scanAtStart);
    group.writeEntry(d->configCleanAtStartEntry,                        d->cleanAtStart);

    // ---------------------------------------------------------------------

    d->databaseParams.writeToConfig();

#ifdef HAVE_KFILEMETADATA

    group = config->group(d->configGroupBaloo);

    group.writeEntry(d->configSyncBalootoDigikamEntry,                  d->syncToDigikam);
    group.writeEntry(d->configSyncDigikamtoBalooEntry,                  d->syncToBaloo);

#endif // HAVE_KFILEMETADATA

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupVersioning);
    d->versionSettings.writeToConfig(group);

    // ---------------------------------------------------------------------

    group = config->group(d->configGroupDuplicatesSearch);

    group.writeEntry(d->configMinimumSimilarityBound,                   d->minimumSimilarityBound);
    group.writeEntry(d->configDuplicatesSearchLastMinSimilarity,        d->duplicatesSearchLastMinSimilarity);
    group.writeEntry(d->configDuplicatesSearchLastMaxSimilarity,        d->duplicatesSearchLastMaxSimilarity);
    group.writeEntry(d->configDuplicatesSearchLastAlbumTagRelation,     d->duplicatesSearchLastAlbumTagRelation);
    group.writeEntry(d->configDuplicatesSearchLastRestrictions,         d->duplicatesSearchLastRestrictions);
    group.writeEntry(d->configDuplicatesSearchReferenceSelectionMethod, (int)d->duplicatesSearchLastReferenceImageSelectionMethod);

    group = config->group(d->configGroupGrouping);

    for (ApplicationSettings::OperationModes::key_iterator it = d->groupingOperateOnAll.keyBegin() ;
         it != d->groupingOperateOnAll.keyEnd() ; ++it)
    {
        group.writeEntry(d->configGroupingOperateOnAll.value(*it),
                         (int)d->groupingOperateOnAll.value(*it));
    }

    config->sync();
}

} // namespace Digikam

#include "moc_applicationsettings.cpp"
