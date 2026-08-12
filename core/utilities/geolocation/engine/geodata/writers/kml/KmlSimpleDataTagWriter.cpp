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

#include "KmlSimpleDataTagWriter.h"

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataSimpleData.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerSimpleData(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataSimpleDataType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlSimpleDataTagWriter);

bool KmlSimpleDataTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataSimpleData* simpleData = static_cast<const GeoDataSimpleData*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_SimpleData));
    writer.writeAttribute(QLatin1String("name"), simpleData->name());
    writer.writeCharacters(simpleData->data());
    writer.writeEndElement();

    return true;
}

} // namespace Marble
