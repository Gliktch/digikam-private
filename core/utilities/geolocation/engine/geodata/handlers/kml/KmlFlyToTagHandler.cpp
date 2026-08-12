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

#include "KmlFlyToTagHandler.h"

// Local includes

#include "KmlObjectTagHandler.h"
#include "GeoDataFlyTo.h"
#include "GeoDataPlaylist.h"
#include "GeoParser.h"
#include "KmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_GX22(FlyTo)

GeoNode* KmlFlyToTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_FlyTo)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataPlaylist>())
    {
        GeoDataFlyTo* flyTo = new GeoDataFlyTo;
        KmlObjectTagHandler::parseIdentifiers(parser, flyTo);
        parentItem.nodeAs<GeoDataPlaylist>()->addPrimitive(flyTo);
        return flyTo;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
