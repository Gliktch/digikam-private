/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Qt item view to import items - the delegate (private container)
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "importdelegate.h"

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "importcategorizedview.h"
#include "itemviewimportdelegate_p.h"

namespace Digikam
{

class ImportCategoryDrawer;

class Q_DECL_HIDDEN ImportDelegate::ImportDelegatePrivate : public ItemViewImportDelegatePrivate
{
public:

    ImportDelegatePrivate()
    {
        actualPixmapRectCache.setMaxCost(250);
    }

    int                    contentWidth             = 0;

    QRect                  dateRect;
    QRect                  pixmapRect;
    QRect                  nameRect;
/*
    QRect                  titleRect;
    QRect                  commentsRect;
*/
    QRect                  resolutionRect;
    QRect                  sizeRect;
    QRect                  downloadRect;
    QRect                  lockRect;
    QRect                  coordinatesRect;
    QRect                  tagRect;
    QRect                  imageInformationRect;
    QRect                  pickLabelRect;
    QRect                  groupRect;

    bool                   drawImageFormat          = false;
    bool                   drawCoordinates          = false;
    bool                   drawFocusFrame           = true;
    bool                   drawMouseOverFrame       = true;
    bool                   ratingOverThumbnail      = false;

    QCache<int, QRect>     actualPixmapRectCache;
    ImportCategoryDrawer*  categoryDrawer           = nullptr;

    ImportCategorizedView* currentView              = nullptr;
    QAbstractItemModel*    currentModel             = nullptr;

public:

    void clearRects() override;
};

// --- ImportThumbnailDelegate ----------------------------------------------------

class Q_DECL_HIDDEN ImportThumbnailDelegatePrivate : public ImportDelegate::ImportDelegatePrivate
{
public:

    ImportThumbnailDelegatePrivate()
    {
        drawFocusFrame      = false;       ///< switch off drawing of frames
        drawMouseOverFrame  = false;
        ratingOverThumbnail = true;        ///< switch off composing rating over background
    }

    void init(ImportThumbnailDelegate* const q);

public:

    QListView::Flow flow     = QListView::LeftToRight;
    QRect           viewSize;
};

// --- ImportNormalDelegate ----------------------------------------------------

class Q_DECL_HIDDEN ImportNormalDelegatePrivate : public ImportDelegate::ImportDelegatePrivate
{
public:

    ImportNormalDelegatePrivate() = default;
    ~ImportNormalDelegatePrivate() override;

    void init(ImportNormalDelegate* const q, ImportCategorizedView* const parent);
};

} // namespace Digikam
