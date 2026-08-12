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

#include "KmlNetworkLinkTagHandler.h"

// Local includes

#include "GeoDataNetworkLink.h"
#include "GeoDataContainer.h"
#include "GeoDataDocument.h"
#include "GeoDataParser.h"
#include "KmlElementDictionary.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(NetworkLink)

GeoNode* KmlNetworkLinkTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_NetworkLink)));

    GeoDataNetworkLink* networkLink = new GeoDataNetworkLink;
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_Folder) || parentItem.represents(kmlTag_Document))
    {
        parentItem.nodeAs<GeoDataContainer>()->append(networkLink);
        return networkLink;
    }

    else if (parentItem.qualifiedName().first == QString::fromUtf8(kmlTag_kml))
    {
        GeoDataDocument* doc = geoDataDoc(parser);
        doc->append(networkLink);
        return networkLink;
    }

    else
    {
        delete networkLink;
        return nullptr;
    }
}

} // namespace kml

} // namespace Marble
