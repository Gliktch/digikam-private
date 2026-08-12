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

#include "GPXlinkTagHandler.h"

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

GPX_DEFINE_TAG_HANDLER_11(link)

// Gpx link provides a link related to the object, and is inserted into the
// waypoint description as this seems to be the simplest means to make it
// available to the user.
// In addition, link properties are saved to extendedData.
// there are text and type properties, type being ignored for now.
GeoNode* GPXlinkTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(gpxTag_link)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(gpxTag_wpt))
    {
        GeoDataPlacemark* placemark = parentItem.nodeAs<GeoDataPlacemark>();

        QXmlStreamAttributes attributes = parser.attributes();
        QString href = attributes.value(QLatin1String("href")).toString();
        QString text = href;

        if (parser.readNextStartElement())
        {
            text = parser.readElementText();
        }

        const QString link = QStringLiteral("Link: <a href=\"%1\">%2</a>")
                             .arg(href).arg(text);

        QString desc = placemark->description();

        if (!desc.isEmpty())
        {
            desc += QLatin1String("<br>");
        }

        placemark->setDescription(desc.append(link));
        placemark->setDescriptionCDATA(true);
    }

    return nullptr;
}

} // namespace gpx

} // namespace Marble
