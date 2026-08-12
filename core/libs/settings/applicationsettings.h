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
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFont>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"
#include "digikam_globals.h"
#include "facescansettings.h"
#include "haariface.h"
#include "sidebar.h"


class KConfigGroup;

namespace Digikam
{

class VersionManagerSettings;
class DbEngineParameters;
class PreviewSettings;

class DIGIKAM_GUI_EXPORT ApplicationSettings : public QObject
{
    Q_OBJECT

public:

    enum AlbumSortRole
    {
        ByFolder = 0,
        ByCategory,
        ByDate
    };

    enum ItemLeftClickAction
    {
        ShowPreview = 0,
        StartEditor,
        ShowOnTable,
        OpenDefault,
    };

    /**
     * Possible ways of comparing strings.
     */
    enum StringComparisonType
    {
        /**
         * Natural compare using KStringHandler::naturalCompare.
         */
        Natural = 0,

        /**
         * Normal comparison using Qt's compare function.
         */
        Normal
    };

    enum ApplyToEntireGroup
    {
        No = 0,
        Yes,
        Ask
    };

    typedef QHash<OperationType, QString> OperationStrings;
    typedef QHash<OperationType, ApplicationSettings::ApplyToEntireGroup> OperationModes;

Q_SIGNALS:

    void setupChanged();
    void recurseSettingsChanged();
    void balooSettingsChanged();

public:

    ApplicationSettings();
    ~ApplicationSettings() override;

    static ApplicationSettings* instance();

    void readSettings();
    void saveSettings();
    void emitSetupChanged();

    QString generalConfigGroupName() const;

    // -- MessageBox Notification ---------------------------------------------------

    /**
     * @return true if the corresponding message box should be shown.
     * @param dontShowAgainName the name that identify the message box.
     */
    bool readMsgBoxShouldBeShown(const QString& dontShowAgainName);

    /**
     * Save the fact that the message box should not be shown again.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, this method does nothing.
     */
    void saveMsgBoxShouldBeShown(const QString& dontShowAgainName);

    // -- Database Settings ---------------------------------------------------------

    DbEngineParameters getDbEngineParameters() const;
    void setDbEngineParameters(const DbEngineParameters& params);

    void setSyncBalooToDigikam(bool val);
    bool getSyncBalooToDigikam() const;

    void setSyncDigikamToBaloo(bool val);
    bool getSyncDigikamToBaloo() const;

    // -- Albums Settings -------------------------------------------------------

    void setTreeViewIconSize(int val);
    int  getTreeViewIconSize() const;

    void setTreeViewFaceSize(int val);
    int  getTreeViewFaceSize() const;

    void setTreeViewFont(const QFont& font);
    QFont getTreeViewFont() const;

    void setAlbumSortRole(const AlbumSortRole role);
    AlbumSortRole getAlbumSortRole() const;

    void setShowFolderTreeViewItemsCount(bool val);
    bool getShowFolderTreeViewItemsCount() const;

    void setAlbumMonitoring(bool val);
    bool getAlbumMonitoring() const;

    void setRecurseAlbums(bool val);
    bool getRecurseAlbums() const;

    void setRecurseTags(bool val);
    bool getRecurseTags() const;

    void setAllGroupsOpen(bool val);
    bool getAllGroupsOpen() const;

    void setAlbumCategoryNames(const QStringList& list);
    QStringList getAlbumCategoryNames() const;

    bool addAlbumCategoryName(const QString& name) const;
    bool delAlbumCategoryName(const QString& name) const;

    // -- Views Settings -------------------------------------------------------

    void setDefaultIconSize(int val);
    int  getDefaultIconSize() const;

    void setIconViewFont(const QFont& font);
    QFont getIconViewFont() const;

    void setImageSortOrder(int order);
    int  getImageSortOrder() const;

    /**
     * means ascending or descending
     */
    void setImageSorting(int sorting);
    int  getImageSorting() const;

    void setImageSeparationMode(int mode);
    int  getImageSeparationMode() const;

