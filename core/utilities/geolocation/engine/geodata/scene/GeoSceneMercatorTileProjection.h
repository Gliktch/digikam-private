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

#include "GeoSceneAbstractTileProjection.h"

namespace Marble
{

/**
 * Converts the x and y indices of tiles to and from geo coordinates.
 * For tiles of maps in Mercator projection.
 *
 * Tiles do have the same width and the same height per zoomlevel.
 * The number of tiles per dimension is twice that of the previous lower zoomlevel.
 * The indexing is done in x dimension eastwards, with the first tiles beginning at -180 degree
 * and an x value of 0 and the last tiles ending at +180 degree,
 * in y dimension southwards with the first tiles beginning at +85.05113 degree and a y value of 0
 * and the last tiles ending at -85.05113 degree.
 *
 * @note The method @c tileIndexes() handles any latitude value >= +85.0 degree as
 * exactly +85.0 degree and any latitude value <= -85.0 as exactly -85.0 degree.
 * So for higher zoomlevels the outermost tiles will be masked by that and not included in any results.
 */
class DIGIKAM_EXPORT GeoSceneMercatorTileProjection : public GeoSceneAbstractTileProjection
{
public:

    /**
     * @brief Construct a new GeoSceneMercatorTileProjection.
     */
    GeoSceneMercatorTileProjection();

    ~GeoSceneMercatorTileProjection() override;

public:

    /**
     * @copydoc
     */
    GeoSceneAbstractTileProjection::Type type() const override;

    /**
     * @copydoc
     */
    QRect tileIndexes(const GeoDataLatLonBox& latLonBox, int zoomLevel) const override;

    /**
     * @copydoc
     */
    GeoDataLatLonBox geoCoordinates(int zoomLevel, int x, int y) const override;

    using GeoSceneAbstractTileProjection::geoCoordinates;

private:

    Q_DISABLE_COPY(GeoSceneMercatorTileProjection)
};

} // namespace Marble
