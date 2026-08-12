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

#include "GPXurlnameTagHandler.h"

// Local includes

#include "GPXElementDictionary.h"
#include "GeoParser.h"
#include "GeoDataPlacemark.h"
#include "GeoDataData.h"
#include "GeoDataExtendedData.h"
#include "digikam_debug.h"

namespace Marble
{

namespace gpx
{

GPX_DEFINE_TAG_HANDLER_10(urlname)

// completes gpx 1.0 url element
GeoNode* GPXurlnameTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(gpxTag_urlname)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(gpxTag_wpt))
    {
        GeoDataPlacemark* placemark = parentItem.nodeAs<GeoDataPlacemark>();
        QString text = parser.readElementText().trimmed();
        // no reason to save for now!
        //         GeoDataExtendedData extendedData = placemark->extendedData();
        //         extendedData.addValue(GeoDataData("urlname", text));
        //         placemark->setExtendedData(extendedData);

        const QString link = QStringLiteral("<br>Link: <a href=\"%1\">%2</a>")
                             .arg(placemark->extendedData().value(QStringLiteral("url")).value().toString())
                             .arg(text);
        placemark->setDescription(placemark->description().append(link));
        placemark->setDescriptionCDATA(true);
    }

    return nullptr;
}

} // namespace gpx

} // namespace Marble
