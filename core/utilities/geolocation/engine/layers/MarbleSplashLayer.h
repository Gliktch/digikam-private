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

#include <QString>

// Local includes

#include "LayerInterface.h"

namespace Marble
{

class MarbleSplashLayer : public LayerInterface
{
public:

    MarbleSplashLayer();

    QStringList renderPosition() const override;

    bool render(GeoPainter* painter, ViewportParams* viewport,
                const QString& renderPos = QLatin1String("SURFACE"),
                GeoSceneLayer* layer = nullptr) override;

    RenderState renderState() const override;

    QString runtimeTrace() const override
    {
        return QStringLiteral("SplashLayer");
    }
};

} // namespace Marble
