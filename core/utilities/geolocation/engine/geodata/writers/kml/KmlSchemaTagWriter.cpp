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

#include "KmlSchemaTagWriter.h"

// Local includes

#include "GeoDataSchema.h"
#include "GeoDataSimpleField.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerSchema(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataSchemaType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlSchemaTagWriter);

bool KmlSchemaTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataSchema* schema = static_cast<const GeoDataSchema*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Schema));
    KmlObjectTagWriter::writeIdentifiers(writer, schema);
    QString name = schema->schemaName();
    writer.writeAttribute(QLatin1String("name"), name);

    for (const GeoDataSimpleField& simpleField : schema->simpleFields())
    {
        writeElement(&simpleField, writer);
    }

    writer.writeEndElement();

    return true;
}

} // namespace Marble
