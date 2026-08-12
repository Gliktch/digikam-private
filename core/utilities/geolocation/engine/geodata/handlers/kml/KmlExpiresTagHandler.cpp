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

#include "KmlExpiresTagHandler.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataNetworkLinkControl.h"
#include "GeoDataParser.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(expires)

GeoNode* KmlexpiresTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_expires)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_NetworkLinkControl))
    {
        QDateTime time = QDateTime::fromString(parser.readElementText(), Qt::ISODate);

        parentItem.nodeAs<GeoDataNetworkLinkControl>()->setExpires(time);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
