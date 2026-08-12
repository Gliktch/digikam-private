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

#include "KmlUpdateTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataNetworkLinkControl.h"
#include "GeoDataUpdate.h"
#include "GeoDataAnimatedUpdate.h"
#include "GeoDataParser.h"
#include "KmlObjectTagHandler.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(Update)

GeoNode* KmlUpdateTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Update)));

    GeoDataUpdate* update = new GeoDataUpdate;
    KmlObjectTagHandler::parseIdentifiers(parser, update);
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_NetworkLinkControl))
    {
        return &parentItem.nodeAs<GeoDataNetworkLinkControl>()->update();
    }

    else if (parentItem.represents(kmlTag_AnimatedUpdate))
    {
        parentItem.nodeAs<GeoDataAnimatedUpdate>()->setUpdate(update);
        return update;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
