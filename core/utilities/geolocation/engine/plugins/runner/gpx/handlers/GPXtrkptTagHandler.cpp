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

#include "GPXtrkptTagHandler.h"

// Qt includes

#include <QStringView>

// Local includes

#include "GPXElementDictionary.h"
#include "GeoParser.h"
#include "GeoDataLineString.h"
#include "GeoDataCoordinates.h"
#include "GeoDataTrack.h"
#include "digikam_debug.h"

namespace Marble
{

namespace gpx
{

GPX_DEFINE_TAG_HANDLER(trkpt)

GeoNode* GPXtrkptTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(gpxTag_trkpt)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(gpxTag_trkseg))
    {
        GeoDataTrack* track = parentItem.nodeAs<GeoDataTrack>();
        GeoDataCoordinates coord;

        QXmlStreamAttributes attributes = parser.attributes();
        QStringView tmp;
        qreal lat = 0;
        qreal lon = 0;
        tmp = attributes.value(QLatin1String(gpxTag_lat));

        if (!tmp.isEmpty())
        {
            lat = tmp.toString().toDouble();
        }

        tmp = attributes.value(QLatin1String(gpxTag_lon));

        if (!tmp.isEmpty())
        {
            lon = tmp.toString().toDouble();
        }

        coord.set(lon, lat, 0, GeoDataCoordinates::Degree);
        track->appendCoordinates(coord);

        return track;
    }

    return nullptr;
}

} // namespace gpx

} // namespace Marble
