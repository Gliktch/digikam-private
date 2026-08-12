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

#include "DgmlPropertyTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "GeoParser.h"
#include "GeoSceneSettings.h"
#include "GeoSceneGroup.h"
#include "GeoSceneProperty.h"
#include "digikam_debug.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(Property)

GeoNode* DgmlPropertyTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Property)));

    QString name = parser.attribute(dgmlAttr_name).trimmed();

    GeoSceneProperty* property = nullptr;

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Settings))
    {
        property = new GeoSceneProperty(name);
        parentItem.nodeAs<GeoSceneSettings>()->addProperty(property);
    }

    if (parentItem.represents(dgmlTag_Group))
    {
        property = new GeoSceneProperty(name);
        parentItem.nodeAs<GeoSceneGroup>()->addProperty(property);
    }

    return property;
}

} // namespace dgml

} // namespace Marble
