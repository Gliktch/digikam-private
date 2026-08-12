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

#include "itemcategorizedview.h"

// Qt includes

#include <QApplication>
#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "coredbfields.h"
#include "iccsettings.h"
#include "itemalbummodel.h"
#include "itemalbumfiltermodel.h"
#include "itemcategorydrawer.h"
#include "itemdelegate.h"
#include "itemdelegateoverlay.h"
#include "itemthumbnailmodel.h"
#include "itemselectionoverlay.h"
#include "itemviewtooltip.h"
#include "loadingcacheinterface.h"
#include "thumbnailloadthread.h"
#include "tooltipfiller.h"
#include "itemfacedelegate.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemCategorizedViewToolTip : public ItemViewToolTip
{
    Q_OBJECT

public:

    explicit ItemCategorizedViewToolTip(ItemCategorizedView* const view)
        : ItemViewToolTip(view)
    {
    }

    ItemCategorizedView* view() const override
    {
        return static_cast<ItemCategorizedView*>(ItemViewToolTip::view());
    }

protected:

    QString tipContents() override
    {
        ItemInfo info = ItemModel::retrieveItemInfo(currentIndex());

        return ToolTipFiller::imageInfoTipContents(info);
    }
};

// -------------------------------------------------------------------------------

class Q_DECL_HIDDEN ItemCategorizedView::Private
{
public:

    Private() = default;

public:

    ItemModel*            model             = nullptr;
    ImageSortFilterModel* filterModel       = nullptr;

    ItemDelegate*         delegate          = nullptr;
    bool                  showToolTip       = false;

    qlonglong             scrollToItemId    = 0;

    QUrl                  unknownCurrentUrl;

    QTimer*               delayedEnterTimer = nullptr;
};

// -------------------------------------------------------------------------------

ItemCategorizedView::ItemCategorizedView(QWidget* const parent)
    : ItemViewCategorized(parent),
      d                  (new Private)
{
    setToolTip(new ItemCategorizedViewToolTip(this));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(QString)));

    connect(IccSettings::instance(), SIGNAL(signalICCSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)),
            this, SLOT(slotIccSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)));

    d->delayedEnterTimer = new QTimer(this);
    d->delayedEnterTimer->setInterval(10);
    d->delayedEnterTimer->setSingleShot(true);

    connect(d->delayedEnterTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedEnter()));
}

ItemCategorizedView::~ItemCategorizedView()
{
    d->delegate->removeAllOverlays();

    delete d;
}

void ItemCategorizedView::installDefaultModels()
{
    ItemAlbumModel* const aModel       = new ItemAlbumModel(this);
    ItemAlbumFilterModel* const fModel = new ItemAlbumFilterModel(this);

    fModel->setSourceItemModel(aModel);

    fModel->setSortRole(ItemSortSettings::SortByFileName);
    fModel->setCategorizationMode(ItemSortSettings::CategoryByAlbum);
    fModel->sort(0); // an initial sorting is necessary

    // set flags that we want to get dataChanged() signals for

    aModel->setWatchFlags(fModel->suggestedWatchFlags());

    setModels(aModel, fModel);
}

void ItemCategorizedView::setModels(ItemModel* model, ImageSortFilterModel* fModel)
{
    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(false);
    }

    if (d->filterModel)
    {
        disconnect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
                   this, SLOT(layoutAboutToBeChanged()));

        disconnect(d->filterModel, SIGNAL(layoutChanged()),
                   this, SLOT(layoutWasChanged()));
    }

    if (d->model)
    {
        disconnect(d->model, SIGNAL(imageInfosAdded(QList<ItemInfo>)),
                   this, SLOT(slotItemInfosAdded()));
    }

    d->model       = model;
    d->filterModel = fModel;

    setModel(d->filterModel);

    connect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(d->filterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutWasChanged()),
            Qt::QueuedConnection);

    connect(d->model, SIGNAL(imageInfosAdded(QList<ItemInfo>)),
            this, SLOT(slotItemInfosAdded()));

    Q_EMIT modelChanged();

    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(true);
    }
}

ItemModel* ItemCategorizedView::itemModel() const
{
    return d->model;
}

ImageSortFilterModel* ItemCategorizedView::itemSortFilterModel() const
{
    return d->filterModel;
}

ItemFilterModel* ItemCategorizedView::itemFilterModel() const
{
    return d->filterModel->imageFilterModel();
}

ItemThumbnailModel* ItemCategorizedView::itemThumbnailModel() const
{
    return qobject_cast<ItemThumbnailModel*>(d->model);
}

