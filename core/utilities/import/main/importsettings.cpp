/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-15
 * Description : Settings for the import tool
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importsettings.h"

// Qt includes

#include <QFontDatabase>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "camitemsortsettings.h"
#include "thumbnailsize.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportSettings::Private
{
public:

    Private() = default;

    // Import icon-view settings
    bool                                iconShowName                         = true;
    bool                                iconShowSize                         = false;
    bool                                iconShowDate                         = true;
    bool                                iconShowTitle                        = true;
    bool                                iconShowResolution                   = false;
    bool                                iconShowTags                         = true;
    bool                                iconShowOverlays                     = true;
    bool                                iconShowRating                       = true;
    bool                                iconShowImageFormat                  = false;
    bool                                iconShowCoordinates                  = false;

    QFont                               iconviewFont                         = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    int                                 thumbnailSize                        = ThumbnailSize::Medium;
    int                                 imageSortOrder                       = CamItemSortSettings::AscendingOrder;
    int                                 imageSortBy                          = CamItemSortSettings::SortByFileName;
    int                                 imageSeparationMode                  = CamItemSortSettings::CategoryByFolder;
    int                                 itemLeftClickAction                  = ImportSettings::ShowPreview;

    // Import icon-view tooltip settings
    bool                                showToolTips                         = false;
    bool                                tooltipShowFileName                  = true;
    bool                                tooltipShowFileDate                  = false;
    bool                                tooltipShowFileSize                  = false;
    bool                                tooltipShowImageType                 = false;
    bool                                tooltipShowImageDim                  = true;
    bool                                tooltipShowPhotoMake                 = true;
    bool                                tooltipShowPhotoLens                 = true;
    bool                                tooltipShowPhotoFocal                = true;
    bool                                tooltipShowPhotoExpo                 = true;
    bool                                tooltipShowPhotoFlash                = false;
    bool                                tooltipShowPhotoWb                   = false;
    bool                                tooltipShowFolderName                = false;
    bool                                tooltipShowTags                      = true;
    bool                                tooltipShowLabelRating               = true;

    QFont                               toolTipsFont                         = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    // preview settings
    bool                                previewLoadFullImageSize             = false;
    bool                                previewItemsWhileDownload            = false;
    bool                                previewShowIcons                     = true;
    bool                                showThumbbar                         = true;

    KSharedConfigPtr                    config;

    const QString                       configGroupDefault                   = QLatin1String("Import Settings");
    const QString                       configImageSortOrderEntry            = QLatin1String("Image Sort Order");
    const QString                       configImageSortByEntry               = QLatin1String("Image Sorting"); // TODO not changed due to backwards compatibility
    const QString                       configImageSeparationModeEntry       = QLatin1String("Image Separation Mode");
    const QString                       configItemLeftClickActionEntry       = QLatin1String("Item Left Click Action");
    const QString                       configDefaultIconSizeEntry           = QLatin1String("Default Icon Size");
/*
    const QString                       configRatingFilterConditionEntry     = QLatin1String("Rating Filter Condition");
*/
    const QString                       configIconShowNameEntry              = QLatin1String("Icon Show Name");
/*
    const QString                       configIconShowResolutionEntry        = QLatin1String("Icon Show Resolution");
*/
    const QString                       configIconShowSizeEntry              = QLatin1String("Icon Show Size");
    const QString                       configIconShowDateEntry              = QLatin1String("Icon Show Date");
    const QString                       configIconShowTitleEntry             = QLatin1String("Icon Show Title");
    const QString                       configIconShowTagsEntry              = QLatin1String("Icon Show Tags");
    const QString                       configIconShowRatingEntry            = QLatin1String("Icon Show Rating");
    const QString                       configIconShowImageFormatEntry       = QLatin1String("Icon Show Image Format");
    const QString                       configIconShowCoordinatesEntry       = QLatin1String("Icon Show Coordinates");
    const QString                       configIconShowOverlaysEntry          = QLatin1String("Icon Show Overlays");
    const QString                       configIconViewFontEntry              = QLatin1String("IconView Font");
    const QString                       configToolTipsFontEntry              = QLatin1String("ToolTips Font");
    const QString                       configShowToolTipsEntry              = QLatin1String("Show ToolTips");
    const QString                       configToolTipsShowFileNameEntry      = QLatin1String("ToolTips Show File Name");
    const QString                       configToolTipsShowFileDateEntry      = QLatin1String("ToolTips Show File Date");
    const QString                       configToolTipsShowFileSizeEntry      = QLatin1String("ToolTips Show File Size");
    const QString                       configToolTipsShowImageTypeEntry     = QLatin1String("ToolTips Show Image Type");
    const QString                       configToolTipsShowImageDimEntry      = QLatin1String("ToolTips Show Image Dim");
    const QString                       configToolTipsShowPhotoMakeEntry     = QLatin1String("ToolTips Show Photo Make");
    const QString                       configToolTipsShowPhotoLensEntry     = QLatin1String("ToolTips Show Photo Lens");
    const QString                       configToolTipsShowPhotoDateEntry     = QLatin1String("ToolTips Show Photo Date");
    const QString                       configToolTipsShowPhotoFocalEntry    = QLatin1String("ToolTips Show Photo Focal");
    const QString                       configToolTipsShowPhotoExpoEntry     = QLatin1String("ToolTips Show Photo Expo");
    const QString                       configToolTipsShowPhotoFlashEntry    = QLatin1String("ToolTips Show Photo Flash");
    const QString                       configToolTipsShowPhotoWBEntry       = QLatin1String("ToolTips Show Photo WB");
    const QString                       configToolTipsShowFolderNameEntry    = QLatin1String("ToolTips Show Folder Name");
    const QString                       configToolTipsShowTagsEntry          = QLatin1String("ToolTips Show Tags");
    const QString                       configToolTipsShowLabelRatingEntry   = QLatin1String("ToolTips Show Label Rating");
    const QString                       configPreviewLoadFullImageSizeEntry  = QLatin1String("Preview Load Full Image Size");
    const QString                       configPreviewItemsWhileDownloadEntry = QLatin1String("Preview Each Item While Downloading it");
    const QString                       configPreviewShowIconsEntry          = QLatin1String("Preview Show Icons");
    const QString                       configShowThumbbarEntry              = QLatin1String("Show Thumbbar");
 };


