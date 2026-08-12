/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Focus point properties container (relative to original image size)
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "focuspoint.h"

// Qt includes

#include <QTransform>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "metaengine_rotation.h"

namespace Digikam
{

class Q_DECL_HIDDEN FocusPoint::Private : public QSharedData
{
public:

    Private() = default;

    float     x_position = 0.0F;       ///< Relative X coordinate of the center of focus point area (depending of original image width).
    float     y_position = 0.0F;       ///< Relative Y coordinate of the center of focus point area (depending of original image height).
    float     width      = 0.0F;       ///< Relative Width of focus point area (depending of original image width).
    float     height     = 0.0F;       ///< Relative Height of focus point area (depending of original image height).
    TypePoint type       = Inactive;   ///< Focus point area type. See TypePoint enum definition for details.
};

FocusPoint::FocusPoint()
    : d(new Private)
{
}

FocusPoint::FocusPoint(float x_position, float y_position, float width, float height, TypePoint type)
    : d(new Private)
{
    d->x_position = x_position;
    d->y_position = y_position;
    d->width      = width;
    d->height     = height;
    d->type       = type;
}

FocusPoint::FocusPoint(float x_position, float y_position, float width, float height)
    : d(new Private)
{
    d->x_position = x_position;
    d->y_position = y_position;
    d->width      = width;
    d->height     = height;
    d->type       = Inactive;
}

FocusPoint::FocusPoint(const QRectF& rectF)
    : d(new Private)
{
    setRect(rectF);
    d->type = Inactive;
}

FocusPoint::FocusPoint(const FocusPoint& other)
    : d(other.d)
{
}

FocusPoint::~FocusPoint()
{
}

FocusPoint& FocusPoint::operator=(const FocusPoint& other)
{
    d = other.d;

    return *this;
}

void FocusPoint::setType(TypePoint type)
{
    d->type = type;
}

FocusPoint::TypePoint FocusPoint::getType() const
{
    return d->type;
}

QString FocusPoint::getTypeDescription() const
{
    switch (getType())
    {
        case InFocus:
        {
            return i18nc("Focus point type description", "In Focus");
        }

        case Selected:
        {
            return i18nc("Focus point type description", "Selected");
        }

        case SelectedInFocus:
        {
            return i18nc("Focus point type description", "Selected In Focus");
        }

        default: // Inactive
        {
            return i18nc("Focus point type description", "Inactive");
        }
    }
}

QRect FocusPoint::getRectBySize(const QSize& size) const
{
    return QRect(static_cast<int>((d->x_position - 0.5 * d->width)  * size.width()),
                 static_cast<int>((d->y_position - 0.5 * d->height) * size.height()),
                 static_cast<int>(d->width  * size.width()),
                 static_cast<int>(d->height * size.height()));
}

void FocusPoint::setCenterPosition(float x_position, float y_position)
{
    d->x_position = x_position;
    d->y_position = y_position;
}

void FocusPoint::setSize(float width, float height)
{
    d->width  = width;
    d->height = height;
}

QPointF FocusPoint::getCenterPosition() const
{
    return QPointF(d->x_position, d->y_position);
}

QSizeF FocusPoint::getSize() const
{
    return QSizeF(d->width, d->height);
}

void FocusPoint::setRect(const QRectF& rectF)
{
    d->x_position = rectF.topLeft().x() + rectF.width()  * 0.5;
    d->y_position = rectF.topLeft().y() + rectF.height() * 0.5;
    d->width      = rectF.width();
    d->height     = rectF.height();
}

QRectF FocusPoint::getRect() const
{
    QRectF rect;
    rect.setSize(getSize());
    rect.moveCenter(getCenterPosition());

    return rect;
}

QDebug operator<<(QDebug dbg, const FocusPoint& fp)
{
    dbg.nospace() << "FocusPoint::area:"
                  << fp.getRect() << ", ";
    dbg.nospace() << "FocusPoint::type:"
                  << fp.getTypeDescription();

    return dbg.space();
}

} // namespace Digikam
