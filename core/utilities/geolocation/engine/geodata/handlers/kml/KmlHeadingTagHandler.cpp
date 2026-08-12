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

#include "KmlHeadingTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataIconStyle.h"
#include "GeoDataCamera.h"
#include "GeoDataOrientation.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(heading)

GeoNode* KmlheadingTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_heading)));

    GeoStackItem parentItem = parser.parentElement();

    int const heading = parser.readElementText().toInt();

    if (heading >= 0 && heading <= 360)
    {
        if (parentItem.represents(kmlTag_IconStyle))
        {
            parentItem.nodeAs<GeoDataIconStyle>()->setHeading(heading);
        }

        else if (parentItem.represents(kmlTag_Camera))
        {
            parentItem.nodeAs<GeoDataCamera>()->setHeading(heading);
        }

        else if (parentItem.represents(kmlTag_Orientation))
        {
            parentItem.nodeAs<GeoDataOrientation>()->setHeading(heading);
        }
    }

    else
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Invalid heading value " << heading << ", must be within 0..360. Using 0 instead.";
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
