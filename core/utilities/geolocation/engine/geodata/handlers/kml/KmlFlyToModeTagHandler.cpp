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

#include "KmlFlyToModeTagHandler.h"

// Local includes

#include "GeoDataFlyTo.h"
#include "GeoParser.h"
#include "KmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_GX22(flyToMode)

GeoNode* KmlflyToModeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_flyToMode)));

    QString content = parser.readElementText().trimmed();

    GeoDataFlyTo::FlyToMode mode;

    if (content == QLatin1String("smooth"))
    {
        mode = GeoDataFlyTo::Smooth;
    }

    else if (content == QLatin1String("bounce"))
    {
        mode = GeoDataFlyTo::Bounce;
    }

    else
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Unknown mode " << content << ", using 'bounce' instead.";
        mode = GeoDataFlyTo::Bounce;
    }

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataFlyTo>())
    {
        parentItem.nodeAs<GeoDataFlyTo>()->setFlyToMode(mode);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
