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

#pragma once

// Qt includes

#include <QGraphicsObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class GraphicsDImgItem;

class DIGIKAM_EXPORT DImgChildItem : public QGraphicsObject     // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public:

    /**
     * @brief This is a base class for items that are positioned on top
     * of a GraphicsDImgItem, positioned in relative coordinates,
     * i.e. [0;1], on the image.
     * From the set relative size, the boundingRect() is calculated.
     *
     * This is a simple example. Just create
     * new SimpleRectChildItem(item);
     * where item is a GrahpicsDImgItem,
     * and at the center of the image,
     * a rectangle of 1% the size of the image will be drawn.
     *
     * class SimpleRectChildItem : public DImgChildItem
     * {
     * public:
     *
     *     SimpleRectChildItem(QGraphicsItem* const parent)
     *         : DImgChildItem(parent)
     *     {
     *         setRelativePos(0.5, 0.5);
     *         setRelativeSize(0.01, 0.01);
     *     }
     *
     *     void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
     *     {
     *         painter->setPen(Qt::red);
     *         painter->drawRect(boundingRect());
     *     }
     * };
     */
    explicit DImgChildItem(QGraphicsItem* const parent = nullptr);
    ~DImgChildItem()                              override;

    /**
     * @brief Sets the position and size of this item, relative to the DImg displayed in the parent item.
     * The values of @param relativePosition must be in the interval [0-1].
     */
    void setRelativePos(const QPointF& relativePosition);
    void setRelativePos(qreal x, qreal y)
    {
        setRelativePos(QPointF(x, y));
    }

    void setRelativeSize(const QSizeF& relativeSize);
    void setRelativeSize(qreal width, qreal height)
    {
        setRelativeSize(QSizeF(width, height));
    }

    void setRelativeRect(const QRectF& rect);
    void setRelativeRect(qreal x, qreal y, qreal width, qreal height)
    {
        setRelativeRect(QRectF(x, y, width, height));
    }

    /**
     * @return the position and size relative to the DImg displayed in the parent item.
     * All four values are in the interval [0-1].
     */
    QRectF  relativeRect()                  const;
    QPointF relativePos()                   const;
    QSizeF  relativeSize()                  const;

    /**
     * @brief Sets the position and size of this item, in coordinates of the original image.
     * Requires a valid parent item.
     */
    void setOriginalPos(const QPointF& posInOriginal);
    void setOriginalPos(qreal x, qreal y)
    {
        setOriginalPos(QPointF(x, y));
    }

    void setOriginalSize(const QSizeF& sizeInOriginal);
    void setOriginalSize(qreal width, qreal height)
    {
        setOriginalSize(QSizeF(width, height));
    }

    void setOriginalRect(const QRectF& rect);
    void setOriginalRect(qreal x, qreal y, qreal width, qreal height)
    {
        setOriginalRect(QRectF(x, y,width, height));
    }

    /**
     * @return the position and size in coordinates of the original image.
     * @note the return value is integer based. At high zoom rates,
     * different values of relativeRect() or zoomedRect() may result in the same originalRect(),
     * when one pixel in the original is represented by more than one pixel on screen.
     */
    QRect  originalRect()                   const;
    QPoint originalPos()                    const;
    QSize  originalSize()                   const;

    /**
     * @brief Sets the position and size of this item, in coordinates of the parent DImg item.
     * This is accepting unscaled parent coordinates, just like the "normal" setPos() does.
     * Requires a valid parent item.
     */
    void setPos(const QPointF& zoomedPos);
    void setPos(qreal x, qreal y)
    {
        setPos(QPointF(x, y));
    }

    void setSize(const QSizeF& zoomedSize);
    void setSize(qreal width, qreal height)
    {
        setSize(QSizeF(width, height));
    }

    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal width, qreal height)
    {
        setPos(QPointF(x,y));
        setSize(QSizeF(width, height));
    }

    /**
     * @brief Equivalent to mapping the scene coordinates to the parent item, and calling setRect().
     */
    void setRectInSceneCoordinates(const QRectF& rect);

    /**
     * @return position and size of this item, in coordinates of the parent DImg with the current zoom.
     * This is the same result as QRectF(pos(), boundingRect()), boundingRect is virtual and may be
     * overridden by base classes.
     */
    QRectF rect()                           const;
    QSizeF size()                           const;

    // Override
    void moveBy(qreal dx, qreal dy)
    {
        setPos(pos().x() + dx, pos().y() + dy);
    }

    /**
     * @brief If the parent item is a GraphicsDImgItem, return it,
     * if the parent item is null or of a different class, returns nullptr.
     */
    GraphicsDImgItem* parentDImgItem()      const;

    /**
     * @brief Reimplemented.
     * @return a rectangle starting at (0,0) (pos() in parent coordinates)
     * and has a size determined by the relative size.
     */
    QRectF boundingRect()                   const override;

protected Q_SLOTS:

    void imageSizeChanged(const QSizeF&);

Q_SIGNALS:

    /**
     * @brief These signals are emitted when the geometry, relative to the original image,
     * of this item has changed. This happens by calling any of the methods above.
     */
    void positionOnImageChanged();
    void sizeOnImageChanged();
    void geometryOnImageChanged();

    /**
     * @brief These signals are emitted in any case when the geometry changed:
     * Either after changing the geometry relative to the original image,
     * or when the size of the parent GraphicsDImgItem changed (zooming).
     * positionChanged() is equivalent to listening to xChanged() and yChanged().
     */
    void positionChanged();
    void sizeChanged();
    void geometryChanged();

protected:

    QVariant itemChange(GraphicsItemChange change,
                        const QVariant& value)    override;

private:

    void updatePos();
    void updateSize();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
