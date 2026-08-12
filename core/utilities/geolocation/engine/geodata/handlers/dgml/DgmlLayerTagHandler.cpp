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

#include "DgmlLayerTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "DgmlAuxillaryDictionary.h"
#include "GeoParser.h"
#include "GeoSceneMap.h"
#include "GeoSceneLayer.h"
#include "digikam_debug.h"

namespace Marble
{
namespace dgml
{
DGML_DEFINE_TAG_HANDLER(Layer)

GeoNode* DgmlLayerTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Layer)));

    QString name      = parser.attribute(dgmlAttr_name).trimmed();
    QString backend    = parser.attribute(dgmlAttr_backend).toLower().trimmed();
    QString role      = parser.attribute(dgmlAttr_role).toLower().trimmed();

    GeoSceneLayer* layer = nullptr;

    // Checking for parent layer
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Map))
    {
        layer = new GeoSceneLayer(name);
        layer->setBackend(backend);
        layer->setRole(role);
        parentItem.nodeAs<GeoSceneMap>()->addLayer(layer);
    }

    return layer;
}

} // namespace dgml

} // namespace Marble
