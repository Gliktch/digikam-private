/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : GPS data file parser.
 *               (GPX format https://www.topografix.com/gpx.asp).
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QMap>
#include <QUrl>

// Local includes

#include "geodatacontainer.h"

namespace DigikamGenericGeolocationEditPlugin
{

class GeoDataParser
{

public:

    GeoDataParser();
    ~GeoDataParser() = default;

    bool loadGPXFile(const QUrl& url);

    void clear();
    int  numPoints() const;
    bool matchDate(const QDateTime& photoDateTime, int maxGapTime, int secondsOffset,
                   bool photoHasSystemTimeZone,
                   bool interpolate, int interpolationDstTime,
                   GeoDataContainer* const gpsData);

private:

    /// Methods used to perform interpolation.

    QDateTime findNextDate(const QDateTime& dateTime, int secs);
    QDateTime findPrevDate(const QDateTime& dateTime, int secs);

protected:

    typedef QMap<QDateTime, GeoDataContainer> GeoDataMap;

    GeoDataMap                                m_GeoDataMap;
};

} // namespace DigikamGenericGeolocationEditPlugin
