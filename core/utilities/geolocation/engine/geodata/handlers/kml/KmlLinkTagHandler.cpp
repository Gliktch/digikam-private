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

#include "KmlLinkTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataLink.h"
#include "GeoDataNetworkLink.h"
#include "GeoDataModel.h"
#include "GeoDataParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(Link)

GeoNode* KmlLinkTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Link)));
    GeoDataLink link;
    KmlObjectTagHandler::parseIdentifiers(parser, &link);
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_NetworkLink))
    {
        return &parentItem.nodeAs<GeoDataNetworkLink>()->link();
    }

    else if (parentItem.represents(kmlTag_Model))
    {
        parentItem.nodeAs<GeoDataModel>()->setLink(link);
        return &parentItem.nodeAs<GeoDataModel>()->link();
    }

    else
    {
        return nullptr;
    }
}

} // namespace kml

} // namespace Marble
