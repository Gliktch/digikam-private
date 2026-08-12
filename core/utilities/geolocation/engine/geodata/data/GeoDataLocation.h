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

#include "GeoDataObject.h"
#include "GeoDataCoordinates.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataLocationPrivate;

class DIGIKAM_EXPORT GeoDataLocation: public GeoDataObject
{
public:

    GeoDataLocation();

    GeoDataLocation(const GeoDataLocation& other);

    GeoDataLocation& operator=(const GeoDataLocation& other);

    bool operator==(const GeoDataLocation& other) const;
    bool operator!=(const GeoDataLocation& other) const;

    ~GeoDataLocation() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    qreal latitude(GeoDataCoordinates::Unit) const;

    qreal longitude(GeoDataCoordinates::Unit) const;

    void setLatitude(qreal latitude, GeoDataCoordinates::Unit unit);

    void setLongitude(qreal longitude, GeoDataCoordinates::Unit unit);

    /** Returns the distance to earth's surface in meters, interpreted according to altitudeMode */
    qreal altitude() const;

    void setAltitude(qreal altitude);

protected:

    GeoDataLocationPrivate* const d = nullptr;
};

} // namespace Marble