// -------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ImportSettingsCreator
{
public:

    ImportSettings object;
};

Q_GLOBAL_STATIC(ImportSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ImportSettings::ImportSettings()
    : QObject(),
      d      (new Private)
{
    d->config = KSharedConfig::openConfig();
    readSettings();
}

ImportSettings::~ImportSettings()
{
    delete d;
}

ImportSettings* ImportSettings::instance()
{
    return &creator->object;
}

void ImportSettings::readSettings()
{
    KSharedConfigPtr config         = d->config;

    KConfigGroup group              = config->group(d->configGroupDefault);

    d->imageSortOrder               = group.readEntry(d->configImageSortOrderEntry, (int)CamItemSortSettings::AscendingOrder);
    d->imageSortBy                  = group.readEntry(d->configImageSortByEntry,    (int)CamItemSortSettings::SortByFileName);
    d->imageSeparationMode          = group.readEntry(d->configImageSeparationModeEntry, (int)CamItemSortSettings::CategoryByFolder);

    d->itemLeftClickAction          = ImportSettings::ItemLeftClickAction(group.readEntry( d->configItemLeftClickActionEntry,
                                                                         (int)ImportSettings::ShowPreview));

    d->thumbnailSize                = group.readEntry(d->configDefaultIconSizeEntry,              (int)ThumbnailSize::Medium);

    d->iconShowName                 = group.readEntry(d->configIconShowNameEntry,                 true);
/*
    d->iconShowResolution           = group.readEntry(d->configIconShowResolutionEntry,           false);
*/
    d->iconShowSize                 = group.readEntry(d->configIconShowSizeEntry,                 false);
    d->iconShowDate                 = group.readEntry(d->configIconShowDateEntry,                 true);
    d->iconShowTitle                = group.readEntry(d->configIconShowTitleEntry,                true);
    d->iconShowTags                 = group.readEntry(d->configIconShowTagsEntry,                 true);
    d->iconShowOverlays             = group.readEntry(d->configIconShowOverlaysEntry,             true);
    d->iconShowRating               = group.readEntry(d->configIconShowRatingEntry,               true);
    d->iconShowImageFormat          = group.readEntry(d->configIconShowImageFormatEntry,          false);
    d->iconShowCoordinates          = group.readEntry(d->configIconShowCoordinatesEntry,          false);
    d->iconviewFont                 = group.readEntry(d->configIconViewFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    d->toolTipsFont                 = group.readEntry(d->configToolTipsFontEntry,                 QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    d->showToolTips                 = group.readEntry(d->configShowToolTipsEntry,                 false);
    d->tooltipShowFileName          = group.readEntry(d->configToolTipsShowFileNameEntry,         true);
    d->tooltipShowFileDate          = group.readEntry(d->configToolTipsShowFileDateEntry,         false);
    d->tooltipShowFileSize          = group.readEntry(d->configToolTipsShowFileSizeEntry,         false);
    d->tooltipShowImageType         = group.readEntry(d->configToolTipsShowImageTypeEntry,        false);
    d->tooltipShowImageDim          = group.readEntry(d->configToolTipsShowImageDimEntry,         true);
    d->tooltipShowPhotoMake         = group.readEntry(d->configToolTipsShowPhotoMakeEntry,        true);
    d->tooltipShowPhotoLens         = group.readEntry(d->configToolTipsShowPhotoLensEntry,        true);
    d->tooltipShowPhotoFocal        = group.readEntry(d->configToolTipsShowPhotoFocalEntry,       true);
    d->tooltipShowPhotoExpo         = group.readEntry(d->configToolTipsShowPhotoExpoEntry,        true);
/*
    d->tooltipShowPhotoMode         = group.readEntry(d->configToolTipsShowPhotoModeEntry,        true);
*/
    d->tooltipShowPhotoFlash        = group.readEntry(d->configToolTipsShowPhotoFlashEntry,       false);
    d->tooltipShowPhotoWb           = group.readEntry(d->configToolTipsShowPhotoWBEntry,          false);
    d->tooltipShowFolderName         = group.readEntry(d->configToolTipsShowFolderNameEntry,      false);
    d->tooltipShowTags              = group.readEntry(d->configToolTipsShowTagsEntry,             true);
    d->tooltipShowLabelRating       = group.readEntry(d->configToolTipsShowLabelRatingEntry,      true);

    d->previewLoadFullImageSize     = group.readEntry(d->configPreviewLoadFullImageSizeEntry,     false);
    d->previewItemsWhileDownload    = group.readEntry(d->configPreviewItemsWhileDownloadEntry,    false);
    d->previewShowIcons             = group.readEntry(d->configPreviewShowIconsEntry,             true);
    d->showThumbbar                 = group.readEntry(d->configShowThumbbarEntry,                 true);

    // ---------------------------------------------------------------------

    Q_EMIT setupChanged();
}

void ImportSettings::saveSettings()
{
    KSharedConfigPtr config = d->config;

    // ---------------------------------------------------------------------

    KConfigGroup group = config->group(d->configGroupDefault);

    group.writeEntry(d->configImageSortOrderEntry,               (int)d->imageSortOrder);
    group.writeEntry(d->configImageSortByEntry,                  (int)d->imageSortBy);
    group.writeEntry(d->configImageSeparationModeEntry,          (int)d->imageSeparationMode);
    group.writeEntry(d->configItemLeftClickActionEntry,          (int)d->itemLeftClickAction);
    group.writeEntry(d->configDefaultIconSizeEntry,              QString::number(d->thumbnailSize));

    group.writeEntry(d->configIconShowNameEntry,                 d->iconShowName);
    group.writeEntry(d->configIconShowSizeEntry,                 d->iconShowSize);
    group.writeEntry(d->configIconShowDateEntry,                 d->iconShowDate);
    group.writeEntry(d->configIconShowTitleEntry,                d->iconShowTitle);
    group.writeEntry(d->configIconShowTagsEntry,                 d->iconShowTags);
    group.writeEntry(d->configIconShowOverlaysEntry,             d->iconShowOverlays);
    group.writeEntry(d->configIconShowRatingEntry,               d->iconShowRating);
    group.writeEntry(d->configIconShowImageFormatEntry,          d->iconShowImageFormat);
    group.writeEntry(d->configIconShowCoordinatesEntry,          d->iconShowCoordinates);
    group.writeEntry(d->configIconViewFontEntry,                 d->iconviewFont);

    group.writeEntry(d->configToolTipsFontEntry,                 d->toolTipsFont);
    group.writeEntry(d->configShowToolTipsEntry,                 d->showToolTips);
    group.writeEntry(d->configToolTipsShowFileNameEntry,         d->tooltipShowFileName);
    group.writeEntry(d->configToolTipsShowFileDateEntry,         d->tooltipShowFileDate);
    group.writeEntry(d->configToolTipsShowFileSizeEntry,         d->tooltipShowFileSize);
    group.writeEntry(d->configToolTipsShowImageTypeEntry,        d->tooltipShowImageType);
    group.writeEntry(d->configToolTipsShowImageDimEntry,         d->tooltipShowImageDim);
    group.writeEntry(d->configToolTipsShowPhotoMakeEntry,        d->tooltipShowPhotoMake);
    group.writeEntry(d->configToolTipsShowPhotoLensEntry,        d->tooltipShowPhotoLens);
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry,       d->tooltipShowPhotoFocal);
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,        d->tooltipShowPhotoExpo);
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry,       d->tooltipShowPhotoFlash);
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,          d->tooltipShowPhotoWb);
    group.writeEntry(d->configToolTipsShowFolderNameEntry,       d->tooltipShowFolderName);
    group.writeEntry(d->configToolTipsShowTagsEntry,             d->tooltipShowTags);
    group.writeEntry(d->configToolTipsShowLabelRatingEntry,      d->tooltipShowLabelRating);

    group.writeEntry(d->configPreviewLoadFullImageSizeEntry,     d->previewLoadFullImageSize);
    group.writeEntry(d->configPreviewItemsWhileDownloadEntry,    d->previewItemsWhileDownload);
    group.writeEntry(d->configPreviewShowIconsEntry,             d->previewShowIcons);
    group.writeEntry(d->configShowThumbbarEntry,                 d->showThumbbar);

    config->sync();
}

