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

#pragma once

// Qt includes

#include <QColor>
#include <QPainterPath>

// Local includes

#include "LayerInterface.h"

namespace Marble
{

class GroundLayer : public LayerInterface
{
public:

    GroundLayer();

    ~GroundLayer() override;

    QStringList renderPosition() const override;

    bool render(GeoPainter* painter, ViewportParams* viewport,
                const QString& renderPos = QLatin1String("NONE"),
                GeoSceneLayer* layer = nullptr) override;

    qreal zValue() const override;

    void setColor(const QColor& color);

    QColor color() const;

    RenderState renderState() const override;

    QString runtimeTrace() const override
    {
        return QStringLiteral("GroundLayer");
    }

private:

    QColor m_color;  // Gets the color specified via DGML's <map bgcolor="">
};

} // namespace Marble