ItemAlbumModel* ItemCategorizedView::itemAlbumModel() const
{
    return qobject_cast<ItemAlbumModel*>(d->model);
}

ItemAlbumFilterModel* ItemCategorizedView::itemAlbumFilterModel() const
{
    return qobject_cast<ItemAlbumFilterModel*>(d->filterModel->imageFilterModel());
}

QSortFilterProxyModel* ItemCategorizedView::filterModel() const
{
    return d->filterModel;
}

ItemDelegate* ItemCategorizedView::itemDelegate() const
{
    return d->delegate;
}

void ItemCategorizedView::setItemDelegate(ItemDelegate* delegate)
{
    ThumbnailSize oldSize                 = thumbnailSize();
    const ItemDelegate* const oldDelegate = d->delegate;

    if (oldDelegate)
    {
        hideIndexNotification();
        d->delegate->setAllOverlaysActive(false);
        d->delegate->setViewOnAllOverlays(nullptr);

        // Note: Be precise, no wildcard disconnect!

        disconnect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
                   this, SLOT(showIndexNotification(QModelIndex,QString)));

        disconnect(d->delegate, SIGNAL(hideNotification()),
                   this, SLOT(hideIndexNotification()));
    }

    d->delegate = delegate;
    d->delegate->setThumbnailSize(oldSize);

    if (oldDelegate)
    {
        d->delegate->setSpacing(oldDelegate->spacing());
    }

    ItemViewCategorized::setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());
    updateDelegateSizes();

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
            this, SLOT(showIndexNotification(QModelIndex,QString)));

    connect(d->delegate, SIGNAL(hideNotification()),
            this, SLOT(hideIndexNotification()));
}

Album* ItemCategorizedView::currentAlbum() const
{
    const ItemAlbumModel* const albumModel = itemAlbumModel();

    // TODO: Change to QList return type

    if (albumModel && !(albumModel->currentAlbums().isEmpty()))
    {
        return albumModel->currentAlbums().constFirst();
    }

    return nullptr;
}

ItemInfo ItemCategorizedView::currentInfo() const
{
    return imageInfo(currentIndex());
}

QUrl ItemCategorizedView::currentUrl() const
{
    return currentInfo().fileUrl();
}

ItemInfo ItemCategorizedView::imageInfo(const QModelIndex& index) const
{
    return d->filterModel->imageInfo(index);
}

ItemInfoList ItemCategorizedView::imageInfos(const QList<QModelIndex>& indexes) const
{
    return ItemInfoList(d->filterModel->imageInfos(indexes));
}

ItemInfoList ItemCategorizedView::allItemInfos() const
{
    return ItemInfoList(d->filterModel->imageInfosSorted());
}

QList<QUrl> ItemCategorizedView::allUrls() const
{
    return allItemInfos().toImageUrlList();
}

ItemInfoList ItemCategorizedView::selectedItemInfos() const
{
    return imageInfos(selectedIndexes());
}

ItemInfoList ItemCategorizedView::selectedItemInfosCurrentFirst() const
{
    QModelIndexList indexes   = selectedIndexes();
    const QModelIndex current = currentIndex();

    if (!indexes.isEmpty())
    {
        if (indexes.first() != current)
        {
            if (indexes.removeOne(current))
            {
                indexes.prepend(current);
            }
        }
    }

    return imageInfos(indexes);
}

void ItemCategorizedView::toIndex(const QUrl& url)
{
    ItemViewCategorized::toIndex(d->filterModel->indexForPath(url.toLocalFile()));
}

QModelIndex ItemCategorizedView::indexForInfo(const ItemInfo& info) const
{
    return d->filterModel->indexForItemInfo(info);
}

ItemInfo ItemCategorizedView::nextInOrder(const ItemInfo& startingPoint, int nth)
{
    QModelIndex index = d->filterModel->indexForItemInfo(startingPoint);

    if (!index.isValid())
    {
        return ItemInfo();
    }

    return imageInfo(d->filterModel->index(index.row() + nth, 0, QModelIndex()));
}

