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

// Local includes

#include "TileLayer.h"
#include "MarbleGlobal.h"

class QAbstractItemModel;
class QImage;
class QSize;

namespace Marble
{

class GeoPainter;
class GeoDataDocument;
class GeoSceneGroup;
class GeoSceneAbstractTileProjection;
class GeoSceneTextureTileDataset;
class HttpDownloadManager;
class SunLocator;
class TileId;
class ViewportParams;
class PluginManager;

class DIGIKAM_EXPORT TextureLayer : public TileLayer
{
    Q_OBJECT

public:

    TextureLayer(HttpDownloadManager* downloadManager,
                 PluginManager* pluginManager,
                 const SunLocator* sunLocator,
                 QAbstractItemModel* groundOverlayModel);

    ~TextureLayer()                                                  override;

    void addSeaDocument(const GeoDataDocument* seaDocument);

    void addLandDocument(const GeoDataDocument* landDocument);

    /**
     * @brief Adds texture sublayer, taking ownership of the object's memory
     *        Does nothing if a texture with the same source directory was already
     *        added with this method.
     * @return returned string is the key for the texture that can be later used to remove it
     */
    QString addTextureLayer(GeoSceneTextureTileDataset* texture);

    /**
     * @brief Removes texture sublayer identified by a key.
     *        Deletes the texture object. Does nothing if key is not found.
     * @param A key to identify the texture, returned from addTextureLayer
     */
    void removeTextureLayer(const QString& key);

    bool showSunShading()                                       const;
    bool showCityLights()                                       const;

    /**
     * @brief Return the current tile zoom level. For example for OpenStreetMap
     *        possible values are 1..18, for BlueMarble 0..6.
     */
    int tileZoomLevel()                                         const;

    quint64 volatileCacheLimit()                                const;

    int preferredRadiusCeil(int radius)                         const;
    int preferredRadiusFloor(int radius)                        const;

    int layerCount()                                            const override;

    QSize tileSize()                                            const override;

    const GeoSceneAbstractTileProjection* tileProjection()      const override;

    int tileColumnCount(int level)                              const override;
    int tileRowCount(int level)                                 const override;


    RenderState renderState()                                   const override;

    QString runtimeTrace()                                      const override;

    bool render(GeoPainter* painter, ViewportParams* viewport,
                const QString& renderPos = QLatin1String("NONE"),
                GeoSceneLayer* layer = nullptr)                       override;

public Q_SLOTS:

    void setShowRelief(bool show);

    void setShowSunShading(bool show);

    void setShowCityLights(bool show);

    void setShowTileId(bool show);

    /**
     * @brief  Set the Projection used for the map
     * @param  projection projection type (e.g. Spherical, Equirectangular, Mercator)
     */
    void setProjection(Projection projection);

    void setNeedsUpdate();

    void setMapTheme(const QVector<const GeoSceneTextureTileDataset*>& textures,
                     const GeoSceneGroup* textureLayerSettings,
                     const QString& seaFile,
                     const QString& landFile);

    void setVolatileCacheLimit(quint64 kilobytes);

    void reset();

    void reload();

    void downloadStackedTile(const TileId& stackedTileId);

Q_SIGNALS:

    void tileLevelChanged(int);
    void repaintNeeded();

private:

    Q_PRIVATE_SLOT(d, void requestDelayedRepaint())
    Q_PRIVATE_SLOT(d, void updateTextureLayers())
    Q_PRIVATE_SLOT(d, void updateTile(const TileId& tileId, const QImage& tileImage))
    Q_PRIVATE_SLOT(d, void addGroundOverlays(const QModelIndex& parent, int first, int last))
    Q_PRIVATE_SLOT(d, void removeGroundOverlays(const QModelIndex& parent, int first, int last))
    Q_PRIVATE_SLOT(d, void resetGroundOverlaysCache())

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Marble
