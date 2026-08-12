/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "applicationsettings.h"

// C++ includes

#include <stdexcept>

// Qt includes

#include <QApplication>
#include <QFontDatabase>
#include <QString>
#include <QStringList>
#include <QFont>
#include <QCheckBox>
#include <QPointer>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "previewsettings.h"
#include "dbengineparameters.h"
#include "imagezoomsettings.h"
#include "versionmanager.h"
#include "itemfiltersettings.h"
#include "itemsortsettings.h"
#include "mimefilter.h"
#include "thumbnailsize.h"
#include "thememanager.h"
#include "drawdecoder.h"
#include "coredbaccess.h"
#include "coredb.h"

namespace Digikam
{

class ApplicationSettings;

class Q_DECL_HIDDEN ApplicationSettings::Private
{
public:

    explicit Private(ApplicationSettings* const qq);
    ~Private() = default;

    void init();

public:

    const QString configGroupDefault                                = QLatin1String("Album Settings");
    const QString configGroupExif                                   = QLatin1String("EXIF Settings");
    const QString configGroupMetadata                               = QLatin1String("Metadata Settings");
    const QString configGroupBaloo                                  = QLatin1String("Baloo Settings");
    const QString configGroupGeneral                                = QLatin1String("General Settings");
    const QString configGroupVersioning                             = QLatin1String("Versioning Settings");
    const QString configGroupDuplicatesSearch                       = QLatin1String("Find Duplicates View");
    const QString configGroupGrouping                               = QLatin1String("Grouping Behaviour");
    const QString configAlbumCollectionsEntry                       = QLatin1String("Album Collections");
    const QString configAlbumMonitoringEntry                        = QLatin1String("Album Monitoring");
    const QString configAlbumSortRoleEntry                          = QLatin1String("Album Sort Role");
    const QString configImageSortOrderEntry                         = QLatin1String("Image Sort Order");
    const QString configImageSortingEntry                           = QLatin1String("Image Sorting");
    const QString configImageSeparationModeEntry                    = QLatin1String("Image Group Mode");
    const QString configImageSeparationSortOrderEntry               = QLatin1String("Image Group Sort Order");
    const QString configItemLeftClickActionEntry                    = QLatin1String("Item Left Click Action");
    const QString configDefaultIconSizeEntry                        = QLatin1String("Default Icon Size");
    const QString configDefaultTreeIconSizeEntry                    = QLatin1String("Default Tree Icon Size");
    const QString configDefaultTreeFaceSizeEntry                    = QLatin1String("Default Tree Face Size");
    const QString configTreeViewFontEntry                           = QLatin1String("TreeView Font");
    const QString configThemeEntry                                  = QLatin1String("Theme");
    const QString configUpdateType                                  = QLatin1String("Update Type");
    const QString configUpdateWithDebug                             = QLatin1String("Update With Debug");
    const QString configSidebarTitleStyleEntry                      = QLatin1String("Sidebar Title Style");
    const QString configRatingFilterConditionEntry                  = QLatin1String("Rating Filter Condition");
    const QString configRecursiveAlbumsEntry                        = QLatin1String("Recursive Albums");
    const QString configRecursiveTagsEntry                          = QLatin1String("Recursive Tags");
    const QString configAllGroupsOpenEntry                          = QLatin1String("All Groups Open");
    const QString configIconShowNameEntry                           = QLatin1String("Icon Show Name");
    const QString configIconShowResolutionEntry                     = QLatin1String("Icon Show Resolution");
    const QString configIconShowSizeEntry                           = QLatin1String("Icon Show Size");
    const QString configIconShowDateEntry                           = QLatin1String("Icon Show Date");
    const QString configIconShowModificationDateEntry               = QLatin1String("Icon Show Modification Date");
    const QString configIconShowTitleEntry                          = QLatin1String("Icon Show Title");
    const QString configIconShowCommentsEntry                       = QLatin1String("Icon Show Comments");
    const QString configIconShowTagsEntry                           = QLatin1String("Icon Show Tags");
    const QString configIconShowRatingEntry                         = QLatin1String("Icon Show Rating");
    const QString configIconShowPickLabelEntry                      = QLatin1String("Icon Show Pick Label");
    const QString configIconShowColorLabelEntry                     = QLatin1String("Icon Show Color Label");
    const QString configIconShowImageFormatEntry                    = QLatin1String("Icon Show Image Format");
    const QString configIconShowCoordinatesEntry                    = QLatin1String("Icon Show Coordinates");
    const QString configIconShowAspectRatioEntry                    = QLatin1String("Icon Show Aspect Ratio");
    const QString configIconShowOverlaysEntry                       = QLatin1String("Icon Show Overlays");
    const QString configIconShowFullscreenEntry                     = QLatin1String("Icon Show Fullscreen");
    const QString configIconViewFontEntry                           = QLatin1String("IconView Font");
    const QString configToolTipsFontEntry                           = QLatin1String("ToolTips Font");
    const QString configShowToolTipsEntry                           = QLatin1String("Show ToolTips");
    const QString configToolTipsShowFileNameEntry                   = QLatin1String("ToolTips Show File Name");
    const QString configToolTipsShowFileDateEntry                   = QLatin1String("ToolTips Show File Date");
    const QString configToolTipsShowFileSizeEntry                   = QLatin1String("ToolTips Show File Size");
    const QString configToolTipsShowImageTypeEntry                  = QLatin1String("ToolTips Show Image Type");
    const QString configToolTipsShowImageDimEntry                   = QLatin1String("ToolTips Show Image Dim");
    const QString configToolTipsShowImageAREntry                    = QLatin1String("ToolTips Show Image AR");
    const QString configToolTipsShowPhotoMakeEntry                  = QLatin1String("ToolTips Show Photo Make");
    const QString configToolTipsShowPhotoLensEntry                  = QLatin1String("ToolTips Show Photo Lens");
    const QString configToolTipsShowPhotoDateEntry                  = QLatin1String("ToolTips Show Photo Date");
    const QString configToolTipsShowPhotoFocalEntry                 = QLatin1String("ToolTips Show Photo Focal");
    const QString configToolTipsShowPhotoExpoEntry                  = QLatin1String("ToolTips Show Photo Expo");
    const QString configToolTipsShowPhotoModeEntry                  = QLatin1String("ToolTips Show Photo Mode");
    const QString configToolTipsShowPhotoFlashEntry                 = QLatin1String("ToolTips Show Photo Flash");
    const QString configToolTipsShowPhotoWBEntry                    = QLatin1String("ToolTips Show Photo WB");
    const QString configToolTipsShowAlbumNameEntry                  = QLatin1String("ToolTips Show Album Name");
    const QString configToolTipsShowTitlesEntry                     = QLatin1String("ToolTips Show Titles");
    const QString configToolTipsShowCommentsEntry                   = QLatin1String("ToolTips Show Comments");
    const QString configToolTipsShowTagsEntry                       = QLatin1String("ToolTips Show Tags");
    const QString configToolTipsShowLabelRatingEntry                = QLatin1String("ToolTips Show Label Rating");
    const QString configToolTipsShowVideoAspectRatioEntry           = QLatin1String("ToolTips Show Video Aspect Ratio");
    const QString configToolTipsShowVideoAudioBitRateEntry          = QLatin1String("ToolTips Show Audio Bit Rate");
    const QString configToolTipsShowVideoAudioChannelTypeEntry      = QLatin1String("ToolTips Show Audio Channel Type");
    const QString configToolTipsShowVideoAudioCodecEntry            = QLatin1String("ToolTips Show Audio Codec");
    const QString configToolTipsShowVideoDurationEntry              = QLatin1String("ToolTips Show Video Duration");
    const QString configToolTipsShowVideoFrameRateEntry             = QLatin1String("ToolTips Show Video Frame Rate");
    const QString configToolTipsShowVideoVideoCodecEntry            = QLatin1String("ToolTips Show Video Codec");
    const QString configShowAlbumToolTipsEntry                      = QLatin1String("Show Album ToolTips");
    const QString configToolTipsShowAlbumTitleEntry                 = QLatin1String("ToolTips Show Album Title");
    const QString configToolTipsShowAlbumDateEntry                  = QLatin1String("ToolTips Show Album Date");
    const QString configToolTipsShowAlbumCollectionEntry            = QLatin1String("ToolTips Show Album Collection");
    const QString configToolTipsShowAlbumCategoryEntry              = QLatin1String("ToolTips Show Album Category");
    const QString configToolTipsShowAlbumCaptionEntry               = QLatin1String("ToolTips Show Album Caption");
    const QString configToolTipsShowAlbumPreviewEntry               = QLatin1String("ToolTips Show Album Preview");
    const QString configPreviewLoadFullItemSizeEntry                = QLatin1String("Preview Load Full Image Size");
    const QString configPreviewRawUseLoadingDataEntry               = QLatin1String("Preview Raw Use Loading Data");
    const QString configPreviewConvertToEightBitEntry               = QLatin1String("Preview Convert To Eight Bit");
    const QString configPreviewImageSmoothScaledEntry               = QLatin1String("Preview Image Smooth Scaled");
    const QString configPreviewScaleFitToWindowEntry                = QLatin1String("Preview Scale Fit To Window");
    const QString configPreviewMagnifierZoomFactorEntry             = QLatin1String("Preview Magnifier Zoom Factor");
    const QString configPreviewMagnifierSizeEntry                   = QLatin1String("Preview Magnifier Size");
    const QString configPreviewShowIconsEntry                       = QLatin1String("Preview Show Icons");
    const QString configPreviewAutoPlayEntry                        = QLatin1String("Preview Auto Play");
    const QString configPreviewOverlayEntry                         = QLatin1String("Preview Overlay");
    const QString configShowThumbbarEntry                           = QLatin1String("Show Thumbbar");
    const QString configShowFolderTreeViewItemsCountEntry           = QLatin1String("Show Folder Tree View Items Count");
    const QString configShowSplashEntry                             = QLatin1String("Show Splash");
    const QString configUseTrashEntry                               = QLatin1String("Use Trash");
    const QString configShowTrashDeleteDialogEntry                  = QLatin1String("Show Trash Delete Dialog");
    const QString configShowPermanentDeleteDialogEntry              = QLatin1String("Show Permanent Delete Dialog");
    const QString configApplySidebarChangesDirectlyEntry            = QLatin1String("Apply Sidebar Changes Directly");
    const QString configUseNativeFileDialogEntry                    = QLatin1String("Use Native File Dialog");
    const QString configDrawFramesToGroupedEntry                    = QLatin1String("Draw Frames To Grouped Items");
    const QString configSelectFirstAlbumItemEntry                   = QLatin1String("Select First Album Item");
    const QString configExpandNewCurrentItemEntry                   = QLatin1String("Expand New Current Item On Click");
    const QString configScrollItemToCenterEntry                     = QLatin1String("Scroll Current Item To Center");
    const QString configShowOnlyPersonTagsInPeopleSidebarEntry      = QLatin1String("Show Only Face Tags For Assigning Name");
    const QString configDetectFacesInNewImagesEntry                 = QLatin1String("Detect faces in newly added images");
    const QString configFaceRecognitionBackgroundScanEntry          = QLatin1String("Automatically start a background face recognition scan when a new face is tagged");
    const QString configSyncBalootoDigikamEntry                     = QLatin1String("Sync Baloo to Digikam");
    const QString configSyncDigikamtoBalooEntry                     = QLatin1String("Sync Digikam to Baloo");
    const QString configStringComparisonTypeEntry                   = QLatin1String("String Comparison Type");
    const QString configApplicationStyleEntry                       = QLatin1String("Application Style");
    const QString configIconThemeEntry                              = QLatin1String("Icon Theme");
    const QString configDateTimeFormatEntry                         = QLatin1String("DateTime Format");
    const QString configApplicationFontEntry                        = QLatin1String("Application Font");
    const QString configScanAtStartEntry                            = QLatin1String("Scan At Start");
    const QString configCleanAtStartEntry                           = QLatin1String("Clean core DB At Start");
    const QString configMinimumSimilarityBound                      = QLatin1String("Lower bound for minimum similarity");
    const QString configDuplicatesSearchLastMinSimilarity           = QLatin1String("Last minimum similarity");
    const QString configDuplicatesSearchLastMaxSimilarity           = QLatin1String("Last maximum similarity");
    const QString configDuplicatesSearchLastAlbumTagRelation        = QLatin1String("Last search album tag relation");
    const QString configDuplicatesSearchLastRestrictions            = QLatin1String("Last search results restriction");
    const QString configDuplicatesSearchReferenceSelectionMethod    = QLatin1String("Last reference image method");

public:

