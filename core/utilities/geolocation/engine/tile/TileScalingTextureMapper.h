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

#include <QObject>
#include <QCache>
#include <QImage>
#include <QPixmap>

// Local includes

#include "TextureMapperInterface.h"
#include "TileId.h"

namespace Marble
{

class StackedTileLoader;

class TileScalingTextureMapper : public QObject, public TextureMapperInterface
{
    Q_OBJECT

public:

    explicit TileScalingTextureMapper(StackedTileLoader* tileLoader, QObject* parent = nullptr);

    void mapTexture(GeoPainter* painter,
                    const ViewportParams* viewport,
                    int tileZoomLevel,
                    const QRect& dirtyRect,
                    TextureColorizer* texColorizer) override;

private Q_SLOTS:

    void removePixmap(const TileId& tileId);
    void clearPixmaps();

private:

    void mapTexture(GeoPainter* painter,
                    const ViewportParams* viewport,
                    int tileZoomLevel,
                    TextureColorizer* texColorizer);

private:

    StackedTileLoader* const        m_tileLoader = nullptr;
    QCache<TileId, const QPixmap>   m_cache;
    QImage                          m_canvasImage;
    int                             m_radius;
};

} // namespace Marble
