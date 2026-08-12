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

#include "FogLayer.h"

// Local includes

#include "GeoPainter.h"
#include "ViewportParams.h"
#include "RenderState.h"

namespace Marble
{

QStringList FogLayer::renderPosition() const
{
    return QStringList(QStringLiteral("ATMOSPHERE"));
}

bool FogLayer::render(GeoPainter* painter,
                      ViewportParams* viewParams,
                      const QString& renderPos,
                      GeoSceneLayer* layer)
{
    Q_UNUSED(renderPos)
    Q_UNUSED(layer)

    // FIXME: The fog layer is really slow. That's why we defer it to
    //        PrintQuality. Either cache on a pixmap - or maybe
    //        better: Add to GlobeScanlineTextureMapper.
    if (painter->mapQuality() != PrintQuality)
    {
        return true;
    }

    if (viewParams->projection() != Spherical)
    {
        return true;
    }

    // No use to draw the fog if it's not visible in the area.
    if (viewParams->mapCoversViewport())
    {
        return true;
    }

    int imgWidth2  = viewParams->width() / 2;
    int imgHeight2 = viewParams->height() / 2;

    int radius = viewParams->radius();

    // Recalculate the atmosphere effect and paint it to canvasImage.
    QRadialGradient grad1(QPointF(imgWidth2, imgHeight2), radius);

    // FIXME: Add a cosine relationship
    grad1.setColorAt(0.85, QColor(255, 255, 255, 0));
    grad1.setColorAt(1.00, QColor(255, 255, 255, 64));

    QBrush    brush1(grad1);
    QPen      pen1(Qt::NoPen);

    painter->save();

    painter->setBrush(brush1);
    painter->setPen(pen1);
    painter->setRenderHint(QPainter::Antialiasing, false);

    // FIXME: Cut out what's really needed
    painter->drawEllipse(imgWidth2  - radius,
                         imgHeight2 - radius,
                         2 * radius,
                         2 * radius);

    painter->restore();

    return true;
}

RenderState FogLayer::renderState() const
{
    return RenderState(QStringLiteral("Fog"));
}

} // namespace Marble
