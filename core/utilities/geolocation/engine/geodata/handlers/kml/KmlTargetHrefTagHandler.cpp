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

#include "KmlTargetHrefTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoParser.h"
#include "GeoDataAlias.h"
#include "GeoDataUpdate.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(targetHref)

GeoNode* KmltargetHrefTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_targetHref)));

    GeoStackItem parentItem = parser.parentElement();
    QString content = parser.readElementText().trimmed();

    if (parentItem.is<GeoDataAlias>())
    {
        parentItem.nodeAs<GeoDataAlias>()->setTargetHref(content);
    }

    else if (parentItem.is<GeoDataUpdate>())
    {
        parentItem.nodeAs<GeoDataUpdate>()->setTargetHref(content);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
