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

// C++ includes

#include <QSize>
#include <QImage>

// Local includes

#include "GeoSceneTileDataset.h"
#include "MarbleMath.h"
#include "MathHelper.h"

namespace Marble
{

class StackedTile;
class StackedTileLoader;
class ViewportParams;

class ScanlineTextureMapperContext
{
public:

    ScanlineTextureMapperContext(StackedTileLoader* const tileLoader, int tileLevel);

    void pixelValueF(const qreal lon, const qreal lat,
                     QRgb* const scanLine);
    void pixelValue(const qreal lon, const qreal lat,
                    QRgb* const scanLine);

    void pixelValueApproxF(const qreal lon, const qreal lat,
                           QRgb* scanLine, const int n);
    void pixelValueApprox(const qreal lon, const qreal lat,
                          QRgb* scanLine, const int n);

    static int interpolationStep(const ViewportParams* viewport, MapQuality mapQuality);

    static QImage::Format optimalCanvasImageFormat(const ViewportParams* viewport);

    int globalWidth() const;
    int globalHeight() const;

private:

    // method for fast integer calculation
    void nextTile(int& posx, int& posy);

    // method for precise interpolation
    void nextTile(qreal& posx, qreal& posy);

    // Converts Radian to global texture coordinates
    // ( with origin in center, measured in pixel)
    qreal rad2PixelX(const qreal lon) const;
    qreal rad2PixelY(const qreal lat) const;

    // Checks whether the pixelValueApprox method will make use of more than
    // one tile
    bool isOutOfTileRange(const int itLon, const int itLat,
                          const int itStepLon, const int itStepLat,
                          const int n) const;

    bool isOutOfTileRangeF(const qreal itLon, const qreal itLat,
                           const qreal itStepLon, const qreal itStepLat,
                           const int n) const;

private:

    StackedTileLoader* const                    m_tileLoader;
    GeoSceneAbstractTileProjection::Type const  m_textureProjection;
    /// size of the tiles of the current texture layer
    QSize const                                 m_tileSize;

    int const                                   m_tileLevel;
    int const                                   m_globalWidth;
    int const                                   m_globalHeight;
    qreal const                                 m_normGlobalWidth;
    qreal const                                 m_normGlobalHeight;

    const StackedTile*                          m_tile = nullptr;

    // Coordinate transformations:

    // Position of the tile in global Texture Coordinates
    // ( with origin in upper left corner, measured in pixel)
    int                                         m_tilePosX;
    int                                         m_tilePosY;

    // Converts global texture coordinates
    // ( with origin in center, measured in pixel)
    // to tile coordinates ( measured in pixel )
    qreal                                       m_toTileCoordinatesLon;
    qreal                                       m_toTileCoordinatesLat;

    // Previous coordinates
    qreal                                       m_prevLat;
    qreal                                       m_prevLon;
    qreal                                       m_prevPixelX;
    qreal                                       m_prevPixelY;
};

inline int ScanlineTextureMapperContext::globalWidth() const
{
    return m_globalWidth;
}

inline int ScanlineTextureMapperContext::globalHeight() const
{
    return m_globalHeight;
}

inline qreal ScanlineTextureMapperContext::rad2PixelX(const qreal lon) const
{
    return lon * m_normGlobalWidth;
}

inline qreal ScanlineTextureMapperContext::rad2PixelY(const qreal lat) const
{
    switch (m_textureProjection)
    {
        case GeoSceneAbstractTileProjection::Equirectangular:
            return -lat * m_normGlobalHeight;

        case GeoSceneAbstractTileProjection::Mercator:
            if (fabs(lat) < 1.4835)
            {
                // We develop the inverse Gudermannian into a MacLaurin Series:
                // In spite of the many elements needed to get decent
                // accuracy this is still faster by far than calculating the
                // trigonometric expression:
                // return - asinh( tan( lat ) ) * 0.5 * m_normGlobalHeight;

                // We are using the Horner Scheme as a polynom representation

                return - gdInv(lat) * 0.5 * m_normGlobalHeight;
            }

            if (lat >= +1.4835)
                // asinh( tan (1.4835)) => 3.1309587
            {
                return - 3.1309587 * 0.5 * m_normGlobalHeight;
            }

            if (lat <= -1.4835)
                // asinh( tan( -1.4835 )) => −3.1309587
            {
                return 3.1309587 * 0.5 * m_normGlobalHeight;
            }
    }

    // Dummy value to avoid a warning.
    return 0.0;
}

} // Namespace Marble
