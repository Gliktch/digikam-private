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

#include "KmlDocumentTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataDocument.h"
#include "GeoDataParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(Document)

GeoNode* KmlDocumentTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Document)));

    GeoStackItem parentItem = parser.parentElement();

    if (!(parentItem.qualifiedName().first.isNull() && parentItem.qualifiedName().second.isNull()))
    {
        // this happens if there is a parent element to the Document tag. We can work around that and simply expect that
        // the new Document tag works like a Folder
        if (parentItem.represents(kmlTag_Folder) || parentItem.represents(kmlTag_Document) || parentItem.represents(kmlTag_Create))
        {
            GeoDataDocument* document = new GeoDataDocument;
            KmlObjectTagHandler::parseIdentifiers(parser, document);
            parentItem.nodeAs<GeoDataContainer>()->append(document);

            return document;
        }

        else if (parentItem.qualifiedName().first == QLatin1String(kmlTag_kml))
        {
            GeoDataDocument* doc = geoDataDoc(parser);
            KmlObjectTagHandler::parseIdentifiers(parser, doc);
            return doc;
        }
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
