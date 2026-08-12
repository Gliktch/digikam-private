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

#include "DgmlSourceDirTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "GeoParser.h"
#include "GeoSceneTileDataset.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(SourceDir)

GeoNode* DgmlSourceDirTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_SourceDir)));

    QString format = parser.attribute(dgmlAttr_format).trimmed();

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Texture) || parentItem.represents(dgmlTag_Vectortile))
    {
        GeoSceneTileDataset* texture = parentItem.nodeAs<GeoSceneTileDataset>();
        texture->setSourceDir(parser.readElementText().trimmed());
        texture->setFileFormat(format);
    }

    return nullptr;
}

} // namespace dgml

} // namespace Marble
