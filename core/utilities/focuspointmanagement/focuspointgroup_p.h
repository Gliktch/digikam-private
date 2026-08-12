/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Managing of focus point items on a GraphicsDImgView
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "focuspointgroup.h"

// Local includes

#include "digikam_debug.h"
#include "focuspointitem.h"

namespace Digikam
{

enum FocusPointGroupState
{
    NoPoints,
    PointsLoad,
    PointsLoaded
};

class Q_DECL_HIDDEN FocusPointGroup::Private
{

public:

    explicit Private(FocusPointGroup* const qq);

    void            applyVisible();
    void            clear(FocusPointGroupState fs);
    FocusPointItem* createItem(const FocusPoint& point) const;
    FocusPointItem* addItem(const FocusPoint& point);

public:

    GraphicsDImgView*           view                    = nullptr;
    ItemInfo                    info;
    bool                        exifRotate              = true;

    QList<FocusPointItem*>      items;

    ClickDragReleaseItem*       manuallyAddWrapItem     = nullptr;
    FocusPointItem*             manuallyAddedItem       = nullptr;

    FocusPointGroupState        state                   = NoPoints;
    ItemVisibilityController*   visibilityController    = nullptr;

    FocusPointGroup* const      q                       = nullptr;
};

} // namespace Digikam
