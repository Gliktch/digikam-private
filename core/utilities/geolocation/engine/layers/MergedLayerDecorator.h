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

#include <QVector>
#include <QList>

// Local includes

#include "MarbleGlobal.h"

class QImage;
class QString;
class QSize;

namespace Marble
{

class GeoDataGroundOverlay;
class GeoSceneAbstractTileProjection;
class GeoSceneTextureTileDataset;
class SunLocator;
class StackedTile;
class Tile;
class TileId;
class TileLoader;
class RenderState;

class MergedLayerDecorator
{
public:

    MergedLayerDecorator(TileLoader* const tileLoader, const SunLocator* sunLocator);
    virtual ~MergedLayerDecorator();

public:

    void setTextureLayers(const QVector<const GeoSceneTextureTileDataset*>& textureLayers);
    void updateGroundOverlays(const QList<const GeoDataGroundOverlay*>& groundOverlays);

    int textureLayersSize()                                 const;

    /**
     * Returns the highest level in which some tiles are theoretically
     * available for the current texture layers.
     */
    int maximumTileLevel()                                  const;

    int tileColumnCount(int level)                          const;

    int tileRowCount(int level)                             const;

    const GeoSceneAbstractTileProjection* tileProjection()  const;

    QSize tileSize()                                        const;

    StackedTile* loadTile(const TileId& id);

    StackedTile* updateTile(const StackedTile& stackedTile,
                            const TileId& tileId,
                            const QImage& tileImage);

    void downloadStackedTile(const TileId& id, DownloadUsage usage);

    void setShowSunShading(bool show);
    bool showSunShading()                                   const;

    void setShowCityLights(bool show);
    bool showCityLights()                                   const;

    void setShowTileId(bool show);

    RenderState renderState(const TileId& stackedTileId)    const;

    bool hasTextureLayer()                                  const;

protected:

    Q_DISABLE_COPY(MergedLayerDecorator)

protected:

    class Private;
    Private* const d = nullptr;
};

} // namespace Marble
