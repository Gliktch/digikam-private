/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-12-22
 * Description : a magnifier tool for the GraphicsDImgView canvas.
 *
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "magnifieritem.h"

// Qt includes

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Digikam
{

class Q_DECL_HIDDEN MagnifierItem::Private
{
public:

    Private() = default;

public:

    QPixmap sourcePixmap;
    QRectF  sourceRect;
    qreal   zoomFactor   = MagnifierItem::zoomFactors().constFirst();
    int     size         = 150;
};

MagnifierItem::MagnifierItem(QGraphicsItem* const parent)
    : QGraphicsItem(parent),
      d            (new Private)

{
    setZValue(10); // Be sure that the loop is always behind the image
}

MagnifierItem::~MagnifierItem()
{
    delete d;
}

QList<qreal> MagnifierItem::zoomFactors()
{
    return (QList<qreal>() << 1.5 << 2.0 << 2.5 << 3.0 << 3.5 << 4.0 << 6.0 << 8.0);
}

void MagnifierItem::setSourcePixmap(const QPixmap& pixmap, const QRectF& rect)
{
    d->sourcePixmap = pixmap;
    d->sourceRect   = rect;
    update();
}

void MagnifierItem::setZoomFactor(qreal factor)
{
    d->zoomFactor = factor;
    update();
}

qreal MagnifierItem::zoomFactor() const
{
    return d->zoomFactor;
}

void MagnifierItem::setMagnifierSize(int magnifierSize)
{
    d->size = magnifierSize;
    update();
}

int MagnifierItem::magnifierSize() const
{
    return d->size;
}

QRectF MagnifierItem::boundingRect() const
{
    return QRectF(-d->size / 2, -d->size / 2, d->size, d->size);
}

void MagnifierItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (d->sourcePixmap.isNull())
    {
        return;
    }

    // First draw the border.

    QPainterPath borderPath;
    borderPath.addEllipse(boundingRect());

    // Define a gradient for the border.

    QRadialGradient borderGradient(0, 0, d->size / 2);
    borderGradient.setColorAt(0,   QColor(50, 50, 50, 255));            // Darkness inside surround.
    borderGradient.setColorAt(0.7, QColor(200, 200, 200, 255));         // Lightness outside surround.

    // Draw a border with a large pencil.

    painter->setPen(QPen(QBrush(borderGradient), 4));                   // Border of 4 pixels.
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(borderPath);

    // Create a ellipsis path for the clipping.

    QPainterPath clipPath;
    clipPath.addEllipse(boundingRect().adjusted(2, 2, -2, -2));         // Adjustment to highlight border.

    // Apply clipping.

    painter->setClipPath(clipPath);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // Draw the background of the magnifier with a gradient

    QRadialGradient fillGradient(0, 0, (d->size / 2) - 2);              // Preserve the border.
    fillGradient.setColorAt(0, QColor(255, 255, 255, 220));             // Center semi-transparent using white.
    fillGradient.setColorAt(1, QColor(255, 255, 255, 100));             // Border more transparent.

    painter->setBrush(fillGradient);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect().adjusted(4, 4, -4, -4));        // Draw a little bit inside the border.

    // Draw the zommed pixmap.

    QPixmap zoomed = d->sourcePixmap.copy(d->sourceRect.toRect())
                                   .scaled(d->size - 2, d->size - 2,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);   // Adjust to preserve the border.

    painter->drawPixmap(QRectF(-(d->size / 2) + 2, -(d->size / 2) + 2, d->size - 4, d->size - 4),
                        zoomed,
                        zoomed.rect());

    // Reset clipping.

    painter->setClipping(false);
}

} // namespace Digikam
