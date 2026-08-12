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

#include "KmlParser.h"

// Local includes

#include "GeoDataDocument.h"
#include "KmlElementDictionary.h"

namespace Marble
{

KmlParser::KmlParser()
    : GeoParser(0)
{
}

KmlParser::~KmlParser()
{
}

bool KmlParser::isValidRootElement()
{
    return isValidElement(QString::fromUtf8(kml::kmlTag_kml));
}

bool KmlParser::isValidElement(const QString& tagName) const
{
    if (!GeoParser::isValidElement(tagName))
    {
        return false;
    }

    return (namespaceUri() == QStringView(QString::fromUtf8(kml::kmlTag_nameSpace20))   ||
            namespaceUri() == QStringView(QString::fromUtf8(kml::kmlTag_nameSpace21))   ||
            namespaceUri() == QStringView(QString::fromUtf8(kml::kmlTag_nameSpace22))   ||
            namespaceUri() == QStringView(QString::fromUtf8(kml::kmlTag_nameSpaceGx22)) ||
            namespaceUri() == QStringView(QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)) ||
            namespaceUri() == QStringView(QString::fromUtf8(kml::kmlTag_nameSpaceMx)));
}

GeoDocument* KmlParser::createDocument() const
{
    return new GeoDataDocument;
}

} // namespace Marble
