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

#include "DgmlProjectionTagHandler.h"

// Local includes

#include "DgmlAttributeDictionary.h"
#include "DgmlElementDictionary.h"
#include "GeoParser.h"
#include "GeoSceneTileDataset.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(Projection)

GeoNode* DgmlProjectionTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Projection)));

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (!parentItem.represents(dgmlTag_Texture) && !parentItem.represents(dgmlTag_Vectortile))
    {
        return nullptr;
    }

    // Attribute name, default to "Equirectangular"
    const QString nameStr = parser.attribute(dgmlAttr_name).trimmed();

    if (!nameStr.isEmpty())
    {
        GeoSceneAbstractTileProjection::Type tileProjectionType = GeoSceneAbstractTileProjection::Equirectangular;

        if (nameStr == QLatin1String("Equirectangular"))
        {
            tileProjectionType = GeoSceneAbstractTileProjection::Equirectangular;
        }

        else if (nameStr == QLatin1String("Mercator"))
        {
            tileProjectionType = GeoSceneAbstractTileProjection::Mercator;
        }

        else
        {
            parser.raiseWarning(QString::fromUtf8("Value not allowed for attribute name: %1").arg(nameStr));
        }

        parentItem.nodeAs<GeoSceneTileDataset>()->setTileProjection(tileProjectionType);
    }

    return nullptr;
}

} // namespace dgml

} // namespace Marble
