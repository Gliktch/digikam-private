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

// be aware that there is another Tag called Scale which uses the name KmlscaleTagHandler.h
// as it is impossible to use the name KmlscaleTagHandler then, use an underscore
// to mark the lower case variant

#include "Kml_scaleTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataIconStyle.h"
#include "GeoDataLabelStyle.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(scale)

GeoNode* KmlscaleTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_scale)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_IconStyle))
    {
        parentItem.nodeAs<GeoDataIconStyle>()->setScale(
            parser.readElementText().trimmed().toFloat());
    }

    else if (parentItem.represents(kmlTag_LabelStyle))
    {
        parentItem.nodeAs<GeoDataLabelStyle>()->setScale(
            parser.readElementText().trimmed().toFloat());
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
