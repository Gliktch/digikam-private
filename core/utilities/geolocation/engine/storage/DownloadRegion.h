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
#include <QVector>

// Local includes

#include "MarbleGlobal.h"
#include "digikam_export.h"

namespace Marble
{

class DownloadRegionPrivate;
class GeoDataLatLonAltBox;
class GeoDataLineString;
class TileCoordsPyramid;
class MarbleModel;
class TileLayer;
class TextureLayer;

class DIGIKAM_EXPORT DownloadRegion : public QObject
{
    Q_OBJECT

public:

    explicit DownloadRegion(QObject* parent = nullptr);

    void setMarbleModel(MarbleModel* model);

    ~DownloadRegion() override;

    void setTileLevelRange(int const minimumTileLevel, int const maximumTileLevel);

    QVector<TileCoordsPyramid> region(const TileLayer* tileLayer, const GeoDataLatLonAltBox& region) const;

    void setVisibleTileLevel(int const tileLevel);

    /**
     * @brief calculates the region to be downloaded around a path
     */
    QVector<TileCoordsPyramid> fromPath(const TileLayer* tileLayer,
                                        qreal offset,
                                        const GeoDataLineString& path) const;

    QVector<int> validTileLevels(const TileType tileType) const;

private:

    DownloadRegionPrivate* const d = nullptr;
};

} // namespace Marble
