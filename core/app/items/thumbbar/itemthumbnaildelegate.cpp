/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : thumbnail bar for items - the delegate
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemthumbnaildelegate.h"
#include "itemthumbnaildelegate_p.h"

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "itemcategorizedview.h"
#include "itemdelegateoverlay.h"
#include "itemmodel.h"
#include "itemfiltermodel.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

ItemThumbnailDelegate::ItemThumbnailDelegate(ItemCategorizedView* parent)
    : ItemDelegate(*new ItemThumbnailDelegatePrivate, parent)
{
    Q_D(ItemThumbnailDelegate);

    d->init(this);
}

void ItemThumbnailDelegatePrivate::init(ItemThumbnailDelegate* const q)
{
    QObject::connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
                     q, SLOT(slotSetupChanged()));
}

void ItemThumbnailDelegate::setFlow(QListView::Flow flow)
{
    Q_D(ItemThumbnailDelegate);

    d->flow = flow;
}

void ItemThumbnailDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemThumbnailDelegate);

    // store before calling parent class

    d->viewSize = option.rect;
    ItemDelegate::setDefaultViewOptions(option);
}

int ItemThumbnailDelegate::maximumSize() const
{
    Q_D(const ItemThumbnailDelegate);

    return ThumbnailLoadThread::maximumThumbnailPixmapSize(true) + 2*d->radius + 2*d->margin;
}

int ItemThumbnailDelegate::minimumSize() const
{
    Q_D(const ItemThumbnailDelegate);

    return (ThumbnailSize::Small + 2*d->radius + 2*d->margin);
}

bool ItemThumbnailDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                              const QModelIndex& index, QRect* activationRect) const
{
    // reuse implementation from grandparent

    return ItemViewDelegate::acceptsActivation(pos, visualRect, index, activationRect); // clazy:exclude=skipped-base-method
}

void ItemThumbnailDelegate::updateContentWidth()
{
    Q_D(ItemThumbnailDelegate);

    int maxSize;

    if (d->flow == QListView::LeftToRight)
    {
        maxSize = d->viewSize.height();
    }
    else
    {
        maxSize = d->viewSize.width();
    }

    d->thumbSize = ThumbnailSize(ThumbnailLoadThread::thumbnailToPixmapSize(true, maxSize - 2*d->radius - 2*d->margin));

    ItemDelegate::updateContentWidth();
}

void ItemThumbnailDelegate::updateRects()
{
    Q_D(ItemThumbnailDelegate);

    d->rect               = QRect(0, 0, d->contentWidth + 2*d->margin, d->contentWidth + 2*d->margin);
    d->pixmapRect         = QRect(d->margin, d->margin, d->contentWidth, d->contentWidth);
    const int iconSize    = qBound(16, (d->contentWidth + 2*d->margin) / 8 - 2, 48);
    d->coordinatesRect    = QRect(d->contentWidth - iconSize+2, d->pixmapRect.top(), iconSize, iconSize);

    d->pickLabelRect      = QRect(d->margin + 2, d->rect.bottom() - iconSize, iconSize, iconSize);
    d->drawCoordinates    = ApplicationSettings::instance()->getIconShowCoordinates();
    d->drawImageFormat    = ApplicationSettings::instance()->getIconShowImageFormat();
    d->drawImageFormatTop = true;
    int top               = d->rect.bottom() - d->margin - d->starPolygonSize.height() - 2;

    if (ApplicationSettings::instance()->getIconShowRating())
    {
        d->ratingRect = QRect(d->margin, top, d->contentWidth, d->starPolygonSize.height());
    }

    d->groupRect          = QRect(d->contentWidth - iconSize + d->margin, d->rect.bottom() - iconSize, iconSize, iconSize);
    d->groupedItemRect    = d->pixmapRect;

    if (d->flow == QListView::LeftToRight)
    {
        d->gridSize = QSize(d->rect.width() + d->spacing, d->rect.height());
    }
    else
    {
        d->gridSize = QSize(d->rect.width(), d->rect.height() + d->spacing);
    }
}

} // namespace Digikam

#include "moc_itemthumbnaildelegate.cpp"