    /// start up setting
    bool                                         showSplash                                         = true;
    /// file ops settings
    bool                                         useTrash                                           = true;
    bool                                         showTrashDeleteDialog                              = true;
    bool                                         showPermanentDeleteDialog                          = true;
    /// metadata setting
    bool                                         sidebarApplyDirectly                               = false;
    /// file dialog setting
    bool                                         useNativeFileDialog                                = false;
    /// grouped item draw setting
    bool                                         drawFramesToGrouped                                = true;
    /// select first item setting
    bool                                         selectFirstAlbumItem                               = true;
    /// expand item setting
    bool                                         expandNewCurrentItem                               = true;
    /// item center setting
    bool                                         scrollItemToCenter                                 = false;
    /// tag filter setting
    bool                                         showOnlyPersonTagsInPeopleSidebar                  = false;

    bool                                         detectFacesInNewImages                             = false;
    bool                                         faceRecognitionBackgroundScan                      = false;

    /// icon view settings
    bool                                         iconShowName                                       = true;
    bool                                         iconShowSize                                       = false;
    bool                                         iconShowDate                                       = true;
    bool                                         iconShowModDate                                    = false;
    bool                                         iconShowTitle                                      = true;
    bool                                         iconShowComments                                   = true;
    bool                                         iconShowResolution                                 = false;
    bool                                         iconShowAspectRatio                                = false;
    bool                                         iconShowTags                                       = true;
    bool                                         iconShowOverlays                                   = true;
    bool                                         iconShowFullscreen                                 = true;
    bool                                         iconShowRating                                     = true;
    bool                                         iconShowPickLabel                                  = false;
    bool                                         iconShowColorLabel                                 = true;
    bool                                         iconShowImageFormat                                = true;
    bool                                         iconShowCoordinates                                = true;
    QFont                                        iconviewFont                                       = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    /// Icon-view tooltip settings
    bool                                         showToolTips                                       = false;
    bool                                         tooltipShowFileName                                = true;
    bool                                         tooltipShowFileDate                                = false;
    bool                                         tooltipShowFileSize                                = false;
    bool                                         tooltipShowImageType                               = false;
    bool                                         tooltipShowImageDim                                = true;
    bool                                         tooltipShowImageAR                                 = true;
    bool                                         tooltipShowPhotoMake                               = true;
    bool                                         tooltipShowPhotoLens                               = true;
    bool                                         tooltipShowPhotoDate                               = true;
    bool                                         tooltipShowPhotoFocal                              = true;
    bool                                         tooltipShowPhotoExpo                               = true;
    bool                                         tooltipShowPhotoMode                               = true;
    bool                                         tooltipShowPhotoFlash                              = false;
    bool                                         tooltipShowPhotoWb                                 = false;
    bool                                         tooltipShowAlbumName                               = false;
    bool                                         tooltipShowTitles                                  = false;
    bool                                         tooltipShowComments                                = true;
    bool                                         tooltipShowTags                                    = true;
    bool                                         tooltipShowLabelRating                             = true;
    bool                                         tooltipShowVideoAspectRatio                        = true;
    bool                                         tooltipShowVideoAudioBitRate                       = true;
    bool                                         tooltipShowVideoAudioChannelType                   = true;
    bool                                         tooltipShowVideoAudioCodec                         = true;
    bool                                         tooltipShowVideoDuration                           = true;
    bool                                         tooltipShowVideoFrameRate                          = true;
    bool                                         tooltipShowVideoVideoCodec                         = true;

