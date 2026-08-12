/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-23
 * Description : face marquer widget for FacesEngine Demo
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Adrien Bustany <madcat at mymadcat dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "demomarquee.h"

// Qt includes

#include <QPointF>
#include <QPen>
#include <QString>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene>
#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "demofancyrect.h"

namespace FaceEngineDemo
{

class Q_DECL_HIDDEN Marquee::Private
{
public:

    enum ResizeType
    {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRight
    };

public:

    explicit Private()
        : handleSize(10),
          htl(nullptr),
          htr(nullptr),
          hbr(nullptr),
          hbl(nullptr),
          rect(nullptr),
          label(nullptr),
          moving(false),
          resizing(false),
          resizeType(0)
    {
    }

public:

    const int                handleSize;
    QPen                     rectPen;    ///< The pen used to draw the frames
    QPen                     outlinePen; ///< Text outline pen

    /// Handles
    FancyRect*               htl;
    FancyRect*               htr;
    FancyRect*               hbr;
    FancyRect*               hbl;

    FancyRect*               rect;       ///< Main frame
    QGraphicsSimpleTextItem* label;

    bool                     moving;     ///< Tells if we are moving the marquee
    QPointF                  moveOffset; ///< where the mouse was when the user began to drag the marquee
    bool                     resizing;   ///< Tells if we are resizing the marquee
    int                      resizeType; ///< See ResizeType values.
};

Marquee::Marquee(FancyRect* const rect, QGraphicsItem* const parent)
    : QObject(nullptr),
      QGraphicsItemGroup(parent),
      d(new Private)
{
    d->rect    = rect;
    d->rectPen.setColor(Qt::red);
    d->rectPen.setWidth(2);
    d->outlinePen.setColor(Qt::red);
    addToGroup(d->rect);
    d->rect->setPen(d->rectPen);
    setPos(d->rect->scenePos());
    d->rect->setPos(0, 0);
    createHandles();

    d->label   = new QGraphicsSimpleTextItem(QString::fromLatin1(""), this);
    d->label->setBrush(QBrush(d->rectPen.color()));
    d->label->setPen(d->outlinePen);
    d->label->setZValue(2);

    QFont font = d->label->font();
    font.setBold(true);
    font.setPointSize(12);
    d->label->setFont(font);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setSelected(true);

    QTimer::singleShot(1000, this, SIGNAL(selected(this)));
}

Marquee::~Marquee()
{
    delete d;
}

QRectF Marquee::boundingRect() const
{
    return d->rect->rect();
}

QRectF Marquee::toRectF() const
{
    return QRectF(x(), y(), d->rect->rect().width(), d->rect->rect().height());
}

void Marquee::createHandles()
{
    d->htl = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->htl->setZValue(1);
    d->htr = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->htr->setZValue(1);
    d->hbl = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->hbl->setZValue(1);
    d->hbr = new FancyRect(0, 0, d->handleSize, d->handleSize);
    d->hbr->setZValue(1);

    d->htl->setPen(d->rectPen);
    d->htr->setPen(d->rectPen);
    d->hbl->setPen(d->rectPen);
    d->hbr->setPen(d->rectPen);

    addToGroup(d->htl);
    addToGroup(d->htr);
    addToGroup(d->hbl);
    addToGroup(d->hbr);
    placeHandles();
}

void Marquee::placeHandles()
{
    qreal rw = d->rect->rect().width();
    qreal rh = d->rect->rect().height();
    qreal ox = d->rect->rect().x();
    qreal oy = d->rect->rect().y();
    qreal hs = d->hbr->boundingRect().width();

    d->htl->setPos(ox,         oy);
    d->htr->setPos(ox + rw - hs + 2, oy);
    d->hbl->setPos(ox,         oy + rh - hs + 2);
    d->hbr->setPos(ox + rw - hs + 2, oy + rh - hs + 2);
}

void Marquee::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    Q_EMIT selected(this);

    // Check for some resize handles under the mouse.

    if (d->htl->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::TopLeft;
        return;
    }

    if (d->htr->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::TopRight;
        return;
    }

    if (d->hbl->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::BottomLeft;
        return;
    }

    if (d->hbr->isUnderMouse())
    {
        d->resizing   = true;
        d->resizeType = Private::BottomRight;
        return;
    }

    // If no handle is under the mouse, then we move the frame.

    d->resizing   = false;
    d->moving     = true;
    d->moveOffset = e->pos();

    Q_EMIT changed();
}

void Marquee::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (d->resizing)
    {
        QRectF r = d->rect->rect();

        switch (d->resizeType)
        {
            case Private::TopLeft:
            {
                r.setTopLeft(e->pos());
                break;
            }

            case Private::TopRight:
            {
                r.setTopRight(e->pos());
                break;
            }

            case Private::BottomLeft:
            {
                r.setBottomLeft(e->pos());
                break;
            }

            case Private::BottomRight:
            {
                r.setBottomRight(e->pos());
                break;
            }

            default:
            {
                break;
            }
        }

        if ((r.width() < 2 * d->handleSize) || (r.height() < 2 * d->handleSize))
        {
            return;
        }

        setPos(pos() + r.topLeft());
        r.moveTopLeft(QPointF(0, 0));
        d->rect->setRect(r);
        placeHandles();
    }

    if (d->moving)
    {
        setPos(e->scenePos() - d->moveOffset);
    }

    Q_EMIT changed();
}

void Marquee::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    Q_UNUSED(e);
    d->resizing = false;
    d->moving   = false;

    Q_EMIT changed();
}

} // namespace FaceEngineDemo

#include "moc_demomarquee.cpp"
