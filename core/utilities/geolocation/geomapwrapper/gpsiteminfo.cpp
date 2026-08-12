/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-06
 * Description : Helper class for geomap interaction
 *
 * SPDX-FileCopyrightText: 2011      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsiteminfo.h"

namespace Digikam
{

GPSItemInfo GPSItemInfo::fromIdCoordinatesRatingDateTime(const qlonglong p_id,
                                                         const GeoCoordinates& p_coordinates,
                                                         const int p_rating,
                                                         const QDateTime& p_creationDate)
{
    GPSItemInfo info;
    info.id          = p_id;
    info.coordinates = p_coordinates;
    info.rating      = p_rating;
    info.dateTime    = p_creationDate;

    return info;
}

} // namespace Digikam