void ImportSettings::emitSetupChanged()
{
    Q_EMIT setupChanged();
}

void ImportSettings::setImageSortOrder(int order)
{
    d->imageSortOrder = order;
}

int ImportSettings::getImageSortOrder() const
{
    return d->imageSortOrder;
}

void ImportSettings::setImageSortBy(int sortBy)
{
    d->imageSortBy = sortBy;
}

int ImportSettings::getImageSortBy() const
{
    return d->imageSortBy;
}

void ImportSettings::setImageSeparationMode(int mode)
{
    d->imageSeparationMode = mode;
}

int ImportSettings::getImageSeparationMode() const
{
    return d->imageSeparationMode;
}

void ImportSettings::setItemLeftClickAction(int action)
{
    d->itemLeftClickAction = action;
}

int ImportSettings::getItemLeftClickAction() const
{
    return d->itemLeftClickAction;
}

void ImportSettings::setDefaultIconSize(int val)
{
    d->thumbnailSize = val;
}

int ImportSettings::getDefaultIconSize() const
{
    return d->thumbnailSize;
}

void ImportSettings::setIconViewFont(const QFont& font)
{
    d->iconviewFont = font;
}

QFont ImportSettings::getIconViewFont() const
{
    return d->iconviewFont;
}

void ImportSettings::setIconShowName(bool val)
{
    d->iconShowName = val;
}

