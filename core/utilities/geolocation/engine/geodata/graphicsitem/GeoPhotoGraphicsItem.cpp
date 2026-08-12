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

#include "GeoPhotoGraphicsItem.h"

// Local includes

#include "GeoPainter.h"
#include "GeoDataStyle.h"
#include "GeoDataIconStyle.h"
#include "GeoDataFeature.h"
#include "StyleBuilder.h"
#include "ViewportParams.h"
#include "digikam_debug.h"

namespace Marble
{

GeoPhotoGraphicsItem::GeoPhotoGraphicsItem(const GeoDataFeature* feature)
    : GeoGraphicsItem(feature)
{
    if (feature)
    {
        QString const paintLayer = QStringLiteral("Photo");
        setPaintLayers(QStringList() << paintLayer);
    }
}

void GeoPhotoGraphicsItem::paint(GeoPainter* painter, const ViewportParams* viewport, const QString& layer, int tileZoomLevel)
{
    Q_UNUSED(layer);
    Q_UNUSED(tileZoomLevel);

    /* The code below loads the image lazily (only
     * when it will actually be displayed). Once it was
     * loaded but moves out of the viewport, it is unloaded
     * again. Otherwise memory consumption gets quite high
     * for a large set of photos
     */

    bool unloadImage = true;

    qreal x(0.0), y(0.0);
    viewport->screenCoordinates(m_point.coordinates(), x, y);

    QRectF position(QPointF(x, y), style()->iconStyle().icon().size());
    position.moveCenter(QPointF(x, y));

    QRectF displayed = position & QRectF(QPointF(0, 0), viewport->size());

    if (!displayed.isEmpty())
    {
        if (m_photo.isNull())
        {
            m_photo = style()->iconStyle().icon();
        }

        unloadImage = false;
        painter->drawImage(position, m_photo);
    }

    if (unloadImage)
    {
        m_photo = QImage();
    }
}

const GeoDataLatLonAltBox& GeoPhotoGraphicsItem::latLonAltBox() const
{
    return m_point.latLonAltBox();
}

bool GeoPhotoGraphicsItem::contains(const QPoint& curpos, const ViewportParams* viewport) const
{
    qreal x(0.0), y(0.0);
    viewport->screenCoordinates(m_point.coordinates(), x, y);
    auto itemStyle = style();

    if (itemStyle != nullptr && !itemStyle->iconStyle().icon().isNull())
    {
        int halfIconWidth = itemStyle->iconStyle().icon().size().width() / 2;
        int halfIconHeight = itemStyle->iconStyle().icon().size().height() / 2;

        if (x - halfIconWidth < curpos.x() &&
            curpos.x() < x + halfIconWidth &&
            y - halfIconHeight / 2 < curpos.y() &&
            curpos.y() < y + halfIconHeight / 2)
        {
            return true;
        }
    }

    else if (curpos.x() == x && curpos.y() == y)
    {
        return true;
    }

    return false;
}

void GeoPhotoGraphicsItem::setPoint(const GeoDataPoint& point)
{
    m_point = point;
}

GeoDataPoint GeoPhotoGraphicsItem::point() const
{
    return m_point;
}

} // namespace Marble
