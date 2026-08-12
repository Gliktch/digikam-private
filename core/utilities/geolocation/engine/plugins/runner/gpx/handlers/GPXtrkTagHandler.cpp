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

#include "GPXtrkTagHandler.h"

// Local includes

#include "GPXElementDictionary.h"
#include "GeoParser.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "GeoDataPoint.h"
#include "GeoDataMultiGeometry.h"
#include "digikam_debug.h"

namespace Marble
{

namespace gpx
{

GPX_DEFINE_TAG_HANDLER(trk)

GeoNode* GPXtrkTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(gpxTag_trk)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(gpxTag_gpx))
    {
        GeoDataDocument* doc = parentItem.nodeAs<GeoDataDocument>();
        GeoDataPlacemark* placemark = new GeoDataPlacemark;
        doc->append(placemark);
        GeoDataMultiGeometry* multigeometry = new GeoDataMultiGeometry;
        placemark->setGeometry(multigeometry);
        placemark->setStyleUrl(QStringLiteral("#map-track"));

        return placemark;
    }

    return nullptr;
}

} // namespace gpx

} // namespace Marble
