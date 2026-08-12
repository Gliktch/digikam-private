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

#include "KmlGridOriginTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataImagePyramid.h"
#include "GeoDataParser.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(gridOrigin)

GeoNode* KmlgridOriginTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_gridOrigin)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_ImagePyramid))
    {
        GeoDataImagePyramid::GridOrigin gridOrigin = GeoDataImagePyramid::UpperLeft;
        QString gridOriginText = parser.readElementText();

        if (gridOriginText == QLatin1String("lowerLeft"))
        {
            gridOrigin = GeoDataImagePyramid::LowerLeft;
        }

        else if (gridOriginText == QLatin1String("upperLeft"))
        {
            gridOrigin = GeoDataImagePyramid::UpperLeft;
        }

        parentItem.nodeAs<GeoDataImagePyramid>()->setGridOrigin(gridOrigin);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
