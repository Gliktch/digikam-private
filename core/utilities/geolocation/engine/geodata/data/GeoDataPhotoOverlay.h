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
#include "digikam_export.h"

namespace Marble
{

class GeoDataPhotoOverlayPrivate;
class GeoDataPoint;
class GeoDataImagePyramid;
class GeoDataViewVolume;

class DIGIKAM_EXPORT GeoDataPhotoOverlay: public GeoDataOverlay
{
public:

    GeoDataPhotoOverlay();

    GeoDataPhotoOverlay(const GeoDataPhotoOverlay& other);

    ~GeoDataPhotoOverlay() override;

    GeoDataPhotoOverlay& operator=(const GeoDataPhotoOverlay& other);
    bool operator==(const GeoDataPhotoOverlay& other) const;
    bool operator!=(const GeoDataPhotoOverlay& other) const;

    GeoDataFeature* clone() const override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    enum Shape
    {
        Rectangle,
        Cylinder,
        Sphere
    };

    qreal rotation() const;
    void setRotation(const qreal rotation);

    GeoDataViewVolume& viewVolume();
    const GeoDataViewVolume& viewVolume() const;
    void setViewVolume(const GeoDataViewVolume& viewVolume);

    GeoDataImagePyramid& imagePyramid();
    const GeoDataImagePyramid& imagePyramid() const;
    void setImagePyramid(const GeoDataImagePyramid& imagePyramid);

    GeoDataPoint& point();
    const GeoDataPoint& point() const;
    void setPoint(const GeoDataPoint& point);

    Shape shape() const;
    void setShape(Shape shape);

private:

    Q_DECLARE_PRIVATE(GeoDataPhotoOverlay)
};

} // namespace Marble