    void setImageSeparationSortOrder(int order);
    int  getImageSeparationSortOrder() const;

    void setItemLeftClickAction(int action);
    int  getItemLeftClickAction() const;

    void setIconShowName(bool val);
    bool getIconShowName() const;

    void setIconShowSize(bool val);
    bool getIconShowSize() const;

    void setIconShowTitle(bool val);
    bool getIconShowTitle() const;

    void setIconShowComments(bool val);
    bool getIconShowComments() const;

    void setIconShowResolution(bool val);
    bool getIconShowResolution() const;

    void setIconShowAspectRatio(bool val);
    bool getIconShowAspectRatio() const;

    void setIconShowTags(bool val);
    bool getIconShowTags() const;

    void setIconShowDate(bool val);
    bool getIconShowDate() const;

    void setIconShowModDate(bool val);
    bool getIconShowModDate() const;

    void setIconShowRating(bool val);
    bool getIconShowRating() const;

    void setIconShowPickLabel(bool val);
    bool getIconShowPickLabel() const;

    void setIconShowColorLabel(bool val);
    bool getIconShowColorLabel() const;

    void setIconShowImageFormat(bool val);
    bool getIconShowImageFormat() const;

    void setIconShowCoordinates(bool val);
    bool getIconShowCoordinates() const;

    /**
     * Sets the visibility of the overlay buttons on the image icons.
     */
    void setIconShowOverlays(bool val);

    /**
     * Determines whether the overlay buttons should be displayed on the icons.
     */
    bool getIconShowOverlays() const;

    void setIconShowFullscreen(bool val);
    bool getIconShowFullscreen() const;

    void setPreviewSettings(const PreviewSettings& settings);
    PreviewSettings getPreviewSettings() const;

    void setMagnifierZoomFactor(qreal factor);
    qreal getMagnifierZoomFactor() const;

    void setMagnifierSize(int size);
    int getMagnifierSize() const;

    void setPreviewShowIcons(bool val);
    bool getPreviewShowIcons() const;

    void setPreviewAutoPlay(bool val);
    bool getPreviewAutoPlay() const;

    void setScaleFitToWindow(bool val);
    bool getScaleFitToWindow() const;

    void setPreviewSmoothScaled(bool val);
    bool getPreviewSmoothScaled() const;

    /**
     * Sets the visibility of the text properties overlay over the image and video preview.
     */
    void setPreviewOverlay(bool val);
    bool getPreviewOverlay() const;

    // -- Mime-Types Settings -------------------------------------------------------

    QString getImageFileFilter() const;
    QString getMovieFileFilter() const;
    QString getAudioFileFilter() const;
    QString getRawFileFilter()   const;
    QString getAllFileFilter()   const;

    void addToImageFileFilter(const QString& extensions);

    // -- Tool-Tips Settings -------------------------------------------------------

    bool showToolTipsIsValid()      const;
    bool showAlbumToolTipsIsValid() const;

    void setToolTipsFont(const QFont& font);
    QFont getToolTipsFont() const;

    void setShowToolTips(bool val);
    bool getShowToolTips() const;

    void setToolTipsShowFileName(bool val);
    bool getToolTipsShowFileName() const;

    void setToolTipsShowFileDate(bool val);
    bool getToolTipsShowFileDate() const;

    void setToolTipsShowFileSize(bool val);
    bool getToolTipsShowFileSize() const;

    void setToolTipsShowImageType(bool val);
    bool getToolTipsShowImageType() const;

    void setToolTipsShowImageDim(bool val);
    bool getToolTipsShowImageDim() const;

    void setToolTipsShowImageAR(bool val);
    bool getToolTipsShowImageAR() const;

    void setToolTipsShowPhotoMake(bool val);
    bool getToolTipsShowPhotoMake() const;

    void setToolTipsShowPhotoLens(bool val);
    bool getToolTipsShowPhotoLens() const;

    void setToolTipsShowPhotoDate(bool val);
    bool getToolTipsShowPhotoDate() const;

