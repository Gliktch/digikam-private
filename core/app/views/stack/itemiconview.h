/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>
#include <QUrl>

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "applicationsettings.h"
#include "metaengine_rotation.h"
#include "searchtextbar.h"
#include "iteminfo.h"
#include "dmodelfactory.h"
#include "sidebarwidget.h"
#include "stackedview.h"
#include "dlayoutbox.h"

namespace Digikam
{

class AlbumIconItem;
class Album;
class PAlbum;
class TAlbum;
class BatchSyncMetadata;
class FilterStatusBar;
class DCategorizedView;
class ItemFilterModel;

class ItemIconView : public DHBox
{
    Q_OBJECT

public:

    explicit ItemIconView(QWidget* const parent,
                          DModelFactory* const modelCollection);
    ~ItemIconView() override;

    void applySettings();

private:

    void setupConnections();
    void loadViewState();
    void saveViewState();

    // ----------------------------------------------------------------------------------------

    //@{
    /// Tools methods (Editor, BQM, Light Table) - itemiconview_tools.cpp.

public Q_SLOTS:

    void slotImageEdit();
    void slotEditor();
    void slotLightTable();
    void slotSurvey();
    void slotQueueMgr();
    void slotFileWithDefaultApplication();
    void slotImageLightTable();
    void slotImageAddToLightTable();
    void slotImageAddToCurrentQueue();
    void slotImageAddToNewQueue();
    void slotImageAddToExistingQueue(int);

    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Zoom management methods - itemiconview_zoom.cpp

public:

    double zoomMin()                                                               const;
    double zoomMax()                                                               const;
    void setThumbSize(int size);

Q_SIGNALS:

    void signalZoomChanged(double);
    void signalThumbSizeChanged(int);

private:

    void toggleZoomActions();

public Q_SLOTS:

    void setZoomFactor(double zoom);
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomTo100Percents();
    void slotFitToWindow();

private Q_SLOTS:

    void slotZoomFactorChanged(double);
    void slotThumbSizeEffect();

    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Side-bars handling methods - itemiconview_sidebars.cpp

    // ----------------------------------------------------------------------------------------

    //@{
    /// Tools methods (Editor, BQM, Light Table) - itemiconview_tools.cpp.

public Q_SLOTS:

    void slotImageQualitySorter();

    //@}

    // ----------------------------------------------------------------------------------------

public:

    QList<SidebarWidget*> leftSidebarWidgets()                                     const;

    void showSideBars();
    void hideSideBars();
    void toggleLeftSidebar();
    void toggleRightSidebar();
    void previousLeftSideBarTab();
    void nextLeftSideBarTab();
    void previousRightSideBarTab();
    void nextRightSideBarTab();

public Q_SLOTS:

    void slotNotificationError(const QString& message, int type);

    void slotLeftSideBarActivate(QWidget* widget);
    void slotLeftSideBarActivate(SidebarWidget* widget);
    void slotLeftSideBarActivateAlbums();
    void slotLeftSideBarActivateTags();

    void slotRightSideBarActivateTitles();
    void slotRightSideBarActivateComments();
    void slotRightSideBarActivateAssignedTags();

Q_SIGNALS:

    void signalChangedTab(QWidget*);
    void signalFuzzySidebarActive(bool active);

private Q_SLOTS:

    void slotLeftSidebarChangedTab(QWidget* w);
    void slotSidebarTabTitleStyleChanged();
    void slotLeftSideBarEnabled(bool busy);
    void slotPopupFiltersView();
    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Item Group methods - itemiconview_groups.cpp

public:

    /**
     * Query whether the operation to be performed on currently selected or all
     * all items in the currently active view should be performed on all
     * grouped items or just the first.
     */
    bool allNeedGroupResolving(const OperationType type)      const;
    bool selectedNeedGroupResolving(const OperationType type) const;
    void setAllGroupsOpen(bool open);

public Q_SLOTS:

    void slotCreateGroupFromSelection();
    void slotCreateGroupByTimeFromSelection();
    void slotCreateGroupByFilenameFromSelection();
    void slotCreateGroupByTimelapseFromSelection();
    void slotRemoveSelectedFromGroup();
    void slotUngroupSelected();

private Q_SLOTS:

    void slotEmptyMessageTimer();
    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Album management methods - itemiconview_album.cpp

public:

    Album* currentAlbum()                                                          const;
    void setRecurseAlbums(bool recursive);
    void clearHistory();
    void getForwardHistory(QStringList& titles);
    void getBackwardHistory(QStringList& titles);

public Q_SLOTS:

    void slotRefresh();
    void slotNewAlbum();
    void slotSortAlbums(int role);
    void slotDeleteAlbum();
    void slotRenameAlbum();
    void slotAlbumPropsEdit();
    void slotAlbumOpenInFileManager();
    void slotAlbumHistoryBack(int steps = 1);
    void slotAlbumHistoryForward(int steps = 1);
    void slotAlbumWriteMetadata();
    void slotAlbumReadMetadata();
    void slotAlbumSelected(const QList<Album*>& albums);

    void slotGotoAlbumAndItem(const ItemInfo& imageInfo);
    void slotGotoDateAndItem(const ItemInfo& imageInfo);
    void slotGotoTagAndItem(int tagID);

    void slotSelectAlbum(const QUrl& url);
    void slotSetCurrentUrlWhenAvailable(const QUrl& url);
    void slotSetCurrentWhenAvailable(const qlonglong id);

    void slotSetAsAlbumThumbnail(const ItemInfo& info);
    void slotMoveSelectionToAlbum();
    void slotCopySelectionTo();

Q_SIGNALS:

    void signalAlbumSelected(Album*);

private:

