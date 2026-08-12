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

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class DIGIKAM_EXPORT GeoDataAccuracy
{
public:

    enum Level
    {
        none = 0,
        Country,
        Region,
        Locality,
        PostalCode,
        Street,
        Detailed
    };

    explicit GeoDataAccuracy(Level level = none, qreal horizontal = 0.0, qreal vertical = 0.0);

    bool operator==(const GeoDataAccuracy& other) const;

    bool operator!=(const GeoDataAccuracy& other) const;

    /**
     * @brief Approximate descriptive accuracy.
     */
    Level level;

    /**
     * @brief Horizontal accuracy in meters.
     */
    qreal horizontal;

    /**
     * @brief Vertical accuracy in meters.
     */
    qreal vertical;
};

} // namespace Marble
