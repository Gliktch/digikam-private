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

// Local includes

#include "digikam_export.h"

class QImage;

namespace Marble
{

class GeoDataCoordinates;

namespace
{
unsigned int const invalidElevationData = 32768;
}

class TileId;
class ElevationModelPrivate;
class HttpDownloadManager;
class PluginManager;

class DIGIKAM_EXPORT ElevationModel : public QObject
{
    Q_OBJECT

public:

    explicit ElevationModel(HttpDownloadManager* downloadManager, PluginManager* pluginManager, QObject* parent = nullptr);
    ~ElevationModel() override;

    qreal height(qreal lon, qreal lat) const;
    QVector<GeoDataCoordinates> heightProfile(qreal fromLon, qreal fromLat, qreal toLon, qreal toLat) const;

Q_SIGNALS:

    /**
     * Elevation tiles loaded. You will get more accurate results when querying height
     * for at least one that was queried before.
     **/
    void updateAvailable();

private:

    Q_PRIVATE_SLOT(d, void tileCompleted(const TileId&, const QImage&))

private:

    friend class ElevationModelPrivate;

    ElevationModelPrivate* d = nullptr;
};

} // namespace Marble
