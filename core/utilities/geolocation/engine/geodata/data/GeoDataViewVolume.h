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
#include "digikam_export.h"

namespace Marble
{

class GeoDataViewVolumePrivate;

class DIGIKAM_EXPORT GeoDataViewVolume : public GeoDataObject
{
public:

    GeoDataViewVolume();

    GeoDataViewVolume(const GeoDataViewVolume& other);

    GeoDataViewVolume& operator=(const GeoDataViewVolume& other);
    bool operator==(const GeoDataViewVolume& other) const;
    bool operator!=(const GeoDataViewVolume& other) const;

    ~GeoDataViewVolume() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    qreal leftFov() const;
    void setLeftFov(qreal leftFov);

    qreal rightFov() const;
    void setRightFov(qreal rightFov);

    qreal bottomFov() const;
    void setBottomFov(qreal bottomFov);

    qreal topFov() const;
    void setTopFov(qreal topFov);

    qreal near() const;
    void setNear(qreal near);

private:

    GeoDataViewVolumePrivate* const d = nullptr;
};

} // namespace Marble
