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

#include "KmlSouthTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(south)

GeoNode* KmlsouthTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_south)));

    GeoStackItem parentItem = parser.parentElement();

    qreal const south = parser.readElementText().trimmed().toDouble();

    if (parentItem.represents(kmlTag_LatLonAltBox))
    {
        parentItem.nodeAs<GeoDataLatLonAltBox>()->setSouth(south, GeoDataCoordinates::Degree);
    }

    else if (parentItem.represents(kmlTag_LatLonBox))
    {
        parentItem.nodeAs<GeoDataLatLonBox>()->setSouth(south, GeoDataCoordinates::Degree);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
