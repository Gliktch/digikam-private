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

#include "KmlLatLonQuadTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoParser.h"
#include "GeoDataLatLonQuad.h"
#include "GeoDataGroundOverlay.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_GX22(LatLonQuad)

GeoNode* KmlLatLonQuadTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_LatLonQuad)));
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_GroundOverlay))
    {
        GeoDataLatLonQuad* quad = &parentItem.nodeAs<GeoDataGroundOverlay>()->latLonQuad();
        KmlObjectTagHandler::parseIdentifiers(parser, quad);
        return quad;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
