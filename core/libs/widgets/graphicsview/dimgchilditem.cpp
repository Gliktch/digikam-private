/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-15
 * Description : Graphics View item for a child item on a DImg item
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgchilditem.h"

// Local includes

#include "graphicsdimgitem.h"
#include "imagezoomsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN DImgChildItem::Private
{
public:

    explicit Private(DImgChildItem* const qq)
        : q(qq)
    {
    }

    void connectParent(bool active = true);

public:

    QPointF              relativePos;
    QSizeF               relativeSize;

    DImgChildItem* const q = nullptr;
};

void DImgChildItem::Private::connectParent(bool active)
{
    GraphicsDImgItem* const parent = q->parentDImgItem();

    if (parent)
    {
        if (active)
        {
            q->connect(parent, SIGNAL(imageSizeChanged(QSizeF)),
                       q, SLOT(imageSizeChanged(QSizeF)));
        }
        else
        {
            q->disconnect(parent, SIGNAL(imageSizeChanged(QSizeF)),
                          q, SLOT(imageSizeChanged(QSizeF)));
        }
    }
}

// ------------------------------------------------------------------------

DImgChildItem::DImgChildItem(QGraphicsItem* const parent)
    : QGraphicsObject(parent),
      d              (new Private(this))
{
    d->connectParent();
}

DImgChildItem::~DImgChildItem()
{
    delete d;
}

void DImgChildItem::setRelativePos(const QPointF& relativePos)
{
    if (d->relativePos == relativePos)
    {
        return;
    }

    d->relativePos = relativePos;
    updatePos();

    Q_EMIT positionOnImageChanged();
    Q_EMIT geometryOnImageChanged();
}

void DImgChildItem::setRelativeSize(const QSizeF& relativeSize)
{
    if (d->relativeSize == relativeSize)
    {
        return;
    }

    d->relativeSize = relativeSize;
    updateSize();

    Q_EMIT sizeOnImageChanged();
    Q_EMIT geometryOnImageChanged();
}

void DImgChildItem::setRelativeRect(const QRectF& rect)
{
    setRelativePos(rect.topLeft());
    setRelativeSize(rect.size());
}

QRectF DImgChildItem::relativeRect() const
{
    return QRectF(d->relativePos, d->relativeSize);
}

QPointF DImgChildItem::relativePos() const
{
    return d->relativePos;
}

QSizeF DImgChildItem::relativeSize() const
{
    return d->relativeSize;
}

void DImgChildItem::setOriginalPos(const QPointF& posInOriginal)
{
    if (!parentItem())
    {
        return;
    }

    QSizeF orgSize = parentDImgItem()->zoomSettings()->originalImageSize();

    setRelativePos(QPointF(posInOriginal.x() / orgSize.width(),
                           posInOriginal.y() / orgSize.height()));
}

void DImgChildItem::setOriginalSize(const QSizeF& sizeInOriginal)
{
    if (!parentItem())
    {
        return;
    }

    QSizeF orgSize = parentDImgItem()->zoomSettings()->originalImageSize();

    setRelativeSize(QSizeF(sizeInOriginal.width()  / orgSize.width(),
                           sizeInOriginal.height() / orgSize.height()));
}

void DImgChildItem::setOriginalRect(const QRectF& rect)
{
    setOriginalPos(rect.topLeft());
    setOriginalSize(rect.size());
}

QRect DImgChildItem::originalRect() const
{
    return QRect(originalPos(), originalSize());
}

QSize DImgChildItem::originalSize() const
{
    QSizeF orgSize = parentDImgItem()->zoomSettings()->originalImageSize();

    return QSizeF(d->relativeSize.width()  * orgSize.width(),
                  d->relativeSize.height() * orgSize.height()).toSize();
}

QPoint DImgChildItem::originalPos() const
{
    QSizeF orgSize = parentDImgItem()->zoomSettings()->originalImageSize();

    return QPointF(d->relativePos.x() * orgSize.width(),
                   d->relativePos.y() * orgSize.height()).toPoint();
}

void DImgChildItem::setPos(const QPointF& pos)
{
    if (!parentItem())
    {
        return;
    }

    const QSizeF imageSize = parentItem()->boundingRect().size();

    setRelativePos(QPointF(pos.x() / imageSize.width(),
                           pos.y() / imageSize.height()));
}

void DImgChildItem::setSize(const QSizeF& size)
{
    if (!parentItem())
    {
        return;
    }

    const QSizeF imageSize = parentItem()->boundingRect().size();

    setRelativeSize(QSizeF(size.width()  / imageSize.width(),
                           size.height() / imageSize.height()));
}

void DImgChildItem::setRect(const QRectF& rect)
{
    setPos(rect.topLeft());
    setSize(rect.size());
}

void DImgChildItem::setRectInSceneCoordinates(const QRectF& rect)
{
    if (!parentItem())
    {
        return;
    }

    QRectF itemRect(parentItem()->mapFromScene(rect.topLeft()),
                    parentItem()->mapFromScene(rect.bottomRight()));
    setRect(itemRect);
}

QRectF DImgChildItem::rect() const
{
    return QRectF(pos(), size());
}

QSizeF DImgChildItem::size() const
{
    if (!parentItem())
    {
        return QSizeF();
    }

    const QSizeF imageSize = parentItem()->boundingRect().size();

    return QSizeF(d->relativeSize.width()  * imageSize.width(),
                  d->relativeSize.height() * imageSize.height());
}

GraphicsDImgItem* DImgChildItem::parentDImgItem() const
{
    return dynamic_cast<GraphicsDImgItem*>(parentItem());
}

void DImgChildItem::updatePos()
{
    if (!parentItem())
    {
        return;
    }

    QSizeF imageSize = parentItem()->boundingRect().size();
    QGraphicsObject::setPos(imageSize.width()  * d->relativePos.x(),
                            imageSize.height() * d->relativePos.y());

    Q_EMIT positionChanged();
    Q_EMIT geometryChanged();
}

void DImgChildItem::updateSize()
{
    prepareGeometryChange();

    Q_EMIT sizeChanged();
    Q_EMIT geometryChanged();
}

void DImgChildItem::imageSizeChanged(const QSizeF&)
{
    updateSize();
    updatePos();
}

QVariant DImgChildItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if      (change == QGraphicsItem::ItemParentChange)
    {
        d->connectParent(false);    // disconnect old parent
    }
    else if (change == QGraphicsItem::ItemParentHasChanged)
    {
        d->connectParent(true);     // connect new parent
    }

    return QGraphicsObject::itemChange(change, value);
}

QRectF DImgChildItem::boundingRect() const
{
    if (!parentItem())
    {
        return QRectF();
    }

    return QRectF(QPointF(0, 0), size());
}

} // namespace Digikam

#include "moc_dimgchilditem.cpp"
