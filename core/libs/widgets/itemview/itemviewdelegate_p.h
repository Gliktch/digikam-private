/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for items - the delegate (private container)
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "itemviewdelegate.h"

// Qt includes

#include <QCache>
#include <QFont>
#include <QWidget>
#include <QPainter>
#include <QPolygon>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

class ItemViewDelegate;

class DIGIKAM_EXPORT ItemViewDelegatePrivate
{
public:

    ItemViewDelegatePrivate();
    virtual ~ItemViewDelegatePrivate() = default;

    void init(ItemViewDelegate* const _q, QWidget* const _widget);

    void makeStarPolygon();

    /**
     * Resets cached rects. Remember to reimplement in subclass for added rects.
     */
    virtual void clearRects();

public:

    int                       spacing           = 0;
    QSize                     gridSize;

    QRect                     rect;                                     ///< Define the rectangle representing whole icon-view item.
    QRect                     ratingRect;                               ///< Used to host the rating widget.
    QRect                     groupedItemRect;                          ///< Used to draw a frame when grouped items are expanded.

    QPixmap                   regPixmap;
    QPixmap                   selPixmap;
    QVector<QPixmap>          ratingPixmaps     = QVector<QPixmap>(10);

    QFont                     font;
    QFont                     fontReg;
    QFont                     fontCom;
    QFont                     fontXtra;

    QPolygon                  starPolygon;
    QSize                     starPolygonSize;

    ThumbnailSize             thumbSize         = ThumbnailSize(0);

    QPersistentModelIndex     editingRating;

    ItemViewDelegate*         q                 = nullptr;

    QWidget*                  displayWidget     = nullptr;

    QRect                     oneRowRegRect;
    QRect                     oneRowComRect;
    QRect                     oneRowXtraRect;

    /// constant values for drawing

    int                       radius            = 3;
    int                       margin            = 5;

private:

    Q_DISABLE_COPY(ItemViewDelegatePrivate)
};

} // namespace Digikam
