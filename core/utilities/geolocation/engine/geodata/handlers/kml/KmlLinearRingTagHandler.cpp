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

#include "KmlLinearRingTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataPlacemark.h"
#include "GeoDataPolygon.h"
#include "GeoDataMultiGeometry.h"
#include "GeoDataLinearRing.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(LinearRing)

GeoNode* KmlLinearRingTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_LinearRing)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_outerBoundaryIs))
    {
        GeoDataLinearRing linearRing;
        KmlObjectTagHandler::parseIdentifiers(parser, &linearRing);
        parentItem.nodeAs<GeoDataPolygon>()->setOuterBoundary(linearRing);
        return &parentItem.nodeAs<GeoDataPolygon>()->outerBoundary();

    }

    else if (parentItem.represents(kmlTag_innerBoundaryIs))
    {
        GeoDataLinearRing linearRing;
        KmlObjectTagHandler::parseIdentifiers(parser, &linearRing);
        parentItem.nodeAs<GeoDataPolygon>()->appendInnerBoundary(linearRing);
        return &parentItem.nodeAs<GeoDataPolygon>()->innerBoundaries().last();

    }

    else if (parentItem.represents(kmlTag_Placemark))
    {
        GeoDataLinearRing* linearRing = new GeoDataLinearRing;
        KmlObjectTagHandler::parseIdentifiers(parser, linearRing);
        GeoDataPlacemark* placemark = parentItem.nodeAs<GeoDataPlacemark>();
        placemark->setGeometry(linearRing);
        return placemark->geometry();

    }

    else if (parentItem.is<GeoDataMultiGeometry>())
    {
        GeoDataLinearRing* linearRing = new GeoDataLinearRing;
        KmlObjectTagHandler::parseIdentifiers(parser, linearRing);
        parentItem.nodeAs<GeoDataMultiGeometry>()->append(linearRing);
        return linearRing;
    }

    else
    {
        return nullptr;
    }
}

} // namespace kml

} // namespace Marble