    void changeAlbumFromHistory(const QList<Album*>& album, QWidget* const widget);

private Q_SLOTS:

    void slotAllAlbumsLoaded();
    void slotAlbumsCleared();
    void slotAlbumRefreshComplete();
    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Search management methods - itemiconview_search.cpp

public Q_SLOTS:

    void slotNewKeywordSearch();
    void slotNewAdvancedSearch();
    void slotNewDuplicatesSearch(const QList<PAlbum*>& albums = {});
    void slotNewDuplicatesSearch(const QList<TAlbum*>& albums);
    void slotImageFindSimilar();
    void slotImageScanForFaces();
    void slotImageRecognizeFaces();
    void slotImageRemoveAllFaces();
    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Tags management methods - itemiconview_tags.cpp

public:

    void setRecurseTags(bool recursive);
    void toggleTag(int tagID);

public Q_SLOTS:

    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();
    void slotOpenTagsManager();
    void slotAssignTag();
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating, bool toggle = true);

private Q_SLOTS:

    //void slotRatingChanged(const QUrl&, int);
    //void slotColorLabelChanged(const QUrl&, int);
    //void slotPickLabelChanged(const QUrl&, int);
    //void slotToggleTag(const QUrl&, int);

    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Items management methods - itemiconview_items.cpp

public:

    int       itemCount()                                                          const;
    QUrl      currentUrl()                                                         const;
    bool      hasCurrentItem()                                                     const;
    ItemInfo  currentInfo()                                                        const;

    /**
     * Get currently selected items. By default only the first images in groups are
     * given, while all can be obtained by setting the grouping parameter to true.
     * Given an operation, it will be determined from settings/user query whether
     * only the first or all items in a group are returned.
     * Ideally only the latter (giving an operation) is used.
     */
    QList<QUrl>  selectedUrls(bool grouping = false)                               const;
    QList<QUrl>  selectedUrls(const OperationType type)       const;
    ItemInfoList selectedInfoList(const bool currentFirst = false,
                                   const bool grouping = false)                    const;
    ItemInfoList selectedInfoList(const OperationType type,
                                   const bool currentFirst = false)                const;
    /**
     * Get all items in the current view.
     * Whether only the first or all grouped items are returned is determined
     * as described above.
     */
    QList<QUrl>  allUrls(bool grouping = false)                                    const;
    ItemInfoList allInfo(const bool grouping = false)                              const;
    ItemInfoList allInfo(const OperationType type)            const;

    void imageTransform(MetaEngineRotation::TransformationAction transform);

Q_SIGNALS:

    void signalNoCurrentItem();
    void signalSeparationModeChanged(int category);
    void signalSelectionChanged(int numberOfSelectedItems);
    void signalTrashSelectionChanged(const QString& text);
    void signalImageSelected(const ItemInfoList& selectedImage,
                             const ItemInfoList& allImages);

public Q_SLOTS:

    void slotImagePreview();
    void slotMapWidgetView();
    void slotTableView();
    void slotIconView();
    void slotImageExifOrientation(int orientation);
    void slotImageRename();
    void slotImageDelete();
    void slotImageDeletePermanently();
    void slotImageDeletePermanentlyDirectly();
    void slotImageTrashDirectly();
    void slotImageWriteMetadata();
    void slotImageReadMetadata();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSortImages(int order);
    void slotSortImagesOrder(int order);
    void slotSeparateImages(int mode);
    void slotImageSeparationSortOrder(int order);
    void slotImagePaste();
    void slotFocusAndNextImage();

private Q_SLOTS:

    void slotFirstItem();
    void slotPrevItem();
    void slotNextItem();
    void slotLastItem();
    void slotSelectItemByUrl(const QUrl&);
    void slotAwayFromSelection();
    void slotImageSelected();
    void slotDispatchImageSelected();
    void slotImageChangeFailed(const QString& message, const QStringList& fileNames);
    //@}

    // ----------------------------------------------------------------------------------------

    //@{
    /// Views management methods - itemiconview_views.cpp

public:

    void setHostWindowActions(const HostActionsMap& actions);
    void setToolsIconView(DCategorizedView* const view);
    void toggleShowBar(bool);
    void toggleFullScreen(bool set);
    void refreshView();
    void connectIconViewFilter(FilterStatusBar* const filter);
    StackedView::StackedViewMode viewMode()                                        const;

Q_SIGNALS:

    void signalSwitchedToPreview();
    void signalSwitchedToIconView();
    void signalSwitchedToMapView();
    void signalSwitchedToTableView();
    void signalSwitchedToTrashView();

    void signalGotoAlbumAndItem(const ItemInfo&);
    void signalGotoDateAndItem(AlbumIconItem*);
    void signalGotoTagAndItem(int tagID);

    void signalStartedLoading();
    void signalLoadingProgress(float progress);
    void signalLoadingComplete();

private Q_SLOTS:

    void slotTogglePreviewMode(const ItemInfo& info);
    void slotEscapePreview();
    void slotRefreshImagePreview();
    void slotViewModeChanged();
    void slotSetupMetadataFilters(int);
    void slotSetupExifTool();

    void slotShowContextMenu(QContextMenuEvent* event,
                             const QList<QAction*>& extraGroupingActions = QList<QAction*>());

    void slotShowContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info,
                                   const QList<QAction*>& extraGroupingActions = QList<QAction*>(),
                                   ItemFilterModel* imageFilterModel = nullptr);

    void slotShowGroupContextMenu(QContextMenuEvent* event,
                                  const QList<ItemInfo>& selectedInfos,
                                  ItemFilterModel* imageFilterModel = nullptr);
    //@}

    // ----------------------------------------------------------------------------------------

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
