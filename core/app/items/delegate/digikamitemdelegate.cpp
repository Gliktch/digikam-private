/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt model-view for items - the delegate
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamitemdelegate.h"
#include "digikamitemdelegate_p.h"

// Local includes

#include "itemcategorydrawer.h"
#include "itemcategorizedview.h"
#include "itemdelegateoverlay.h"
#include "itemmodel.h"
#include "itemfiltermodel.h"
#include "applicationsettings.h"
#include "digikam_debug.h"

namespace Digikam
{

void DigikamItemDelegatePrivate::init(DigikamItemDelegate* const q, ItemCategorizedView* const parent)
{
    categoryDrawer = new ItemCategoryDrawer(parent);

    QObject::connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
                     q, SLOT(slotSetupChanged()));
}

DigikamItemDelegatePrivate::~DigikamItemDelegatePrivate()
{
    delete categoryDrawer;
}

// ------------------------------------------------------------------------------------------------

DigikamItemDelegate::DigikamItemDelegate(ItemCategorizedView* const parent)
    : ItemDelegate(*new DigikamItemDelegatePrivate, parent)
{
    Q_D(DigikamItemDelegate);

    d->init(this, parent);
}

DigikamItemDelegate::DigikamItemDelegate(DigikamItemDelegatePrivate& dd, ItemCategorizedView* parent)
    : ItemDelegate(dd, parent)
{
    Q_D(DigikamItemDelegate);

    d->init(this, parent);
}

void DigikamItemDelegate::updateRects()
{
    Q_D(DigikamItemDelegate);

    int y                                          = d->margin;
    d->pixmapRect                                  = QRect(d->margin, y, d->contentWidth, d->contentWidth);
    y                                              = d->pixmapRect.bottom();
    d->imageInformationRect                        = QRect(d->margin, y, d->contentWidth, 0);
    const ApplicationSettings* const albumSettings = ApplicationSettings::instance();
    d->drawImageFormat                             = albumSettings->getIconShowImageFormat();
    d->drawCoordinates                             = albumSettings->getIconShowCoordinates();
    const int iconSize                             = qBound(16, (d->contentWidth + 2 * d->margin) / 8 - 2, 48);

    if (albumSettings->getIconShowPickLabel())
    {
        d->pickLabelRect                           = QRect(d->margin, y - iconSize, iconSize, iconSize);
    }

    d->coordinatesRect                             = QRect(d->contentWidth - iconSize + 2, d->pixmapRect.top(), iconSize, iconSize);
    d->groupRect                                   = QRect(d->contentWidth - iconSize + d->margin, y - iconSize, iconSize, iconSize);
    const bool showInfos                           = ((d->contentWidth - 2 * d->radius) > ThumbnailSize::Small);

    if (albumSettings->getIconShowRating())
    {
        d->ratingRect = QRect(d->margin, y, d->contentWidth, d->starPolygonSize.height());
        y             = d->ratingRect.bottom();
    }

    if (albumSettings->getIconShowName())
    {
        d->nameRect = QRect(d->margin, y, d->contentWidth-d->margin, d->oneRowRegRect.height());
        y           = d->nameRect.bottom();
    }

    if (albumSettings->getIconShowTitle())
    {
        d->titleRect = QRect(d->margin, y, d->contentWidth, d->oneRowRegRect.height());
        y            = d->titleRect.bottom();
    }

    if (showInfos && albumSettings->getIconShowComments())
    {
        d->commentsRect = QRect(d->margin, y, d->contentWidth, d->oneRowComRect.height());
        y               = d->commentsRect.bottom();
    }

    if (showInfos && albumSettings->getIconShowDate())
    {
        d->dateRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y           = d->dateRect.bottom();
    }

    if (showInfos && albumSettings->getIconShowModDate())
    {
        d->modDateRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y              = d->modDateRect.bottom();
    }

    if (showInfos && albumSettings->getIconShowResolution())
    {
        d->resolutionRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y                 = d->resolutionRect.bottom() ;
    }

    if (showInfos && albumSettings->getIconShowAspectRatio())
    {
        d->arRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y         = d->arRect.bottom() ;
    }

    if (showInfos && albumSettings->getIconShowSize())
    {
        d->sizeRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y           = d->sizeRect.bottom();
    }

    if (showInfos && albumSettings->getIconShowTags())
    {
        d->tagRect = QRect(d->margin, y, d->contentWidth, d->oneRowComRect.height());
        y          = d->tagRect.bottom();
    }

    d->imageInformationRect.setBottom(y);

    d->rect            = QRect(0, 0, d->contentWidth + 2 * d->margin, y + d->margin + d->radius);
    d->gridSize        = QSize(d->rect.width() + d->spacing, d->rect.height() + d->spacing);
    d->groupedItemRect = QRect(1, 1, d->rect.width() - 3, d->rect.height() - 3);
}

} // namespace Digikam

#include "moc_digikamitemdelegate.cpp"
