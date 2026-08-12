/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Item delegate for import interface items - private container.
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "itemviewimportdelegate.h"

// Qt includes

#include <QCache>
#include <QFont>
#include <QWidget>
#include <QPainter>
#include <QPolygon>
#include <QModelIndex>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ItemDelegateOverlay;
class ItemViewImportDelegate;

class Q_DECL_HIDDEN ItemViewImportDelegatePrivate
{
public:

    ItemViewImportDelegatePrivate();
    virtual ~ItemViewImportDelegatePrivate() = default;

public:

    void init(ItemViewImportDelegate* const _q, QWidget* const _widget);

    void makeStarPolygon();

    /// Resets cached rects. Remember to reimplement in subclass for added rects.
    virtual void clearRects();

public:

    int                       spacing       = 0;
    QSize                     gridSize;

    QRect                     rect;
    QRect                     ratingRect;

    QPixmap                   regPixmap;
    QPixmap                   selPixmap;
    QVector<QPixmap>          ratingPixmaps = QVector<QPixmap>(10);

    QFont                     font;
    QFont                     fontReg;
    QFont                     fontCom;
    QFont                     fontXtra;

    QPolygon                  starPolygon;
    QSize                     starPolygonSize;

    ThumbnailSize             thumbSize     = ThumbnailSize(0);

    QPersistentModelIndex     editingRating;

    QWidget*                  displayWidget = nullptr;

    QRect                     oneRowRegRect;
    QRect                     oneRowComRect;
    QRect                     oneRowXtraRect;

    /// constant values for drawing
    int                       radius        = 3;
    int                       margin        = 5;

    ItemViewImportDelegate*   q             = nullptr;

private:

    Q_DISABLE_COPY(ItemViewImportDelegatePrivate)
};

} // namespace Digikam
