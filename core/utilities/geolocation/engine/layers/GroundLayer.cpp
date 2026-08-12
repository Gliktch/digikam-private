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

#include "GroundLayer.h"

// Local includes

#include "GeoPainter.h"
#include "ViewportParams.h"
#include "RenderState.h"

namespace Marble
{

GroundLayer::GroundLayer()
    : m_color(QColor(153, 179, 204))
{
}

GroundLayer::~GroundLayer()
{
}

QStringList GroundLayer::renderPosition() const
{
    return QStringList(QStringLiteral("SURFACE"));
}

bool GroundLayer::render(GeoPainter* painter,
                         ViewportParams* viewParams,
                         const QString& renderPos,
                         GeoSceneLayer* layer)
{
    Q_UNUSED(renderPos)
    Q_UNUSED(layer)

    QBrush backgroundBrush(m_color);
    QPen backgroundPen(Qt::NoPen);

    painter->setBrush(backgroundBrush);
    painter->setPen(backgroundPen);
    painter->drawPath(viewParams->mapShape());

    return true;
}

qreal GroundLayer::zValue() const
{
    return -50.0;
}

void GroundLayer::setColor(const QColor& color)
{
    m_color = color;
}

QColor GroundLayer::color() const
{
    return m_color;
}

RenderState GroundLayer::renderState() const
{
    return RenderState(QStringLiteral("Ground"));
}

} // namespace Marble
