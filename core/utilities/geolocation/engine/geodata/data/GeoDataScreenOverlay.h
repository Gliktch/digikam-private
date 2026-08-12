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
#include "GeoDataVec2.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataScreenOverlayPrivate;

class DIGIKAM_EXPORT GeoDataScreenOverlay: public GeoDataOverlay
{
public:

    GeoDataScreenOverlay();

    GeoDataScreenOverlay(const GeoDataScreenOverlay& other);

    ~GeoDataScreenOverlay() override;

    GeoDataScreenOverlay& operator=(const GeoDataScreenOverlay& other);
    bool operator==(const GeoDataScreenOverlay& other) const;
    bool operator!=(const GeoDataScreenOverlay& other) const;

    GeoDataFeature* clone() const override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    GeoDataVec2 overlayXY() const;
    void setOverlayXY(const GeoDataVec2& vec2);
    void setOverlayXY(qreal x, qreal y, GeoDataVec2::Unit xunit, GeoDataVec2::Unit yunit);

    GeoDataVec2 screenXY() const;
    void setScreenXY(const GeoDataVec2& vec2);
    void setScreenXY(qreal x, qreal y, GeoDataVec2::Unit xunit, GeoDataVec2::Unit yunit);

    GeoDataVec2 rotationXY() const;
    void setRotationXY(const GeoDataVec2& vec2);
    void setRotationXY(qreal x, qreal y, GeoDataVec2::Unit xunit, GeoDataVec2::Unit yunit);

    GeoDataVec2 size() const;
    void setSize(const GeoDataVec2& vec2);
    void setSize(qreal x, qreal y, GeoDataVec2::Unit xunit, GeoDataVec2::Unit yunit);

    qreal rotation() const;
    void setRotation(qreal rotation);

private:

    Q_DECLARE_PRIVATE(GeoDataScreenOverlay)
};

} // namespace Marble