QModelIndex ItemCategorizedView::nextIndexHint(const QModelIndex& anchor, const QItemSelectionRange& removed) const
{
    QModelIndex hint = ItemViewCategorized::nextIndexHint(anchor, removed);
    ItemInfo info    = imageInfo(anchor);
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Having initial hint" << hint << "for" << anchor << d->model->numberOfIndexesForItemInfo(info);
*/
    // Fixes a special case of multiple (face) entries for the same image.
    // If one is removed, any entry of the same image shall be preferred.

    if (d->model->numberOfIndexesForItemInfo(info) > 1)
    {
        // The hint is for a different info, but we may have a hint for the same info

        if (info != imageInfo(hint))
        {
            int minDiff                           = d->filterModel->rowCount();
            QList<QModelIndex> indexesForItemInfo = d->filterModel->mapListFromSource(d->model->indexesForItemInfo(info));

            for (const QModelIndex& index : std::as_const(indexesForItemInfo))
            {
                if ((index == anchor) || !index.isValid() || removed.contains(index))
                {
                    continue;
                }

                int distance = qAbs(index.row() - anchor.row());

                if (distance < minDiff)
                {
                    minDiff = distance;
                    hint    = index;
/*
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Chose index" << hint << "at distance" << minDiff << "to" << anchor;
*/
                }
            }
        }
    }

    return hint;
}

void ItemCategorizedView::openAlbum(const QList<Album*>& albums)
{
    ItemAlbumModel* const albumModel = itemAlbumModel();

    if (albumModel)
    {
        albumModel->openAlbum(albums);
    }
}

ThumbnailSize ItemCategorizedView::thumbnailSize() const
{
/*
    ItemThumbnailModel* const thumbModel = imageThumbnailModel();

    if (thumbModel)
    {
        return thumbModel->thumbnailSize();
    }
*/
    if (d->delegate)
    {
        return d->delegate->thumbnailSize();
    }

    return ThumbnailSize();
}

void ItemCategorizedView::setThumbnailSize(int size)
{
    setThumbnailSize(ThumbnailSize(size));
}

void ItemCategorizedView::setThumbnailSize(const ThumbnailSize& s)
{
    // we abuse this pair of method calls to restore scroll position

    layoutAboutToBeChanged();
    d->delegate->setThumbnailSize(s);
    layoutWasChanged();
}

void ItemCategorizedView::setCurrentWhenAvailable(qlonglong imageId)
{
    if (!d->scrollToItemId)
    {
        d->scrollToItemId = imageId;
    }
}

void ItemCategorizedView::setCurrentUrlWhenAvailable(const QUrl& url)
{
    if (url.isEmpty())
    {
        clearSelection();
        setCurrentIndex(QModelIndex());

        return;
    }

    QString path      = url.toLocalFile();
    QModelIndex index = d->filterModel->indexForPath(path);

    if (!index.isValid())
    {
        d->unknownCurrentUrl = url;

        return;
    }

    clearSelection();
    setCurrentIndex(index);
    d->unknownCurrentUrl.clear();
}

void ItemCategorizedView::setCurrentUrl(const QUrl& url)
{
    if (url.isEmpty())
    {
        clearSelection();
        setCurrentIndex(QModelIndex());

        return;
    }

    QString path      = url.toLocalFile();
    QModelIndex index = d->filterModel->indexForPath(path);

    if (!index.isValid())
    {
        return;
    }

    clearSelection();
    setCurrentIndex(index);
}

void ItemCategorizedView::setCurrentInfo(const ItemInfo& info)
{
    QModelIndex index = d->filterModel->indexForItemInfo(info);
    clearSelection();
    setCurrentIndex(index);
}

void ItemCategorizedView::setSelectedUrls(const QList<QUrl>& urlList)
{
    QItemSelection mySelection;

    for (QList<QUrl>::const_iterator it = urlList.constBegin() ; it!=urlList.constEnd() ; ++it)
    {
        const QString path      = it->toLocalFile();
        const QModelIndex index = d->filterModel->indexForPath(path);

        if (!index.isValid())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "no QModelIndex found for" << *it;
        }
        else
        {
            // TODO: is there a better way?

            mySelection.select(index, index);
        }
    }

    clearSelection();
    selectionModel()->select(mySelection, QItemSelectionModel::Select);
}

