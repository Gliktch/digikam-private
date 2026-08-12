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

#include "GPXTrackPointExtensionTagHandler.h"

// Local includes

#include "GPXElementDictionary.h"
#include "GeoParser.h"
#include "GeoDataPoint.h"
#include "GeoDataTrack.h"
#include "GeoDataExtendedData.h"
#include "digikam_debug.h"

namespace Marble
{

namespace gpx
{

GPX_DEFINE_TAG_HANDLER_GARMIN_TRACKPOINTEXT1(TrackPointExtension)

GeoNode* GPXTrackPointExtensionTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(gpxTag_TrackPointExtension)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataTrack>())
    {
        GeoDataTrack* track = parentItem.nodeAs<GeoDataTrack>();
        return track;
    }

    return nullptr;
}

} // namespace gpx

} // namespace Marble