    QFont                                        toolTipsFont                                       = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    /// Folder-view tooltip settings
    bool                                         showAlbumToolTips                                  = false;
    bool                                         tooltipShowAlbumTitle                              = true;
    bool                                         tooltipShowAlbumDate                               = true;
    bool                                         tooltipShowAlbumCollection                         = true;
    bool                                         tooltipShowAlbumCategory                           = true;
    bool                                         tooltipShowAlbumCaption                            = true;
    bool                                         tooltipShowAlbumPreview                            = false;

    /// preview settings
    PreviewSettings                              previewSettings;
    qreal                                        magnifierZoomFactor                                = 1.5;
    int                                          magnifierSize                                      = 150;
    bool                                         imageSmoothScaled                                  = true;
    bool                                         scaleFitToWindow                                   = false;
    bool                                         previewShowIcons                                   = true;
    bool                                         previewAutoPlay                                    = true;
    bool                                         previewOverlay                                     = false;
    bool                                         showThumbbar                                       = true;

    bool                                         showFolderTreeViewItemsCount                       = false;

    /// tree-view settings
    int                                          treeThumbnailSize                                  = 22;
    int                                          treeThumbFaceSize                                  = 48;
    QFont                                        treeviewFont                                       = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    /// icon view settings
    int                                          thumbnailSize                                      = ThumbnailSize::Medium;
    int                                          ratingFilterCond                                   = ItemFilterSettings::GreaterEqualCondition;
    bool                                         recursiveAlbums                                    = false;
    bool                                         recursiveTags                                      = true;
    bool                                         allGroupsOpen                                      = false;

