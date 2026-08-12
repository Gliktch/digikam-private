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

#include "focuspointitem.h"

// Qt includes

#include <QPainter>
#include <QPen>

// Local includes

#include "digikam_debug.h"
#include "assignnamewidgetstates.h"

namespace Digikam
{

class Q_DECL_HIDDEN FocusPointItem::Private
{
public:

    Private() = default;

    FocusPoint point;
    QColor     color = QColor::fromRgb(0, 0, 0, 255); ///< alpha is 100 to let more transparency;
    float      width = 3.0F;
};

FocusPointItem::FocusPointItem(QGraphicsItem* const parent)
    : RegionFrameItem(parent),
      d              (new Private)
{
}

FocusPointItem::~FocusPointItem()
{
    delete d;
}

void FocusPointItem::setPoint(const FocusPoint& point)
{
    d->point = point;
    setEditable(false);

    switch (d->point.getType())
    {
        case FocusPoint::TypePoint::Inactive:
        {
            d->color.setAlpha(130);
            d->width = 1;

            break;
        }

        case FocusPoint::TypePoint::Selected:
        case FocusPoint::TypePoint::SelectedInFocus:
        {
            d->color.setRed(255);

            break;
        }

        default: // FocusPoint::TypePoint::InFocus
        {
            // TODO

            break;
        }
    }
}

FocusPoint FocusPointItem::point() const
{
    return d->point;
}

void FocusPointItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QPen pen;
    pen.setWidth(d->width);
    pen.setColor(d->color);

    QRectF drawRect = boundingRect();

    qCDebug(DIGIKAM_GENERAL_LOG) << "FocusPointsItem: rectangle:" << drawRect;

    painter->setPen(pen);
    painter->drawRect(drawRect);
}

void FocusPointItem::setEditable(bool allowEdit)
{
    changeFlags(GeometryEditable, allowEdit);
}

} // namespace Digikam

#include "moc_focuspointitem.cpp"
