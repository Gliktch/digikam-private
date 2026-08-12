/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "ScreenOverlayGraphicsItem.h"

// Local includes

#include "ViewportParams.h"
#include "GeoDataScreenOverlay.h"
#include "GeoPainter.h"

namespace Marble
{

ScreenOverlayGraphicsItem::ScreenOverlayGraphicsItem(const GeoDataScreenOverlay* screenOverlay)
    : m_screenOverlay(screenOverlay)
{
    /** @todo: take the possible size values into account according to kml spec */
    setSize(QSizeF(m_screenOverlay->size().x(), m_screenOverlay->size().y()));

    if (!m_screenOverlay->icon().isNull())
    {
        m_pixmap = QPixmap::fromImage(m_screenOverlay->icon().scaled(size().toSize()));
    }
}

const GeoDataScreenOverlay* ScreenOverlayGraphicsItem::screenOverlay() const
{
    return m_screenOverlay;
}

void ScreenOverlayGraphicsItem::setProjection(const ViewportParams* viewport)
{
    ScreenGraphicsItem::setProjection(viewport);
    /** @todo: take overlayXY into account */
    setPosition(QPointF(pixelValue(m_screenOverlay->screenXY().xunit(),
                                   viewport->width(),
                                   size().width(),
                                   m_screenOverlay->screenXY().x()),
                        viewport->height() - pixelValue(m_screenOverlay->screenXY().yunit(),
                                                        viewport->height(),
                                                        size().height(),
                                                        m_screenOverlay->screenXY().y())));
}

qreal ScreenOverlayGraphicsItem::pixelValue(GeoDataVec2::Unit unit, qreal screenSize, qreal imageSize, qreal value)
{
    switch (unit)
    {
        case GeoDataVec2::Pixels:
            return imageSize;

        case GeoDataVec2::Fraction:
            return value * screenSize;

        case GeoDataVec2::InsetPixels:
            return screenSize - imageSize - value;
    }

    Q_ASSERT(false);
    return 0.0;
}

void ScreenOverlayGraphicsItem::paint(QPainter* painter)
{
    if (m_pixmap.isNull())
    {
        painter->setBrush(m_screenOverlay->color());
        painter->drawRect(QRectF(QPointF(0.0, 0.0), size()));
    }

    else
    {
        painter->drawPixmap(QPointF(0.0, 0.0), m_pixmap);
    }
}

} // namespace Marble
