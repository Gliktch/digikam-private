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

#include "KmlStyleTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataStyle.h"
#include "GeoDataFeature.h"
#include "GeoParser.h"
#include "GeoDataDocument.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(Style)

GeoNode* KmlStyleTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Style)));


    GeoStackItem parentItem = parser.parentElement();

    /// for documents several styles are allowed: document wide styles are saved different!!!!!
    if (parentItem.represents(kmlTag_Document))
    {
        GeoDataStyle::Ptr style(new GeoDataStyle);
        KmlObjectTagHandler::parseIdentifiers(parser, style.data());
        parentItem.nodeAs<GeoDataDocument>()->addStyle(style);
        return parentItem.nodeAs<GeoDataDocument>()->style(style->id()).data();
    }

    else if (parentItem.represents(kmlTag_Placemark))
    {
        GeoDataStyle::Ptr style(new GeoDataStyle);
        KmlObjectTagHandler::parseIdentifiers(parser, style.data());
        parentItem.nodeAs<GeoDataFeature>()->setStyle(style);
        return style.data();
    }

    // FIXME: KMLStyle can be contained in MultiGeometry as well
    return nullptr;
}

} // namespace kml

} // namespace Marble
