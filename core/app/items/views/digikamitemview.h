/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt model-view for items
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_globals.h"
#include "applicationsettings.h"
#include "itemcategorizedview.h"
#include "itemviewutilities.h"
#include "groupingviewimplementation.h"

namespace Digikam
{

class ItemViewUtilities;
class ItemInfoList;

class DigikamItemView : public ItemCategorizedView,
                        public GroupingViewImplementation
{
    Q_OBJECT

public:

    enum SidebarViewMode
    {
        NormalView = 0,
        SearchView,
        FuzzySView,
        PeopleView
    };

public:

    explicit DigikamItemView(QWidget* const parent = nullptr);
    ~DigikamItemView()                                                                             override;

    ItemViewUtilities* utilities()                                                           const;

    int  fitToWidthIcons();
    bool isPeopleViewMode()                                                                  const;
    QList<int> getFaceIds(const QList<QModelIndex>& indexes)                                 const;

    void setThumbnailSize(const ThumbnailSize& size)                                               override;
    void setSidebarViewMode(int mode);

    ItemInfoList  allItemInfos(bool grouping = false)                                        const;
    ItemInfoList  selectedItemInfos(bool grouping = false)                                   const;
    ItemInfoList  selectedItemInfosCurrentFirst(bool grouping = false)                       const;
    bool          allNeedGroupResolving(const OperationType type)                            const;
    bool          selectedNeedGroupResolving(const OperationType type)                       const;

public Q_SLOTS:

    void openFile(const ItemInfo& info);

    void deleteSelected(const ItemViewUtilities::DeleteMode deleteMode = ItemViewUtilities::DeleteUseTrash);
    void deleteSelectedDirectly(const ItemViewUtilities::DeleteMode deleteMode = ItemViewUtilities::DeleteUseTrash);

    void rename();

    void assignRating(const QList<QModelIndex>& index, int rating);

    /**
     * @brief Confirm the face with a face tag (name) in the database.
     */
    void confirmFaces(const QList<QModelIndex>& indexes, int tagId);

    /**
     * @brief Removes the face from the database.
     * You will have to run face detection again,
     * to recover the face.
     */
    void removeFaces(const QList<QModelIndex>& indexes);

    /**
     * @brief Ignored Face back to Unknown face.
     */
    void unknownFaces(const QList<QModelIndex>& indexes);

    /**
     * @brief This slot is connected to the reject signal of
     * AssignNameOverlay, and handles two cases.
     *
     * If reject is done on an Unknown Face,
     * it will mark the face as Ignored.
     *
     * If reject is done on Unconfirmed suggestions,
     * the suggestion is rejected and the face is
     * marked as Unknown.
     */
    void rejectFaces(const QList<QModelIndex>& indexes);

    /**
     * @brief Set Face to Ignore ID.
     */
    void ignoreFaces(const QList<QModelIndex>& indexes);


    void dragDropSort(const ItemInfo& pick, const QList<ItemInfo>& infos);

Q_SIGNALS:

    void previewRequested(const ItemInfo& info);
    void signalSeparationModeChanged(int category);
    void signalShowContextMenu(QContextMenuEvent* event,
                               const QList<QAction*>& actions = QList<QAction*>());

    void signalShowContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info,
                                     const QList<QAction*>& actions,
                                     ItemFilterModel* filterModel);

    void signalShowGroupContextMenu(QContextMenuEvent* event,
                                    const QList<ItemInfo>& selectedInfos,
                                    ItemFilterModel* filterModel);

    void signalOpenGeolocationMap();

protected Q_SLOTS:

    void slotOpenGeolocationMap(const QModelIndex& index);
    void groupIndicatorClicked(const QModelIndex& index);
    void showGroupContextMenu(const QModelIndex& index, QContextMenuEvent* event);

protected:

    void addRejectionOverlay(ItemDelegate* delegate = nullptr);
    void addAssignNameOverlay(ItemDelegate* delegate = nullptr);

    void activated(const ItemInfo& info, Qt::KeyboardModifiers modifiers)                          override;
    void showContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info)                     override;
    void showContextMenu(QContextMenuEvent* event)                                                 override;
    void slotSetupChanged()                                                                        override;

    bool hasHiddenGroupedImages(const ItemInfo& info)                                        const override;

    ItemInfoList imageInfos(const QList<QModelIndex>& indexes,
                            OperationType type)                                              const;

private Q_SLOTS:

    void slotRotateAuto(const QList<QModelIndex>&);
    void slotRotateLeft(const QList<QModelIndex>&);
    void slotRotateRight(const QList<QModelIndex>&);
    void slotFullscreen(const QList<QModelIndex>&);
    void slotInitProgressIndicator();
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible)                       override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
