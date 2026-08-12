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

#include "KmlRangeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "MarbleGlobal.h"
#include "GeoDataLookAt.h"
#include "GeoParser.h"
#include "GeoDataCoordinates.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(range)
GeoNode* KmlrangeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement()
             && parser.isValidElement(QString::fromUtf8(kmlTag_range)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataLookAt>())
    {

        QString rangeTemp = parser.readElementText().trimmed();
        qreal range = rangeTemp.toDouble();

        parentItem.nodeAs<GeoDataLookAt>()->setRange(range);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble

