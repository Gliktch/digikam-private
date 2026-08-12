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

#include "KmlFeatureTagWriter.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "GeoDataOverlay.h"
#include "GeoDataTimeStamp.h"
#include "GeoDataTimeSpan.h"
#include "GeoDataDocument.h"
#include "GeoDataStyle.h"
#include "GeoDataStyleMap.h"
#include "GeoDataExtendedData.h"
#include "GeoDataLookAt.h"
#include "GeoDataPlacemark.h"
#include "GeoDataCamera.h"
#include "GeoWriter.h"
#include "GeoDataRegion.h"
#include "GeoDataLatLonAltBox.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"
#include "KmlOsmPlacemarkDataTagWriter.h"
#include "OsmPlacemarkData.h"

namespace Marble
{

KmlFeatureTagWriter::KmlFeatureTagWriter(const QString& elementName)
    : m_elementName(elementName)
{
    // nothing to do
}

bool KmlFeatureTagWriter::write(const Marble::GeoNode* node, GeoWriter& writer) const
{
    const GeoDataFeature* feature = static_cast<const GeoDataFeature*>(node);

    if (const GeoDataDocument* document = geodata_cast<GeoDataDocument>(feature))
    {
        // when a document has only one feature and no styling
        // the document tag is excused
        if ((document->id().isEmpty())
            && (document->name().isEmpty())
            && (document->targetId().isEmpty())
            && (document->styles().count() == 0)
            && (document->styleMaps().count() == 0)
            && (document->extendedData().isEmpty())
            && (document->featureList().count() == 1))
        {
            writeElement(document->featureList()[0], writer);
            return true;
        }
    }

    writer.writeStartElement(m_elementName);

    KmlObjectTagWriter::writeIdentifiers(writer, feature);

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_name), feature->name());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_visibility), QString::number(feature->isVisible()), QString::fromUtf8("1"));
    writer.writeOptionalElement(QString::fromUtf8("address"), feature->address());

    if (!feature->description().isEmpty())
    {
        writer.writeStartElement(QString::fromUtf8("description"));

        if (feature->descriptionIsCDATA())
        {
            writer.writeCDATA(feature->description());
        }

        else
        {
            writer.writeCharacters(feature->description());
        }

        writer.writeEndElement();
    }

    GeoDataLookAt const* lookAt = dynamic_cast<const GeoDataLookAt*>(feature->abstractViewBase());

    if (lookAt)
    {
        writeElement(lookAt, writer);
    }

    GeoDataCamera const* camera = dynamic_cast<const GeoDataCamera*>(feature->abstractViewBase());

    if (camera)
    {
        writeElement(camera, writer);
    }

    if (feature->timeStamp().when().isValid())
    {
        writeElement(&feature->timeStamp(), writer);
    }

    if (feature->timeSpan().isValid())
    {
        writeElement(&feature->timeSpan(), writer);
    }

    if (!feature->region().latLonAltBox().isNull())
    {
        writeElement(&feature->region(), writer);
    }

    bool const result = writeMid(node, writer);

    if (geodata_cast<GeoDataPlacemark>(feature))
    {
        KmlOsmPlacemarkDataTagWriter::write(feature, writer);
    }

    if (!feature->extendedData().isEmpty())
    {
        writeElement(&feature->extendedData(), writer);
    }

    writer.writeEndElement();
    return result;
}

} // namespace Marble
