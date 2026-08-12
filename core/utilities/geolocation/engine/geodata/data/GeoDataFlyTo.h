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

#include "GeoDataTourPrimitive.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataAbstractView;
class GeoDataFlyToPrivate;

class DIGIKAM_EXPORT GeoDataFlyTo: public GeoDataTourPrimitive
{
public:

    enum FlyToMode
    {
        Bounce,
        Smooth
    };

    GeoDataFlyTo();

    GeoDataFlyTo(const GeoDataFlyTo& other);

    GeoDataFlyTo& operator=(const GeoDataFlyTo& other);
    bool operator==(const GeoDataFlyTo& other) const;
    bool operator!=(const GeoDataFlyTo& other) const;

    ~GeoDataFlyTo() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    double duration() const;

    void setDuration(double duration);

    const GeoDataAbstractView* view() const;

    GeoDataAbstractView* view();

    void setView(GeoDataAbstractView* view);

    FlyToMode flyToMode() const;

    void setFlyToMode(const FlyToMode flyToMode);

private:

    GeoDataFlyToPrivate* const d = nullptr;
};

} // namespace Marble
