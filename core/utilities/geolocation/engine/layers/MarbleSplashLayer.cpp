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

#include "MarbleSplashLayer.h"

// Qt includes

#include <QPixmap>

// Local includes

#include "GeoPainter.h"
#include "MarbleDirs.h"
#include "ViewportParams.h"
#include "RenderState.h"

namespace Marble
{

MarbleSplashLayer::MarbleSplashLayer()
{
}

QStringList MarbleSplashLayer::renderPosition() const
{
    return QStringList(QStringLiteral("SURFACE"));
}

bool MarbleSplashLayer::render(GeoPainter* painter, ViewportParams* viewport,
                               const QString& renderPos, GeoSceneLayer* layer)
{
    Q_UNUSED(renderPos);
    Q_UNUSED(layer);

    painter->save();

    QPixmap logoPixmap(MarbleDirs::path(QStringLiteral("svg/marble-logo-inverted-72dpi.png")));

    if (logoPixmap.width() > viewport->width() * 0.7
        || logoPixmap.height() > viewport->height() * 0.7)
    {
        logoPixmap = logoPixmap.scaled(QSize(viewport->width(), viewport->height()) * 0.7,
                                       Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QPoint logoPosition((viewport->width()  - logoPixmap.width()) / 2,
                        (viewport->height() - logoPixmap.height()) / 2);
    painter->drawPixmap(logoPosition, logoPixmap);

    QString message; // "Please assign a map theme!";

    painter->setPen(Qt::white);

    int yTop = logoPosition.y() + logoPixmap.height() + 10;
    QRect textRect(0, yTop,
                   viewport->width(), viewport->height() - yTop);
    painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, message);

    painter->restore();

    return true;
}

RenderState MarbleSplashLayer::renderState() const
{
    return RenderState(QStringLiteral("Splash Screen"));
}

} // namespace Marble
