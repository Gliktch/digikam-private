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

#include "GeoDataAccuracy.h"

namespace Marble
{

GeoDataAccuracy::GeoDataAccuracy(Level level, qreal horizontal, qreal _vertical)
    : level(level),
      horizontal(horizontal),
      vertical(_vertical)
{
    // nothing to do
}

bool GeoDataAccuracy::operator==(const GeoDataAccuracy& other) const
{
    return (level == other.level) && (horizontal == other.horizontal) && (vertical == other.vertical);
}

bool GeoDataAccuracy::operator!=(const GeoDataAccuracy& other) const
{
    return !this->operator==(other);
}

} // namespace Marble
