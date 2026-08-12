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

#include "ElevationModel.h"

// Qt includes

#include <QCache>
#include <QImage>
#include <qmath.h>

// Local includes

#include "GeoSceneHead.h"
#include "GeoSceneLayer.h"
#include "GeoSceneMap.h"
#include "GeoSceneDocument.h"
#include "GeoSceneTextureTileDataset.h"
#include "HttpDownloadManager.h"
#include "Tile.h"
#include "TileLoader.h"
#include "TileLoaderHelper.h"
#include "MapThemeManager.h"
#include "TileId.h"
#include "PluginManager.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN ElevationModelPrivate
{
public:

    ElevationModelPrivate(ElevationModel* _q,
                          HttpDownloadManager* downloadManager,
                          PluginManager* pluginManager)
        : q             (_q),
          m_tileLoader  (downloadManager, pluginManager),
          m_textureLayer(nullptr),
          m_srtmTheme   (nullptr)
    {
        m_cache.setMaxCost(10);   //keep 10 tiles in memory (~17MB)

        m_srtmTheme = MapThemeManager::loadMapTheme(QString::fromUtf8("earth/srtm2/srtm2.dgml"));

        if (!m_srtmTheme)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Failed to load map theme earth/srtm2/srtm2.dgml. Check your installation. No elevation will be returned.";

            return;
        }

        const GeoSceneHead* head        = m_srtmTheme->head();

        Q_ASSERT(head);

        const GeoSceneMap* map          = m_srtmTheme->map();

        Q_ASSERT(map);

        const GeoSceneLayer* sceneLayer = map->layer(head->theme());

        if (!sceneLayer)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Failed to instantiate elevation map. No elevation will be returned.";
            return;
        }

        Q_ASSERT(sceneLayer);

        m_textureLayer                  = dynamic_cast<GeoSceneTextureTileDataset*>(sceneLayer->datasets().first());

        Q_ASSERT(m_textureLayer);
    }

    ~ElevationModelPrivate()
    {
        delete m_srtmTheme;
    }

    void tileCompleted(const TileId& tileId, const QImage& image)
    {
        m_cache.insert(tileId, new QImage(image));

        Q_EMIT q->updateAvailable();
    }

public:

    ElevationModel*                     q               = nullptr;

    TileLoader                          m_tileLoader;
    const GeoSceneTextureTileDataset*   m_textureLayer  = nullptr;
    QCache<TileId, const QImage>        m_cache;
    GeoSceneDocument*                   m_srtmTheme     = nullptr;

private:

    ElevationModelPrivate(const ElevationModelPrivate&)            = delete;
    ElevationModelPrivate& operator=(const ElevationModelPrivate&) = delete;
};

ElevationModel::ElevationModel(HttpDownloadManager* downloadManager, PluginManager* pluginManager, QObject* parent) :
    QObject(parent),
    d      (new ElevationModelPrivate(this, downloadManager, pluginManager))
{
    connect(&d->m_tileLoader, SIGNAL(tileCompleted(TileId,QImage)),
            this, SLOT(tileCompleted(TileId,QImage)));
}

ElevationModel::~ElevationModel()
{
    delete d;
}

