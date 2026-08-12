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

#include "KmlBgColorTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlColorTagHandler.h"
#include "GeoDataBalloonStyle.h"
#include "GeoDataListStyle.h"
#include "GeoDataParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(bgColor)

GeoNode* KmlbgColorTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_bgColor)));

    GeoStackItem parentItem = parser.parentElement();

    QColor const color = KmlcolorTagHandler::parseColor(parser.readElementText().trimmed());

    if (parentItem.represents(kmlTag_BalloonStyle))
    {
        parentItem.nodeAs<GeoDataBalloonStyle>()->setBackgroundColor(color);
    }

    else if (parentItem.represents(kmlTag_ListStyle))
    {
        parentItem.nodeAs<GeoDataListStyle>()->setBackgroundColor(color);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
