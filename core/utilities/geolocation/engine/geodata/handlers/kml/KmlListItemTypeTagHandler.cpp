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

#include "KmlListItemTypeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataListStyle.h"
#include "GeoDataParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(listItemType)

GeoNode* KmllistItemTypeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_listItemType)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_ListStyle))
    {
        QString typeText = parser.readElementText().trimmed();
        GeoDataListStyle::ListItemType type = GeoDataListStyle::Check;

        if (typeText == QLatin1String("check"))
        {
            type = GeoDataListStyle::Check;
        }

        else if (typeText == QLatin1String("radioFolder"))
        {
            type = GeoDataListStyle::RadioFolder;
        }

        else if (typeText == QLatin1String("checkOffOnly"))
        {
            type = GeoDataListStyle::CheckOffOnly;
        }

        else if (typeText == QLatin1String("checkHideChildren"))
        {
            type = GeoDataListStyle::CheckHideChildren;
        }

        else
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "listItemType value is invalid"
                                        << "falling back to default - check";
        }

        parentItem.nodeAs<GeoDataListStyle>()->setListItemType(type);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
