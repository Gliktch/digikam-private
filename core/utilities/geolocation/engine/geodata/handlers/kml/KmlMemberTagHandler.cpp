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

#include "KmlMemberTagHandler.h"

// Qt includes

#include <QtDebug>

// Local includes

#include "GeoDataPlacemark.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoDataGeometry.h"
#include "GeoDataTypes.h"
#include "KmlElementDictionary.h"
#include "OsmPlacemarkData.h"
#include "GeoParser.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_MX(member)

GeoNode* KmlmemberTagHandler::parse(GeoParser& parser) const
{
    int memberIndex = parser.attribute("index").toInt();

    /* Only possible case: member of polygon placemark:
     *...
    * <Placemark>
    *      <ExtendedData>
    *          <mx:OsmPlacemarkData>
    *              <mx:member index="-1">
    *                   <mx:OsmPlacemarkData>
    *                       <mx:nd index="0">...</nd>
    *                       <mx:nd index="1">...</nd>
    * ...
    */
    if (parser.parentElement(2).is<GeoDataPlacemark>())
    {
        GeoDataPlacemark* placemark = parser.parentElement(2).nodeAs<GeoDataPlacemark>();
        GeoDataPolygon* polygon = geodata_cast<GeoDataPolygon>(placemark->geometry());

        if (!polygon)
        {
            return nullptr;
        }

        // The memberIndex is used to determine which member this tag represents
        if (memberIndex == -1)
        {
            return &polygon->outerBoundary();
        }

        else
        {
            if (memberIndex >= polygon->innerBoundaries().size())
            {
                return nullptr;
            }

            return &polygon->innerBoundaries()[ memberIndex ];
        }
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
