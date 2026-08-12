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

#include "KmlLodTagWriter.h"

// Local includes

#include "GeoDataLod.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLod(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLodType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLodTagWriter);

bool KmlLodTagWriter::write(const GeoNode* node,
                            GeoWriter& writer) const
{
    const GeoDataLod* lod = static_cast<const GeoDataLod*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Lod));
    KmlObjectTagWriter::writeIdentifiers(writer, lod);
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_minLodPixels),  QString::number(lod->minLodPixels()));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_maxLodPixels),  QString::number(lod->maxLodPixels()));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_minFadeExtent), QString::number(lod->minFadeExtent()));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_maxFadeExtent), QString::number(lod->maxFadeExtent()));
    writer.writeEndElement();
    return true;
}

} // namespace Marble
