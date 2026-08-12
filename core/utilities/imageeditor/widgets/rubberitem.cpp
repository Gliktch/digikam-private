/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-09-13
 * Description : rubber item for Canvas
 *
 * SPDX-FileCopyrightText: 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rubberitem.h"

namespace Digikam
{

class Q_DECL_HIDDEN RubberItem::Private
{
public:

    Private() = default;

    Canvas* canvas = nullptr;
};

RubberItem::RubberItem(ImagePreviewItem* const parent)
    : RegionFrameItem(parent),
      d              (new Private)
{
}

RubberItem::~RubberItem()
{
    delete d;
}

void RubberItem::setCanvas(Canvas* const canvas)
{
    d->canvas = canvas;
}

void RubberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    RegionFrameItem::mouseReleaseEvent(event);
    d->canvas->slotSelected();
}

void RubberItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    RegionFrameItem::mouseMoveEvent(event);
    d->canvas->slotSelectionMoved();
}

} // namespace Digikam

#include "moc_rubberitem.cpp"
