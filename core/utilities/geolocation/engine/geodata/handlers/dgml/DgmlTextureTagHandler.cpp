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

#include "DgmlTextureTagHandler.h"

// C++ includes

#include <limits>

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "DgmlAuxillaryDictionary.h"
#include "GeoParser.h"
#include "GeoSceneLayer.h"
#include "GeoSceneTextureTileDataset.h"
#include "digikam_debug.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(Texture)

GeoNode* DgmlTextureTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Texture)));

    const QString name = parser.attribute(dgmlAttr_name).trimmed();

    const QString expireStr = parser.attribute(dgmlAttr_expire).trimmed();
    int expire = std::numeric_limits<int>::max();

    if (!expireStr.isEmpty())
    {
        expire = expireStr.toInt();
    }

    GeoSceneTileDataset* texture = nullptr;

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    // Check parent type and make sure that the dataSet type
    // matches the backend of the parent layer
    if (parentItem.represents(dgmlTag_Layer)
        && parentItem.nodeAs<GeoSceneLayer>()->backend() == QString::fromUtf8(dgmlValue_texture))
    {

        texture = new GeoSceneTextureTileDataset(name);
        texture->setExpire(expire);
        parentItem.nodeAs<GeoSceneLayer>()->addDataset(texture);
    }

    return texture;
}

} // namespace dgml

} // namespace Marble
