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

#include "GPXdescTagHandler.h"

// Local includes

#include "GPXElementDictionary.h"
#include "GeoParser.h"
#include "GeoDataPlacemark.h"
#include "GeoDataPoint.h"
#include "digikam_debug.h"

namespace Marble
{

namespace gpx
{

GPX_DEFINE_TAG_HANDLER(desc)

GeoNode* GPXdescTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(gpxTag_desc)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(gpxTag_wpt)
        || parentItem.represents(gpxTag_trk)
        || parentItem.represents(gpxTag_rtept))
    {
        GeoDataPlacemark* placemark = parentItem.nodeAs<GeoDataPlacemark>();

        QString desc = placemark->description();

        if (!desc.isEmpty())
        {
            desc += QLatin1String("<br>");
        }

        placemark->setDescription(desc.append(parser.readElementText().trimmed().replace(QLatin1Char('\n'), QLatin1String("\n<br>"))));
        placemark->setDescriptionCDATA(true);

    }

    else if (parentItem.represents(gpxTag_rte))
    {
        GeoDataFeature* route = parentItem.nodeAs<GeoDataFeature>();
        QString desc = route->description();

        if (!desc.isEmpty())
        {
            desc += QLatin1String("<br>");
        }

        route->setDescription(desc.append(parser.readElementText().trimmed().replace(QLatin1Char('\n'), QLatin1String("\n<br>"))));
        route->setDescriptionCDATA(true);
    }

    return nullptr;
}

} // namespace gpx

} // namespace Marble
