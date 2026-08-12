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

#include "KmlItemIconTagHandler.h"

// Local includes

#include "GeoDataListStyle.h"
#include "GeoDataItemIcon.h"
#include "GeoDataParser.h"
#include "KmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(ItemIcon)

GeoNode* KmlItemIconTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_ItemIcon)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_ListStyle))
    {
        GeoDataItemIcon* itemIcon = new GeoDataItemIcon;
        parentItem.nodeAs<GeoDataListStyle>()->append(itemIcon);
        return itemIcon;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
