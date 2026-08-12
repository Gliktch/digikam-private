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

#include "DgmlPaletteTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "DgmlAuxillaryDictionary.h"
#include "GeoParser.h"
#include "GeoSceneFilter.h"
#include "GeoScenePalette.h"
#include "digikam_debug.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(Palette)

GeoNode* DgmlPaletteTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Palette)));

    QString type      = parser.attribute(dgmlAttr_type).toLower().trimmed();
    QString file      = parser.readElementText().trimmed();

    GeoScenePalette* palette = nullptr;

    // Checking for parent palette
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Filter))
    {
        palette = new GeoScenePalette(type, file);
        parentItem.nodeAs<GeoSceneFilter>()->addPalette(palette);
    }

    return palette;
}

} // namespace dgml

} // namespace Marble
