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

#include "KmlPopTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataPlacemark.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(pop)

GeoNode* KmlpopTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_pop)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataPlacemark>())
    {
        QString population = parser.readElementText().trimmed();
        qint64 pop = population.toLongLong();

        if (pop < 0)
        {
            pop = 0;
        }

        parentItem.nodeAs<GeoDataPlacemark>()->setPopulation(pop);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
