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

#include "KmlGxTimeSpanTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataTimeSpan.h"
#include "GeoDataFeature.h"
#include "GeoDataAbstractView.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

namespace gx
{

KML_DEFINE_TAG_HANDLER_GX22(TimeSpan)

GeoNode* KmlTimeSpanTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_TimeSpan)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataFeature>())
    {
        GeoDataTimeSpan timeSpan;
        KmlObjectTagHandler::parseIdentifiers(parser, &timeSpan);
        parentItem.nodeAs<GeoDataFeature>()->setTimeSpan(timeSpan);
        return &parentItem.nodeAs<GeoDataFeature>()->timeSpan();
    }

    else if (parentItem.is<GeoDataAbstractView>())
    {
        GeoDataTimeSpan timeSpan;
        KmlObjectTagHandler::parseIdentifiers(parser, &timeSpan);
        parentItem.nodeAs<GeoDataAbstractView>()->setTimeSpan(timeSpan);
        return &parentItem.nodeAs<GeoDataAbstractView>()->timeSpan();
    }

    return nullptr;
}

} // namespace gx

} // namespace kml

} // namespace Marble