    /// theme settings
    QString                                      currentTheme;

    /// database settings
    DbEngineParameters                           databaseParams;
    bool                                         scanAtStart                                        = true;
    bool                                         cleanAtStart                                       = true;
    bool                                         databaseDirSetAtCmd                                = false;

    /// album settings
    bool                                         albumMonitoring                                    = false;

    QStringList                                  albumCategoryNames;

    KSharedConfigPtr                             config;

    int                                          updateType                                         = 0;
    bool                                         updateWithDebug                                    = false;
    DMultiTabBar::TextStyle                      sidebarTitleStyle                                  = DMultiTabBar::AllIconsText;

    /// album view settings
    ApplicationSettings::AlbumSortRole           albumSortRole                                      = ApplicationSettings::ByFolder;

    /// icon view settings
    int                                          imageSortOrder                                     = ItemSortSettings::SortByFileName;
    int                                          imageSorting                                       = ItemSortSettings::AscendingOrder;
    int                                          imageSeparationMode                                = ItemSortSettings::CategoryByAlbum;
    int                                          imageSeparationSortOrder                           = ItemSortSettings::AscendingOrder;
    int                                          itemLeftClickAction                                = ApplicationSettings::ShowPreview;

    /// Baloo settings
    bool                                         syncToDigikam                                      = false;
    bool                                         syncToBaloo                                        = false;