bool ImportSettings::getIconShowName() const
{
    return d->iconShowName;
}

void ImportSettings::setIconShowSize(bool val)
{
    d->iconShowSize = val;
}

bool ImportSettings::getIconShowSize() const
{
    return d->iconShowSize;
}

void ImportSettings::setIconShowTitle(bool val)
{
    d->iconShowTitle = val;
}

bool ImportSettings::getIconShowTitle() const
{
    return d->iconShowTitle;
}

void ImportSettings::setIconShowTags(bool val)
{
    d->iconShowTags = val;
}

bool ImportSettings::getIconShowTags() const
{
    return d->iconShowTags;
}

void ImportSettings::setIconShowDate(bool val)
{
    d->iconShowDate = val;
}

bool ImportSettings::getIconShowDate() const
{
    return d->iconShowDate;
}

void ImportSettings::setIconShowRating(bool val)
{
    d->iconShowRating = val;
}

bool ImportSettings::getIconShowRating() const
{
    return d->iconShowRating;
}

void ImportSettings::setIconShowImageFormat(bool val)
{
    d->iconShowImageFormat = val;
}

bool ImportSettings::getIconShowImageFormat() const
{
    return d->iconShowImageFormat;
}

void ImportSettings::setIconShowOverlays(bool val)
{
    d->iconShowOverlays = val;
}

bool ImportSettings::getIconShowOverlays() const
{
    return d->iconShowOverlays;
}

void ImportSettings::setToolTipsFont(const QFont& font)
{
    d->toolTipsFont = font;
}

QFont ImportSettings::getToolTipsFont() const
{
    return d->toolTipsFont;
}

void ImportSettings::setShowToolTips(bool val)
{
    d->showToolTips = val;
}

bool ImportSettings::getShowToolTips() const
{
    return d->showToolTips;
}

void ImportSettings::setToolTipsShowFileName(bool val)
{
    d->tooltipShowFileName = val;
}

bool ImportSettings::getToolTipsShowFileName() const
{
    return d->tooltipShowFileName;
}

void ImportSettings::setToolTipsShowFileDate(bool val)
{
    d->tooltipShowFileDate = val;
}

bool ImportSettings::getToolTipsShowFileDate() const
{
    return d->tooltipShowFileDate;
}

