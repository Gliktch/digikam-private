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

#include "KmlScreenOverlayTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataScreenOverlay.h"
#include "GeoDataContainer.h"
#include "GeoDataDocument.h"
#include "GeoDataParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(ScreenOverlay)

GeoNode* KmlScreenOverlayTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_ScreenOverlay)));

    GeoDataScreenOverlay* overlay = new GeoDataScreenOverlay();
    KmlObjectTagHandler::parseIdentifiers(parser, overlay);

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_Folder) || parentItem.represents(kmlTag_Document) ||
        parentItem.represents(kmlTag_Change) || parentItem.represents(kmlTag_Create) || parentItem.represents(kmlTag_Delete))
    {
        parentItem.nodeAs<GeoDataContainer>()->append(overlay);
        return overlay;
    }

    else if (parentItem.qualifiedName().first == QString::fromUtf8(kmlTag_kml))
    {
        GeoDataDocument* doc = geoDataDoc(parser);
        doc->append(overlay);
        return overlay;
    }

    else
    {
        delete overlay;
        return nullptr;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