    /// versioning settings

    VersionManagerSettings                       versionSettings;

    /// misc
    ApplicationSettings::StringComparisonType    stringComparisonType                               = ApplicationSettings::Natural;
    QString                                      applicationStyle                                   = qApp->style()->objectName();
    QString                                      applicationIcon;
    QFont                                        applicationFont                                    = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    int                                          minimumSimilarityBound                             = 40;
    int                                          duplicatesSearchLastMinSimilarity                  = 90;
    int                                          duplicatesSearchLastMaxSimilarity                  = 100;
    int                                          duplicatesSearchLastAlbumTagRelation               = 0;
    int                                          duplicatesSearchLastRestrictions                   = 0;
    HaarIface::RefImageSelMethod                 duplicatesSearchLastReferenceImageSelectionMethod  = HaarIface::RefImageSelMethod::OlderOrLarger;

    /// Grouping operation settings
    ApplicationSettings::OperationModes          groupingOperateOnAll                               = ApplicationSettings::OperationModes();

private:

    ApplicationSettings*                         q                                                  = nullptr;

private:

    ApplicationSettings::OperationStrings        createConfigGroupingOperateOnAll();

public:

    const ApplicationSettings::OperationStrings configGroupingOperateOnAll                          = createConfigGroupingOperateOnAll();
};

} // namespace Digikam
