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

#include "KmlDurationTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "MarbleGlobal.h"
#include "GeoDataFlyTo.h"
#include "GeoDataWait.h"
#include "GeoDataAnimatedUpdate.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_GX22(duration)

GeoNode* KmldurationTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_duration)));

    GeoStackItem parentItem = parser.parentElement();

    qreal const duration = parser.readElementText().trimmed().toDouble();

    if (parentItem.is<GeoDataFlyTo>())
    {
        parentItem.nodeAs<GeoDataFlyTo>()->setDuration(duration);
    }

    if (parentItem.is<GeoDataWait>())
    {
        parentItem.nodeAs<GeoDataWait>()->setDuration(duration);
    }

    if (parentItem.is<GeoDataAnimatedUpdate>())
    {
        parentItem.nodeAs<GeoDataAnimatedUpdate>()->setDuration(duration);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
