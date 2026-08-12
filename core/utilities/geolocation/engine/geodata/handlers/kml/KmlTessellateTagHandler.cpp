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

#include "KmlTessellateTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoDataGeometry.h"
#include "GeoDataPoint.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(tessellate)

GeoNode* KmltessellateTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_tessellate)));

    GeoStackItem parentItem = parser.parentElement();

    QString content = parser.readElementText().trimmed();

    if (parentItem.is<GeoDataLineString>())
    {
        GeoDataLineString* lineString = parentItem.nodeAs<GeoDataLineString>();

        const bool tesselate = (content == QLatin1String("1"));
        lineString->setTessellate(tesselate);

    }

    else if (parentItem.is<GeoDataLinearRing>())
    {
        GeoDataLinearRing* linearRing = parentItem.nodeAs<GeoDataLinearRing>();

        const bool tesselate = (content == QLatin1String("1"));
        linearRing->setTessellate(tesselate);

    }

    else if (parentItem.is<GeoDataPolygon>())
    {
        GeoDataPolygon* polygon = parentItem.nodeAs<GeoDataPolygon>();

        const bool tesselate = (content == QLatin1String("1"));
        polygon->setTessellate(tesselate);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