void ImportSettings::setToolTipsShowFileSize(bool val)
{
    d->tooltipShowFileSize = val;
}

bool ImportSettings::getToolTipsShowFileSize() const
{
    return d->tooltipShowFileSize;
}

void ImportSettings::setToolTipsShowImageType(bool val)
{
    d->tooltipShowImageType = val;
}

bool ImportSettings::getToolTipsShowImageType() const
{
    return d->tooltipShowImageType;
}

void ImportSettings::setToolTipsShowImageDim(bool val)
{
    d->tooltipShowImageDim = val;
}

bool ImportSettings::getToolTipsShowImageDim() const
{
    return d->tooltipShowImageDim;
}

void ImportSettings::setToolTipsShowPhotoMake(bool val)
{
    d->tooltipShowPhotoMake = val;
}

bool ImportSettings::getToolTipsShowPhotoMake() const
{
    return d->tooltipShowPhotoMake;
}

void ImportSettings::setToolTipsShowPhotoLens(bool val)
{
    d->tooltipShowPhotoLens = val;
}

bool ImportSettings::getToolTipsShowPhotoLens() const
{
    return d->tooltipShowPhotoLens;
}

void ImportSettings::setToolTipsShowPhotoFocal(bool val)
{
    d->tooltipShowPhotoFocal = val;
}

bool ImportSettings::getToolTipsShowPhotoFocal() const
{
    return d->tooltipShowPhotoFocal;
}

void ImportSettings::setToolTipsShowPhotoExpo(bool val)
{
    d->tooltipShowPhotoExpo = val;
}

bool ImportSettings::getToolTipsShowPhotoExpo() const
{
    return d->tooltipShowPhotoExpo;
}

void ImportSettings::setToolTipsShowPhotoFlash(bool val)
{
    d->tooltipShowPhotoFlash = val;
}

bool ImportSettings::getToolTipsShowPhotoFlash() const
{
    return d->tooltipShowPhotoFlash;
}

void ImportSettings::setToolTipsShowPhotoWB(bool val)
{
    d->tooltipShowPhotoWb = val;
}

bool ImportSettings::getToolTipsShowPhotoWB() const
{
    return d->tooltipShowPhotoWb;
}

void ImportSettings::setToolTipsShowTags(bool val)
{
    d->tooltipShowTags = val;
}

bool ImportSettings::getToolTipsShowTags() const
{
    return d->tooltipShowTags;
}

void ImportSettings::setToolTipsShowLabelRating(bool val)
{
    d->tooltipShowLabelRating = val;
}

bool ImportSettings::getToolTipsShowLabelRating() const
{
    return d->tooltipShowLabelRating;
}

bool ImportSettings::showToolTipsIsValid() const
{
    if (d->showToolTips)
    {
        if (
            d->tooltipShowFileName   ||
            d->tooltipShowFileDate   ||
            d->tooltipShowFileSize   ||
            d->tooltipShowImageType  ||
            d->tooltipShowImageDim   ||
            d->tooltipShowPhotoMake  ||
            d->tooltipShowPhotoLens  ||
            d->tooltipShowPhotoFocal ||
            d->tooltipShowPhotoExpo  ||
            d->tooltipShowPhotoFlash ||
            d->tooltipShowPhotoWb    ||
            d->tooltipShowFolderName ||
            d->tooltipShowLabelRating
/*
            || d->tooltipShowTags
*/
           )
        {
            return true;
        }
    }

    return false;
}

void ImportSettings::setShowThumbbar(bool val)
{
    d->showThumbbar = val;
}

bool ImportSettings::getShowThumbbar() const
{
    return d->showThumbbar;
}

void ImportSettings::setPreviewLoadFullImageSize(bool val)
{
    d->previewLoadFullImageSize = val;
}

bool ImportSettings::getPreviewLoadFullImageSize() const
{
    return d->previewLoadFullImageSize;
}

void ImportSettings::setPreviewItemsWhileDownload(bool val)
{
    d->previewItemsWhileDownload = val;
}

bool ImportSettings::getPreviewItemsWhileDownload() const
{
    return d->previewItemsWhileDownload;
}

void ImportSettings::setPreviewShowIcons(bool val)
{
    d->previewShowIcons = val;
}

bool ImportSettings::getPreviewShowIcons() const
{
    return d->previewShowIcons;
}

void ImportSettings::setIconShowCoordinates(bool val)
{
    d->iconShowCoordinates = val;
}

bool ImportSettings::getIconShowCoordinates() const
{
    return d->iconShowCoordinates;
}

} // namespace Digikam

#include "moc_importsettings.cpp"