    void setToolTipsShowPhotoFocal(bool val);
    bool getToolTipsShowPhotoFocal() const;

    void setToolTipsShowPhotoExpo(bool val);
    bool getToolTipsShowPhotoExpo() const;

    void setToolTipsShowPhotoMode(bool val);
    bool getToolTipsShowPhotoMode() const;

    void setToolTipsShowPhotoFlash(bool val);
    bool getToolTipsShowPhotoFlash() const;

    void setToolTipsShowPhotoWB(bool val);
    bool getToolTipsShowPhotoWB() const;

    void setToolTipsShowAlbumName(bool val);
    bool getToolTipsShowAlbumName() const;

    void setToolTipsShowTitles(bool val);
    bool getToolTipsShowTitles() const;

    void setToolTipsShowComments(bool val);
    bool getToolTipsShowComments() const;

    void setToolTipsShowTags(bool val);
    bool getToolTipsShowTags() const;

    void setToolTipsShowLabelRating(bool val);
    bool getToolTipsShowLabelRating() const;

    void setShowAlbumToolTips(bool val);
    bool getShowAlbumToolTips() const;

    void setToolTipsShowAlbumTitle(bool val);
    bool getToolTipsShowAlbumTitle() const;

    void setToolTipsShowAlbumDate(bool val);
    bool getToolTipsShowAlbumDate() const;

    void setToolTipsShowAlbumCollection(bool val);
    bool getToolTipsShowAlbumCollection() const;

    void setToolTipsShowAlbumCategory(bool val);
    bool getToolTipsShowAlbumCategory() const;

    void setToolTipsShowAlbumCaption(bool val);
    bool getToolTipsShowAlbumCaption() const;

    void setToolTipsShowAlbumPreview(bool val);
    bool getToolTipsShowAlbumPreview() const;

    void setToolTipsShowVideoAspectRatio(bool val);
    bool getToolTipsShowVideoAspectRatio() const;

    void setToolTipsShowVideoAudioBitRate(bool val);
    bool getToolTipsShowVideoAudioBitRate() const;

    void setToolTipsShowVideoAudioChannelType(bool val);
    bool getToolTipsShowVideoAudioChannelType() const;

    void setToolTipsShowVideoAudioCodec(bool val);
    bool getToolTipsShowVideoAudioCodec() const;

    void setToolTipsShowVideoDuration(bool val);
    bool getToolTipsShowVideoDuration() const;

    void setToolTipsShowVideoFrameRate(bool val);
    bool getToolTipsShowVideoFrameRate() const;

    void setToolTipsShowVideoVideoCodec(bool val);
    bool getToolTipsShowVideoVideoCodec() const;

    // -- Miscs Settings -------------------------------------------------------

    void setScanAtStart(bool val);
    bool getScanAtStart() const;

    void setCleanAtStart(bool val);
    bool getCleanAtStart() const;

    void setDatabaseDirSetAtCmd(bool val);
    bool getDatabaseDirSetAtCmd() const;

    void setUseTrash(bool val);
    bool getUseTrash() const;

    void setShowTrashDeleteDialog(bool val);
    bool getShowTrashDeleteDialog() const;

    void setShowPermanentDeleteDialog(bool val);
    bool getShowPermanentDeleteDialog() const;

    void setApplySidebarChangesDirectly(bool val);
    bool getApplySidebarChangesDirectly() const;

    void setUseNativeFileDialog(bool val);
    bool getUseNativeFileDialog() const;

    void setDrawFramesToGrouped(bool val);
    bool getDrawFramesToGrouped() const;

    void setSelectFirstAlbumItem(bool val);
    bool getSelectFirstAlbumItem() const;

    void setExpandNewCurrentItem(bool val);
    bool getExpandNewCurrentItem() const;

    void setScrollItemToCenter(bool val);
    bool getScrollItemToCenter() const;

    void setShowOnlyPersonTagsInPeopleSidebar(bool val);
    bool showOnlyPersonTagsInPeopleSidebar() const;

    void setDetectFacesInNewImages(bool val);
    bool getDetectFacesInNewImages() const;

