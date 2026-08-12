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

#include "KmlHrefTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataIconStyle.h"
#include "GeoDataGroundOverlay.h"
#include "GeoDataLink.h"
#include "GeoDataPhotoOverlay.h"
#include "GeoDataScreenOverlay.h"
#include "GeoDataSoundCue.h"
#include "GeoDataItemIcon.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(href)

GeoNode* KmlhrefTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_href)));

    GeoStackItem parentItem = parser.parentElement();

    QString content = parser.readElementText().trimmed();

    if (parentItem.represents(kmlTag_Icon))
    {
        // we need a more elaborate version of this part
        if (parentItem.is<GeoDataIconStyle>())
        {
            parentItem.nodeAs<GeoDataIconStyle>()->setIconPath(content);
        }

        else if (parentItem.is<GeoDataGroundOverlay>())
        {
            parentItem.nodeAs<GeoDataGroundOverlay>()->setIconFile(content);
        }

        else if (parentItem.is<GeoDataPhotoOverlay>())
        {
            parentItem.nodeAs<GeoDataPhotoOverlay>()->setIconFile(content);
        }

        else if (parentItem.is<GeoDataScreenOverlay>())
        {
            parentItem.nodeAs<GeoDataScreenOverlay>()->setIconFile(content);
        }
    }

    else if (parentItem.represents(kmlTag_ItemIcon))
    {
        parentItem.nodeAs<GeoDataItemIcon>()->setIconPath(content);
    }

    else if (parentItem.is<GeoDataLink>())
    {
        parentItem.nodeAs<GeoDataLink>()->setHref(content);
    }

    else if (parentItem.is<GeoDataSoundCue>())
    {
        parentItem.nodeAs<GeoDataSoundCue>()->setHref(content);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
