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

#include "KmlShapeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataPhotoOverlay.h"
#include "GeoDataParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(shape)

GeoNode* KmlshapeTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_shape)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_PhotoOverlay))
    {
        GeoDataPhotoOverlay::Shape shape;
        QString shapeText = parser.readElementText();

        if (shapeText == QLatin1String("rectangle"))
        {
            shape = GeoDataPhotoOverlay::Rectangle;
        }

        else if (shapeText == QLatin1String("cylinder"))
        {
            shape = GeoDataPhotoOverlay::Cylinder;
        }

        else if (shapeText == QLatin1String("sphere"))
        {
            shape = GeoDataPhotoOverlay::Sphere;
        }

        else
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Unknown shape attribute" << shapeText << ", falling back to default value 'rectangle'";
            shape = GeoDataPhotoOverlay::Rectangle;
        }

        parentItem.nodeAs<GeoDataPhotoOverlay>()->setShape(shape);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
