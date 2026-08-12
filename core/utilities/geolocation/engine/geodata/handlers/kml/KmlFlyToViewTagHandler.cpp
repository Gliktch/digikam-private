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

#include "KmlFlyToViewTagHandler.h"

// Local includes

#include "GeoDataNetworkLink.h"
#include "GeoParser.h"
#include "KmlElementDictionary.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(flyToView)

GeoNode* KmlflyToViewTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_flyToView)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataNetworkLink>())
    {
        QString content = parser.readElementText().trimmed();
        GeoDataNetworkLink* networkLink = parentItem.nodeAs<GeoDataNetworkLink>();
        networkLink->setFlyToView(content == QLatin1String("1"));
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
