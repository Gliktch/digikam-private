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

#include "KmlTrackTagHandler.h"

// Local includes

#include "KmlObjectTagHandler.h"
#include "GeoDataMultiTrack.h"
#include "GeoDataTrack.h"
#include "GeoDataPlacemark.h"
#include "GeoDataMultiGeometry.h"
#include "KmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER_GX22(Track)

GeoNode* KmlTrackTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_Track)));

    GeoStackItem parentItem = parser.parentElement();
    GeoDataTrack* track = new GeoDataTrack();
    KmlObjectTagHandler::parseIdentifiers(parser, track);

    if (parentItem.represents(kmlTag_Placemark))
    {
        parentItem.nodeAs<GeoDataPlacemark>()->setGeometry(track);
        return track;
    }

    else if (parentItem.represents(kmlTag_MultiTrack))
    {
        parentItem.nodeAs<GeoDataMultiTrack>()->append(track);
        return track;
    }

    else if (parentItem.represents(kmlTag_MultiGeometry))
    {
        parentItem.nodeAs<GeoDataMultiGeometry>()->append(track);
        return track;
    }

    else
    {
        delete track;
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
