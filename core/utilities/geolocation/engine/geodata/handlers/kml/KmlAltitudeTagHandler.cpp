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

#include "KmlAltitudeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "MarbleGlobal.h"
#include "GeoDataLookAt.h"
#include "GeoDataCamera.h"
#include "GeoDataGroundOverlay.h"
#include "GeoParser.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLocation.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(altitude)
GeoNode* KmlaltitudeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_altitude)));

    GeoStackItem parentItem = parser.parentElement();

    qreal const altitude = parser.readElementText().trimmed().toDouble();

    if (parentItem.is<GeoDataLookAt>())
    {
        parentItem.nodeAs<GeoDataLookAt>()->setAltitude(altitude);
    }

    else if (parentItem.is<GeoDataCamera>())
    {
        parentItem.nodeAs<GeoDataCamera>()->setAltitude(altitude);
    }

    else if (parentItem.is<GeoDataGroundOverlay>())
    {
        parentItem.nodeAs<GeoDataGroundOverlay>()->setAltitude(altitude);
    }

    else if (parentItem.is<GeoDataLocation>())
    {
        parentItem.nodeAs<GeoDataLocation>()->setAltitude(altitude);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble


