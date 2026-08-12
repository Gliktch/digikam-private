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

#include "GeoDataOverlay.h"
#include "MarbleGlobal.h"
#include "GeoDataLatLonBox.h"

namespace Marble
{

class GeoDataGroundOverlayPrivate;
class GeoDataLatLonQuad;

/**
 */
class DIGIKAM_EXPORT GeoDataGroundOverlay: public GeoDataOverlay
{
public:
    GeoDataGroundOverlay();

    GeoDataGroundOverlay(const GeoDataGroundOverlay& other);

    ~GeoDataGroundOverlay() override;

    GeoDataGroundOverlay& operator=(const GeoDataGroundOverlay& other);
    bool operator==(const GeoDataGroundOverlay& other) const;
    bool operator!=(const GeoDataGroundOverlay& other) const;

    GeoDataFeature* clone() const override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    double altitude() const;

    void setAltitude(double altitude);

    AltitudeMode altitudeMode() const;

    void setAltitudeMode(const AltitudeMode altitudeMode);

    const GeoDataLatLonBox& latLonBox() const;

    GeoDataLatLonBox& latLonBox();

    void setLatLonBox(const GeoDataLatLonBox& box);

    const GeoDataLatLonQuad& latLonQuad() const;

    GeoDataLatLonQuad& latLonQuad();

    void setLatLonQuad(const GeoDataLatLonQuad& quad);

private:

    Q_DECLARE_PRIVATE(GeoDataGroundOverlay)
};

} // namespace Marble
