/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-06
 * Description : Thumbnail bar for items
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemthumbnailbar.h"

// Qt includes

#include <QEvent>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "itemalbumfiltermodel.h"
#include "itemalbummodel.h"
#include "itemdragdrop.h"
#include "itemratingoverlay.h"
#include "itemcoordinatesoverlay.h"
#include "groupindicatoroverlay.h"
#include "itemthumbnaildelegate.h"
#include "fileactionmngr.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemThumbnailBar::Private
{
public:

    Private() = default;

public:

    Qt::ScrollBarPolicy          scrollPolicy       = Qt::ScrollBarAlwaysOn;
    NoDuplicatesItemFilterModel* duplicatesFilter   = nullptr;
};

ItemThumbnailBar::ItemThumbnailBar(QWidget* const parent)
    : ItemCategorizedView(parent),
      d                  (new Private)
{
    setItemDelegate(new ItemThumbnailDelegate(this));
    setSpacing(3);
    setUsePointingHandCursor(false);
    setScrollStepGranularity(5);
    setScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    // Disable QListView::Batched optimization
    // for the thumbnail bar, see bug #468593

    setLayoutMode(QListView::SinglePass);

    setScrollCurrentToCenter(ApplicationSettings::instance()->getScrollItemToCenter());
    setToolTipEnabled(ApplicationSettings::instance()->showToolTipsIsValid());

    // --- NOTE: use dynamic binding as slotSetupChanged() is a virtual slot which can be re-implemented in derived classes.

    connect(ApplicationSettings::instance(), &ApplicationSettings::setupChanged,
            this, &ItemThumbnailBar::slotSetupChanged);

    this->slotSetupChanged();

    // ---

    setFlow(LeftToRight);
}

ItemThumbnailBar::~ItemThumbnailBar()
{
    delete d;
}

void ItemThumbnailBar::setModelsFiltered(ItemModel* model, ImageSortFilterModel* filterModel)
{
    if (!d->duplicatesFilter)
    {
        d->duplicatesFilter = new NoDuplicatesItemFilterModel(this);
    }

    d->duplicatesFilter->setSourceFilterModel(filterModel);
    ItemCategorizedView::setModels(model, d->duplicatesFilter);
}

void ItemThumbnailBar::installOverlays()
{
    // Rating overlay

    ItemRatingOverlay* const ratingOverlay = new ItemRatingOverlay(this);
    addOverlay(ratingOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

    // Group overlay

    GroupIndicatorOverlay* const groupOverlay = new GroupIndicatorOverlay(this);
    addOverlay(groupOverlay);

    connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            this, SLOT(slotGroupIndicatorClicked(QModelIndex)));

    // Geolocation overlay

    ItemCoordinatesOverlay* const geoOverlay = new ItemCoordinatesOverlay(this);
    addOverlay(geoOverlay);

    connect(geoOverlay, SIGNAL(signalOpenGeolocationMap(QModelIndex)),
            this, SLOT(slotOpenGeolocationMap(QModelIndex)));
}

void ItemThumbnailBar::slotGroupIndicatorClicked(const QModelIndex& index)
{
    ItemInfo info = ItemModel::retrieveItemInfo(index);

    if (info.isNull())
    {
        return;
    }

    setCurrentIndex(index);
    itemFilterModel()->toggleGroupOpen(info.id());
    itemAlbumModel()->ensureHasGroupedImages(info);
}

void ItemThumbnailBar::slotOpenGeolocationMap(const QModelIndex& index)
{
    ItemInfo info              = itemFilterModel()->imageInfo(index);
    ItemInfoList imageInfoList = selectedItemInfos();

    if (!imageInfoList.contains(info))
    {
        setCurrentIndex(index);
    }

    Q_EMIT signalOpenGeolocationMap();
}

void ItemThumbnailBar::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    if ((area == Qt::LeftDockWidgetArea) || (area == Qt::RightDockWidgetArea))
    {
        if (flow() != TopToBottom)
        {
            setFlow(TopToBottom);
        }
    }
    else
    {
        if (flow() != LeftToRight)
        {
            setFlow(LeftToRight);
        }
    }

    scrollTo(currentIndex());
}

void ItemThumbnailBar::setScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    if (policy == Qt::ScrollBarAsNeeded)
    {
        // Delegate resizing will cause endless relayouting, see bug #228807

        qCDebug(DIGIKAM_GENERAL_LOG) << "The Qt::ScrollBarAsNeeded policy is not supported by ItemThumbnailBar";
    }

    d->scrollPolicy = policy;

    if (flow() == TopToBottom)
    {
        setVerticalScrollBarPolicy(d->scrollPolicy);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        setHorizontalScrollBarPolicy(d->scrollPolicy);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

void ItemThumbnailBar::setFlow(QListView::Flow flow)
{
    setWrapping(false);

    ItemCategorizedView::setFlow(flow);

    ItemThumbnailDelegate* const del = static_cast<ItemThumbnailDelegate*>(delegate());
    del->setFlow(flow);

    // Reset the minimum and maximum sizes.

    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    // Adjust minimum and maximum width to thumbnail sizes.

    if (flow == TopToBottom)
    {
        int viewportFullWidgetOffset = size().width() - viewport()->size().width();
        setMinimumWidth(del->minimumSize() + viewportFullWidgetOffset);
        setMaximumWidth(del->maximumSize() + viewportFullWidgetOffset);
    }
    else
    {
        int viewportFullWidgetOffset = size().height() - viewport()->size().height();
        setMinimumHeight(del->minimumSize() + viewportFullWidgetOffset);
        setMaximumHeight(del->maximumSize() + viewportFullWidgetOffset);
    }

    setScrollBarPolicy(d->scrollPolicy);
}

void ItemThumbnailBar::slotSetupChanged()
{
    setScrollCurrentToCenter(ApplicationSettings::instance()->getScrollItemToCenter());
    setToolTipEnabled(ApplicationSettings::instance()->showToolTipsIsValid());
    setFont(ApplicationSettings::instance()->getIconViewFont());

    ItemCategorizedView::slotSetupChanged();
}

void ItemThumbnailBar::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    ItemInfoList infos(itemSortFilterModel()->imageInfos(indexes));

    if (needGroupResolving(MetadataOps, infos))
    {
        infos = resolveGrouping(infos);
    }

    FileActionMngr::instance()->assignRating(infos, rating);
}

bool ItemThumbnailBar::event(QEvent* e)
{
    // reset widget max/min sizes

    if ((e->type() == QEvent::StyleChange) || (e->type() == QEvent::Show))
    {
        setFlow(flow());
    }

    return ItemCategorizedView::event(e);
}

QModelIndex ItemThumbnailBar::nextIndex(const QModelIndex& index) const
{
    return itemFilterModel()->index(index.row() + 1, 0);
}

QModelIndex ItemThumbnailBar::previousIndex(const QModelIndex& index) const
{
    return itemFilterModel()->index(index.row() - 1, 0);
}

QModelIndex ItemThumbnailBar::firstIndex() const
{
    return itemFilterModel()->index(0, 0);
}

QModelIndex ItemThumbnailBar::lastIndex() const
{
    return itemFilterModel()->index(itemFilterModel()->rowCount() - 1, 0);
}

bool ItemThumbnailBar::hasHiddenGroupedImages(const ItemInfo& info) const
{
    return (
            info.hasGroupedImages()                &&
            !itemFilterModel()->isAllGroupsOpen() &&
            !itemFilterModel()->isGroupOpen(info.id())
           );
}

} // namespace Digikam

#include "moc_itemthumbnailbar.cpp"
