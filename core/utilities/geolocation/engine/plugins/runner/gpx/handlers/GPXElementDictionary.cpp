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

#include "GPXElementDictionary.h"

namespace Marble
{

namespace gpx
{

/**
 * One static list of all tag names, to avoid string comparisons all-over-the-place.
 */

const char gpxTag_nameSpace10[] = "http://www.topografix.com/GPX/1/0";              // krazy:exclude=insecurenet
const char gpxTag_nameSpace11[] = "http://www.topografix.com/GPX/1/1";              // krazy:exclude=insecurenet

const char gpxTag_ele[] = "ele";
const char gpxTag_extensions[] = "extensions";
const char gpxTag_gpx[] = "gpx";
const char gpxTag_lat[] = "lat";
const char gpxTag_lon[] = "lon";
const char gpxTag_name[] = "name";
const char gpxTag_time[] = "time";
const char gpxTag_desc[] = "desc";
const char gpxTag_type[] = "type";
const char gpxTag_trk[] = "trk";
const char gpxTag_trkpt[] = "trkpt";
const char gpxTag_trkseg[] = "trkseg";
const char gpxTag_wpt[] = "wpt";
const char gpxTag_rte[] = "rte";
const char gpxTag_rtept[] = "rtept";
const char gpxTag_cmt [] = "cmt";
const char gpxTag_link [] = "link";
const char gpxTag_url [] = "url";
const char gpxTag_urlname [] = "urlname";

const char gpxTag_nameSpaceGarminTrackPointExt1[] = "http://www.garmin.com/xmlschemas/TrackPointExtension/v1";  // krazy:exclude=insecurenet
const char gpxTag_TrackPointExtension[] = "TrackPointExtension";
const char gpxTag_hr[] = "hr";

} // namespace gpx

} // namespace Marble
