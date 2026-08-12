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

#pragma once

// Local includes

#include "GeoTagWriter.h"

namespace Marble
{

class GeoDataFeature;
class OsmPlacemarkData;
class GeoDataGeometry;

/**
 * @brief The KmlOsmPlacemarkDataTagWriter class is not a standard GeoTagWriter extension
 * because the osm data fields have to be written in relation to the placemark's geometry
 */
class KmlOsmPlacemarkDataTagWriter
{
public:

    /**
     * @brief write function writes a custom XML schema made to store
     * OsmPlacemarkData in a valid KML context
     * @see https://developers.google.com/kml/documentation/extendeddata#opaquedata
     *
     * The custom schema has the following structure ( most complex one, for polygons ):
     *@code
     *<Placemark>
     *    ...
     *    <ExtendedData xmlns:mx="http://marble.kde.org">                       // krazy:exclude=insecurenet
     *        <mx:OsmPlacemarkData id=...>
     *             <mx:tag k="key" v="value"/>
     *             .. more tags
     *
     *             <mx:member index="-1">     //outer boundary
     *                 <mx:OsmPlacemarkData id=...>
     *                 <mx:nd index="0">
     *                     <mx:OsmPlacemarkData id= .../> //nd #0
     *                 </nd>
     *                 <mx:nd index="1">
     *                     <mx:OsmPlacemarkData id= .../> //nd #1
     *                 </nd>
     *                 ... more nds
     *             </member>
     *
     *             <mx:member index="0">     //first inner boundary
     *             ...
     *             </member>
     *             ... more inner boundaries
     *
     *        </mx:OsmPlacemarkData>
     *    </ExtendedData>
     * </Placemark>
     *@endcode
     */
    static bool write(const GeoDataFeature* feature, GeoWriter& writer);
    static bool writeOsmData(const GeoDataGeometry* geometry, const OsmPlacemarkData& osmData, GeoWriter& writer);
};

} // namespace Marble