    void setFaceRecognitionBackgroundScan(bool val);
    bool getFaceRecognitionBackgroundScan() const;

    /**
     * Defines the way in which string comparisons are performed.
     *
     * @param val new way to compare strings
     */
    void setStringComparisonType(ApplicationSettings::StringComparisonType val);

    /**
     * Tells in which way strings are compared at the moment.
     *
     * @return string comparison type to use.
     */
    StringComparisonType getStringComparisonType() const;

    bool isStringTypeNatural() const;

    void setApplicationStyle(const QString& style);
    QString getApplicationStyle() const;

    void setIconTheme(const QString& theme);
    QString getIconTheme() const;

    void setDateTimeFormat(const QString& fmt);
    QString getDateTimeFormat() const;

    void setApplicationFont(const QFont& fnt);
    QFont getApplicationFont() const;

    void setShowSplashScreen(bool val);
    bool getShowSplashScreen() const;

    void setCurrentTheme(const QString& theme);
    QString getCurrentTheme() const;

    void setSidebarTitleStyle(DMultiTabBar::TextStyle style);
    DMultiTabBar::TextStyle getSidebarTitleStyle() const;

    int getUpdateType() const;
    void setUpdateType(int type);

    bool getUpdateWithDebug() const;
    void setUpdateWithDebug(bool dbg);

    void setVersionManagerSettings(const VersionManagerSettings& settings);
    VersionManagerSettings getVersionManagerSettings() const;

    void setShowThumbbar(bool val);
    bool getShowThumbbar() const;

    void setRatingFilterCond(int val);
    int  getRatingFilterCond() const;

    void setMinimumSimilarityBound(int val);
    int  getMinimumSimilarityBound() const;

    void setDuplicatesSearchLastMinSimilarity(int val);
    int  getDuplicatesSearchLastMinSimilarity() const;

    void setDuplicatesSearchLastMaxSimilarity(int val);
    int  getDuplicatesSearchLastMaxSimilarity() const;

    /**
     * Defines whether an operation should be performed on all grouped items
     * or just the head item.
     *
     * @param type Operation to be performed
     * @param applyAll Whether to apply to all images or just one, or ask
     */
    void setGroupingOperateOnAll(OperationType type, ApplyToEntireGroup applyAll);

    /**
     * Tells whether an operation should be performed on all grouped items
     * or just the head item.
     *
     * @param type Operation to be performed
     * @return Whether to apply to all images or just one, or ask
     */
    ApplyToEntireGroup getGroupingOperateOnAll(OperationType type) const;

    /**
     * Asks the user whether the operation should be performed on all grouped
     * images or just the first. Also supplies an option to remember the answer.
     *
     * @param type Operation to be performed
     * @return Whether to apply to all images or just one
     */
    bool askGroupingOperateOnAll(OperationType type);

    /**
     * Gives the translated title/short explanation of the operation
     *
     * @param type Operation to be performed
     * @return Translated operation title/short explanation
     */
    static QString operationTypeTitle(OperationType type);

    /**
     * Gives a translated explanation of the operation and an empty string,
     * if there is none (e.g. for tooltips)
     *
     * @param type Operation to be performed
     * @return Translated operation explanation
     */
    static QString operationTypeExplanation(OperationType type);

    void setDuplicatesAlbumTagRelation(int val);
    int  getDuplicatesAlbumTagRelation() const;

    void setDuplicatesReferenceImageSelectionMethod(HaarIface::RefImageSelMethod val);
    HaarIface::RefImageSelMethod getDuplicatesRefImageSelMethod() const;

    void setDuplicatesSearchRestrictions(int val);
    int  getDuplicatesSearchRestrictions() const;

    void setHelpBoxNotificationSeen(bool val);
    bool getHelpBoxNotificationSeen();

private Q_SLOTS:

    void applyBalooSettings();

private:

    /// @note disabledd
    explicit ApplicationSettings(QObject*) = delete;

    KConfigGroup generalConfigGroup() const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
