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

#include "KmlViewRefreshModeTagHandler.h"

// Local includes

#include "GeoDataLink.h"
#include "GeoParser.h"
#include "KmlElementDictionary.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(viewRefreshMode)

GeoNode* KmlviewRefreshModeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_viewRefreshMode)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataLink>())
    {
        QString content = parser.readElementText().trimmed();

        GeoDataLink::ViewRefreshMode mode = GeoDataLink::Never;

        if (content == QLatin1String("onStop"))
        {
            mode = GeoDataLink::OnStop;
        }

        else if (content == QLatin1String("onRegion"))
        {
            mode = GeoDataLink::OnRegion;
        }

        else if (content == QLatin1String("onRequest"))
        {
            mode = GeoDataLink::OnRequest;
        }

        parentItem.nodeAs<GeoDataLink>()->setViewRefreshMode(mode);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
