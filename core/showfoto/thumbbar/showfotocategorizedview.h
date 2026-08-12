/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Qt categorized item view for showfoto items
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "itemviewcategorized.h"
#include "showfotoiteminfo.h"

namespace Digikam
{
class ItemDelegateOverlay;
class ThumbnailSize;
};

namespace ShowFoto
{

class ShowfotoItemModel;
class ShowfotoDelegate;
class ShowfotoSortFilterModel;
class ShowfotoThumbnailModel;
class ShowfotoFilterModel;

class ShowfotoCategorizedView : public ItemViewCategorized
{
public:

    Q_OBJECT

public:

    explicit ShowfotoCategorizedView(QWidget* const parent = nullptr);
    ~ShowfotoCategorizedView() override;

    void setModels(ShowfotoItemModel* model, ShowfotoSortFilterModel* filterModel);

    ShowfotoItemModel*          showfotoItemModel()                     const;
    ShowfotoSortFilterModel*    showfotoSortFilterModel()               const;

    QItemSelectionModel*        getSelectionModel()                     const;

    /**
     * @return any ShowfotoFilterModel in chain. May not be sourceModel().
     */
    ShowfotoFilterModel*        showfotoFilterModel()                   const;

    /**
     * @return null if the ShowfotoItemModel is not an ShowfotoThumbnailModel.
     */
    ShowfotoThumbnailModel*     showfotoThumbnailModel()                const;

    ShowfotoDelegate*           showfotoDelegate()                      const;

    ShowfotoItemInfo            currentInfo()                           const;
    QUrl                        currentUrl()                            const;

    QList<ShowfotoItemInfo>     selectedShowfotoItemInfos()             const;
    QList<ShowfotoItemInfo>     selectedShowfotoItemInfosCurrentFirst() const;
    QList<QUrl>                 selectedUrls()                          const;

    QList<ShowfotoItemInfo>     showfotoItemInfos()                     const;
    QList<QUrl>                 urls()                                  const;

    /**
     * @brief selects the index as current and scrolls to it.
     */
    void toIndex(const QUrl& url);

    /**
     * @return the n-th info after the given one.
     * Specifically, return the previous info for nth = -1
     * and the next info for n = 1.
     *
     * @return a null info if either startingPoint or the nth info are
     * not contained in the model
     */
    ShowfotoItemInfo nextInOrder(const ShowfotoItemInfo& startingPoint, int nth);

    ShowfotoItemInfo previousInfo(const ShowfotoItemInfo& info);
    ShowfotoItemInfo nextInfo(const ShowfotoItemInfo& info);

    /**
     * @brief add and remove an overlay. It will as well be removed automatically when destroyed.
     * Unless you pass a different delegate, the current delegate will be used.
     */
    void addOverlay(ItemDelegateOverlay* overlay, ShowfotoDelegate* delegate = nullptr);
    void removeOverlay(ItemDelegateOverlay* overlay);

    /// @todo implement This
/*
    void addSelectionOverlay(ShowfotoDelegate* delegate = 0);
*/
    ThumbnailSize thumbnailSize()                                       const;

    virtual void setThumbnailSize(const ThumbnailSize& size);

public Q_SLOTS:

    void setThumbnailSize(int size);

    /**
     * @brief scroll the view to the given item when it becomes available.
     */
    void setCurrentWhenAvailable(qlonglong ShowfotoItemId);

    /**
     * @brief set as current item the item identified by its file url.
     */
    void setCurrentUrl(const QUrl& url);

    /**
     * @brief set as current item the item identified by the ShowfotoItemInfo
     */
    void setCurrentInfo(const ShowfotoItemInfo& info);

    /**
     * @brief set selected items identified by their file urls
     */
    void setSelectedUrls(const QList<QUrl>& urlList);

    /**
     * @brief set selected items
     */
    void setSelectedShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos);

    /**
     * @brief does something to gain attention for info, but not changing current selection
     */
    void hintAt(const ShowfotoItemInfo& info);

Q_SIGNALS:

    void currentInfoChanged(const ShowfotoItemInfo& info);

    /**
     * @brief emitted when new items are selected. The parameter includes only the newly selected infos,
     * there may be other already selected infos.
     */
    void selected(const QList<ShowfotoItemInfo>& newSelectedInfos);

    /**
     * @brief emitted when items are deselected. There may be other selected infos left.
     * This signal is not emitted when the model is reset; then only selectionCleared is emitted.
     */
    void deselected(const QList<ShowfotoItemInfo>& nowDeselectedInfos);

    /**
     * @brief emitted when the given ShowfotoItemInfo is activated. Info is never null.
     */
    void showfotoItemInfoActivated(const ShowfotoItemInfo& info);

    /**
     * @brief emitted when a new model is set.
     */
    void modelChanged();

protected:

    /// @note reimplemented from parent class.
    QSortFilterProxyModel*       filterModel()                                                                       const override;
    AbstractItemDragDropHandler* dragDropHandler()                                                                   const override;
    QModelIndex                  nextIndexHint(const QModelIndex& indexToAnchor, const QItemSelectionRange& removed) const override;

    void setItemDelegate(ShowfotoDelegate* delegate);
    void indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers)                                         override;
    void currentChanged(const QModelIndex& index, const QModelIndex& previous)                                             override;
    void selectionChanged(const QItemSelection&, const QItemSelection&)                                                    override;
    void updateGeometries()                                                                                                override;

    /// @note reimplement these in a subclass.
    virtual void activated(const ShowfotoItemInfo& info, Qt::KeyboardModifiers modifiers);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const ShowfotoItemInfo& info);
    void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index) override;

private Q_SLOTS:

    void slotFileChanged(const QString& filePath);
    void slotDelayedEnter();

private:

    void scrollToStoredItem();

private:

    /// @note disabled.
    ShowfotoCategorizedView(const ShowfotoCategorizedView&)            = delete;
    ShowfotoCategorizedView& operator=(const ShowfotoCategorizedView&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Showfoto
