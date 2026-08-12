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

#include "GeoSceneParser.h"

// Local includes

#include "GeoDocument.h"
#include "GeoSceneDocument.h"
#include "GeoTagHandler.h"
#include "DgmlElementDictionary.h"
#include "digikam_debug.h"

namespace Marble
{

GeoSceneParser::GeoSceneParser(GeoSceneSourceType source)
    : GeoParser(source)
{
}

GeoSceneParser::~GeoSceneParser()
{
    // nothing to do
}

bool GeoSceneParser::isValidRootElement()
{
    switch ((GeoSceneSourceType) m_source)
    {
        case GeoScene_DGML:
            return isValidElement(QString::fromUtf8(dgml::dgmlTag_Dgml));

        default:
            Q_ASSERT(false);
            return false;
    }
}

bool GeoSceneParser::isValidElement(const QString& tagName) const
{
    if (!GeoParser::isValidElement(tagName))
    {
        return false;
    }

    switch ((GeoSceneSourceType) m_source)
    {
        case GeoScene_DGML:
            return (namespaceUri() == QLatin1String(dgml::dgmlTag_nameSpace20));

        default:
            break;
    }

    // Should never be reached.
    Q_ASSERT(false);
    return false;
}

GeoDocument* GeoSceneParser::createDocument() const
{
    return new GeoSceneDocument;
}

// Global helper function for the tag handlers
GeoSceneDocument* geoSceneDoc(GeoParser& parser)
{
    GeoDocument* document = parser.activeDocument();
    Q_ASSERT(document->isGeoSceneDocument());
    return static_cast<GeoSceneDocument*>(document);
}

} // namespace Marble
