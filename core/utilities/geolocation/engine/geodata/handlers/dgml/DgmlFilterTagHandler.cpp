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

#include "DgmlFilterTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "DgmlAuxillaryDictionary.h"
#include "GeoParser.h"
#include "GeoSceneLayer.h"
#include "GeoSceneMap.h"
#include "GeoSceneFilter.h"
#include "digikam_debug.h"

namespace Marble
{
namespace dgml
{
DGML_DEFINE_TAG_HANDLER(Filter)

GeoNode* DgmlFilterTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Filter)));

    QString name      = parser.attribute(dgmlAttr_name).trimmed();
    QString type      = parser.attribute(dgmlAttr_type).toLower().trimmed();

    GeoSceneFilter* filter = nullptr;

    // Checking for parent layer
    GeoStackItem parentItem = parser.parentElement();
    GeoStackItem grandParentItem = parser.parentElement(1);

    if (parentItem.represents(dgmlTag_Layer) &&
        grandParentItem.represents(dgmlTag_Map))
    {

        filter = new GeoSceneFilter(name);
        filter->setType(type);
        parentItem.nodeAs<GeoSceneLayer>()->addFilter(filter);
        grandParentItem.nodeAs<GeoSceneMap>()->addFilter(filter);
    }

    return filter;
}

} // namespace dgml

} // namespace Marble
