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

#include "KmlMxSizeTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataIconStyle.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

namespace mx
{

KML_DEFINE_TAG_HANDLER_MX(size)

GeoNode* KmlsizeTagHandler::parse(GeoParser& parser) const
{
    if (parser.parentElement().is<GeoDataIconStyle>())
    {
        auto const width = parser.attribute(kmlTag_width).toInt();
        auto const height = parser.attribute(kmlTag_height).toInt();
        auto const size = QSize(width, height);

        if (size.isEmpty())
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Invalid size in icon style: width=" << width << ", height=" << height;
        }

        else
        {
            parser.parentElement().nodeAs<GeoDataIconStyle>()->setSize(size);
        }
    }

    return nullptr;
}

} // namespace mx

} // namespace kml

} // namespace Marble
