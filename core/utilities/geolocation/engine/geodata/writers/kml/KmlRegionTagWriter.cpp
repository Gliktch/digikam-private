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

#include "KmlRegionTagWriter.h"

// Local includes

#include "GeoDataRegion.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataTypes.h"
#include "GeoDataLod.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlLatLonAltBoxWriter.h"
#include "KmlLodTagWriter.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerRegion(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataRegionType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlRegionTagWriter);

bool KmlRegionTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataRegion* region = static_cast<const GeoDataRegion*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Region));
    KmlObjectTagWriter::writeIdentifiers(writer, region);
    writeElement(&region->latLonAltBox(), writer);
    writeElement(&region->lod(), writer);
    writer.writeEndElement();
    return true;
}

} // namespace Marble