qreal ElevationModel::height(qreal lon, qreal lat) const
{
    if (!d->m_textureLayer)
    {
        return invalidElevationData;
    }

    const int tileZoomLevel = TileLoader::maximumTileLevel(*(d->m_textureLayer));

    Q_ASSERT(tileZoomLevel == 9);

    const int width         = d->m_textureLayer->tileSize().width();
    const int height        = d->m_textureLayer->tileSize().height();

    const int numTilesX     = TileLoaderHelper::levelToColumn(d->m_textureLayer->levelZeroColumns(), tileZoomLevel);
    const int numTilesY     = TileLoaderHelper::levelToRow(d->m_textureLayer->levelZeroRows(), tileZoomLevel);

    Q_ASSERT(numTilesX > 0);
    Q_ASSERT(numTilesY > 0);

    qreal textureX = 180 + lon;
    textureX      *= numTilesX * width / 360;

    qreal textureY = 90 - lat;
    textureY      *= numTilesY * height / 180;

    qreal ret      = 0;
    bool hasHeight = false;
    qreal noData   = 0;

    for (int i = 0 ; i < 4 ; ++i)
    {
        const int x = static_cast<int>(textureX + (i % 2));
        const int y = static_cast<int>(textureY + (i / 2));

        //qCDebug(DIGIKAM_GEOENGINE_LOG) << "x" << x << ( x / width );
        //qCDebug(DIGIKAM_GEOENGINE_LOG) << "y" << y << ( y / height );

        const TileId id(0, tileZoomLevel, (x % (numTilesX * width)) / width, (y % (numTilesY * height)) / height);

        //qCDebug(DIGIKAM_GEOENGINE_LOG) << "LAT" << lat << "LON" << lon << "tile" << ( x % ( numTilesX * width ) ) / width << ( y % ( numTilesY * height ) ) / height;

        const QImage* image = d->m_cache[id];

        if (image == nullptr)
        {
            image = new QImage(d->m_tileLoader.loadTileImage(d->m_textureLayer, id, DownloadBrowse));
            d->m_cache.insert(id, image);
        }

        Q_ASSERT(image);
        Q_ASSERT(!image->isNull());
        Q_ASSERT(width == image->width());
        Q_ASSERT(height == image->height());

        const qreal dx = (textureX > (qreal)x) ? textureX - (qreal)x : (qreal)x - textureX;
        const qreal dy = (textureY > (qreal)y) ? textureY - (qreal)y : (qreal)y - textureY;

        Q_ASSERT((0 <= dx) && (dx <= 1));
        Q_ASSERT((0 <= dy) && (dy <= 1));

        unsigned int pixel  = image->pixel(x % width, y % height) & 0xffff;   // 16 valid bits
        short int elevation = (short int) pixel; // and signed type, so just cast it

        //qCDebug(DIGIKAM_GEOENGINE_LOG) << "(1-dx)" << (1-dx) << "(1-dy)" << (1-dy);

        if (pixel != invalidElevationData)     //no data?
        {
            //qCDebug(DIGIKAM_GEOENGINE_LOG) << "got at x" << x % width << "y" << y % height << "a height of" << pixel << "** RGB" << qRed(pixel) << qGreen(pixel) << qBlue(pixel);

            ret += (qreal)elevation * (1 - dx) * (1 - dy);
            hasHeight = true;
        }
        else
        {
            //qCDebug(DIGIKAM_GEOENGINE_LOG) << "no data at" <<  x % width << "y" << y % height;

            noData += (1 - dx) * (1 - dy);
        }
    }

    if (!hasHeight)
    {
        ret = invalidElevationData; //no data
    }
    else
    {
        if (noData)
        {
            //qCDebug(DIGIKAM_GEOENGINE_LOG) << "NO DATA" << noData;

            ret += (ret / (1 - noData)) * noData;
        }
    }

    //qCDebug(DIGIKAM_GEOENGINE_LOG) << ">>>" << lat << lon << "returning an elevation of" << ret;

    return ret;
}

QVector<GeoDataCoordinates> ElevationModel::heightProfile(qreal fromLon, qreal fromLat, qreal toLon, qreal toLat) const
{
    if (!d->m_textureLayer)
    {
        return QVector<GeoDataCoordinates>();
    }

    const int tileZoomLevel = TileLoader::maximumTileLevel(*(d->m_textureLayer));
    const int width         = d->m_textureLayer->tileSize().width();
    const int numTilesX     = TileLoaderHelper::levelToColumn(d->m_textureLayer->levelZeroColumns(), tileZoomLevel);
    qreal distPerPixel      = (qreal)360 / (width * numTilesX);

    //qCDebug(DIGIKAM_GEOENGINE_LOG) << "heightProfile" << fromLat << fromLon << toLat << toLon << "distPerPixel" << distPerPixel;

    qreal lat   = fromLat;
    qreal lon   = fromLon;
    char dirLat = fromLat < toLat ? 1 : -1;
    char dirLon = fromLon < toLon ? 1 : -1;
    qreal k     = qAbs((fromLat - toLat) / (fromLon - toLon));

    //qCDebug(DIGIKAM_GEOENGINE_LOG) << "fromLon" << fromLon << "fromLat" << fromLat;
    //qCDebug(DIGIKAM_GEOENGINE_LOG) << "diff lon" << ( fromLon - toLon ) << "diff lat" << ( fromLat - toLat );
    //qCDebug(DIGIKAM_GEOENGINE_LOG) << "dirLon" << QString::number(dirLon) << "dirLat" << QString::number(dirLat) << "k" << k;

    QVector<GeoDataCoordinates> ret;

    while (((lat * dirLat) <= (toLat * dirLat)) && ((lon * dirLon) <= (toLon * dirLon)))
    {
        //qCDebug(DIGIKAM_GEOENGINE_LOG) << lat << lon;

        qreal h = height(lon, lat);

        if (h < 32000)
        {
            ret << GeoDataCoordinates(lon, lat, h, GeoDataCoordinates::Degree);
        }

        if (k < 0.5)
        {
            //qCDebug(DIGIKAM_GEOENGINE_LOG) << "lon(x) += distPerPixel";

            lat += distPerPixel * k * dirLat;
            lon += distPerPixel * dirLon;
        }
        else
        {
            //qCDebug(DIGIKAM_GEOENGINE_LOG) << "lat(y) += distPerPixel";

            lat += distPerPixel * dirLat;
            lon += distPerPixel / k * dirLon;
        }
    }

    //qCDebug(DIGIKAM_GEOENGINE_LOG) << ret;

    return ret;
}

} // namespace Marble

#include "moc_ElevationModel.cpp"
