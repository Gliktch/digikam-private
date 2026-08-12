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

#include "KmlColorTagHandler.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataColorStyle.h"
#include "GeoDataOverlay.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(color)

GeoNode* KmlcolorTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_color)));

    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.is<GeoDataColorStyle>() || parentItem.is<GeoDataOverlay>())
    {
        QColor const color = parseColor(parser.readElementText().trimmed());

        if (parentItem.is<GeoDataColorStyle>())
        {
            parentItem.nodeAs<GeoDataColorStyle>()->setColor(color);
        }

        else if (parentItem.is<GeoDataOverlay>())
        {
            parentItem.nodeAs<GeoDataOverlay>()->setColor(color);
        }
    }

    return nullptr;
}

QColor KmlcolorTagHandler::parseColor(const QString& colorString)
{
    // color tag uses AABBGGRR whereas QColor uses AARRGGBB - use some shifts for that
    // be aware that QRgb needs to be a typedef for 32 bit UInt for this to work properly
    bool ok;
    QRgb abgr = colorString.toUInt(&ok, 16);
    unsigned a = abgr >> 24;
    abgr = abgr << 8; //"rgb0"
    unsigned b = abgr >> 24;
    abgr = abgr << 8; //"gb00"
    unsigned g = abgr >> 24;
    abgr = abgr << 8; //"b000"
    unsigned r = abgr >> 24;
    QRgb rgba = (a << 24) | (r << 16) | (g << 8) | (b);
    return ok ? QColor::fromRgba(rgba) : QColor();
}

} // namespace kml

} // namespace Marble