void ItemCategorizedView::setSelectedItemInfos(const QList<ItemInfo>& infos)
{
    QItemSelection mySelection;

    for (const ItemInfo& info : std::as_const(infos))
    {
        QModelIndex index = d->filterModel->indexForItemInfo(info);
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ItemCategorizedView::hintAt(const ItemInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    QModelIndex index = d->filterModel->indexForItemInfo(info);

    if (!index.isValid())
    {
        return;
    }

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    scrollTo(index);
}

void ItemCategorizedView::addOverlay(ItemDelegateOverlay* overlay, ItemDelegate* delegate)
{
    if (!delegate)
    {
        delegate = d->delegate;
    }

    delegate->installOverlay(overlay);

    if (delegate == d->delegate)
    {
        overlay->setView(this);
        overlay->setActive(true);
    }
}

void ItemCategorizedView::removeOverlay(ItemDelegateOverlay* overlay)
{
    ItemDelegate* const delegate = dynamic_cast<ItemDelegate*>(overlay->delegate());

    if (delegate)
    {
        delegate->removeOverlay(overlay);
    }

    overlay->setView(nullptr);
}

void ItemCategorizedView::updateGeometries()
{
    ItemViewCategorized::updateGeometries();
    d->delayedEnterTimer->start();
}

void ItemCategorizedView::slotDelayedEnter()
{
    // re-emit entered() for index under mouse (after layout).

    QModelIndex mouseIndex = indexAt(mapFromGlobal(QCursor::pos()));

    if (mouseIndex.isValid())
    {
        Q_EMIT DCategorizedView::entered(mouseIndex);
    }
}

void ItemCategorizedView::addSelectionOverlay(ItemDelegate* delegate)
{
    addOverlay(new ItemSelectionOverlay(this), delegate);
}

void ItemCategorizedView::slotScrollToStoredItem()
{
    if (d->scrollToItemId)
    {
        if (d->model->hasImage(d->scrollToItemId))
        {
            QModelIndex index = d->filterModel->indexForImageId(d->scrollToItemId);
            clearSelection();
            setCurrentIndex(index);
            scrollToRelaxed(index, QAbstractItemView::PositionAtCenter);
            d->scrollToItemId = 0;
        }
    }
}

void ItemCategorizedView::slotItemInfosAdded()
{
    if      (d->scrollToItemId)
    {
        QTimer::singleShot(100, this, SLOT(slotScrollToStoredItem()));
    }
    else if (!d->unknownCurrentUrl.isEmpty())
    {
        QTimer::singleShot(100, this, SLOT(slotCurrentUrlTimer()));
    }
}

void ItemCategorizedView::slotCurrentUrlTimer()
{
    setCurrentUrl(d->unknownCurrentUrl);
    d->unknownCurrentUrl.clear();
}

void ItemCategorizedView::slotFileChanged(const QString& filePath)
{
    QModelIndex index = d->filterModel->indexForPath(filePath);

    if (index.isValid())
    {
        update(index);
    }
}

void ItemCategorizedView::indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers)
{
    ItemInfo info = imageInfo(index);

    if (!info.isNull())
    {
        activated(info, modifiers);

        Q_EMIT imageActivated(info);
    }
}

void ItemCategorizedView::currentChanged(const QModelIndex& index, const QModelIndex& previous)
{
    ItemViewCategorized::currentChanged(index, previous);

    Q_EMIT currentInfoChanged(imageInfo(index));
}

void ItemCategorizedView::selectionChanged(const QItemSelection& selectedItems,
                                           const QItemSelection& deselectedItems)
{
    ItemViewCategorized::selectionChanged(selectedItems, deselectedItems);

    if (!selectedItems.isEmpty())
    {
        Q_EMIT selected(imageInfos(selectedItems.indexes()));
    }

    if (!deselectedItems.isEmpty())
    {
        Q_EMIT deselected(imageInfos(deselectedItems.indexes()));
    }
}

Album* ItemCategorizedView::albumAt(const QPoint& pos) const
{
    if (itemFilterModel()->imageSortSettings().categorizationMode == ItemSortSettings::CategoryByAlbum)
    {
        QModelIndex categoryIndex = indexForCategoryAt(pos);

        if (categoryIndex.isValid())
        {
            int albumId = categoryIndex.data(ItemFilterModel::CategoryAlbumIdRole).toInt();

            return AlbumManager::instance()->findPAlbum(albumId);
        }
    }

    return currentAlbum();
}

void ItemCategorizedView::activated(const ItemInfo&, Qt::KeyboardModifiers)
{
    // implemented in subclass
}

void ItemCategorizedView::showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)
{
    showContextMenuOnInfo(event, imageInfo(index));
}

void ItemCategorizedView::showContextMenuOnInfo(QContextMenuEvent*, const ItemInfo&)
{
    // implemented in subclass
}

QItemSelectionModel* ItemCategorizedView::getSelectionModel() const
{
    return selectionModel();
}

AbstractItemDragDropHandler* ItemCategorizedView::dragDropHandler() const
{
    return d->model->dragDropHandler();
}

void ItemCategorizedView::slotIccSettingsChanged(const ICCSettingsContainer&, const ICCSettingsContainer&)
{
    viewport()->update();
}

} // namespace Digikam

#include "itemcategorizedview.moc"

#include "moc_itemcategorizedview.cpp"
