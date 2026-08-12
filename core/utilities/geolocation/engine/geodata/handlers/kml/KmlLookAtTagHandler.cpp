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

#include "KmlLookAtTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataLookAt.h"
#include "GeoParser.h"
#include "GeoDataFeature.h"
#include "GeoDataFlyTo.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(LookAt)

GeoNode* KmlLookAtTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_LookAt)));

    GeoDataLookAt* lookAt = new GeoDataLookAt();
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataFeature>())
    {
        GeoDataFeature* feature = parentItem.nodeAs<GeoDataFeature>();
        feature->setAbstractViewBase(lookAt);
        return lookAt;
    }

    if (parentItem.is<GeoDataFlyTo>())
    {
        GeoDataFlyTo* feature = parentItem.nodeAs<GeoDataFlyTo>();
        feature->setView(lookAt);
        return lookAt;
    }

    else
    {
        delete lookAt;
        return nullptr;
    }
}

} // namespace kml

} // namespace Marble
