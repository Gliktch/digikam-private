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

#include "DgmlLegendTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "GeoParser.h"
#include "GeoSceneDocument.h"
#include "GeoSceneLegend.h"
#include "digikam_debug.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(Legend)

GeoNode* DgmlLegendTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Legend)));

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Document))
    {
        return parentItem.nodeAs<GeoSceneDocument>()->legend();
    }

    return nullptr;
}

} // namespace dgml

} // namespace Marble
