/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface - Zoom methods.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemiconview_p.h"

namespace Digikam
{

double ItemIconView::zoomMin() const
{
    return d->stackedView->zoomMin();
}

double ItemIconView::zoomMax() const
{
    return d->stackedView->zoomMax();
}

void ItemIconView::setZoomFactor(double zoom)
{
    d->stackedView->setZoomFactorSnapped(zoom);
}

void ItemIconView::slotZoomFactorChanged(double zoom)
{
    toggleZoomActions();

    Q_EMIT signalZoomChanged(zoom);
}

void ItemIconView::toggleZoomActions()
{
    if      (viewMode() == StackedView::PreviewImageMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->stackedView->maxZoom())
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->stackedView->minZoom())
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else if (
             (viewMode() == StackedView::IconViewMode) ||
             (viewMode() == StackedView::TableViewMode)
            )
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->thumbSize >= ThumbnailSize::maxThumbsSize())
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->thumbSize <= ThumbnailSize::Small)
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else
    {
        d->parent->enableZoomMinusAction(false);
        d->parent->enableZoomPlusAction(false);
    }
}

void ItemIconView::slotZoomIn()
{
    if      (
             (viewMode() == StackedView::IconViewMode) ||
             (viewMode() == StackedView::TableViewMode)
            )
    {
        setThumbSize(d->thumbSize + ThumbnailSize::Step);
        toggleZoomActions();

        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }
    else if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedView->increaseZoom();
    }
}

void ItemIconView::slotZoomOut()
{
    if      (
             (viewMode() == StackedView::IconViewMode) ||
             (viewMode() == StackedView::TableViewMode)
            )
    {
        setThumbSize(d->thumbSize - ThumbnailSize::Step);
        toggleZoomActions();

        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }
    else if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedView->decreaseZoom();
    }
}

void ItemIconView::slotZoomTo100Percents()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedView->toggleFitToWindowOr100();
    }
}

void ItemIconView::slotFitToWindow()
{
    if      (viewMode() == StackedView::TableViewMode)
    {
        /// @todo We should choose an appropriate thumbnail size here
    }
    else if (viewMode() == StackedView::IconViewMode)
    {
        int nts = d->iconView->fitToWidthIcons();
        qCDebug(DIGIKAM_GENERAL_LOG) << "new thumb size = " << nts;
        setThumbSize(nts);
        toggleZoomActions();

        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }
    else if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedView->fitToWindow();
    }
}

void ItemIconView::setThumbSize(int size)
{
    if      (viewMode() == StackedView::PreviewImageMode)
    {
        double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());
        setZoomFactor(z);
    }
    else if (
             (viewMode() == StackedView::IconViewMode)  ||
             (viewMode() == StackedView::TableViewMode) ||
             (viewMode() == StackedView::TrashViewMode)
            )
    {
        if      (size > ThumbnailSize::maxThumbsSize())
        {
            d->thumbSize = ThumbnailSize::maxThumbsSize();
        }
        else if (size < ThumbnailSize::Small)
        {
            d->thumbSize = ThumbnailSize::Small;
        }
        else
        {
            d->thumbSize = size;
        }

        Q_EMIT signalThumbSizeChanged(d->thumbSize);

        d->thumbSizeTimer->start();
    }
}

void ItemIconView::slotThumbSizeEffect()
{
    d->iconView->setThumbnailSize(ThumbnailSize(d->thumbSize));
    d->tableView->setThumbnailSize(ThumbnailSize(d->thumbSize));
    d->trashView->setThumbnailSize(ThumbnailSize(d->thumbSize));
    toggleZoomActions();

    ApplicationSettings::instance()->setDefaultIconSize(d->thumbSize);
}

} // namespace Digikam
