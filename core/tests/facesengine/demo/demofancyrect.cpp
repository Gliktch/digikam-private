/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-23
 * Description : QGraphicsRectItem wrapper for FacesEngine Demo
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Adrien Bustany <madcat at mymadcat dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "demofancyrect.h"

// Qt includes

#include <QGraphicsScene>

namespace FaceEngineDemo
{

FancyRect::FancyRect(QGraphicsItem* const parent)
    : QGraphicsRectItem(parent)
{
}

FancyRect::FancyRect(const QRectF& rect, QGraphicsItem* const parent)
    : QGraphicsRectItem(rect, parent)
{
}

FancyRect::FancyRect(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* const parent)
    : QGraphicsRectItem(x, y, w, h, parent)
{
}

FancyRect::FancyRect(QGraphicsRectItem* const other, QGraphicsItem* const parent)
    : QGraphicsRectItem(parent)
{
    setPos(other->pos());
    setRect(other->rect());
    setPen(other->pen());
}

void FancyRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (option->state.testFlag(QStyle::State_Selected))
    {
        QPen selectedPen = pen();
        selectedPen.setColor(Qt::red);
        painter->setPen(selectedPen);
        painter->drawRect(rect());
    }

    else
    {
        QGraphicsRectItem::paint(painter, option, widget);
    }
}

} // Namespace FacesEngineDemo
