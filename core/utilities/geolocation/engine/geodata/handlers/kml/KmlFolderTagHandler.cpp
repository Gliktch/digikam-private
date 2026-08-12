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

#include "KmlFolderTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "GeoDataContainer.h"
#include "GeoDataFolder.h"
#include "GeoDataParser.h"
#include "GeoDataDocument.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(Folder)

GeoNode* KmlFolderTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Folder)));

    GeoStackItem parentItem = parser.parentElement();
    GeoDataFolder* folder = new GeoDataFolder;
    KmlObjectTagHandler::parseIdentifiers(parser, folder);

    if (parentItem.represents(kmlTag_Folder) || parentItem.represents(kmlTag_Document))
    {
        GeoDataContainer* parentPtr = parentItem.nodeAs<GeoDataContainer>();
        parentPtr->append(folder);

        return folder;
    }

    else if (parentItem.qualifiedName().first == QLatin1String(kmlTag_kml))
    {
        GeoDataDocument* doc = geoDataDoc(parser);
        doc->append(folder);
        return folder;
    }

    else
    {
        delete folder;
        return nullptr;
    }
}

} // namespace kml

} // namespace Marble
