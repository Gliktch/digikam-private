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

#pragma once

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "itemviewdelegate_p.h"

namespace Digikam
{

class ItemCategoryDrawer;

class Q_DECL_HIDDEN ItemDelegate::ItemDelegatePrivate : public ItemViewDelegatePrivate
{
public:

    ItemDelegatePrivate()
    {
        actualPixmapRectCache.setMaxCost(250);
    }

public:

    int                   contentWidth          = 0;

    QRect                 dateRect              = QRect(0, 0, 0, 0);
    QRect                 modDateRect           = QRect(0, 0, 0, 0);
    QRect                 pixmapRect            = QRect(0, 0, 0, 0);
    QRect                 specialInfoRect       = QRect(0, 0, 0, 0);
    QRect                 nameRect              = QRect(0, 0, 0, 0);
    QRect                 titleRect             = QRect(0, 0, 0, 0);
    QRect                 commentsRect          = QRect(0, 0, 0, 0);
    QRect                 resolutionRect        = QRect(0, 0, 0, 0);
    QRect                 arRect                = QRect(0, 0, 0, 0);
    QRect                 sizeRect              = QRect(0, 0, 0, 0);
    QRect                 tagRect               = QRect(0, 0, 0, 0);
    QRect                 imageInformationRect  = QRect(0, 0, 0, 0);
    QRect                 coordinatesRect       = QRect(0, 0, 0, 0);
    QRect                 pickLabelRect         = QRect(0, 0, 0, 0);
    QRect                 groupRect             = QRect(0, 0, 0, 0);

    bool                  drawFocusFrame        = true;
    bool                  drawCoordinates       = false;
    bool                  drawImageFormat       = false;
    bool                  drawImageFormatTop    = false;
    bool                  drawMouseOverFrame    = true;
    bool                  ratingOverThumbnail   = false;

    QCache<int, QRect>    actualPixmapRectCache;
    ItemCategoryDrawer*   categoryDrawer        = nullptr;

    ItemCategorizedView*  currentView           = nullptr;
    QAbstractItemModel*   currentModel          = nullptr;

public:

    void clearRects() override;
};

} // namespace Digikam
