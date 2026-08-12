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

#include "KmlExtrudeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataGeometry.h"
#include "GeoDataPoint.h"
#include "GeoDataPolygon.h"
#include "GeoDataLineString.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPlacemark.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(extrude)

GeoNode* KmlextrudeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_extrude)));

    GeoStackItem parentItem = parser.parentElement();

    GeoDataGeometry* geometry;
    bool validParents = false;

    if (parentItem.is<GeoDataPoint>())
    {
        geometry = parentItem.nodeAs<GeoDataPoint>();
        validParents = true;
    }

    else if (parentItem.is<GeoDataPlacemark>())
    {
        geometry = parentItem.nodeAs<GeoDataPlacemark>()->geometry();
        validParents = true;
    }

    else if (parentItem.is<GeoDataPolygon>())
    {
        geometry = parentItem.nodeAs<GeoDataPolygon>();
        validParents = true;
    }

    else if (parentItem.is<GeoDataLineString>())
    {
        geometry = parentItem.nodeAs<GeoDataLineString>();
        validParents = true;
    }

    else if (parentItem.is<GeoDataLinearRing>())
    {
        geometry = parentItem.nodeAs<GeoDataLinearRing>();
        validParents = true;
    }

    if (validParents)
    {
        QString content = parser.readElementText().trimmed();

        const bool extrude = (content == QLatin1String("1"));
        geometry->setExtrude(extrude);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
