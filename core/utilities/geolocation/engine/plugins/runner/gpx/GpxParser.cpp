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

#include "GpxParser.h"

// Local includes

#include "GPXElementDictionary.h"
#include "GeoDataDocument.h"

namespace Marble
{

GpxParser::GpxParser()
    : GeoParser(0)
{
}

GpxParser::~GpxParser()
{
}

bool GpxParser::isValidRootElement()
{
    return isValidElement(QString::fromUtf8(gpx::gpxTag_gpx));
}

bool GpxParser::isValidElement(const QString& tagName) const
{
    if (!GeoParser::isValidElement(tagName))
    {
        return false;
    }

    return (
               (namespaceUri() == QStringView(QString::fromUtf8(gpx::gpxTag_nameSpace10)))    ||
               (namespaceUri() == QStringView(QString::fromUtf8(gpx::gpxTag_nameSpace11)))    ||
               (namespaceUri() == QStringView(QString::fromUtf8(gpx::gpxTag_nameSpaceGarminTrackPointExt1)))
           );
}

GeoDocument* GpxParser::createDocument() const
{
    return new GeoDataDocument;
}

} // namespace Marble
