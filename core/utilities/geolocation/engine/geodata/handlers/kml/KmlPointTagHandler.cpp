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

#include "KmlPointTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataPlacemark.h"
#include "GeoDataPoint.h"
#include "GeoDataMultiGeometry.h"
#include "GeoDataPhotoOverlay.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(Point)

GeoNode* KmlPointTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Point)));
    // FIXME: there needs to be a check that a coordinates subtag is contained

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_Placemark))
    {
        return parentItem.nodeAs<GeoDataPlacemark>();

    }

    else if (parentItem.represents(kmlTag_MultiGeometry))
    {
        GeoDataPoint* point = new GeoDataPoint;
        KmlObjectTagHandler::parseIdentifiers(parser, point);
        parentItem.nodeAs<GeoDataMultiGeometry>()->append(point);
        return point;
    }

    else if (parentItem.represents(kmlTag_PhotoOverlay))
    {
        GeoDataPoint* point = &parentItem.nodeAs<GeoDataPhotoOverlay>()->point();
        KmlObjectTagHandler::parseIdentifiers(parser, point);
        return point;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
