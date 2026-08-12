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

#include "KmlTourTagHandler.h"

// Local includes

#include "KmlObjectTagHandler.h"
#include "GeoDataTour.h"
#include "GeoDataDocument.h"
#include "GeoDataParser.h"
#include "KmlElementDictionary.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_GX22(Tour)

GeoNode* KmlTourTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Tour)));

    GeoStackItem parentItem = parser.parentElement();

    GeoDataTour* tour = new GeoDataTour;
    KmlObjectTagHandler::parseIdentifiers(parser, tour);

    if (parentItem.represents(kmlTag_Folder) || parentItem.represents(kmlTag_Document))
    {
        parentItem.nodeAs<GeoDataContainer>()->append(tour);
        return tour;
    }

    else if (parentItem.qualifiedName().first == QString::fromUtf8(kmlTag_kml))
    {
        GeoDataDocument* doc = geoDataDoc(parser);
        doc->append(tour);
        return tour;
    }

    else
    {
        delete tour;
        return nullptr;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
