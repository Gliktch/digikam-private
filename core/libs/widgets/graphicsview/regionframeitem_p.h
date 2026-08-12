/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-09
 * Description : tag region frame
 *
 * SPDX-FileCopyrightText: 2007      by Aurelien Gateau <agateau at kde dot org>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "regionframeitem.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QRect>
#include <QTimer>
#include <QToolButton>

// Local includes

#include "graphicsdimgitem.h"
#include "itemvisibilitycontroller.h"

namespace
{
static const int HANDLE_SIZE = 15;
}

namespace Digikam
{

enum CropHandleFlag
{
    CH_None,
    CH_Top         = 1,
    CH_Left        = 2,
    CH_Right       = 4,
    CH_Bottom      = 8,
    CH_TopLeft     = CH_Top    | CH_Left,
    CH_BottomLeft  = CH_Bottom | CH_Left,
    CH_TopRight    = CH_Top    | CH_Right,
    CH_BottomRight = CH_Bottom | CH_Right,
    CH_Content     = 16
};

enum HudSide
{
    HS_None         = 0, ///< Special value used to avoid initial animation
    HS_Top          = 1,
    HS_Bottom       = 2,
    HS_Inside       = 4,
    HS_TopInside    = HS_Top    | HS_Inside,
    HS_BottomInside = HS_Bottom | HS_Inside
};

typedef QPair<QPointF, HudSide> OptimalPosition;

Q_DECLARE_FLAGS(CropHandle, CropHandleFlag)

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::CropHandle)

// --------------------------------------------------------------------------------

namespace Digikam
{

class Q_DECL_HIDDEN RegionFrameItem::Private
{
public:

    explicit Private(RegionFrameItem* const qq);

    QRectF handleRect(CropHandle handle) const;
    CropHandle handleAt(const QPointF& pos) const;
    void updateCursor(CropHandle handle, bool buttonDown);
    QRectF keepRectInsideImage(const QRectF& rect, bool moving = true) const;
    OptimalPosition computeOptimalHudWidgetPosition() const;
    void updateHudWidgetPosition();

public:

    RegionFrameItem* const q                                = nullptr;

    HudSide                hudSide                          = HS_None;
    QRectF                 viewportRect;
    QList<CropHandle>      cropHandleList;
    CropHandle             movingHandle                     = CH_None;
    QPointF                lastMouseMovePos;
    double                 fixedRatio                       = 0.0;
    QGraphicsWidget*       hudWidget                        = nullptr;

    RegionFrameItem::Flags flags                            = NoFlags;

    AnimatedVisibility*    resizeHandleVisibility           = nullptr;
    qreal                  hoverAnimationOpacity            = 1.0;
    QTimer*                hudTimer                         = nullptr;
    QPointF                hudEndPos;

    const int              HUD_TIMER_MAX_PIXELS_PER_UPDATE  = 20;
    const int              HUD_TIMER_ANIMATION_INTERVAL     = 20;

    QTimer*                marchingAntsTimer                = nullptr;
    int                    marchingAntsOffset               = 0;
};

} // namespace Digikam
