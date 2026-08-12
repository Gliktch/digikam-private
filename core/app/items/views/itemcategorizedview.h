/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-22
 * Description : Qt model-view for items
 *
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "iteminfo.h"
#include "itemviewcategorized.h"
#include "thumbnailsize.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class Album;
class ItemAlbumModel;
class ItemAlbumFilterModel;
class ItemModel;
class ItemFilterModel;
class ImageSortFilterModel;
class ItemDelegate;
class ItemDelegateOverlay;
class ItemThumbnailModel;

class ItemCategorizedView : public ItemViewCategorized
{
    Q_OBJECT

public:

    explicit ItemCategorizedView(QWidget* const parent = nullptr);
    ~ItemCategorizedView()                                                                   override;

    void setModels(ItemModel* model, ImageSortFilterModel* filterModel);

    ItemModel*               itemModel()                                               const;
    ImageSortFilterModel*    itemSortFilterModel()                                     const;

    QItemSelectionModel*     getSelectionModel()                                       const;

    /// @return Any ItemFilterMode in chain. May not be sourceModel()
    virtual ItemFilterModel* itemFilterModel()                                         const;

    /// @return 0 if the ItemModel is not an ItemThumbnailModel
    ItemThumbnailModel*      itemThumbnailModel()                                      const;

    /// @return 0 if the ItemModel is not an ItemAlbumModel
    ItemAlbumModel*          itemAlbumModel()                                          const;
    ItemAlbumFilterModel*    itemAlbumFilterModel()                                    const;

    ItemDelegate*            itemDelegate()                                            const;

    Album*                   currentAlbum()                                            const;
    ItemInfo                 currentInfo()                                             const;
    QUrl                     currentUrl()                                              const;

    ItemInfoList             allItemInfos()                                            const;
    QList<QUrl>              allUrls()                                                 const;
    ItemInfoList             selectedItemInfos()                                       const;
    ItemInfoList             selectedItemInfosCurrentFirst()                           const;

    /**
     * @brief Selects the index as current and scrolls to it.
     */
    void toIndex(const QUrl& url);

    /**
     * @return The n-th info after the given one.
     * Specifically, return the previous info for nth = -1
     * and the next info for n = 1.
     * A null info is returned if either startingPoint or the nth info are
     * not contained in the model.
     */
    ItemInfo nextInOrder(const ItemInfo& startingPoint, int nth);

    ItemInfo previousInfo(const ItemInfo& info)
    {
        return nextInOrder(info, -1);
    }

    ItemInfo nextInfo(const ItemInfo& info)
    {
        return nextInOrder(info, 1);
    }

    QModelIndex indexForInfo(const ItemInfo& info)                                  const;

    ThumbnailSize thumbnailSize()                                                   const;

    virtual void setThumbnailSize(const ThumbnailSize& size);

    /**
     * @brief If the model is categorized by an album, returns the album of the category
     * that contains the position.
     * If this is not applicable, return the current album. May return 0.
     */
    Album* albumAt(const QPoint& pos)                                               const;

    /**
     * @brief Add and remove an overlay. It will as well be removed automatically when destroyed.
     * Unless you pass a different delegate, the current delegate will be used.
     */
    void addOverlay(ItemDelegateOverlay* overlay, ItemDelegate* delegate = nullptr);

    void removeOverlay(ItemDelegateOverlay* overlay);

    void addSelectionOverlay(ItemDelegate* delegate = nullptr);

public Q_SLOTS:

    void openAlbum(const QList<Album*>& album);

    void setThumbnailSize(int size);

    /**
     * @brief Scroll the view to the given item when it becomes available.
     */
    void setCurrentWhenAvailable(qlonglong imageId);

    /**
     * @brief Set as current item when it becomes available, the item identified by its file url.
     */
    void setCurrentUrlWhenAvailable(const QUrl& url);

    /**
     * @brief Set as current item the item identified by its file url.
     */
    void setCurrentUrl(const QUrl& url);

    /**
     * @brief Set as current item the item identified by the imageinfo.
     */
    void setCurrentInfo(const ItemInfo& info);

    /**
     * @brief Set selected items identified by their file urls.
     */
    void setSelectedUrls(const QList<QUrl>& urlList);

    /**
     * @brief Set selected items.
     */
    void setSelectedItemInfos(const QList<ItemInfo>& infos);

    /**
     * @brief Does something to gain attention for info, but not changing current selection.
     */
    void hintAt(const ItemInfo& info);

Q_SIGNALS:

    void currentInfoChanged(const ItemInfo& info);

    /**
     * @brief Emitted when new items are selected. The parameter includes only the newly selected infos,
     * there may be other already selected infos.
     */
    void selected(const QList<ItemInfo>& newSelectedInfos);

    /**
     * @brief Emitted when items are deselected. There may be other selected infos left.
     * This signal is not emitted when the model is reset; then only selectionCleared is emitted.
     */
    void deselected(const QList<ItemInfo>& nowDeselectedInfos);

    /// @brief Emitted when the given image is activated. Info is never null.
    void imageActivated(const ItemInfo& info);

    /// @brief Emitted when a new model is set
    void modelChanged();

protected Q_SLOTS:

    void slotItemInfosAdded();
    void slotCurrentUrlTimer();

protected:

    /// @brief Install default ItemAlbumModel and filter model, ready for use
    void installDefaultModels();

    // reimplemented from parent class

    QSortFilterProxyModel*       filterModel()                                      const override;
    AbstractItemDragDropHandler* dragDropHandler()                                  const override;
    QModelIndex                  nextIndexHint(const QModelIndex& indexToAnchor,
                                               const QItemSelectionRange& removed)  const override;

    void setItemDelegate(ItemDelegate* delegate);
    void indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers)        override;
    void currentChanged(const QModelIndex& index, const QModelIndex& previous)            override;
    void selectionChanged(const QItemSelection&, const QItemSelection&)                   override;
    void updateGeometries()                                                               override;

    /// Reimplement these in a subclass
    virtual void activated(const ItemInfo& info, Qt::KeyboardModifiers modifiers);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info);
    void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)       override;

    ItemInfo     imageInfo(const QModelIndex& index)                                const;
    ItemInfoList imageInfos(const QList<QModelIndex>& indexes)                      const;

private Q_SLOTS:

    void slotIccSettingsChanged(const ICCSettingsContainer&, const ICCSettingsContainer&);
    void slotFileChanged(const QString& filePath);
    void slotScrollToStoredItem();
    void slotDelayedEnter();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
