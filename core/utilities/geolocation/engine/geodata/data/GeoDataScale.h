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
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataScalePrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataScale: public GeoDataObject
{
public:

    GeoDataScale();

    GeoDataScale(const GeoDataScale& other);

    GeoDataScale& operator=(const GeoDataScale& other);

    bool operator==(const GeoDataScale& other) const;
    bool operator!=(const GeoDataScale& other) const;

    ~GeoDataScale() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    double x() const;

    void setX(double x);

    double y() const;

    void setY(double y);

    double z() const;

    void setZ(double z);

private:

    GeoDataScalePrivate* const d = nullptr;
};

} // namespace Marble
