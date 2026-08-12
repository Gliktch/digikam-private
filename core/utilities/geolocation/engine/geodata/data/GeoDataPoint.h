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

#include <cmath>

// Qt includes

#include <QMetaType>
#include <QVector>

// Local includes

#include "GeoDataGeometry.h"
#include "GeoDataCoordinates.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataPointPrivate;

/**
 * @short A Geometry object representing a 3d point
 *
 * GeoDataPoint is the GeoDataGeometry class representing a single three
 * dimensional point. It reflects the Point tag of KML spec and can be contained
 * in objects holding GeoDataGeometry objects.
 * Nevertheless GeoDataPoint shouldn't be used if you just want to store
 * 3d coordinates of a point that doesn't need to be inherited from GeoDataGeometry
 * In that case use GeoDataCoordinates instead which has nearly the same features
 * and is much more light weight.
 * Please consider this especially if you expect to have a high
 * amount of points e.g. for line strings, linear rings and polygons.
 * @see GeoDataCoordinates
 * @see GeoDataGeometry
 */
class DIGIKAM_EXPORT GeoDataPoint : public GeoDataGeometry
{
public:

    using Notation = GeoDataCoordinates::Notation;
    using Unit     = GeoDataCoordinates::Unit;

    GeoDataPoint(const GeoDataPoint& other);
    explicit GeoDataPoint(const GeoDataCoordinates& other);
    GeoDataPoint();

    /**
     * @brief create a geopoint from longitude and latitude
     * @param lon longitude
     * @param lat latitude
     * @param alt altitude (default: 0)
     * @param _unit units that lon and lat get measured in
     * (default for Radian: north pole at pi/2, southpole at -pi/2)
     */
    GeoDataPoint(qreal lon, qreal lat, qreal alt = 0,
                 GeoDataPoint::Unit _unit = GeoDataCoordinates::Radian);

    ~GeoDataPoint() override;

    EnumGeometryId geometryId() const override;

    GeoDataGeometry* copy() const override;

    bool operator==(const GeoDataPoint& other) const;
    bool operator!=(const GeoDataPoint& other) const;

    void setCoordinates(const GeoDataCoordinates& coordinates);

    const GeoDataCoordinates& coordinates() const;

    /// Provides type information for downcasting a GeoData
    const char* nodeType() const override;

    // Type definitions
    using Vector = QVector<GeoDataPoint>;


    // Serialize the Placemark to @p stream
    void pack(QDataStream& stream) const override;
    // Unserialize the Placemark from @p stream
    void unpack(QDataStream& stream) override;

    virtual void detach();

private:

    Q_DECLARE_PRIVATE(GeoDataPoint)
};

} // namespace Marble

Q_DECLARE_METATYPE(Marble::GeoDataPoint)
