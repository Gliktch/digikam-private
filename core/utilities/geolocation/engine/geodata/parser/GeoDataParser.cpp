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

#include "GeoDataParser.h"

// Qt includes

#include <QStringView>

// Local includes

#include "GeoDataDocument.h"
#include "GeoDocument.h"
#include "GeoTagHandler.h"
#include "KmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

GeoDataParser::GeoDataParser(GeoDataSourceType source)
    : GeoParser(source)
{
}

GeoDataParser::~GeoDataParser()
{
}

bool GeoDataParser::isValidRootElement()
{
    if (m_source == GeoData_UNKNOWN)
    {
        if (GeoParser::isValidElement(QString::fromUtf8(kml::kmlTag_kml)))
        {
            m_source = GeoData_KML;
        }

        else
        {
            Q_ASSERT(false);
            return false;
        }
    }

    switch ((GeoDataSourceType) m_source)
    {
        // TODO: case GeoData_GeoRSS:
        case GeoData_KML:
            return isValidElement(QString::fromUtf8(kml::kmlTag_kml));

        default:
            Q_ASSERT(false);
            return false;
    }
}

bool GeoDataParser::isValidElement(const QString& tagName) const
{
    if (!GeoParser::isValidElement(tagName))
    {
        return false;
    }

    switch ((GeoDataSourceType) m_source)
    {
        // TODO: case GeoData_GeoRSS:
        case GeoData_KML:
        {
            const QStringView namespaceUri = this->namespaceUri();
            return (namespaceUri == QLatin1String(kml::kmlTag_nameSpace20) ||
                    namespaceUri == QLatin1String(kml::kmlTag_nameSpace21) ||
                    namespaceUri == QLatin1String(kml::kmlTag_nameSpace22) ||
                    namespaceUri == QLatin1String(kml::kmlTag_nameSpaceOgc22) ||
                    namespaceUri == QLatin1String(kml::kmlTag_nameSpaceGx22) ||
                    namespaceUri == QLatin1String(kml::kmlTag_nameSpaceMx));
        }

        default:
            break;
    }

    // Should never be reached.
    Q_ASSERT(false);
    return false;
}

GeoDocument* GeoDataParser::createDocument() const
{
    return new GeoDataDocument;
}

// Global helper function for the tag handlers
GeoDataDocument* geoDataDoc(GeoParser& parser)
{
    GeoDocument* document = parser.activeDocument();
    Q_ASSERT(document->isGeoDataDocument());
    return static_cast<GeoDataDocument*>(document);
}

} // namespace Marble
