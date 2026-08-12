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

#include "KmlLatLonBoxTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoParser.h"
#include "GeoDataLatLonBox.h"
#include "GeoDataGroundOverlay.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(LatLonBox)

GeoNode* KmlLatLonBoxTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_LatLonBox)));

    GeoDataLatLonBox box;
    KmlObjectTagHandler::parseIdentifiers(parser, &box);

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_GroundOverlay))
    {
        parentItem.nodeAs<GeoDataGroundOverlay>()->setLatLonBox(box);
        return &parentItem.nodeAs<GeoDataGroundOverlay>()->latLonBox();
    }

    else
    {
        return nullptr;
    }
}

} // namespace kml

} // namespace Marble
