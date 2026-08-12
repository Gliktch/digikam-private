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

#include "KmlOsmPlacemarkDataTagHandler.h"

// Qt includes

#include <QVariant>

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataExtendedData.h"
#include "GeoDataGeometry.h"
#include "GeoDataPlacemark.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoDataData.h"
#include "GeoParser.h"
#include "GeoDataPoint.h"
#include "OsmPlacemarkData.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_MX(OsmPlacemarkData)

GeoNode* KmlOsmPlacemarkDataTagHandler::parse(GeoParser& parser) const
{
    OsmPlacemarkData osmData = OsmPlacemarkData::fromParserAttributes(parser.attributes());

    /* Case 1: This is the main OsmPlacemarkData of a placemark:
     * <Placemark>
     *      <ExtendedData>
     *          <mx:OsmPlacemarkData>
     * ...
     */
    if (
        parser.parentElement().is<GeoDataExtendedData>() &&
        parser.parentElement(1).is<GeoDataPlacemark>()
       )
    {
        auto placemark = parser.parentElement(1).nodeAs<GeoDataPlacemark>();
        placemark->setOsmData(osmData);

        return &placemark->osmData();
    }

    /* Case 2: This is the OsmPlacemarkData of a Nd
     * <Placemark>
     *      <ExtendedData>
     *          <mx:OsmPlacemarkData>
     *              <mx:nd>
     *                  <mx:OsmPlacemarkData>
     * ...
     */
    else if (
             parser.parentElement(1).is<OsmPlacemarkData>() &&
             parser.parentElement().is<GeoDataPoint>()
            )
    {
        OsmPlacemarkData* placemarkOsmData  = parser.parentElement(1).nodeAs<OsmPlacemarkData>();
        GeoDataPoint* point                 = parser.parentElement().nodeAs<GeoDataPoint>();
        GeoDataCoordinates coordinates      = point->coordinates();

        /* The GeoDataPoint object was only used as GeoNode wrapper for the GeoDataCoordinates
         * and it is no longer needed
         */
        delete point;

        placemarkOsmData->addNodeReference(coordinates, osmData);

        return &placemarkOsmData->nodeReference(coordinates);
    }

    /* Case 3: This is the OsmPlacemarkData of a polygon's member
     * <Placemark>
     *      <ExtendedData>
     *          <mx:OsmPlacemarkData>
     *              <mx:member>
     *                  <mx:OsmPlacemarkData>
     * ...
     */
    else if (
             parser.parentElement(1).is<OsmPlacemarkData>() &&
             parser.parentElement().is<GeoDataLinearRing>() &&
             parser.parentElement(3).is<GeoDataPlacemark>()
            )
    {
        OsmPlacemarkData* placemarkOsmData = parser.parentElement(1).nodeAs<OsmPlacemarkData>();
        GeoDataPlacemark* placemark        = parser.parentElement(3).nodeAs<GeoDataPlacemark>();
        const GeoDataLinearRing& ring      = *parser.parentElement().nodeAs<GeoDataLinearRing>();
        GeoDataPolygon* polygon            = geodata_cast<GeoDataPolygon>(placemark->geometry());

        if (!polygon)
        {
            return nullptr;
        }

        /* The QVector's indexOf function is perfect: returns the index of the ring
         * within the vector if the ring is an innerBoundary;
         * Else it returns -1, meaning it's an outerBoundary
         */
        int memberIndex                    = polygon->innerBoundaries().indexOf(ring);

        placemarkOsmData->addMemberReference(memberIndex, osmData);

        return &placemarkOsmData->memberReference(memberIndex);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
