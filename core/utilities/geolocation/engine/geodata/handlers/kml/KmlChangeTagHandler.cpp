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

#include "KmlChangeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataUpdate.h"
#include "GeoDataChange.h"
#include "GeoDataParser.h"
#include "KmlObjectTagHandler.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(Change)

GeoNode* KmlChangeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Change)));

    GeoDataChange* change = new GeoDataChange;
    KmlObjectTagHandler::parseIdentifiers(parser, change);
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_Update))
    {
        parentItem.nodeAs<GeoDataUpdate>()->setChange(change);
        return change;
    }

    else
    {
        delete change;
        return nullptr;
    }
}

} // namespace kml

} // namespace Marble
