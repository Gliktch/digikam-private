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

#include "KmlHotSpotTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataIconStyle.h"
#include "GeoDataHotSpot.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(hotSpot)

GeoNode* KmlhotSpotTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_hotSpot)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_IconStyle))
    {
        QPointF pf(parser.attribute("x").trimmed().toFloat(),
                   parser.attribute("y").trimmed().toFloat());
        QString xu = parser.attribute("xunits").trimmed();
        QString yu = parser.attribute("yunits").trimmed();

        GeoDataHotSpot::Units xunits;
        GeoDataHotSpot::Units yunits;

        if (xu == QLatin1String("pixels"))
        {
            xunits = GeoDataHotSpot::Pixels;
        }

        else if (xu == QLatin1String("insetPixels"))
        {
            xunits = GeoDataHotSpot::InsetPixels;
        }

        else
        {
            xunits = GeoDataHotSpot::Fraction;
        }

        if (yu == QLatin1String("pixels"))
        {
            yunits = GeoDataHotSpot::Pixels;
        }

        else if (yu == QLatin1String("insetPixels"))
        {
            yunits = GeoDataHotSpot::InsetPixels;
        }

        else
        {
            yunits = GeoDataHotSpot::Fraction;
        }

        parentItem.nodeAs<GeoDataIconStyle>()->setHotSpot(pf, xunits, yunits);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
