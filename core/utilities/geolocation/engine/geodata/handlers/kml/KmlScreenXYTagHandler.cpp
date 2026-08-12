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

#include "KmlScreenXYTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataScreenOverlay.h"
#include "GeoDataParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(screenXY)

GeoNode* KmlscreenXYTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_screenXY)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_ScreenOverlay))
    {
        GeoDataVec2 vec2(parser.attribute("x").trimmed().toFloat(),
                         parser.attribute("y").trimmed().toFloat(),
                         parser.attribute("xunits").trimmed(),
                         parser.attribute("yunits").trimmed());

        parentItem.nodeAs<GeoDataScreenOverlay>()->setScreenXY(vec2);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
