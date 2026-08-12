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

#include "GeoDataAbstractView.h"
#include "GeoDataCoordinates.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataCameraPrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataCamera : public GeoDataAbstractView
{
public:

    GeoDataCamera();

    GeoDataCamera(const GeoDataCamera& other);

    GeoDataCamera& operator=(const GeoDataCamera& other);
    bool operator==(const GeoDataCamera& other)                                 const;
    bool operator!=(const GeoDataCamera& other)                                 const;

    ~GeoDataCamera()                                                                  override;

public:

    GeoDataAbstractView* copy()                                                 const override;

    /**
     * @brief set the altitude in a GeoDataCamera object
     * @param altitude latitude
     *
     */
    void setAltitude(qreal altitude);

    /**
     * @brief retrieves the altitude of the GeoDataCamera object
     * @return latitude
     */
    qreal altitude()                                                            const;

    /**
     * @brief set the latitude in a GeoDataCamera object
     * @param latitude latitude
     * @param unit units that lon and lat get measured in
     * (default for Radian: north pole at pi/2, southpole at -pi/2)
     */
    void setLatitude(qreal latitude, GeoDataCoordinates::Unit unit = GeoDataCoordinates::Radian);

    /**
     * @brief retrieves the latitude of the GeoDataCamera object
     * use the unit parameter to switch between Radian and DMS
     * @param unit units that lon and lat get measured in
     * (default for Radian: north pole at pi/2, southpole at -pi/2)
     * @return latitude
     */
    qreal latitude(GeoDataCoordinates::Unit unit = GeoDataCoordinates::Radian)  const;

    /**
     * @brief set the longitude in a GeoDataCamera object
     * @param longitude longitude
     * @param unit units that lon and lat get measured in
     * (default for Radian: north pole at pi/2, southpole at -pi/2)
     */
    void setLongitude(qreal longitude, GeoDataCoordinates::Unit unit = GeoDataCoordinates::Radian);

    /**
     * @brief retrieves the longitude of the GeoDataCamera object
     * use the unit parameter to switch between Radian and DMS
     * @param unit units that lon and lat get measured in
     * (default for Radian: north pole at pi/2, southpole at -pi/2)
     * @return latitude
     */
    qreal longitude(GeoDataCoordinates::Unit unit = GeoDataCoordinates::Radian) const;

    /**
     * @brief retrieve the lat/lon/alt triple as a GeoDataCoordinates object
     * @return GeoDataCoordinates
     * @see longitude latitude altitude
     */
    GeoDataCoordinates coordinates()                                            const override;

    void setRoll(qreal roll);

    qreal roll()                                                                const;

    qreal heading()                                                             const;

    void setHeading(qreal heading);

    qreal tilt()                                                                const;

    void setTilt(qreal tilt);

    void setCoordinates(const GeoDataCoordinates& coordinates);

    /**
     * Provides type information for downcasting a GeoNode
     */
    const char* nodeType()                                                      const override;

    void detach();

private:

    GeoDataCameraPrivate* d = nullptr;
};

} // namespace Marble

Q_DECLARE_METATYPE(Marble::GeoDataCamera)
