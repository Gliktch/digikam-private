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

#include "ServerLayout.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QUrlQuery>

// Local includes

#include "GeoSceneTileDataset.h"
#include "GeoDataLatLonBox.h"
#include "MarbleGlobal.h"
#include "TileId.h"

namespace Marble
{

ServerLayout::ServerLayout(GeoSceneTileDataset* textureLayer)
    : m_textureLayer(textureLayer)
{
}

ServerLayout::~ServerLayout()
{
}

MarbleServerLayout::MarbleServerLayout(GeoSceneTileDataset* textureLayer)
    : ServerLayout(textureLayer)
{
}

QUrl MarbleServerLayout::downloadUrl(const QUrl& prototypeUrl, const TileId& id) const
{
    const QString path = QString::fromUtf8("%1/%2/%3/%3_%4.%5")
                         .arg(prototypeUrl.path())
                         .arg(id.zoomLevel())
                         .arg(id.y(), tileDigits, 10, QLatin1Char('0'))
                         .arg(id.x(), tileDigits, 10, QLatin1Char('0'))
                         .arg(m_textureLayer->fileFormat().toLower());

    QUrl url = prototypeUrl;
    url.setPath(path);

    return url;
}

QString MarbleServerLayout::name() const
{
    return QString::fromUtf8("Marble");
}

QString ServerLayout::sourceDir() const
{
    return m_textureLayer ? m_textureLayer->sourceDir() : QString();
}


OsmServerLayout::OsmServerLayout(GeoSceneTileDataset* textureLayer)
    : ServerLayout(textureLayer)
{
}

QUrl OsmServerLayout::downloadUrl(const QUrl& prototypeUrl, const TileId& id) const
{
    const QString suffix = m_textureLayer->fileFormat().toLower();
    const QString path = QString::fromUtf8("%1/%2/%3.%4").arg(id.zoomLevel())
                         .arg(id.x())
                         .arg(id.y())
                         .arg(suffix);

    QUrl url = prototypeUrl;
    url.setPath(url.path() + path);

    return url;
}

QString OsmServerLayout::name() const
{
    return QString::fromUtf8("OpenStreetMap");
}


CustomServerLayout::CustomServerLayout(GeoSceneTileDataset* texture)
    : ServerLayout(texture)
{
}

QUrl CustomServerLayout::downloadUrl(const QUrl& prototypeUrl, const TileId& id) const
{
    const GeoDataLatLonBox bbox = m_textureLayer->tileProjection()->geoCoordinates(id);

    QString urlStr = prototypeUrl.toString(QUrl::DecodeReserved);

    urlStr.replace(QString::fromUtf8("{z}"), QString::number(id.zoomLevel()));
    urlStr.replace(QString::fromUtf8("{zoomLevel}"), QString::number(id.zoomLevel()));
    urlStr.replace(QString::fromUtf8("{x}"), QString::number(id.x()));
    urlStr.replace(QString::fromUtf8("{y}"), QString::number(id.y()));
    urlStr.replace(QString::fromUtf8("{west}"), QString::number(bbox.west(GeoDataCoordinates::Degree), 'f', 12));
    urlStr.replace(QString::fromUtf8("{south}"), QString::number(bbox.south(GeoDataCoordinates::Degree), 'f', 12));
    urlStr.replace(QString::fromUtf8("{east}"), QString::number(bbox.east(GeoDataCoordinates::Degree), 'f', 12));
    urlStr.replace(QString::fromUtf8("{north}"), QString::number(bbox.north(GeoDataCoordinates::Degree), 'f', 12));

    return QUrl(urlStr);
}

QString CustomServerLayout::name() const
{
    return QString::fromUtf8("Custom");
}

WmsServerLayout::WmsServerLayout(GeoSceneTileDataset* texture)
    : ServerLayout(texture)
{
}

QUrl WmsServerLayout::downloadUrl(const QUrl& prototypeUrl, const Marble::TileId& tileId) const
{
    const GeoDataLatLonBox box = m_textureLayer->tileProjection()->geoCoordinates(tileId);

    QUrlQuery url(prototypeUrl.query());
    url.addQueryItem(QString::fromUtf8("service"), QString::fromUtf8("WMS"));
    url.addQueryItem(QString::fromUtf8("request"), QString::fromUtf8("GetMap"));
    url.addQueryItem(QString::fromUtf8("version"), QString::fromUtf8("1.1.1"));

    if (!url.hasQueryItem(QString::fromUtf8("styles")))
    {
        url.addQueryItem(QString::fromUtf8("styles"), QString::fromUtf8(""));
    }

    if (!url.hasQueryItem(QString::fromUtf8("format")))
    {
        url.addQueryItem(QString::fromUtf8("format"), QLatin1String("image/") + m_textureLayer->fileFormat().toLower());
    }

    if (!url.hasQueryItem(QString::fromUtf8("srs")))
    {
        url.addQueryItem(QString::fromUtf8("srs"), epsgCode());
    }

    if (!url.hasQueryItem(QString::fromUtf8("layers")))
    {
        url.addQueryItem(QString::fromUtf8("layers"), m_textureLayer->name());
    }

    url.addQueryItem(QString::fromUtf8("width"), QString::number(m_textureLayer->tileSize().width()));
    url.addQueryItem(QString::fromUtf8("height"), QString::number(m_textureLayer->tileSize().height()));
    double west, south, east, north;

    if (m_textureLayer->tileProjectionType() == GeoSceneAbstractTileProjection::Mercator)
    {
        // Oddly enough epsg:3857 is measured in meters - so let's convert this accordingly
        west = (box.west(GeoDataCoordinates::Degree) * 20037508.34) / 180;
        south = 20037508.34 / M_PI * log(tan(((90 + box.south(GeoDataCoordinates::Degree)) * M_PI) / 360));
        east = (box.east(GeoDataCoordinates::Degree) * 20037508.34) / 180;
        north = 20037508.34 / M_PI * log(tan(((90 + box.north(GeoDataCoordinates::Degree)) * M_PI) / 360));
    }

    else
    {
        west = box.west(GeoDataCoordinates::Degree);
        south = box.south(GeoDataCoordinates::Degree);
        east = box.east(GeoDataCoordinates::Degree);
        north = box.north(GeoDataCoordinates::Degree);
    }

    url.addQueryItem(QString::fromUtf8("bbox"), QString::fromUtf8("%1,%2,%3,%4").arg(QString::number(west, 'f', 12),
                                                                                     QString::number(south, 'f', 12),
                                                                                     QString::number(east, 'f', 12),
                                                                                     QString::number(north, 'f', 12)));
    QUrl finalUrl = prototypeUrl;
    finalUrl.setQuery(url);
    return finalUrl;
}

QString WmsServerLayout::name() const
{
    return QString::fromUtf8("WebMapService");
}

QString WmsServerLayout::epsgCode() const
{
    switch (m_textureLayer->tileProjectionType())
    {
        case GeoSceneAbstractTileProjection::Equirectangular:
            return QString::fromUtf8("EPSG:4326");

        case GeoSceneAbstractTileProjection::Mercator:
            return QString::fromUtf8("EPSG:3857");
    }

    Q_ASSERT(false);   // not reached
    return QString();
}

WmtsServerLayout::WmtsServerLayout(GeoSceneTileDataset* texture)
    : ServerLayout(texture)
{
}

QUrl WmtsServerLayout::downloadUrl(const QUrl& prototypeUrl, const Marble::TileId& id) const
{
    //    const GeoDataLatLonBox bbox = m_textureLayer->tileProjection()->geoCoordinates(id);

    QString urlStr = prototypeUrl.toString(QUrl::DecodeReserved);

    urlStr.replace(urlStr.indexOf(QLatin1String("{TileMatrix}")), QLatin1String("{TileMatrix}").size(),  QString::number(id.zoomLevel()));
    urlStr.replace(urlStr.indexOf(QLatin1String("{TileRow}")), QLatin1String("{TileRow}").size(),  QString::number(id.y()));
    urlStr.replace(urlStr.indexOf(QLatin1String("{TileCol}")), QLatin1String("{TileCol}").size(),  QString::number(id.x()));
    return QUrl(urlStr);
}

QString WmtsServerLayout::name() const
{
    return QString::fromUtf8("WebMapTileService");
}

QString WmtsServerLayout::epsgCode() const
{
    switch (m_textureLayer->tileProjectionType())
    {
        case GeoSceneAbstractTileProjection::Equirectangular:
            return QString::fromUtf8("EPSG:4326");

        case GeoSceneAbstractTileProjection::Mercator:
            return QString::fromUtf8("EPSG:3857");
    }

    Q_ASSERT(false);   // not reached
    return QString();
}


QuadTreeServerLayout::QuadTreeServerLayout(GeoSceneTileDataset* textureLayer)
    : ServerLayout(textureLayer)
{
}

QUrl QuadTreeServerLayout::downloadUrl(const QUrl& prototypeUrl, const Marble::TileId& id) const
{
    QString urlStr = prototypeUrl.toString(QUrl::DecodeReserved);

    urlStr.replace(QString::fromUtf8("{quadIndex}"), encodeQuadTree(id));

    return QUrl(urlStr);
}

QString QuadTreeServerLayout::name() const
{
    return QString::fromUtf8("QuadTree");
}

QString QuadTreeServerLayout::encodeQuadTree(const Marble::TileId& id)
{
    QString tileNum;

    for (int i = id.zoomLevel(); i >= 0; i--)
    {
        const int tileX = (id.x() >> i) % 2;
        const int tileY = (id.y() >> i) % 2;
        const int num = (2 * tileY) + tileX;

        tileNum += QString::number(num);
    }

    return tileNum;
}

TmsServerLayout::TmsServerLayout(GeoSceneTileDataset* textureLayer)
    : ServerLayout(textureLayer)
{
}

QUrl TmsServerLayout::downloadUrl(const QUrl& prototypeUrl, const TileId& id) const
{
    const QString suffix = m_textureLayer->fileFormat().toLower();
    // y coordinate in TMS start at the bottom of the map (South) and go upwards,
    // opposed to OSM which start at the top.
    //
    // https://wiki.osgeo.org/wiki/Tile_Map_Service_Specification
    int y_frombottom = (1 << id.zoomLevel()) - id.y() - 1 ;

    const QString path = QString::fromUtf8("%1/%2/%3.%4").arg(id.zoomLevel())
                         .arg(id.x())
                         .arg(y_frombottom)
                         .arg(suffix);
    QUrl url = prototypeUrl;
    url.setPath(url.path() + path);

    return url;
}

QString TmsServerLayout::name() const
{
    return QString::fromUtf8("TileMapService");
}

} // namespace Marble
