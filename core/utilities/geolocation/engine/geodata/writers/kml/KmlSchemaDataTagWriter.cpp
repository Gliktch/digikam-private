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

#include "KmlSchemaDataTagWriter.h"

// Qt includes

#include <QString>

// Local includes

#include "KmlElementDictionary.h"
#include "GeoWriter.h"
#include "GeoDataTypes.h"
#include "GeoDataSchemaData.h"
#include "GeoDataSimpleData.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerSchemaData(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataSchemaDataType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlSchemaDataTagWriter);

bool KmlSchemaDataTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataSchemaData* schemaData = static_cast<const GeoDataSchemaData*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_SchemaData));
    QString schemaUrl = schemaData->schemaUrl();
    writer.writeAttribute(QLatin1String("schemaUrl"), schemaUrl);

    for (const GeoDataSimpleData& data : schemaData->simpleDataList())
    {
        writeElement(&data, writer);
    }

    writer.writeEndElement();

    return true;
}

} // namespace Marble
