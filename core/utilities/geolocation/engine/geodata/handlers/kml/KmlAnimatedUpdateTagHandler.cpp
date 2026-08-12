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

#include "KmlAnimatedUpdateTagHandler.h"

// Local includes

#include "GeoDataAnimatedUpdate.h"
#include "GeoDataPlaylist.h"
#include "GeoParser.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagHandler.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

namespace gx
{

KML_DEFINE_TAG_HANDLER_GX22(AnimatedUpdate)

GeoNode* KmlAnimatedUpdateTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_AnimatedUpdate)));

    GeoStackItem parentItem = parser.parentElement();

    GeoDataAnimatedUpdate* animatedUpdate = new GeoDataAnimatedUpdate;
    KmlObjectTagHandler::parseIdentifiers(parser, animatedUpdate);

    if (parentItem.is<GeoDataPlaylist>())
    {
        parentItem.nodeAs<GeoDataPlaylist>()->addPrimitive(animatedUpdate);
        return animatedUpdate;
    }

    else
    {
        delete animatedUpdate;
    }

    return nullptr;
}

} // namespace gx

} // namespace kml

} // namespace Marble
