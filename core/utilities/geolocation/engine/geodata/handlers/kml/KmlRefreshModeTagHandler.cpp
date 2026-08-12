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

#include "KmlRefreshModeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataLink.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(refreshMode)

GeoNode* KmlrefreshModeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_refreshMode)));

    GeoStackItem parentItem = parser.parentElement();
    QString content = parser.readElementText().trimmed();
    GeoDataLink::RefreshMode mode;

    if (parentItem.is<GeoDataLink>())
    {
        if (content == QLatin1String("onChange"))
        {
            mode = GeoDataLink::OnChange;
        }

        else if (content == QLatin1String("onExpire"))
        {
            mode = GeoDataLink::OnExpire;
        }

        else if (content == QLatin1String("onInterval"))
        {
            mode = GeoDataLink::OnInterval;
        }

        else
        {
            mode = GeoDataLink::OnChange;
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Value " << content << "set in kml file is invalid."
                                        << "Value of <refreshMode> has been reset to onChange" ;
        }

        parentItem.nodeAs<GeoDataLink>()->setRefreshMode(mode);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
