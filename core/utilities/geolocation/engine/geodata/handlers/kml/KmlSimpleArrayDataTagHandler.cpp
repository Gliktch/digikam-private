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

#include "KmlSimpleArrayDataTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataExtendedData.h"
#include "GeoDataSimpleArrayData.h"
#include "GeoDataSchemaData.h"
#include "GeoDataTypes.h"
#include "GeoParser.h"
#include "GeoDocument.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_GX22(SimpleArrayData)

GeoNode* KmlSimpleArrayDataTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_SimpleArrayData)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataExtendedData>())
    {
        GeoDataSimpleArrayData* arrayData = new GeoDataSimpleArrayData();
        QString name = parser.attribute("name").trimmed();
        parentItem.nodeAs<GeoDataExtendedData>()->setSimpleArrayData(name, arrayData);
        return arrayData;
    }

    if (parentItem.is<GeoDataSchemaData>())
    {
        GeoDataExtendedData* extendedData = parentItem.nodeAs<GeoDataSchemaData>()->parent();
        GeoDataSimpleArrayData* arrayData = new GeoDataSimpleArrayData;
        const QString name = parser.attribute("name").trimmed();
        extendedData->setSimpleArrayData(name, arrayData);
        return arrayData;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
