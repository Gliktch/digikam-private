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

#pragma once

// Qt includes

#include <QDateTime>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "geocoordinates.h"
#include "geogroupstate.h"

namespace Digikam
{

class DIGIKAM_EXPORT GPSItemInfo
{
public:

    GPSItemInfo()  = default;
    ~GPSItemInfo() = default;

public:

    static GPSItemInfo fromIdCoordinatesRatingDateTime(const qlonglong p_id,
                                                       const GeoCoordinates& p_coordinates,
                                                       const int p_rating,
                                                       const QDateTime& p_creationDate);

public:

    qlonglong                  id           = -2;
    GeoCoordinates             coordinates;
    int                        rating       = -1;
    QDateTime                  dateTime;
    QUrl                       url;

    typedef QList<GPSItemInfo> List;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::GPSItemInfo)
